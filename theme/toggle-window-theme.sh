#!/usr/bin/env bash
set -euo pipefail

config_home="${XDG_CONFIG_HOME:-$HOME/.config}"
runtime_dir="${XDG_RUNTIME_DIR:-/tmp}"
palette_file="$config_home/theme/palette.json"
mode_file="$config_home/theme/mode"
generation_file="$config_home/theme/generation"

notify() {
    notify-send -a Theme -i preferences-desktop-theme "Window theme" "$1" >/dev/null 2>&1 || true
}

global_mode="dark"
if [[ -r "$mode_file" ]]; then
    read -r global_mode < "$mode_file" || global_mode="dark"
fi
[[ "$global_mode" == "light" ]] || global_mode="dark"
global_generation="$global_mode"
if [[ -r "$generation_file" ]]; then
    read -r global_generation < "$generation_file" || global_generation="$global_mode"
fi
[[ -n "$global_generation" ]] || global_generation="$global_mode"

active_json="${WINDOW_THEME_ACTIVE_JSON:-$(hyprctl activewindow -j 2>/dev/null)}"
window_class=$(jq -r '.class // empty' <<< "$active_json")
window_pid=$(jq -r '.pid // 0' <<< "$active_json")
window_class_lower=${window_class,,}

opposite() {
    if [[ "$1" == "dark" ]]; then printf 'light\n'; else printf 'dark\n'; fi
}

write_kitty_palette() {
    local selected_mode="$1" output_file="$2"
    jq -r --arg mode "$selected_mode" '.[$mode] | [
      "foreground " + .fg2,
      "background " + .bg,
      "selection_foreground " + .fg,
      "selection_background " + .sel,
      "cursor " + .accent,
      "cursor_text_color " + .bg,
      "url_color " + .blue,
      "active_border_color " + .accent,
      "inactive_border_color " + .bg3,
      "active_tab_foreground " + .accent,
      "active_tab_background " + .bg,
      "inactive_tab_foreground " + .fg3,
      "inactive_tab_background " + .bg,
      "color0 " + .bg2, "color1 " + .red, "color2 " + .green,
      "color3 " + .yellow, "color4 " + .blue, "color5 " + .purple,
      "color6 " + .teal, "color7 " + .fg2, "color8 " + .fg3,
      "color9 " + .red, "color10 " + .green, "color11 " + .yellow,
      "color12 " + .blue, "color13 " + .purple, "color14 " + .teal,
      "color15 " + .fg
    ] | .[]' "$palette_file" > "$output_file"
}

write_nvim_override() {
    local nvim_pid="$1" selected_mode="$2"
    [[ "$nvim_pid" =~ ^[0-9]+$ ]] || return 0
    [[ -r "/proc/$nvim_pid/comm" ]] || return 0
    [[ $(< "/proc/$nvim_pid/comm") == "nvim" ]] || return 0

    local override_file="$runtime_dir/nvim-window-theme-$nvim_pid"
    printf '%s\n%s\n' "$selected_mode" "$global_generation" > "$override_file"
    kill -USR1 "$nvim_pid" 2>/dev/null || true
}

toggle_kitty() {
    local socket="unix:$runtime_dir/kitty-$window_pid"
    local colors current_bg light_bg target palette_path kitty_tree nvim_pid
    local -a nvim_pids=()

    colors=$(kitten @ --to "$socket" get-colors --match state:focused 2>/dev/null) || {
        notify "请重启一次 Kitty 以启用当前窗口配色切换"
        return 1
    }
    current_bg=$(awk '$1 == "background" { print $2; exit }' <<< "$colors")
    light_bg=$(jq -r '.light.bg' "$palette_file")
    if [[ "${current_bg,,}" == "${light_bg,,}" ]]; then target="dark"; else target="light"; fi

    palette_path="$runtime_dir/kitty-window-theme-$target.conf"
    write_kitty_palette "$target" "$palette_path"
    kitten @ --to "$socket" set-colors --match state:focused_os_window "$palette_path" >/dev/null
    kitten @ --to "$socket" set-user-vars --match state:focused_os_window \
        "SYSTEM_THEME_OVERRIDE=$target" \
        "SYSTEM_THEME_GENERATION=$global_generation" >/dev/null

    if [[ "$target" == "light" ]]; then
        kitten @ --to "$socket" set-background-image \
            --match state:focused_os_window none >/dev/null
    elif [[ -r "$config_home/kitty/grid-current.png" ]]; then
        kitten @ --to "$socket" set-background-image \
            --match state:focused_os_window --layout tiled \
            "$config_home/kitty/grid-current.png" >/dev/null
    fi

    kitty_tree=$(kitten @ --to "$socket" ls --match state:focused_os_window 2>/dev/null || true)
    mapfile -t nvim_pids < <(jq -r '.[].tabs[].windows[] |
      .foreground_processes[]? |
      select(((.cmdline[0] // "") | split("/")[-1]) == "nvim") |
      .pid' <<< "$kitty_tree" 2>/dev/null | sort -nu || true)
    for nvim_pid in "${nvim_pids[@]}"; do
        write_nvim_override "$nvim_pid" "$target"
    done

    notify "$window_class → $target"
}

toggle_neovim_window() {
    local nvim_pid="$window_pid" current="$global_mode" target override_file
    if [[ ! -r "/proc/$nvim_pid/comm" ]] || [[ $(< "/proc/$nvim_pid/comm") != "nvim" ]]; then
        nvim_pid=$(pgrep -P "$window_pid" -x nvim | head -n 1 || true)
    fi
    [[ -n "$nvim_pid" ]] || return 1

    override_file="$runtime_dir/nvim-window-theme-$nvim_pid"
    if [[ -r "$override_file" ]]; then
        mapfile -t override_lines < "$override_file"
        if [[ "${override_lines[1]:-}" == "$global_generation" ]]; then
            current="${override_lines[0]:-$global_mode}"
        fi
    fi
    target=$(opposite "$current")
    write_nvim_override "$nvim_pid" "$target"
    notify "$window_class → $target"
}

case "$window_class_lower" in
    kitty)
        toggle_kitty
        ;;
    neovide|nvim)
        toggle_neovim_window || notify "未找到当前窗口中的 Neovim 进程"
        ;;
    "")
        notify "当前没有活动窗口"
        ;;
    *)
        notify "$window_class 暂不支持安全的单窗口热切换"
        ;;
esac
