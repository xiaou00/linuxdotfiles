#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <GLES3/gl32.h>

#include <hyprland/src/config/values/types/BoolValue.hpp>
#include <hyprland/src/config/values/types/ColorValue.hpp>
#include <hyprland/src/config/values/types/FloatValue.hpp>
#include <hyprland/src/desktop/rule/windowRule/WindowRuleApplicator.hpp>
#include <hyprland/src/desktop/state/WindowState.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/output/Monitor.hpp>
#include <hyprland/src/output/MonitorResources.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/render/Framebuffer.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/render/Shader.hpp>
#include <hyprland/src/render/Texture.hpp>
#include <hyprland/src/render/transformer/Transformer.hpp>

using namespace Desktop::View;
using namespace Render;
using namespace Render::GL;

namespace {

HANDLE g_handle = nullptr;

struct SConfigValues {
    SP<Config::Values::CBoolValue>  enabled;
    SP<Config::Values::CBoolValue>  kittyFrost;
    SP<Config::Values::CFloatValue> duration;
    SP<Config::Values::CFloatValue> closeDuration;
    SP<Config::Values::CFloatValue> openSettle;
    SP<Config::Values::CFloatValue> cellSize;
    SP<Config::Values::CFloatValue> lineWidth;
    SP<Config::Values::CFloatValue> glowWidth;
    SP<Config::Values::CFloatValue> kittyFrostContrast;
    SP<Config::Values::CColorValue> color;
} g_config;

struct SLayoutSample {
    PHLWINDOWREF window;
    CBox         before;
};

struct SClosingSnapshot {
    WP<ITexture>                           texture;
    CBox                                   sourceBox;
    CBox                                   layoutBox;
    std::vector<SLayoutSample>             layoutSamples;
    std::chrono::steady_clock::time_point  created;
    float                                  verticalSpine = 0.F;
    float                                  collapseAnchor = 0.F;
    bool                                   layoutResolved = false;
};

std::unordered_map<ITexture*, SClosingSnapshot> g_closingSnapshots;
SP<CShader>                                     g_hexShader;
CFunctionHook*                                  g_snapshotHook      = nullptr;
CFunctionHook*                                  g_renderTextureHook = nullptr;
CHyprSignalListener                             g_openListener;
CHyprSignalListener                             g_closeListener;
CHyprSignalListener                             g_tickListener;
CHyprSignalListener                             g_reloadListener;
CHyprSignalListener                             g_openLateListener;
CHyprSignalListener                             g_classListener;
bool                                            g_shaderFailed = false;
GLint                                           g_directionEnabledLocation  = -1;
GLint                                           g_directionVerticalLocation = -1;
GLint                                           g_directionAnchorLocation   = -1;

enum class eEffectType : int {
    OPEN        = 0,
    CLOSE       = 1,
    KITTY_FROST = 2,
};

constexpr const char* VERTEX_SHADER = R"GLSL(
#version 300 es

uniform mat3 proj;
uniform vec4 color;

in vec2 pos;
in vec2 texcoord;

out vec2 v_texcoord;

void main() {
    gl_Position = vec4(proj * vec3(pos, 1.0), 1.0);
    v_texcoord = texcoord;
}
)GLSL";

constexpr const char* FRAGMENT_SHADER = R"GLSL(
#version 300 es

precision highp float;

in vec2 v_texcoord;

uniform sampler2D tex;
uniform vec4 color;
uniform int useAlphaMatte;
uniform int texType;
uniform float alpha;
uniform float time;
uniform float radius;
uniform float thick;
uniform float range;
uniform vec2 fullSize;
uniform vec2 topLeft;
uniform vec2 bottomRight;
uniform float directionEnabled;
uniform float directionVertical;
uniform float directionAnchor;

layout(location = 0) out vec4 fragColor;

float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float hexDistance(vec2 p) {
    p = abs(p);
    return max(dot(p, normalize(vec2(1.0, 1.7320508))), p.x);
}

vec4 hexCell(vec2 p) {
    const vec2 spacing = vec2(1.0, 1.7320508);
    const vec2 halfSpacing = spacing * 0.5;
    vec2 a = mod(p, spacing) - halfSpacing;
    vec2 b = mod(p - halfSpacing, spacing) - halfSpacing;
    vec2 local = dot(a, a) < dot(b, b) ? a : b;
    vec2 id = floor((p - local) / halfSpacing + 0.5);

    // Convert the staggered center ID to integer axial coordinates, then
    // divide a compact 6-cell super-grid into alternating upright/inverted
    // triangles. The small period reads as a dense repeating energy array.
    vec2 axial = vec2((id.x + id.y) * 0.5, (id.x - id.y) * 0.5);
    const float trianglePeriod = 6.0;
    vec2 triangleCell = mod(axial, trianglePeriod);
    float diagonal = triangleCell.x + triangleCell.y;
    float triangleDepth;
    if (diagonal < trianglePeriod) {
        triangleDepth = min(min(triangleCell.x, triangleCell.y), trianglePeriod - 1.0 - diagonal);
    } else {
        triangleDepth = min(min(trianglePeriod - 1.0 - triangleCell.x, trianglePeriod - 1.0 - triangleCell.y), diagonal - trianglePeriod);
    }
    // At this compact period, keep the triangle edge at level zero and assign
    // its inner cells to a deterministic three-phase circuit sequence. This
    // retains all four frost strengths without adding random-looking grain.
    float circuitPhase = mod(floor(axial.x + axial.y * 2.0), 3.0);
    float triangularTier = step(0.5, triangleDepth) * (1.0 + circuitPhase);
    return vec4(local, hash21(id), triangularTier);
}

