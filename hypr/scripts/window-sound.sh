#!/bin/bash
SOCKET="$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/.socket2.sock"
SOUND="$HOME/.config/hypr/sounds/bubble-pop.ogg"

# while true; do
#   socat -U - UNIX-CONNECT:"$SOCKET" 2>/dev/null \
#     | while read -r line; do
#         case "$line" in
#           openwindow*)
#             setsid pw-play "$SOUND" &
#             ;;
#         esac
#       done
#   sleep 1
# done
