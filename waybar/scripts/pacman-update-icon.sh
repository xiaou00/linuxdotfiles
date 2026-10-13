#!/usr/bin/env bash

pkgs=$(checkupdates 2>/dev/null | wc -l)

if [ "$pkgs" -eq 0 ]; then
    echo ""
else
    echo "󰏖 $pkgs"
fi
