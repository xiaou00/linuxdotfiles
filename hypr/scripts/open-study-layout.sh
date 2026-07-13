#!/usr/bin/env bash

LOG="$HOME/.cache/hypr-study-layout.log"
exec >"$LOG" 2>&1

set -u

LEFT_CLASS="study-left"
CAVA_CLASS="study-cava"
CLOCK_CLASS="study-clock"
BATTERY_CLASS="study-battery"

PDF_FILE="$HOME/学习资料/FOAG.pdf"

echo "==== $(date) ===="

need() {
  if ! command -v "$1" >/dev/null 2>&1; then
    notify-send "study layout" "缺少命令: $1"
    echo "Missing command: $1"
    exit 1
  fi
}

need kitty
need cava
need zathura
need jq

# 可选：清掉上一次跑歪的 study 窗口
# 如果你不想自动关闭旧窗口，把这几行删掉
hyprctl dispatch closewindow "class:^($LEFT_CLASS)$" 2>/dev/null || true
hyprctl dispatch closewindow "class:^($CAVA_CLASS)$" 2>/dev/null || true
hyprctl dispatch closewindow "class:^($CLOCK_CLASS)$" 2>/dev/null || true
hyprctl dispatch closewindow "class:^($BATTERY_CLASS)$" 2>/dev/null || true

sleep 0.4

# 当前 workspace
WS_ID="$(hyprctl activeworkspace -j | jq -r '.id')"

# 当前 monitor，可自动避开顶部 waybar / hyprpanel 预留区域
MONITOR_JSON="$(hyprctl monitors -j | jq '.[] | select(.focused == true)')"

MX="$(echo "$MONITOR_JSON" | jq -r '.x')"
MY="$(echo "$MONITOR_JSON" | jq -r '.y')"
MW="$(echo "$MONITOR_JSON" | jq -r '.width')"
MH="$(echo "$MONITOR_JSON" | jq -r '.height')"

RES_TOP="$(echo "$MONITOR_JSON" | jq -r '.reserved[0] // 0')"
RES_RIGHT="$(echo "$MONITOR_JSON" | jq -r '.reserved[1] // 0')"
RES_BOTTOM="$(echo "$MONITOR_JSON" | jq -r '.reserved[2] // 0')"
RES_LEFT="$(echo "$MONITOR_JSON" | jq -r '.reserved[3] // 0')"

# 可用区域，不覆盖状态栏
AX=$(( MX + RES_LEFT ))
AY=$(( MY + RES_TOP ))
AW=$(( MW - RES_LEFT - RES_RIGHT ))
AH=$(( MH - RES_TOP - RES_BOTTOM ))

# 按你截图的比例
LEFT_W=$(( AW * 56 / 100 ))
RIGHT_X=$(( AX + LEFT_W ))
RIGHT_W=$(( AW - LEFT_W ))

TOP_H=$(( AH * 18 / 100 ))
PDF_Y=$(( AY + TOP_H ))
PDF_H=$(( AH - TOP_H ))

# 右上三块：cava 小，clock 大，battery 很小
CAVA_W=$(( RIGHT_W * 26 / 100 ))
CLOCK_W=$(( RIGHT_W * 58 / 100 ))
BATTERY_W=$(( RIGHT_W - CAVA_W - CLOCK_W ))

CAVA_X="$RIGHT_X"
CLOCK_X=$(( RIGHT_X + CAVA_W ))
BATTERY_X=$(( RIGHT_X + CAVA_W + CLOCK_W ))

echo "monitor: $AX $AY $AW $AH"
echo "left: $AX $AY $LEFT_W $AH"
echo "cava: $CAVA_X $AY $CAVA_W $TOP_H"
echo "clock: $CLOCK_X $AY $CLOCK_W $TOP_H"
echo "battery: $BATTERY_X $AY $BATTERY_W $TOP_H"
echo "pdf: $RIGHT_X $PDF_Y $RIGHT_W $PDF_H"

get_addr_by_class() {
  local CLASS="$1"
  hyprctl clients -j | jq -r \
    --arg c "$CLASS" \
    --argjson ws "$WS_ID" \
    '.[] | select((.class == $c or .initialClass == $c) and .workspace.id == $ws) | .address' \
    | tail -n 1
}

get_addr_by_pid() {
  local PID="$1"
  hyprctl clients -j | jq -r \
    --argjson pid "$PID" \
    '.[] | select(.pid == $pid) | .address' \
    | tail -n 1
}

wait_addr_by_class() {
  local CLASS="$1"
  local ADDR=""

  for i in $(seq 1 40); do
    ADDR="$(get_addr_by_class "$CLASS")"
    if [ -n "$ADDR" ] && [ "$ADDR" != "null" ]; then
      echo "$ADDR"
      return 0
    fi
    sleep 0.1
  done

  echo ""
  return 1
}