void main() {
    vec2 safeBoxSize = max(bottomRight, vec2(1.0));
    vec2 pixel = v_texcoord * fullSize;
    vec2 rel = (pixel - topLeft) / safeBoxSize;
    float inBox = step(0.0, rel.x) * step(0.0, rel.y) * step(rel.x, 1.0) * step(rel.y, 1.0);
    float progress = clamp(time, 0.0, 1.0);

    if (texType == 2) {
        // This pass receives Hyprland's grayscale blur matte. Keep each cell
        // perfectly flat; four strengths form nested, alternating triangular
        // clusters without random grain or per-pixel variation.
        vec4 source = texture(tex, v_texcoord);
        float cellScale = max(radius, 8.0);
        vec4 cell = hexCell((pixel - topLeft) / cellScale);
        float cellFrost = cell.w < 0.5 ? 0.18 : (cell.w < 1.5 ? 0.42 : (cell.w < 2.5 ? 0.72 : 1.0));
        float frost = mix(0.62, cellFrost, clamp(alpha, 0.0, 1.0));
        float matte = source.r * frost * inBox;
        fragColor = vec4(vec3(matte), 1.0);
        return;
    }

    if (texType == 1) {
        vec4 source = texture(tex, v_texcoord);
        float cellScale = max(radius, 8.0);
        vec4 cell = hexCell((pixel - topLeft) / cellScale);
        float edgePx = abs(0.5 - hexDistance(cell.xy)) * cellScale;
        float lattice = 1.0 - smoothstep(max(thick, 0.25), max(thick, 0.25) + 1.2, edgePx);
        float halo = 1.0 - smoothstep(max(thick, 0.25), max(range, thick + 0.5), edgePx);

        // Use the centre of each hexagon for the sweep timing, so a cell
        // disappears as one tile instead of being cut by a straight mask.
        vec2 cellCenterPixel = pixel - cell.xy * cellScale;
        vec2 cellRel = clamp((cellCenterPixel - topLeft) / safeBoxSize, 0.0, 1.0);

        // Geometry comparison on the compositor side supplies the reflow
        // axis and the boundary eventually reached by the remaining tiles.
        // On that axis, a one-sided fill starts at the opposite edge and ends
        // at its target edge. A balanced fill starts at both outer edges and
        // meets at the centre, with no arbitrary direction chosen.
        float verticalSpine = step(0.5, directionVertical);
        float axisRel = mix(cellRel.y, cellRel.x, verticalSpine);
        float edgeToCentre = 1.0 - abs(axisRel * 2.0 - 1.0);
        float targetEdge = directionAnchor < 0.0 ? 1.0 - axisRel : axisRel;
        float directional = smoothstep(0.12, 0.35, abs(directionAnchor));
        float sweep = mix(edgeToCentre, targetEdge, directional);

        // A small stable stagger makes the middle of the sweep less rigid.
        // It tapers to zero at both endpoints, so the last row still lands
        // exactly on the reflow boundary (or centre for balanced fill).
        float stagger = cell.z * 0.025 * sin(sweep * 3.14159265);
        float phase = 0.025 + sweep * 0.90 + stagger;
        float vanished = smoothstep(phase - 0.042, phase + 0.026, progress);
        float visible = 1.0 - vanished;
        float front = 1.0 - smoothstep(0.010, 0.100, abs(progress - phase));
        float facePulse = 1.0 - smoothstep(0.0, 0.060, abs(progress - phase));

        // Clip the red lattice to actual window content. This avoids drawing
        // a rectangular overlay from transparent snapshot padding.
        float sourceMask = smoothstep(0.002, 0.030, source.a) * inBox;
        float redAlpha = (lattice * 0.92 + halo * 0.24 + facePulse * 0.075) * front;
        redAlpha = clamp(redAlpha * color.a * sourceMask, 0.0, 1.0);

        vec4 remaining = source * visible * alpha * inBox;
        vec3 redPremultiplied = color.rgb * redAlpha;
        fragColor.rgb = redPremultiplied + remaining.rgb * (1.0 - redAlpha);
        fragColor.a = redAlpha + remaining.a * (1.0 - redAlpha);
        return;
    }

    vec4 source = texture(tex, v_texcoord);

    float cellScale = max(radius, 8.0);
    vec4 cell = hexCell((pixel - topLeft) / cellScale);
    float edgePx = abs(0.5 - hexDistance(cell.xy)) * cellScale;
    float lattice = 1.0 - smoothstep(max(thick, 0.25), max(thick, 0.25) + 1.2, edgePx);
    float halo = 1.0 - smoothstep(max(thick, 0.25), max(range, thick + 0.5), edgePx);

    vec2 clampedRel = clamp(rel, 0.0, 1.0);
    float diagonal = clampedRel.x * 0.62 + (1.0 - clampedRel.y) * 0.38;
    float freePhase = 0.03 + diagonal * 0.78 + cell.z * 0.10;

    // Once another tiled window exists, reveal from the outer edge toward the
    // shared insertion boundary, following the direction in which the old
    // layout was squeezed. Timing is based on each cell centre so complete
    // hexagons enter together. The first/undirected window keeps the original
    // free diagonal field.
    vec2 cellCenterPixel = pixel - cell.xy * cellScale;
    vec2 cellRel = clamp((cellCenterPixel - topLeft) / safeBoxSize, 0.0, 1.0);
    float axisRel = mix(cellRel.y, cellRel.x, step(0.5, directionVertical));
    float edgeSweep = directionAnchor < 0.0 ? 1.0 - axisRel : axisRel;
    float edgeStagger = cell.z * 0.025 * sin(edgeSweep * 3.14159265);
    float edgePhase = 0.03 + edgeSweep * 0.86 + edgeStagger;
    float phase = mix(freePhase, edgePhase, step(0.5, directionEnabled));
    float reveal = smoothstep(phase - 0.060, phase + 0.035, progress);
    float wave = 1.0 - smoothstep(0.012, 0.105, abs(progress - phase));
    // A transformer may deliberately hold at progress zero while a newly
    // mapped client negotiates its initial size. Keep that settling frame
    // fully invisible instead of leaving an isolated red lattice fragment.
    float effectStarted = step(0.0001, progress);
    reveal *= effectStarted;
    wave *= effectStarted * smoothstep(0.0, 0.025, progress);

    // The transform is also called for Hyprland's black/white blur matte.
    // Reveal only its white window area, keeping the rest solid black so an
    // unrevealed window cannot leave a fullscreen translucent blur rectangle.
    if (useAlphaMatte == 1) {
        float matte = source.r * reveal * inBox;
        fragColor = vec4(vec3(matte), 1.0);
        return;
    }

    // The sampled alpha keeps the lattice inside rounded/translucent window pixels.
    float sourceMask = smoothstep(0.001, 0.035, source.a) * inBox;
    float redAlpha = (lattice * 0.90 + halo * 0.24) * wave * color.a * sourceMask;
    redAlpha *= sqrt(clamp(alpha, 0.0, 1.0));

    vec4 base = source * (reveal * alpha);
    vec3 redPremultiplied = color.rgb * redAlpha;
    fragColor.rgb = redPremultiplied + base.rgb * (1.0 - redAlpha);
    fragColor.a = redAlpha + base.a * (1.0 - redAlpha);
}
)GLSL";

