function themecolor --description '统一修改桌面应用背景色'
    # Fish removes an unquoted #RRGGBB as a comment before building argv.
    # Recover it only from a standalone interactive invocation, never history,
    # a script, or a compound command that could contain another invocation.
    if status is-interactive; and test (count $argv) -eq 0
        set -l input (status current-commandline | string collect)
        set -l color (string match --regex --groups-only \
            '^[ \t]*themecolor[ \t]+(#[^ \t\r\n]+)[ \t]*$' -- "$input")
        if test (count $color) -eq 1
            set argv $color[1]
        end
    end
    command "$HOME/.local/bin/themecolor" $argv
end
