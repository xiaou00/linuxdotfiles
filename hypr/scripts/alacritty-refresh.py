#!/usr/bin/env python3
"""Ask an Alacritty process to redraw, preserving all IPC config overrides."""

import argparse
import json
import os
from pathlib import Path
import socket
import sys


def refresh(pid):
    runtime = Path(os.environ.get("XDG_RUNTIME_DIR", f"/run/user/{os.getuid()}"))
    # An empty config update reloads the existing window config and marks it dirty.
    # -1 redraws the windows of this process; reset=false preserves their overrides.
    message = {"Config": {"options": [], "window_id": -1, "reset": False}}
    for path in runtime.glob(f"Alacritty-*-{pid}.sock"):
        try:
            with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as connection:
                connection.settimeout(0.5)
                connection.connect(str(path))
                connection.sendall(json.dumps(message).encode() + b"\n")
            return True
        except OSError:
            continue
    return False


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pid", type=int, nargs="+")
    args = parser.parse_args()
    failed = [pid for pid in set(args.pid) if pid <= 0 or not refresh(pid)]
    if failed:
        print(f"Alacritty refresh: IPC unavailable for PID(s) {failed}", file=sys.stderr)
        sys.exit(1)