float durationMs() {
    return g_config.duration ? std::max(g_config.duration->value(), 80.F) : 420.F;
}

float closeDurationMs() {
    return g_config.closeDuration ? std::max(g_config.closeDuration->value(), 80.F) : 240.F;
}

CBox windowBoxInFramebuffer(PHLWINDOW window) {
    if (!window)
        return {};

    const auto monitor = window->m_monitor.lock();
    if (!monitor)
        return {};

    return window->getFullWindowBoundingBox().translate(-monitor->m_position).scale(monitor->m_scale).round();
}

bool boxChanged(const CBox& a, const CBox& b, double tolerance = 1.0) {
    return a.empty() || b.empty() || std::abs(a.x - b.x) > tolerance || std::abs(a.y - b.y) > tolerance ||
        std::abs(a.width - b.width) > tolerance || std::abs(a.height - b.height) > tolerance;
}

double overlapLength(double a0, double a1, double b0, double b1) {
    return std::max(0.0, std::min(a1, b1) - std::max(a0, b0));
}

double intersectionArea(const CBox& a, const CBox& b) {
    return overlapLength(a.x, a.x + a.width, b.x, b.x + b.width) * overlapLength(a.y, a.y + a.height, b.y, b.y + b.height);
}

struct SOpenDirection {
    float enabled  = 0.F;
    float vertical = 0.F;
    float anchor   = 0.F;
};

SOpenDirection resolveOpenDirection(PHLWINDOW opened) {
    if (!opened || opened->m_isFloating || !opened->m_workspace)
        return {};

    const CBox openedBox = opened->layoutBox();
    if (openedBox.empty())
        return {};

    // Accumulate contact length on the four sides of the new layout slot.
    // This works for both a single neighbour and a side made from several
    // smaller tiles. Gaps are discounted so a distant aligned window cannot
    // win over the actual insertion boundary.
    double scores[4] = {0.0, 0.0, 0.0, 0.0}; // left, right, top, bottom
    const double maximumGap = std::max(32.0, std::min(openedBox.width, openedBox.height) * 0.12);
    const double openedCenterX = openedBox.x + openedBox.width * 0.5;
    const double openedCenterY = openedBox.y + openedBox.height * 0.5;

    auto addContact = [maximumGap, &scores](size_t side, double gap, double overlap) {
        gap = std::abs(gap);
        if (gap > maximumGap || overlap < 2.0)
            return;
        scores[side] += overlap * (1.0 - gap / maximumGap);
    };

    for (const auto& candidate : Desktop::windowState()->windows()) {
        if (!candidate || candidate == opened || !candidate->m_isMapped || candidate->m_isFloating || candidate->isHidden() ||
            candidate->m_workspace != opened->m_workspace)
            continue;

        const CBox other = candidate->layoutBox();
        if (other.empty())
            continue;

        const double overlapY = overlapLength(openedBox.y, openedBox.y + openedBox.height, other.y, other.y + other.height);
        const double overlapX = overlapLength(openedBox.x, openedBox.x + openedBox.width, other.x, other.x + other.width);
        const double otherCenterX = other.x + other.width * 0.5;
        const double otherCenterY = other.y + other.height * 0.5;

        if (otherCenterX <= openedCenterX)
            addContact(0, openedBox.x - other.x - other.width, overlapY);
        else
            addContact(1, other.x - openedBox.x - openedBox.width, overlapY);

        if (otherCenterY <= openedCenterY)
            addContact(2, openedBox.y - other.y - other.height, overlapX);
        else
            addContact(3, other.y - openedBox.y - openedBox.height, overlapX);
    }

    const auto best = std::max_element(std::begin(scores), std::end(scores));
    if (best == std::end(scores) || *best <= 0.0)
        return {};

    switch (std::distance(std::begin(scores), best)) {
        case 0: return {.enabled = 1.F, .vertical = 1.F, .anchor = -1.F}; // left -> right
        case 1: return {.enabled = 1.F, .vertical = 1.F, .anchor = 1.F};  // right -> left
        case 2: return {.enabled = 1.F, .vertical = 0.F, .anchor = -1.F}; // top -> bottom
        case 3: return {.enabled = 1.F, .vertical = 0.F, .anchor = 1.F};  // bottom -> top
        default: return {};
    }
}

