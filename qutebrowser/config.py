"""Minimal gray/red/white TUI configuration for qutebrowser."""

import json
from pathlib import Path

config.load_autoconfig(False)

palette = {
    "bg": "#151515", "bg1": "#1b1b1b", "bg2": "#242424",
    "bg3": "#3a3a3a", "fg": "#f2f2f2", "fg2": "#d0d0d0",
    "fg3": "#888888", "red": "#ff4444", "sel": "#4a1f1f",
}
mode = "dark"
try:
    palette_file = Path.home() / ".config/theme/palette.json"
    mode_file = Path.home() / ".config/theme/mode"
    mode = mode_file.read_text().strip()
    if mode not in {"dark", "light"}:
        mode = "dark"
    palette.update(json.loads(palette_file.read_text())[mode])
except (OSError, KeyError, TypeError, json.JSONDecodeError):
    pass

BG = palette["bg"]
BG1 = palette["bg1"]
BG2 = palette["bg2"]
BG3 = palette["bg3"]
FG = palette["fg"]
FG2 = palette["fg2"]
MUTED = palette["fg3"]
RED = palette["red"]
RED_DARK = palette["sel"]

# Layout: browser chrome appears only when it carries useful information.
c.tabs.show = "multiple"
c.tabs.position = "top"
c.tabs.favicons.show = "never"
c.tabs.indicator.width = 2
c.tabs.indicator.padding = {"top": 0, "bottom": 0, "left": 0, "right": 0}
c.tabs.padding = {"top": 5, "bottom": 5, "left": 9, "right": 9}
c.tabs.title.format = "{audio}{index}: {current_title}"
c.tabs.title.format_pinned = "{index}"
c.tabs.last_close = "close"
c.statusbar.show = "in-mode"
c.statusbar.position = "bottom"
c.statusbar.padding = {"top": 4, "bottom": 4, "left": 8, "right": 8}
c.completion.height = "32%"
c.completion.scrollbar.width = 3
c.completion.scrollbar.padding = 0
c.messages.timeout = 2200
c.scrolling.bar = "never"
c.window.transparent = True

# Typography and compact geometry.
c.fonts.default_family = ["JetBrains Mono", "LXGW WenKai Screen"]
c.fonts.default_size = "10pt"
c.fonts.web.family.fixed = "JetBrains Mono"
c.fonts.web.size.default_fixed = 15
c.fonts.completion.entry = "10pt default_family"
c.fonts.completion.category = "bold 10pt default_family"
c.fonts.statusbar = "10pt default_family"
c.fonts.tabs.selected = "bold 10pt default_family"
c.fonts.tabs.unselected = "10pt default_family"
c.fonts.hints = "bold 10pt default_family"
c.fonts.prompts = "10pt default_family"
c.fonts.messages.info = "10pt default_family"
c.fonts.messages.warning = "10pt default_family"
c.fonts.messages.error = "bold 10pt default_family"

# Completion menu.
c.colors.completion.fg = FG2
c.colors.completion.odd.bg = BG1
c.colors.completion.even.bg = BG1
c.colors.completion.category.fg = RED
c.colors.completion.category.bg = BG
c.colors.completion.category.border.top = BG3
c.colors.completion.category.border.bottom = BG3
c.colors.completion.item.selected.fg = FG
c.colors.completion.item.selected.bg = BG2
c.colors.completion.item.selected.border.top = RED
c.colors.completion.item.selected.border.bottom = RED
c.colors.completion.item.selected.match.fg = RED
c.colors.completion.match.fg = RED
c.colors.completion.scrollbar.fg = RED
c.colors.completion.scrollbar.bg = BG

# Status/command line.
c.colors.statusbar.normal.fg = FG2
c.colors.statusbar.normal.bg = BG
c.colors.statusbar.insert.fg = BG
c.colors.statusbar.insert.bg = RED
c.colors.statusbar.command.fg = FG
c.colors.statusbar.command.bg = BG
c.colors.statusbar.command.private.fg = FG
c.colors.statusbar.command.private.bg = BG1
c.colors.statusbar.passthrough.fg = BG
c.colors.statusbar.passthrough.bg = FG2
c.colors.statusbar.private.fg = FG2
c.colors.statusbar.private.bg = BG1
c.colors.statusbar.caret.fg = BG
c.colors.statusbar.caret.bg = RED
c.colors.statusbar.caret.selection.fg = FG
c.colors.statusbar.caret.selection.bg = RED_DARK
c.colors.statusbar.progress.bg = RED
c.colors.statusbar.url.fg = FG2
c.colors.statusbar.url.error.fg = RED
c.colors.statusbar.url.hover.fg = FG
c.colors.statusbar.url.success.http.fg = FG2
c.colors.statusbar.url.success.https.fg = FG
c.colors.statusbar.url.warn.fg = RED

