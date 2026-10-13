# Restore Alacritty's configured cursor color when returning to the shell.
# Programs may leave an OSC 12 override behind after changing themes.
if status is-interactive; and set -q ALACRITTY_WINDOW_ID
    function __alacritty_restore_cursor --on-event fish_prompt
        if isatty stdout
            printf '\e]112\a'
        end
    end

    __alacritty_restore_cursor
end