std::vector<SLayoutSample> captureLayoutSamples(PHLWINDOW closing) {
    std::vector<SLayoutSample> samples;
    if (!closing || closing->m_isFloating || !closing->m_workspace)
        return samples;

    for (const auto& candidate : Desktop::windowState()->windows()) {
        if (!candidate || candidate == closing || !candidate->m_isMapped || candidate->m_isFloating || candidate->isHidden() || candidate->m_workspace != closing->m_workspace)
            continue;

        const CBox before = candidate->layoutBox();
        if (!before.empty())
            samples.push_back({.window = PHLWINDOWREF{candidate}, .before = before});
    }

    return samples;
}

void resolveLayoutCollapse(SClosingSnapshot& state) {
    if (state.layoutResolved)
        return;

    state.layoutResolved = true;
    state.verticalSpine  = state.sourceBox.height >= state.sourceBox.width ? 1.F : 0.F;
    state.collapseAnchor = 0.F;

    if (state.layoutBox.empty() || state.layoutSamples.empty())
        return;

    double fillTowardLeft = 0.0, fillTowardRight = 0.0;
    double fillTowardTop = 0.0, fillTowardBottom = 0.0;

    for (const auto& sample : state.layoutSamples) {
        const auto window = sample.window.lock();
        if (!window || !window->m_isMapped)
            continue;

        const CBox after = window->layoutBox();
        if (after.empty())
            continue;

        const CBox& before = sample.before;
        if (after.x < before.x)
            fillTowardLeft += intersectionArea(state.layoutBox, CBox{after.x, after.y, before.x - after.x, after.height});
        if (after.x + after.width > before.x + before.width)
            fillTowardRight += intersectionArea(state.layoutBox,
                                                CBox{before.x + before.width, after.y, after.x + after.width - before.x - before.width, after.height});
        if (after.y < before.y)
            fillTowardTop += intersectionArea(state.layoutBox, CBox{after.x, after.y, after.width, before.y - after.y});
        if (after.y + after.height > before.y + before.height)
            fillTowardBottom += intersectionArea(state.layoutBox,
                                                 CBox{after.x, before.y + before.height, after.width, after.y + after.height - before.y - before.height});
    }

    const double horizontalFill = fillTowardLeft + fillTowardRight;
    const double verticalFill   = fillTowardTop + fillTowardBottom;
    const double minimumFill    = std::max(64.0, state.layoutBox.width * state.layoutBox.height * 0.0025);

    if (std::max(horizontalFill, verticalFill) < minimumFill)
        return;

    if (horizontalFill > verticalFill) {
        state.verticalSpine  = 1.F;
        state.collapseAnchor = sc<float>((fillTowardRight - fillTowardLeft) / horizontalFill);
    } else {
        state.verticalSpine  = 0.F;
        state.collapseAnchor = sc<float>((fillTowardBottom - fillTowardTop) / verticalFill);
    }

    // Small opposing differences read as an intentional inward sweep; a
    // single clear fill direction should finish precisely at the far edge.
    if (std::abs(state.collapseAnchor) < 0.12F)
        state.collapseAnchor = 0.F;
    else if (std::abs(state.collapseAnchor) > 0.82F)
        state.collapseAnchor = std::copysign(1.F, state.collapseAnchor);
}

bool ensureShader() {
    if (g_hexShader && g_hexShader->program())
        return true;
    if (g_shaderFailed || !g_pHyprOpenGL)
        return false;

    auto shader = makeShared<CShader>();
    if (!shader->createProgram(VERTEX_SHADER, FRAGMENT_SHADER, true, false)) {
        g_shaderFailed = true;
        HyprlandAPI::addNotification(g_handle, "[hyprhex] Failed to compile the hex-lattice shader", CHyprColor{1.F, 0.1F, 0.1F, 1.F}, 6000);
        return false;
    }

    g_directionEnabledLocation  = glGetUniformLocation(shader->program(), "directionEnabled");
    g_directionVerticalLocation = glGetUniformLocation(shader->program(), "directionVertical");
    g_directionAnchorLocation   = glGetUniformLocation(shader->program(), "directionAnchor");
    g_hexShader                  = shader;
    return true;
}

