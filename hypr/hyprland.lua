-- Hyprland 0.55+ Lua configuration.
-- Migrated from hyprland.conf for compatibility with Hyprland 0.57+.

local terminal = "alacritty"
local menu = "wofi -n"
local mainMod = "SUPER"

local configHome = os.getenv("XDG_CONFIG_HOME") or (os.getenv("HOME") .. "/.config")
local theme = dofile(configHome .. "/theme/current.lua")

-- Apply this rule to any output without a more specific monitor rule.
hl.monitor({
    output = "",
    mode = "preferred",
    position = "auto",
    scale = "auto",
})

hl.config({
    render = {
        cm_enabled = false,
    },

    general = {
        -- Seamless tiling, aligned with Waybar's zero outer margins.
        gaps_in = 0,
        gaps_out = 0,
        border_size = 0,
        col = {
            active_border = theme.accent,
            inactive_border = theme.bg,
        },
        resize_on_border = true,
        allow_tearing = false,
        layout = "dwindle",
    },

    decoration = {
        rounding = 0,
        active_opacity = 0.95,
        inactive_opacity = 0.95,
        fullscreen_opacity = 1,
        blur = {
            enabled = false,
            size = 1,
            passes = 1,
            new_optimizations = true,
            ignore_opacity = true,
            xray = false,
            popups = false,
        },
        shadow = {
            enabled = false,
        },
    },

    animations = {
        enabled = true,
    },

    dwindle = {
        preserve_split = true,
    },

    misc = {
        force_default_wallpaper = -1,
        disable_hyprland_logo = true,
        focus_on_activate = true,
    },

    input = {
        kb_layout = "us",
        follow_mouse = 1,
        sensitivity = 0,
        repeat_delay = 150,
        repeat_rate = 55,
        kb_options = "compose:ralt",
        touchpad = {
            natural_scroll = true,
        },
    },

    xwayland = {
        force_zero_scaling = true,
    },

    cursor = {
        no_hardware_cursors = false,
    },

    group = {
        groupbar = {
            enabled = true,
            font_size = 8,
            height = 2,
            priority = 3,
            render_titles = false,
            gradients = false,
            col = {
                active = theme.accent_rgba,
                inactive = "rgba(00000000)",
            },
        },
    },

})

-- Do not load hyprhex. If it is still present in the current session, keep it
-- disabled until Hyprland is restarted.
local hyprhexLoaded = false
for _, plugin in ipairs(hl.get_loaded_plugins()) do
    if plugin.name == "hyprhex" then
        hyprhexLoaded = true
        break
    end
end

if hyprhexLoaded then
    hl.config({
        plugin = {
            hyprhex = {
                enabled = false,
            },
        },
    })
end

hl.curve("fluid", {
    type = "bezier",
    points = { { 0.15, 0.85 }, { 0.25, 1 } },
})
hl.curve("snappy", {
    type = "bezier",
    points = { { 0.3, 1 }, { 0.4, 1 } },
})
hl.curve("uniform", {
    type = "bezier",
    points = { { 0, 0 }, { 1, 1 } },
})

-- Windows open, close and retile immediately to keep the layout seamless.
hl.animation({ leaf = "windows", enabled = false })
hl.animation({ leaf = "fadeIn", enabled = false })
hl.animation({ leaf = "fadeOut", enabled = false })
hl.animation({ leaf = "fadeSwitch", enabled = false })
hl.animation({ leaf = "fade", enabled = true, speed = 1.5, bezier = "uniform" })
hl.animation({ leaf = "workspaces", enabled = true, speed = 1.7, bezier = "snappy", style = "slide" })
hl.animation({ leaf = "specialWorkspace", enabled = true, speed = 4, bezier = "fluid", style = "slidefadevert -35%" })
hl.animation({ leaf = "layers", enabled = true, speed = 2, bezier = "snappy", style = "popin 95%" })
hl.animation({ leaf = "layersOut", enabled = true, speed = 2, bezier = "snappy", style = "slide" })

hl.device({
    name = "epic-mouse-v1",
    sensitivity = 0,
})

hl.device({
    name = "wacom-intuos-s-pen",
    output = "DP-1",
    transform = 0,
})

hl.env("ELECTRON_OZONE_PLATFORM_HINT", "wayland")
hl.env("XCURSOR_THEME", "ArcDusk-cursors")
hl.env("XCURSOR_SIZE", "32")
hl.env("HYPRCURSOR_SIZE", "32")
hl.env("GTK_IM_MODULE", "fcitx")
hl.env("XMODIFIERS", "@im=fcitx")
hl.env("QT_QPA_PLATFORMTHEME", "qt5ct")
hl.env("QT_STYLE_OVERRIDE", "Fusion")
hl.env("LITELOADERQQNT_PROFILE", "/home/xiaou0/.config/LiteLoaderQQNT")

