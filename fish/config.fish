fish_add_path ~/.cargo/bin

if status is-interactive
    set -g fish_greeting ""

    # obsidian-ember color theme
    set -U fish_color_normal         d3d3d3
    set -U fish_color_command        44bbbb
    set -U fish_color_keyword        ff8548
    set -U fish_color_quote          76c793
    set -U fish_color_redirection    00c3a5
    set -U fish_color_end            e77726
    set -U fish_color_error          ff6464
    set -U fish_color_param          d3d3d3
    set -U fish_color_comment        565656
    set -U fish_color_match          --background=2c2c2c
    set -U fish_color_selection      --background=333333
    set -U fish_color_operator       e77726
    set -U fish_color_escape         bd5e91
    set -U fish_color_autosuggestion 565656
    set -U fish_color_cwd            44bbbb
    set -U fish_color_cwd_root       ff6464
    set -U fish_color_user           76c793
    set -U fish_color_host           44bbbb
    set -U fish_color_host_remote    00c3a5
    set -U fish_pager_color_prefix        ff8548
    set -U fish_pager_color_completion    d3d3d3
    set -U fish_pager_color_description  565656
    set -U fish_pager_color_progress      e77726
    # 别名设置 (Aliases)
    alias ls='ls --color=auto'
    alias la='ls -a'
    alias ll='ls -l'
    alias hypr='start-hyprland'
    alias kde='startplasma-wayland'
    alias kde-x11='startplasma-x11'
    alias grep='grep --color=auto'
    alias nv='nvim'
    alias v='nvim'
    starship init fish | source
end