bool renderEffectTexture(SP<ITexture> texture, const CBox& box, CHyprOpenGLImpl::STextureRenderData data, float progress, const CBox& sourceBox, float opacity,
                         bool alphaMatte = false, eEffectType effect = eEffectType::OPEN, float directionEnabled = 0.F, float directionVertical = 0.F,
                         float directionAnchor = 0.F) {
    if (!texture || !texture->ok() || !ensureShader())
        return false;

    auto& renderData = g_pHyprRenderer->m_renderData;
    if (!renderData.pMonitor)
        return false;

    if (!data.damage) {
        if (renderData.damage.empty())
            return true;
        data.damage = &renderData.damage;
    }

    CBox newBox = box;
    renderData.renderModif.applyToBox(newBox);

    const auto monitorInverse = Math::wlTransformToHyprutils(Math::invertTransform(renderData.pMonitor->m_transform));
    auto       textureTransform = texture->m_transform;
    if (g_pHyprRenderer->monitorTransformEnabled())
        textureTransform = Math::composeTransform(monitorInverse, textureTransform);

    const auto& matrix = g_pHyprRenderer->projectBoxToTarget(newBox, textureTransform);

    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    texture->setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    texture->setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    texture->setTexParameter(GL_TEXTURE_MAG_FILTER, texture->magFilter);
    texture->setTexParameter(GL_TEXTURE_MIN_FILTER, texture->minFilter);

    auto shader = g_pHyprOpenGL->useShader(g_hexShader);
    shader->setUniformMatrix3fv(SHADER_PROJ, 1, GL_TRUE, matrix.getMatrix());
    shader->setUniformInt(SHADER_TEX, 0);
    shader->setUniformInt(SHADER_USE_ALPHA_MATTE, alphaMatte ? 1 : 0);
    shader->setUniformInt(SHADER_TEX_TYPE, sc<int>(effect));
    shader->setUniformFloat(SHADER_ALPHA, std::clamp(opacity, 0.F, 1.F));
    shader->setUniformFloat(SHADER_TIME, std::clamp(progress, 0.F, 1.F));
    shader->setUniformFloat(SHADER_RADIUS, g_config.cellSize->value());
    shader->setUniformFloat(SHADER_THICK, g_config.lineWidth->value());
    shader->setUniformFloat(SHADER_RANGE, g_config.glowWidth->value());
    shader->setUniformFloat2(SHADER_FULL_SIZE, texture->m_size.x, texture->m_size.y);
    shader->setUniformFloat2(SHADER_TOP_LEFT, sourceBox.x, sourceBox.y);
    shader->setUniformFloat2(SHADER_BOTTOM_RIGHT, sourceBox.width, sourceBox.height);
    if (g_directionEnabledLocation >= 0)
        glUniform1f(g_directionEnabledLocation, std::clamp(directionEnabled, 0.F, 1.F));
    if (g_directionVerticalLocation >= 0)
        glUniform1f(g_directionVerticalLocation, std::clamp(directionVertical, 0.F, 1.F));
    if (g_directionAnchorLocation >= 0)
        glUniform1f(g_directionAnchorLocation, std::clamp(directionAnchor, -1.F, 1.F));

    const CHyprColor color{sc<uint64_t>(g_config.color->value())};
    shader->setUniformFloat4(SHADER_COLOR, color.r, color.g, color.b, color.a);

    glBindVertexArray(shader->getUniformLocation(SHADER_SHADER_VAO));

    if (!renderData.clipBox.empty() || !data.clipRegion.empty()) {
        CRegion damageClip = renderData.clipBox;
        if (!data.clipRegion.empty()) {
            if (damageClip.empty())
                damageClip = data.clipRegion;
            else
                damageClip.intersect(data.clipRegion);
        }

        damageClip.forEachRect([](const auto& rect) {
            g_pHyprOpenGL->scissor(&rect, g_pHyprRenderer->m_renderData.transformDamage);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        });
    } else {
        data.damage->forEachRect([](const auto& rect) {
            g_pHyprOpenGL->scissor(&rect, g_pHyprRenderer->m_renderData.transformDamage);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        });
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    g_pHyprOpenGL->scissor(nullptr);
    texture->unbind();
    return true;
}

class CHexOpenTransformer final : public IWindowTransformer {
  public:
    explicit CHexOpenTransformer(PHLWINDOWREF window) : m_window(window), m_created(std::chrono::steady_clock::now()), m_lastGeometryChange(m_created) {}

    SP<IFramebuffer> transform(SP<IFramebuffer> input) override {
        const auto window = m_window.lock();
        if (!window || !input || !g_config.enabled->value())
            return input;

        // Hyprland calls the transformer once for window content and again for
        // its blur alpha matte.  The second pass uses a synchronized black/
        // white reveal instead of the colored content path.
        const bool alphaMatte = m_transformCalls++ > 0;

        const float progress = this->progress();
        if (progress >= 1.F)
            return input;

        const auto monitor = window->m_monitor.lock();
        if (!monitor || !monitor->resources())
            return input;

        const auto output = monitor->resources()->getUnusedWorkBuffer();
        if (!output || output == input)
            return input;

        auto&         renderData     = g_pHyprRenderer->m_renderData;
        const CRegion oldDamage      = renderData.damage.copy();
        const CRegion oldFinalDamage = renderData.finalDamage.copy();
        const CBox    oldClipBox     = renderData.clipBox;
        const CRegion fullDamage{0, 0, monitor->m_transformedSize.x, monitor->m_transformedSize.y};

        auto guard = g_pHyprRenderer->bindTempFB(output);

        // The nested window pass leaves renderData.damage restricted to the
        // window.  Work buffers are pooled and retain pixels outside that
        // region, while Hyprland later composites the returned FB fullscreen.
        // Rewrite every pixel so no stale contents can leak into this frame.
        renderData.damage      = fullDamage;
        renderData.finalDamage = fullDamage;
        renderData.clipBox     = {};

        const bool blendWasEnabled = glIsEnabled(GL_BLEND) == GL_TRUE;
        g_pHyprOpenGL->blend(false);

        const CBox fullMonitor{0, 0, monitor->m_transformedSize.x, monitor->m_transformedSize.y};
        const bool rendered = renderEffectTexture(input->getTexture(), fullMonitor, {}, progress, windowBoxInFramebuffer(window), 1.F, alphaMatte,
                                                  eEffectType::OPEN, m_direction.enabled, m_direction.vertical, m_direction.anchor);

        g_pHyprOpenGL->blend(blendWasEnabled);
        renderData.damage      = oldDamage;
        renderData.finalDamage = oldFinalDamage;
        renderData.clipBox     = oldClipBox;
        return rendered ? output : input;
    }

    void preWindowRender(CSurfacePassElement::SRenderData* renderData) override {
        m_transformCalls = 0;
        updateGeometryState();
        if (renderData && !done()) {
            renderData->fadeAlpha = 1.F;
            // Rapid initial buffer commits should be clipped, not repeatedly
            // stretched into Hyprland's animated geometry.
            renderData->squishOversized = false;
        }
    }

    bool done() const {
        return progress() >= 1.F;
    }

  private:
    void updateGeometryState() {
        if (m_effectStarted)
            return;

        const auto window = m_window.lock();
        if (!window)
            return;

        const auto now = std::chrono::steady_clock::now();
        const CBox goalBox = window->geometricBox(IGeometric::GEOMETRIC_GOAL).round();
        if (!goalBox.empty() && boxChanged(goalBox, m_observedGoalBox)) {
            m_observedGoalBox = goalBox;
            m_lastGeometryChange = now;
        }

        const float settleMs = g_config.openSettle ? std::max(g_config.openSettle->value(), 0.F) : 32.F;
        const float stableFor = std::chrono::duration<float, std::milli>(now - m_lastGeometryChange).count();
        const float aliveFor = std::chrono::duration<float, std::milli>(now - m_created).count();
        const float maximumWait = settleMs <= 0.F ? 0.F : std::clamp(settleMs * 3.F, 72.F, 140.F);

        if (stableFor >= settleMs || aliveFor >= maximumWait) {
            m_direction = resolveOpenDirection(window);
            m_started = now;
            m_effectStarted = true;
        }
    }

    float progress() const {
        if (!m_effectStarted)
            return 0.F;
        const auto elapsed = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - m_started).count();
        return std::clamp(elapsed / durationMs(), 0.F, 1.F);
    }

    PHLWINDOWREF                           m_window;
    std::chrono::steady_clock::time_point m_created;
    std::chrono::steady_clock::time_point m_lastGeometryChange;
    std::chrono::steady_clock::time_point m_started{};
    CBox                                   m_observedGoalBox;
    SOpenDirection                         m_direction;
    bool                                   m_effectStarted = false;
    uint32_t                               m_transformCalls = 0;
};

