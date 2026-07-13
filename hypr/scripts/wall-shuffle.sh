#!/bin/bash

DIR="home/xiaou0/壁纸/TH/"
INTERVAL=10
while true; do
    # 随机挑选一张图片
    WALLPAPER=$(find "$DIR" -type f | shuf -n 1)

    # 使用 swww 切换，并添加动画效果
    swww img "$WALLPAPER" --transition-type wipe --transition-duration 2

    sleep $INTERVAL
done