# Tabs: quiet gray text, red active marker.
c.colors.tabs.bar.bg = BG
c.colors.tabs.odd.fg = MUTED
c.colors.tabs.odd.bg = BG
c.colors.tabs.even.fg = MUTED
c.colors.tabs.even.bg = BG
c.colors.tabs.selected.odd.fg = FG
c.colors.tabs.selected.odd.bg = BG2
c.colors.tabs.selected.even.fg = FG
c.colors.tabs.selected.even.bg = BG2
c.colors.tabs.indicator.start = MUTED
c.colors.tabs.indicator.stop = RED
c.colors.tabs.indicator.error = RED
c.colors.tabs.pinned.odd.fg = FG2
c.colors.tabs.pinned.odd.bg = BG
c.colors.tabs.pinned.even.fg = FG2
c.colors.tabs.pinned.even.bg = BG
c.colors.tabs.pinned.selected.odd.fg = FG
c.colors.tabs.pinned.selected.odd.bg = BG2
c.colors.tabs.pinned.selected.even.fg = FG
c.colors.tabs.pinned.selected.even.bg = BG2

# Prompts, messages, hints and downloads.
c.colors.prompts.fg = FG
c.colors.prompts.bg = BG1
c.colors.prompts.border = f"1px solid {BG3}"
c.colors.prompts.selected.bg = RED_DARK
c.colors.messages.info.fg = FG2
c.colors.messages.info.bg = BG1
c.colors.messages.info.border = BG3
c.colors.messages.warning.fg = FG
c.colors.messages.warning.bg = RED_DARK
c.colors.messages.warning.border = RED
c.colors.messages.error.fg = FG
c.colors.messages.error.bg = RED_DARK
c.colors.messages.error.border = RED
c.colors.hints.fg = BG
c.colors.hints.bg = RED
c.colors.hints.match.fg = FG
c.hints.border = "0px"
c.colors.downloads.start.fg = FG
c.colors.downloads.start.bg = BG2
c.colors.downloads.stop.fg = BG
c.colors.downloads.stop.bg = FG2
c.colors.downloads.error.fg = FG
c.colors.downloads.error.bg = RED
c.colors.keyhint.fg = FG2
c.colors.keyhint.suffix.fg = RED
c.colors.keyhint.bg = BG1
c.colors.contextmenu.menu.bg = BG1
c.colors.contextmenu.menu.fg = FG2
c.colors.contextmenu.selected.bg = BG2
c.colors.contextmenu.selected.fg = FG

# Web content and privacy defaults.
c.colors.webpage.bg = BG
c.colors.webpage.darkmode.enabled = False
c.content.user_stylesheets = (
    [str(Path.home() / ".config/qutebrowser/styles/dark-red-preview.css")]
    if mode == "dark"
    else []
)
c.content.autoplay = False
c.content.cookies.accept = "no-3rdparty"
c.content.headers.do_not_track = True
c.content.notifications.enabled = False
c.content.geolocation = False
c.content.canvas_reading = False
c.content.webgl = True
c.zoom.default = "100%"

# Blank, distraction-free startup and concise search aliases.
c.url.start_pages = ["about:blank"]
c.url.default_page = "about:blank"
c.url.searchengines = {
    "DEFAULT": "https://www.google.com/search?q={}",
    "g": "https://www.google.com/search?q={}",
    "gh": "https://github.com/search?q={}",
    "w": "https://zh.wikipedia.org/wiki/Special:Search?search={}",
}

# Small quality-of-life bindings without replacing qutebrowser's native set.
config.bind("<Ctrl-r>", "reload")
config.bind("<Ctrl-Shift-r>", "reload -f")
config.bind("<Ctrl-l>", "set-cmd-text -s :open")
config.bind("<Ctrl-t>", "open -t about:blank")
config.bind("<Ctrl-w>", "tab-close")
config.bind("<Alt-h>", "back")
config.bind("<Alt-l>", "forward")
config.bind("jk", "mode-leave", mode="insert")