class CKittyFrostTransformer final : public IWindowTransformer {
  public:
    explicit CKittyFrostTransformer(PHLWINDOWREF window) : m_window(window) {}

    SP<IFramebuffer> transform(SP<IFramebuffer> input) override {
        const auto window = m_window.lock();
        if (!window || !input || !g_config.enabled->value() || !g_config.kittyFrost->value())
            return input;

        // Leave Kitty's visible content untouched. The second transformer call
        // is Hyprland's grayscale blur matte and receives the regular pattern.
        const uint32_t transformCall = m_transformCalls++;
        if (transformCall != 1)
            return input;

        const auto monitor = window->m_monitor.lock();
        if (!monitor || !monitor->resources())
            return input;

        const auto output = monitor->resources()->getUnusedWorkBuffer();
        if (!output || output == input)
            return input;

        auto&         renderData = g_pHyprRenderer->m_renderData;
        const CRegion oldDamage = renderData.damage.copy();
        const CRegion oldFinalDamage = renderData.finalDamage.copy();
        const CBox    oldClipBox = renderData.clipBox;
        const CRegion fullDamage{0, 0, monitor->m_transformedSize.x, monitor->m_transformedSize.y};

        auto guard = g_pHyprRenderer->bindTempFB(output);
        renderData.damage = fullDamage;
        renderData.finalDamage = fullDamage;
        renderData.clipBox = {};

        const bool blendWasEnabled = glIsEnabled(GL_BLEND) == GL_TRUE;
        g_pHyprOpenGL->blend(false);

        const CBox fullMonitor{0, 0, monitor->m_transformedSize.x, monitor->m_transformedSize.y};
        const bool rendered = renderEffectTexture(input->getTexture(), fullMonitor, {}, 1.F, windowBoxInFramebuffer(window),
                                                  g_config.kittyFrostContrast->value(), false, eEffectType::KITTY_FROST);

        g_pHyprOpenGL->blend(blendWasEnabled);
        renderData.damage = oldDamage;
        renderData.finalDamage = oldFinalDamage;
        renderData.clipBox = oldClipBox;
        return rendered ? output : input;
    }

    void preWindowRender(CSurfacePassElement::SRenderData*) override {
        m_transformCalls = 0;
    }

  private:
    PHLWINDOWREF m_window;
    uint32_t     m_transformCalls = 0;
};

bool isHexTransformer(const UP<IWindowTransformer>& transformer) {
    return dynamic_cast<CHexOpenTransformer*>(transformer.get()) != nullptr;
}

bool isKittyFrostTransformer(const UP<IWindowTransformer>& transformer) {
    return dynamic_cast<CKittyFrostTransformer*>(transformer.get()) != nullptr;
}

bool containsCaseInsensitive(const std::string& value, const std::string& needle) {
    std::string lowered;
    lowered.reserve(value.size());
    std::ranges::transform(value, std::back_inserter(lowered), [](unsigned char c) { return sc<char>(std::tolower(c)); });
    return lowered.contains(needle);
}

bool containsExcludedApp(const std::string& value) {
    return containsCaseInsensitive(value, "fcitx");
}

bool isEffectExcluded(PHLWINDOW window) {
    return window && (containsExcludedApp(window->m_class) || containsExcludedApp(window->m_initialClass) || containsExcludedApp(window->m_title) ||
                      containsExcludedApp(window->m_initialTitle));
}

void removeHexTransformer(PHLWINDOW window) {
    if (!window)
        return;
    std::erase_if(window->m_transformers, isHexTransformer);
}

bool isKittyWindow(PHLWINDOW window) {
    return window && (containsCaseInsensitive(window->m_class, "kitty") || containsCaseInsensitive(window->m_initialClass, "kitty"));
}

void removeKittyFrostTransformer(PHLWINDOW window) {
    if (!window)
        return;
    std::erase_if(window->m_transformers, isKittyFrostTransformer);
}

void syncKittyFrostTransformer(PHLWINDOW window) {
    if (!window)
        return;

    const auto existing = std::ranges::find_if(window->m_transformers, isKittyFrostTransformer);
    const bool wanted = g_config.enabled->value() && g_config.kittyFrost->value() && isKittyWindow(window);

    if (!wanted) {
        if (existing != window->m_transformers.end()) {
            window->m_transformers.erase(existing);
            if (g_pHyprRenderer)
                g_pHyprRenderer->damageWindow(window);
        }
        return;
    }

    if (existing == window->m_transformers.end()) {
        window->m_transformers.emplace_back(makeUnique<CKittyFrostTransformer>(PHLWINDOWREF{window}));
        if (g_pHyprRenderer)
            g_pHyprRenderer->damageWindow(window);
    }
}

