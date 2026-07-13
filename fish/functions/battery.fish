function battery
    kitty --title battery \
        --override font_size=9 \
        --override initial_window_width=20c \
        --override initial_window_height=6c \
        fish -c "while true; /home/xiaou0/.cargo/bin/battery; sleep 5; end" &
    disown
end