hl.on("hyprland.start", function()
    hl.exec_cmd("ydotoold")
    hl.exec_cmd("hypridle")
    hl.exec_cmd("waybar")
    hl.exec_cmd("awww-daemon")
    hl.exec_cmd("sleep .5 && awww restore")
    hl.exec_cmd("dunst")
    hl.exec_cmd("pypr")
    hl.exec_cmd("pactl set-sink-mute @DEFAULT_SINK@ 0")
    hl.exec_cmd("fcitx5 -d")
    hl.exec_cmd("/home/xiaou0/.config/hypr/scripts/wall-shuffle.sh")
end)

-- Application and window controls.
hl.bind(mainMod .. " + Q", hl.dsp.exec_cmd(terminal))
hl.bind(mainMod .. " + N", hl.dsp.exec_cmd(terminal .. " -e nvim"))
hl.bind(mainMod .. " + C", hl.dsp.window.close())
hl.bind(mainMod .. " + E", hl.dsp.exec_cmd(terminal .. " -e yazi"))
hl.bind(mainMod .. " + V", hl.dsp.window.float({ action = "toggle" }))
hl.bind(mainMod .. " + R", hl.dsp.exec_cmd(menu))
hl.bind(mainMod .. " + P", hl.dsp.window.pseudo())
hl.bind(mainMod .. " + F", hl.dsp.window.fullscreen())
hl.bind(mainMod .. " + Z", hl.dsp.exec_cmd("firefox"))

-- Focus the window in the chosen direction.
hl.bind(mainMod .. " + left", hl.dsp.focus({ direction = "left" }))
hl.bind(mainMod .. " + right", hl.dsp.focus({ direction = "right" }))
hl.bind(mainMod .. " + up", hl.dsp.focus({ direction = "up" }))
hl.bind(mainMod .. " + down", hl.dsp.focus({ direction = "down" }))

for i = 1, 10 do
    local key = i % 10
    hl.bind(mainMod .. " + " .. key, hl.dsp.focus({ workspace = i }))
    hl.bind(mainMod .. " + SHIFT + " .. key, hl.dsp.window.move({ workspace = i }))
end

hl.bind(mainMod .. " + SHIFT + F", hl.dsp.exec_cmd("zen-browser"))
hl.bind(mainMod .. " + SHIFT + C", hl.dsp.exec_cmd(terminal .. " claude"))

-- Audio, brightness, and media keys.
local repeatLocked = { locked = true, repeating = true }
local locked = { locked = true }
hl.bind("XF86AudioRaiseVolume", hl.dsp.exec_cmd("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%+"), repeatLocked)
hl.bind("XF86AudioLowerVolume", hl.dsp.exec_cmd("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-"), repeatLocked)
hl.bind("XF86AudioMute", hl.dsp.exec_cmd("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle"), repeatLocked)
hl.bind("XF86MonBrightnessUp", hl.dsp.exec_cmd("brightnessctl s 10%+"), repeatLocked)
hl.bind("XF86MonBrightnessDown", hl.dsp.exec_cmd("brightnessctl s 10%-"), repeatLocked)
hl.bind("XF86AudioNext", hl.dsp.exec_cmd("playerctl next"), locked)
hl.bind("XF86AudioPause", hl.dsp.exec_cmd("playerctl play-pause"), locked)
hl.bind("XF86AudioPlay", hl.dsp.exec_cmd("playerctl play-pause"), locked)
hl.bind("XF86AudioPrev", hl.dsp.exec_cmd("playerctl previous"), locked)

hl.bind(mainMod .. " + bracketright", hl.dsp.exec_cmd("playerctl --player=spotatui next"))
hl.bind(mainMod .. " + bracketleft", hl.dsp.exec_cmd("playerctl --player=spotatui previous"))

hl.bind(mainMod .. " + SHIFT + A", hl.dsp.exec_cmd("grimblast --freeze copy active"))
hl.bind(mainMod .. " + SHIFT + S", hl.dsp.exec_cmd("grimblast --freeze copy area"))
hl.bind(mainMod .. " + SHIFT + D", hl.dsp.exec_cmd("grimblast copy output"))
hl.bind(mainMod .. " + L", hl.dsp.exec_cmd("hyprlock"))
hl.bind(mainMod .. " + S", hl.dsp.workspace.toggle_special("magic"))
hl.bind(mainMod .. " + D", hl.dsp.window.move({ workspace = "special:magic" }))

hl.bind(mainMod .. " + mouse:272", hl.dsp.window.drag(), { mouse = true })
hl.bind(mainMod .. " + mouse:273", hl.dsp.window.resize(), { mouse = true })