void onWindowOpen(PHLWINDOW window) {
    if (!window)
        return;

    syncKittyFrostTransformer(window);

    if (!g_config.enabled->value() || window->m_ruleApplicator->noAnim().valueOrDefault() || isEffectExcluded(window))
        return;

    removeHexTransformer(window);
    window->m_transformers.emplace_back(makeUnique<CHexOpenTransformer>(PHLWINDOWREF{window}));
}

void onWindowClose(PHLWINDOW window) {
    removeHexTransformer(window);
    removeKittyFrostTransformer(window);
}

using OriginalSnapshot = SP<IFramebuffer> (*)(IHyprRenderer*, PHLWINDOW);
using OriginalRenderTexture = void (*)(CHyprOpenGLImpl*, SP<ITexture>, const CBox&, CHyprOpenGLImpl::STextureRenderData);

SP<IFramebuffer> hookedSnapshot(IHyprRenderer* renderer, PHLWINDOW window) {
    removeHexTransformer(window);

    if (isEffectExcluded(window))
        return (reinterpret_cast<OriginalSnapshot>(g_snapshotHook->m_original))(renderer, window);

    const CBox                  sourceBox = windowBoxInFramebuffer(window);
    const CBox                  layoutBox = window && !window->m_isFloating ? window->layoutBox() : CBox{};
    std::vector<SLayoutSample>  layoutSamples = captureLayoutSamples(window);
    const auto                  snapshot = (reinterpret_cast<OriginalSnapshot>(g_snapshotHook->m_original))(renderer, window);

    if (g_config.enabled->value() && snapshot && snapshot->getTexture()) {
        g_closingSnapshots[snapshot->getTexture().get()] = {
            .texture = snapshot->getTexture(),
            .sourceBox = sourceBox,
            .layoutBox = layoutBox,
            .layoutSamples = std::move(layoutSamples),
            .created = std::chrono::steady_clock::now(),
        };

        // Hyprland's ordinary fadeout blur is a full rectangular plate and
        // cannot follow the collapsing geometry. Keep this snapshot unblurred.
        if (window && window->m_ruleApplicator)
            window->m_ruleApplicator->noBlur().set(true, Desktop::Types::PRIORITY_SET_PROP);
    }

    return snapshot;
}

void hookedRenderTexture(CHyprOpenGLImpl* renderer, SP<ITexture> texture, const CBox& box, CHyprOpenGLImpl::STextureRenderData data) {
    auto found = texture ? g_closingSnapshots.find(texture.get()) : g_closingSnapshots.end();

    if (found != g_closingSnapshots.end()) {
        const auto trackedTexture = found->second.texture.lock();
        if (!trackedTexture || trackedTexture != texture) {
            g_closingSnapshots.erase(found);
            found = g_closingSnapshots.end();
        }
    }

    if (!g_config.enabled->value() || found == g_closingSnapshots.end() || data.finalMonitorCM) {
        (reinterpret_cast<OriginalRenderTexture>(g_renderTextureHook->m_original))(renderer, texture, box, std::move(data));
        return;
    }

    SClosingSnapshot& state = found->second;
    resolveLayoutCollapse(state);
    const float elapsedMs = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - state.created).count();
    const float progress = std::clamp(elapsedMs / closeDurationMs(), 0.F, 1.F);

    // windowsOut keeps the snapshot alive, but this monotonic clock owns the
    // actual effect progress. Native opacity is deliberately ignored so the
    // red hexagonal front retains its intended brightness and timing.
    if (!renderEffectTexture(texture, box, data, progress, state.sourceBox, 1.F, false, eEffectType::CLOSE, 1.F, state.verticalSpine,
                             state.collapseAnchor))
        (reinterpret_cast<OriginalRenderTexture>(g_renderTextureHook->m_original))(renderer, texture, box, std::move(data));
}

void cleanupAnimations() {
    for (const auto& window : Desktop::windowState()->windows()) {
        if (!window)
            continue;

        std::erase_if(window->m_transformers, [](const UP<IWindowTransformer>& transformer) {
            const auto hex = dynamic_cast<CHexOpenTransformer*>(transformer.get());
            return hex && (!g_config.enabled->value() || hex->done());
        });

        // The settling clock and shader clock are independent of Hyprland's
        // native animation clock, so request frames until our transformer ends.
        if (std::ranges::find_if(window->m_transformers, isHexTransformer) != window->m_transformers.end() && g_pHyprRenderer)
            g_pHyprRenderer->damageWindow(window);
    }

    const auto staleBefore = std::chrono::steady_clock::now() - std::chrono::seconds(3);
    std::erase_if(g_closingSnapshots, [staleBefore](const auto& item) { return item.second.texture.expired() || item.second.created < staleBefore; });

}

