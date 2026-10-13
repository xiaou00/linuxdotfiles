const colors = {
    bg: "#151515",
    bg1: "#1b1b1b",
    surface: "#242424",
    border: "#3a3a3a",
    fg: "#f2f2f2",
    fg2: "#d0d0d0",
    muted: "#888888",
    red: "#ff4444",
    redHover: "#ff6666",
    redDark: "#4a1f1f",
};

const themeVars = {
    "--global-bg_standard": colors.bg,
    "--global-bg_secondary": colors.bg,
    "--global-bg_tertiary": colors.bg,
    "--global-bg_bottom": colors.bg,
    "--global-bg_middle": colors.bg,
    "--global-bg_top": colors.bg,
    "--bg_nav": colors.bg,
    "--bg_nav_secondary": colors.bg,
    "--bg_bottom_standard": colors.bg,
    "--bg_bottom_light": colors.bg,
    "--bg_middle_standard": colors.bg,
    "--bg_middle_light": colors.bg,
    "--bg_top_standard": colors.bg,
    "--bg_top_light": colors.bg,
    "--bg_grey_standard": colors.bg,
    "--bg_list": colors.bg,
    "--bg_white": colors.bg,
    "--background-01": colors.bg,
    "--background-02": colors.bg,
    "--background-03": colors.bg,
    "--background-04": colors.bg,
    "--background-05": colors.bg,
    "--background-dialogue": colors.surface,
    "--background_01": colors.bg,
    "--background_02": colors.bg,
    "--background_03": colors.bg,
    "--background_04": colors.bg,
    "--background_05": colors.bg,
    "--background_dialogue": colors.surface,
    "--text_primary": colors.fg,
    "--text-primary": colors.fg,
    "--text_secondary": colors.fg2,
    "--text_tertiary": colors.muted,
    "--text_anti": colors.bg,
    "--text_link": colors.red,
    "--text-link": colors.red,
    "--brand_standard": colors.red,
    "--brand_normal": colors.red,
    "--brand_theme": colors.red,
    "--brand_theme_hover": colors.redHover,
    "--brand_theme_active": colors.redDark,
    "--brand_text": colors.red,
    "--global-brand_standard": colors.red,
    "--global_brand_standard": colors.red,
    "--badge-brand": colors.red,
    "--divider-brand": colors.red,
    "--icon-brand": colors.red,
    "--icon_brand": colors.red,
    "--button-primary-default": colors.red,
    "--button-primary-hover": colors.redHover,
    "--button-primary-pressed": colors.redDark,
    "--button-primary-disable": colors.border,
    "--button_brand_bg": colors.red,
    "--button_brand_bg_hover": colors.redHover,
    "--button_brand_bg_pressed": colors.redDark,
    "--border_control": colors.border,
    "--border_default": colors.border,
    "--border_primary": `1px solid ${colors.border}`,
    "--divider_standard": colors.border,
    "--bubble_guest": colors.surface,
    "--bubble-guest": colors.surface,
    "--bubble_host": colors.redDark,
    "--bubble-host": colors.redDark,
    "--bubble_host_1": colors.redDark,
    "--bubble_host_2": colors.redDark,
    "--host_bubble_bg_css_value": colors.redDark,
    "--bubble_guest_text": colors.fg2,
    "--scrollbar_color": colors.border,
    "--hover-list": colors.surface,
    "--list-hover": colors.surface,
    "--list-pressed": colors.border,
    "--icon_primary": colors.fg2,
    "--icon_secondary": colors.muted,
    "--icon_brand": colors.red,
    "--itbr": "4px",
    "--title-bar-height": "0px",
};

function applyVars() {
    const root = document.documentElement;
    for (const [name, value] of Object.entries(themeVars)) {
        root.style.setProperty(name, value, "important");
    }
}

