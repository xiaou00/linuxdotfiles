fish_add_path ~/.cargo/bin

if status is-interactive
    set -g fish_greeting ""

    # Colors are generated from ~/.config/theme/palette.json.
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