CFunctionHook* findAndCreateHook(const std::string& name, const std::string& owner, const std::string& argumentType, void* replacement) {
    for (const auto& function : HyprlandAPI::findFunctionsByName(g_handle, name)) {
        if (!function.demangled.contains(owner) || (!argumentType.empty() && !function.demangled.contains(argumentType)))
            continue;
        return HyprlandAPI::createFunctionHook(g_handle, function.address, replacement);
    }
    return nullptr;
}

} // namespace

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    g_handle = handle;

    if (std::string{__hyprland_api_get_hash()} != std::string{__hyprland_api_get_client_hash()}) {
        HyprlandAPI::addNotification(g_handle, "[hyprhex] Hyprland/header version mismatch", CHyprColor{1.F, 0.1F, 0.1F, 1.F}, 6000);
        throw std::runtime_error("hyprhex: Hyprland/header version mismatch");
    }

    g_config.enabled = makeShared<Config::Values::CBoolValue>("plugin:hyprhex:enabled", "Enable the red hex-lattice window animation", true);
    g_config.kittyFrost = makeShared<Config::Values::CBoolValue>("plugin:hyprhex:kitty_frost", "Give Kitty a regular four-level hexagonal blur matte", true);
    g_config.duration = makeShared<Config::Values::CFloatValue>("plugin:hyprhex:duration", "Animation duration in milliseconds", 420.F,
                                                                 Config::Values::SFloatValueOptions{.min = 80.F, .max = 2000.F});
    g_config.closeDuration = makeShared<Config::Values::CFloatValue>("plugin:hyprhex:close_duration", "Closing animation duration in milliseconds", 240.F,
                                                                      Config::Values::SFloatValueOptions{.min = 80.F, .max = 1200.F});
    g_config.openSettle = makeShared<Config::Values::CFloatValue>("plugin:hyprhex:open_settle", "Quiet geometry time before opening animation", 32.F,
                                                                   Config::Values::SFloatValueOptions{.min = 0.F, .max = 120.F});
    g_config.cellSize = makeShared<Config::Values::CFloatValue>("plugin:hyprhex:cell_size", "Hexagon cell size in pixels", 44.F,
                                                                 Config::Values::SFloatValueOptions{.min = 12.F, .max = 160.F});
    g_config.lineWidth = makeShared<Config::Values::CFloatValue>("plugin:hyprhex:line_width", "Bright lattice line width", 1.6F,
                                                                  Config::Values::SFloatValueOptions{.min = 0.25F, .max = 8.F});
    g_config.glowWidth = makeShared<Config::Values::CFloatValue>("plugin:hyprhex:glow_width", "Lattice glow width", 8.F,
                                                                  Config::Values::SFloatValueOptions{.min = 1.F, .max = 32.F});
    g_config.kittyFrostContrast = makeShared<Config::Values::CFloatValue>("plugin:hyprhex:kitty_frost_contrast", "Difference between regular Kitty frost levels", 0.90F,
                                                                          Config::Values::SFloatValueOptions{.min = 0.F, .max = 1.F});
    g_config.color = makeShared<Config::Values::CColorValue>("plugin:hyprhex:col.lattice", "Lattice color", 0xEEFF2028);

    HyprlandAPI::addConfigValueV2(g_handle, g_config.enabled);
    HyprlandAPI::addConfigValueV2(g_handle, g_config.kittyFrost);
    HyprlandAPI::addConfigValueV2(g_handle, g_config.duration);
    HyprlandAPI::addConfigValueV2(g_handle, g_config.closeDuration);
    HyprlandAPI::addConfigValueV2(g_handle, g_config.openSettle);
    HyprlandAPI::addConfigValueV2(g_handle, g_config.cellSize);
    HyprlandAPI::addConfigValueV2(g_handle, g_config.lineWidth);
    HyprlandAPI::addConfigValueV2(g_handle, g_config.glowWidth);
    HyprlandAPI::addConfigValueV2(g_handle, g_config.kittyFrostContrast);
    HyprlandAPI::addConfigValueV2(g_handle, g_config.color);

    g_snapshotHook = findAndCreateHook("makeSnapshotFB", "IHyprRenderer::makeSnapshotFB(", "CWindow", reinterpret_cast<void*>(hookedSnapshot));
    g_renderTextureHook = findAndCreateHook("renderTexture", "CHyprOpenGLImpl::renderTexture(", "STextureRenderData", reinterpret_cast<void*>(hookedRenderTexture));

    if (!g_snapshotHook || !g_renderTextureHook || !g_snapshotHook->hook() || !g_renderTextureHook->hook()) {
        HyprlandAPI::addNotification(g_handle, "[hyprhex] Failed to install closing-animation renderer hooks", CHyprColor{1.F, 0.1F, 0.1F, 1.F}, 6000);
        throw std::runtime_error("hyprhex: failed to install closing-animation renderer hooks");
    }

    g_openListener = Event::bus()->m_events.window.open.listen(onWindowOpen);
    g_openLateListener = Event::bus()->m_events.window.openLate.listen(syncKittyFrostTransformer);
    g_classListener = Event::bus()->m_events.window.class_.listen(syncKittyFrostTransformer);
    g_closeListener = Event::bus()->m_events.window.close.listen(onWindowClose);
    g_tickListener = Event::bus()->m_events.tick.listen(cleanupAnimations);
    g_reloadListener = Event::bus()->m_events.config.reloaded.listen([] {
        for (const auto& window : Desktop::windowState()->windows())
            syncKittyFrostTransformer(window);
        if (!g_config.enabled->value())
            cleanupAnimations();
    });

    HyprlandAPI::reloadConfig();
    HyprlandAPI::addNotification(g_handle, "[hyprhex 1.15.0] Fast directional close loaded", CHyprColor{1.F, 0.13F, 0.16F, 1.F}, 3500);
    return {"hyprhex", "Fast directional hex animations, Wofi integration, and dense Kitty frost", "Codex", "1.15.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    g_openListener.reset();
    g_openLateListener.reset();
    g_classListener.reset();
    g_closeListener.reset();
    g_tickListener.reset();
    g_reloadListener.reset();

    for (const auto& window : Desktop::windowState()->windows()) {
        removeHexTransformer(window);
        removeKittyFrostTransformer(window);
    }

    g_closingSnapshots.clear();
    if (g_hexShader && g_pHyprOpenGL)
        g_pHyprOpenGL->makeEGLCurrent();
    g_hexShader.reset();
    g_directionEnabledLocation  = -1;
    g_directionVerticalLocation = -1;
    g_directionAnchorLocation   = -1;
    g_config = {};
}