const css = `
html, body, #app {
    background: ${colors.bg} !important;
    color: ${colors.fg2} !important;
}

#app, #app > div, .qq-app, .app-container,
[class*="app-container"], [class*="main-window"] {
    background-color: ${colors.bg} !important;
    border-color: ${colors.border} !important;
}

/* QQ's own frameless window: outer title strip and every major inner pane. */
.window-title-bar, .title-bar, .qq-titlebar, .top-bar,
[class*="window-title"], [class*="titlebar"], [class*="title-bar"],
[class*="top-bar"], [class*="top_bar"] {
    background-color: ${colors.bg} !important;
    border-color: ${colors.border} !important;
    box-shadow: none !important;
}

/* Hyprland handles the window; remove QQNT's redundant full-width chrome. */
.window-title-bar, .qq-titlebar,
[class*="window-title-bar"], [class*="window_title_bar"],
[class*="qq-titlebar"], [class*="qq_titlebar"],
[class*="qq-title-bar"], [class*="qq_title_bar"],
[class*="window-titlebar"], [class*="window_titlebar"],
[class~="titlebar"], [class~="title-bar"],
.top-bar, .topBar,
[class*="top-bar"], [class*="top_bar"], [class*="topBar"] {
    display: none !important;
    height: 0 !important;
    min-height: 0 !important;
    border: 0 !important;
}

.main-layout, .main-panel, .content-panel, .contact-layout,
.aio, .aio-container, .aio-independent, .aio-chat,
[class*="main-layout"], [class*="main-panel"], [class*="content-panel"],
[class*="aio-container"], [class*="aio-independent"] {
    background-color: ${colors.bg} !important;
    border-color: ${colors.border} !important;
    box-shadow: none !important;
}

.sidebar, .sidebar-nav, .recent-contact, .recent-contact-list,
[class*="sidebar"], [class*="recent-contact"], [class*="recent-list"] {
    background-color: ${colors.bg} !important;
    border-color: ${colors.border} !important;
}

body, input, textarea, button {
    font-family: "VictorMono Nerd Font Mono", "LXGW WenKai Screen", monospace;
}

/* Terminal-symbol language: square geometry, thin rules and explicit marks. */
*, *::before, *::after { --tui-rule: 1px solid ${colors.border}; }

/* Large structural surfaces stay square; controls regain restrained curves. */
[class*="panel"], [class*="card"] {
    border-radius: 0 !important;
    box-shadow: none !important;
}

[class*="avatar"] img, img[class*="avatar"] {
    border-radius: 50% !important;
    outline: 1px solid ${colors.border};
    outline-offset: -1px;
}

button, [role="button"], [class*="button"], [class*="btn"] {
    border-radius: 4px !important;
    box-shadow: none !important;
}

[class*="badge"] { border-radius: 4px !important; }

[class*="bubble"], [class*="message-content"], [class*="msg-content"] {
    border-radius: 6px !important;
}

[class*="sidebar"] { border-right: 0 !important; }

/* Remove the pale gutter between the navigation rail and conversation list. */
[class*="splitter"], [class*="split-line"], [class*="split_line"],
[class*="resize-handle"], [class*="resize_handle"],
[class*="drag-handle"], [class*="drag_handle"] {
    background: transparent !important;
    border: 0 !important;
    box-shadow: none !important;
}

/* Keep the rail focused: chat and friend requests only. */
.nav-bar-main .nav-item:nth-of-type(n + 3),
.sidebar-nav .nav-item:nth-of-type(n + 3),
[class*="nav-bar"] > [class*="nav-item"]:nth-of-type(n + 3),
[class*="sidebar-nav"] > [class*="nav-item"]:nth-of-type(n + 3) {
    display: none !important;
}

.nav-bar-main [class*="bottom"],
.sidebar-nav [class*="bottom"],
[class*="nav-bar"] > [class*="bottom"],
[class*="sidebar-nav"] > [class*="bottom"] {
    display: none !important;
}

button, input, textarea,
[class*="search"], [class*="editor"] {
    border-radius: 4px !important;
    box-shadow: none !important;
}

input, textarea, [contenteditable="true"] {
    color: ${colors.fg} !important;
    caret-color: ${colors.red} !important;
}

.nav-bar-main, .sidebar-wrap, .c2c-list-wrap,
.contact-list-wrap, [class*="recent-contact"] {
    background: ${colors.bg} !important;
}

.chat-content-wrap, .main-content-wrap, .msg-list-wrap,
[class*="chat-content"], [class*="message-list"] {
    background: ${colors.bg} !important;
}

.chat-input-area, .editor-wrap, .chat-input, .aio-input,
[class*="input-area"], [class*="chat-input"], [class*="aio-input"],
[class*="editor-container"], [class*="editor-wrap"] {
    background: ${colors.bg} !important;
    border-top: 1px solid ${colors.border} !important;
    box-shadow: none !important;
}

.header-bar, .title-bar, .aio-title, [class*="title-bar"],
[class*="window-title"], [class*="window-header"] {
    background: ${colors.bg} !important;
    border-bottom: 1px solid ${colors.border} !important;
}

[class*="chat-header"], [class*="aio-header"] {
    background: ${colors.bg} !important;
    border-bottom: 1px solid ${colors.border} !important;
}

[class*="chat-header"], [class*="aio-header"],
.chat-input-area, .aio-input, [class*="input-area"] {
    position: relative;
}

.chat-input-area::before, .aio-input::before,
[class*="input-area"]::before {
    content: "[ INPUT ]";
    position: absolute;
    top: 5px;
    right: 9px;
    z-index: 4;
    color: ${colors.muted};
    font: 700 10px/1 "VictorMono Nerd Font Mono", monospace;
    letter-spacing: .08em;
    pointer-events: none;
}

[class*="search"] input, [class*="search-input"] {
    color: ${colors.fg2} !important;
    background: ${colors.surface} !important;
    border: 1px solid ${colors.border} !important;
}

[class*="list-item"], [class*="contact-item"],
.nav-item, .item-hover {
    border-radius: 0 !important;
    border-left: 3px solid transparent !important;
}

[class*="list-item"]:hover, [class*="contact-item"]:hover,
.item-hover:hover, .nav-item:hover {
    background: ${colors.surface} !important;
}

[class*="list-item"][class*="active"],
[class*="contact-item"][class*="active"],
.nav-item.active, .item-active {
    background: ${colors.surface} !important;
    border-left-color: ${colors.red} !important;
    color: ${colors.fg} !important;
}

[class*="message-content"], [class*="msg-content"],
[class*="bubble"] {
    border-radius: 4px !important;
    box-shadow: none !important;
}

.btn-send, .send-btn, .send-button, [class*="send-btn"],
[class*="send-button"], button[type="submit"] {
    color: ${colors.bg} !important;
    background: ${colors.red} !important;
    border: 0 !important;
    border-radius: 4px !important;
}

.btn-send::before, .send-btn::before, .send-button::before,
[class*="send-btn"]::before, [class*="send-button"]::before {
    content: "> ";
    font-weight: 800;
}

[class*="toolbar"] button, [class*="tool-bar"] button,
[class*="action"] button {
    color: ${colors.fg2} !important;
    background: transparent !important;
    border-left: 1px solid transparent !important;
}

[class*="toolbar"] button:hover, [class*="tool-bar"] button:hover,
[class*="action"] button:hover {
    color: ${colors.red} !important;
    border-left-color: ${colors.red} !important;
}

button[class*="primary"], [class*="primary-btn"] {
    color: ${colors.bg} !important;
    background: ${colors.red} !important;
    border-color: ${colors.red} !important;
}

button[class*="primary"]:hover, [class*="primary-btn"]:hover {
    background: ${colors.redHover} !important;
    border-color: ${colors.redHover} !important;
}

.btn-send:hover, .send-btn:hover, .send-button:hover,
[class*="send-btn"]:hover, [class*="send-button"]:hover {
    background: ${colors.redHover} !important;
}

.badge-wrap .badge, [class*="unread"] [class*="badge"] {
    color: ${colors.fg} !important;
    background: ${colors.red} !important;
}

::selection {
    color: ${colors.fg} !important;
    background: ${colors.redDark} !important;
}

::-webkit-scrollbar { width: 4px; height: 4px; }
::-webkit-scrollbar-track { background: transparent; }
::-webkit-scrollbar-thumb {
    background: ${colors.border};
    border-radius: 0;
}
::-webkit-scrollbar-thumb:hover { background: ${colors.muted}; }
`;

function init() {
    document.documentElement.dataset.obsidianEmberTui = "loaded";
    if (!document.getElementById("obsidian-ember-tui")) {
        const style = document.createElement("style");
        style.id = "obsidian-ember-tui";
        style.textContent = css;
        (document.head ?? document.documentElement).appendChild(style);
    }
    applyVars();
    setTimeout(applyVars, 300);
    setTimeout(applyVars, 1500);
}

if (document.head) {
    init();
} else {
    new MutationObserver((_, observer) => {
        if (!document.head) return;
        observer.disconnect();
        init();
    }).observe(document.documentElement, { childList: true });
}
