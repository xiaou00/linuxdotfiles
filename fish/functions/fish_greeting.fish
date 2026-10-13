function fish_greeting
    set -l body_color white
    set -l mode_file "$HOME/.config/theme/mode"

    # ANSI "white" is intentionally gray in the light terminal palette.
    # Use true black here so this mark keeps a crisp black-on-paper outline.
    if test -r "$mode_file"; and test (string trim < "$mode_file") = light
        set body_color 000000
    end

    echo (set_color $body_color)"▄█████████▄"
    echo (set_color $body_color)"█████████▓▓"
    echo (set_color $body_color)"█████████▓▓"
    echo (set_color $body_color)"█▄▄"(set_color red)"▄"(set_color $body_color)"▄▄▄▄▄█████▀"
    echo (set_color $body_color)"▀██"(set_color red)"█"(set_color $body_color)"████████▀"(set_color normal)
end
