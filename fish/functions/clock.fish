function clock
    kitty --title tty-clock \
        --override font_size=7 \
        --override initial_window_width=56c \
        --override initial_window_height=9c \
        fish -c "while true; tty-clock -c -C 7 -b -s; end" &
    disown
end