hl.bind(mainMod .. " + ALT + M", hl.dsp.exit())
hl.bind(mainMod .. " + G", hl.dsp.group.toggle())
hl.bind(mainMod .. " + TAB", hl.dsp.group.next())
hl.bind(mainMod .. " + H", hl.dsp.group.lock_active({ action = "toggle" }))
hl.bind(mainMod .. " + T", hl.dsp.exec_cmd(configHome .. "/theme/apply-theme.sh toggle"))
hl.bind(mainMod .. " + SHIFT + T", hl.dsp.exec_cmd(configHome .. "/theme/toggle-window-theme.sh"))

hl.bind(mainMod .. " + CTRL + right", hl.dsp.window.resize({ x = 20, y = 0, relative = true }), { repeating = true })
hl.bind(mainMod .. " + CTRL + left", hl.dsp.window.resize({ x = -20, y = 0, relative = true }), { repeating = true })
hl.bind(mainMod .. " + CTRL + up", hl.dsp.window.resize({ x = 0, y = -20, relative = true }), { repeating = true })
hl.bind(mainMod .. " + CTRL + down", hl.dsp.window.resize({ x = 0, y = 20, relative = true }), { repeating = true })

hl.bind(mainMod .. " + B", hl.dsp.exec_cmd("pkill -SIGUSR1 waybar || waybar"))
-- Toggle the focused window between fully opaque and its normal opacity.
local alacrittyRefresh = dofile(configHome .. "/hypr/scripts/alacritty-refresh.lua")
hl.bind(mainMod .. " + O", alacrittyRefresh.toggle_opacity)
-- Timers need the event loop; do not create them during config parsing.
hl.on("hyprland.start", alacrittyRefresh.watch_resizes)
hl.on("config.reloaded", alacrittyRefresh.watch_resizes)
hl.bind(mainMod .. " + mouse_up", hl.dsp.exec_cmd("/home/xiaou0/.config/hypr/opacity.sh up"))
hl.bind(mainMod .. " + mouse_down", hl.dsp.exec_cmd("/home/xiaou0/.config/hypr/opacity.sh down"))

-- Keyboard-driven mouse movement.
local mouseMove = "/home/xiaou0/.config/hypr/scripts/mouse-move.sh"
for _, direction in ipairs({ "up", "down", "left", "right" }) do
    hl.bind("ALT + " .. direction, hl.dsp.exec_cmd(mouseMove .. " start " .. direction))
    hl.bind("ALT + " .. direction, hl.dsp.exec_cmd(mouseMove .. " stop " .. direction), { release = true })
end
hl.bind("Alt_L", hl.dsp.exec_cmd(mouseMove .. " stopall"), { release = true })
hl.bind("Alt_R", hl.dsp.exec_cmd(mouseMove .. " stopall"), { release = true })
hl.bind("ALT + SPACE", hl.dsp.exec_cmd(mouseMove .. " stopall"), { release = true })
hl.bind("ALT + return", hl.dsp.exec_cmd("ydotool click 0xC0"))
hl.bind("ALT + backslash", hl.dsp.exec_cmd("ydotool click 0xC1"))

-- Layer rules.
hl.layer_rule({
    name = "fcitx5-no-animation",
    match = { namespace = "^(fcitx|fcitx5|Fcitx|Fcitx5).*$" },
    no_anim = true,
})

hl.layer_rule({
    name = "waybar-clean-panel",
    match = { namespace = "waybar" },
    blur = false,
})

hl.layer_rule({
    name = "wofi-frosted-glass",
    match = { namespace = "wofi" },
    blur = false,
    ignore_alpha = 0.2,
    animation = "fade",
})

hl.layer_rule({
    name = "notification-animation",
    match = { namespace = "notifications" },
    blur = false,
    ignore_alpha = 0.2,
    animation = "slide",
})

-- Window rules.
-- Alacritty draws its own translucent background; avoid a second opacity stage.
hl.window_rule({
    name = "alacritty-native-opacity",
    match = { class = "^Alacritty$" },
    opacity = "1 override 1 override 1 override",
    no_anim = true,
})

hl.window_rule({
    name = "fcitx5-no-animation-by-class",
    match = { class = "^(fcitx|fcitx5|Fcitx|Fcitx5).*$" },
    no_anim = true,
})

hl.window_rule({
    name = "fcitx5-no-animation-by-title",
    match = { title = "^(fcitx|fcitx5|Fcitx|Fcitx5).*$" },
    no_anim = true,
})

hl.window_rule({
    name = "wofi-floating-launcher",
    match = { class = "^(wofi|Wofi).*$" },
    float = true,
    center = true,
})

hl.window_rule({
    name = "tty-clock",
    match = { title = "tty-clock" },
    float = true,
})

-- Removing the tag restores the normal global and per-window opacity rules.
hl.window_rule({
    name = "manual-opaque",
    match = { tag = "manual-opaque" },
    opacity = "1 override 1 override 1 override",
    force_rgbx = true,
})
