#!/bin/bash
action=$1
dir=$2
pidfile="/tmp/mouse-move-${dir}.pid"

case $dir in
    up)    dx=0;  dy=-8 ;;
    down)  dx=0;  dy=8  ;;
    left)  dx=-8; dy=0  ;;
    right) dx=8;  dy=0  ;;
esac

stop_loop() {
    if [ -f "$pidfile" ]; then
        kill "$(cat "$pidfile")" 2>/dev/null
        rm -f "$pidfile"
    fi
}

if [ "$action" = "start" ]; then
    stop_loop
    (
        deadline=$(($(date +%s) + 30))
        while [ $(date +%s) -lt $deadline ]; do
            ydotool mousemove -- $dx $dy
            sleep 0.02
        done
        rm -f "$pidfile"
    ) &
    echo $! > "$pidfile"
elif [ "$action" = "stop" ]; then
    stop_loop
elif [ "$action" = "stopall" ]; then
    for f in /tmp/mouse-move-*.pid; do
        [ -f "$f" ] && kill "$(cat "$f")" 2>/dev/null && rm -f "$f"
    done
fi
