#!/usr/bin/env bash
STEP=0.05
direction="$1"

data=$(hyprctl activewindow -j)
address=$(echo "$data" | jq -r '.address')
current=$(echo "$data" | jq -r '.opacity')

if [[ "$direction" == "up" ]]; then
    new=$(awk "BEGIN {v=$current+$STEP; if(v>1)v=1; printf \"%.2f\", v}")
else
    new=$(awk "BEGIN {v=$current-$STEP; if(v<0.1)v=0.1; printf \"%.2f\", v}")
fi

hyprctl dispatch setprop address:"$address" alpha "$new"