wait_addr_by_pid() {
  local PID="$1"
  local ADDR=""

  for i in $(seq 1 50); do
    ADDR="$(get_addr_by_pid "$PID")"
    if [ -n "$ADDR" ] && [ "$ADDR" != "null" ]; then
      echo "$ADDR"
      return 0
    fi
    sleep 0.1
  done

  echo ""
  return 1
}

place_addr() {
  local ADDR="$1"
  local X="$2"
  local Y="$3"
  local W="$4"
  local H="$5"

  if [ -z "$ADDR" ]; then
    echo "empty address, skip"
    return 1
  fi

  local MATCH="address:$ADDR"

  hyprctl dispatch focuswindow "$MATCH"
  sleep 0.05

  hyprctl dispatch setfloating "$MATCH"
  sleep 0.05

  hyprctl dispatch resizewindowpixel "exact $W $H,$MATCH"
  sleep 0.05

  hyprctl dispatch movewindowpixel "exact $X $Y,$MATCH"
  sleep 0.05
}

# 启动左侧 nvim
kitty --class "$LEFT_CLASS" --title "$LEFT_CLASS" bash -lc 'nvim' &
LEFT_PID=$!

sleep 0.3

# 启动 cava
kitty --class "$CAVA_CLASS" --title "$CAVA_CLASS" bash -lc '
cava
echo
echo "cava exited"
read -r
' &
CAVA_PID=$!

sleep 0.3

# 启动 clock
kitty --class "$CLOCK_CLASS" --title "$CLOCK_CLASS" bash -lc '
while true; do
  clear
  printf "\n\n"
  date +"%H:%M:%S"
  sleep 1
done
' &
CLOCK_PID=$!

sleep 0.3

# 启动 battery
kitty --class "$BATTERY_CLASS" --title "$BATTERY_CLASS" bash -lc '
while true; do
  clear
  printf "\n"
  BAT_PATH=$(ls -d /sys/class/power_supply/BAT* 2>/dev/null | head -n1)

  if [ -z "$BAT_PATH" ]; then
    echo "Battery"
    echo
    echo "No BAT"
  else
    CAP=$(cat "$BAT_PATH/capacity")
    STATUS=$(cat "$BAT_PATH/status")
    echo "Battery"
    echo
    echo "${CAP}%"
    echo
    echo "$STATUS"
  fi

  sleep 5
done
' &
BATTERY_PID=$!

sleep 0.3

# 启动 zathura
if [ -f "$PDF_FILE" ]; then
  zathura "$PDF_FILE" &
  PDF_PID=$!
else
  notify-send "study layout" "PDF 不存在: $PDF_FILE"
  echo "PDF not found: $PDF_FILE"
  PDF_PID=""
fi

# 等窗口真正出现
LEFT_ADDR="$(wait_addr_by_class "$LEFT_CLASS")"
CAVA_ADDR="$(wait_addr_by_class "$CAVA_CLASS")"
CLOCK_ADDR="$(wait_addr_by_class "$CLOCK_CLASS")"
BATTERY_ADDR="$(wait_addr_by_class "$BATTERY_CLASS")"

PDF_ADDR=""
if [ -n "$PDF_PID" ]; then
  PDF_ADDR="$(wait_addr_by_pid "$PDF_PID")"
fi

# zathura 有些版本会 fork，pid 匹配不到时，退回找当前 workspace 的 zathura
if [ -z "$PDF_ADDR" ] || [ "$PDF_ADDR" = "null" ]; then
  PDF_ADDR="$(hyprctl clients -j | jq -r \
    --argjson ws "$WS_ID" \
    '.[] | select((.class == "org.pwmt.zathura" or .initialClass == "org.pwmt.zathura") and .workspace.id == $ws) | .address' \
    | tail -n 1)"
fi

# 左侧 PDF，占左边整块
place_addr "$PDF_ADDR" "$AX" "$AY" "$LEFT_W" "$AH"

# 右上三个状态窗口
place_addr "$CAVA_ADDR" "$CAVA_X" "$AY" "$CAVA_W" "$TOP_H"
place_addr "$CLOCK_ADDR" "$CLOCK_X" "$AY" "$CLOCK_W" "$TOP_H"
place_addr "$BATTERY_ADDR" "$BATTERY_X" "$AY" "$BATTERY_W" "$TOP_H"

# 右下 nvim
place_addr "$LEFT_ADDR" "$RIGHT_X" "$MAIN_Y" "$RIGHT_W" "$MAIN_H"

# 最后聚焦 nvim
hyprctl dispatch focuswindow "address:$LEFT_ADDR"

echo "done"
