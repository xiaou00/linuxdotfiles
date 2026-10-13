"""Keep Kitty's subtle background grid aligned to two text cells."""

from __future__ import annotations

import struct
import zlib
from pathlib import Path
from typing import Any

from kitty.boss import Boss
from kitty.fast_data_types import cell_size_for_window
from kitty.window import Window


GRID_PATH = Path.home() / ".config" / "kitty" / "grid-current.png"
MODE_PATH = Path.home() / ".config" / "theme" / "mode"
GENERATION_PATH = Path.home() / ".config" / "theme" / "generation"
LINE_RGBA = bytes((208, 208, 208, 4))  # ~1.6% alpha
CLEAR_RGBA = bytes((0, 0, 0, 0))
DOT_RADIUS = 1  # 3x3 px dot centered on each lattice point

_last_grid: dict[int, tuple[int, int, int, int]] = {}


def _chunk(kind: bytes, payload: bytes) -> bytes:
    body = kind + payload
    return struct.pack(">I", len(payload)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)


def _grid_png(width: int, height: int, line_x: int, line_y: int) -> bytes:
    """Build one grid period with a small dot at each lattice point."""
    rows = []
    for y in range(height):
        pixels = bytearray()
        for x in range(width):
            # Toroidal distance keeps dots intact when their center lies on
            # an image edge and the texture repeats.
            dx = min((x - line_x) % width, (line_x - x) % width)
            dy = min((y - line_y) % height, (line_y - y) % height)
            is_dot = dx <= DOT_RADIUS and dy <= DOT_RADIUS
            pixels.extend(LINE_RGBA if is_dot else CLEAR_RGBA)
        rows.append(b"\x00" + pixels)  # PNG filter: none

    header = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    return (
        b"\x89PNG\r\n\x1a\n"
        + _chunk(b"IHDR", header)
        + _chunk(b"IDAT", zlib.compress(b"".join(rows), 9))
        + _chunk(b"IEND", b"")
    )


def on_resize(boss: Boss, window: Window, data: dict[str, Any]) -> None:
    """Align lattice points to exactly two columns by one terminal row."""
    # A per-window override is valid only for the global generation in which it
    # was created. This prevents an old override from returning after the
    # global theme cycles dark -> light -> dark.
    try:
        global_mode = MODE_PATH.read_text(encoding="utf-8").strip()
    except OSError:
        global_mode = "dark"
    try:
        global_generation = GENERATION_PATH.read_text(encoding="utf-8").strip()
    except OSError:
        global_generation = global_mode

    override = window.user_vars.get("SYSTEM_THEME_OVERRIDE")
    override_generation = window.user_vars.get("SYSTEM_THEME_GENERATION")
    effective_mode = override if override_generation == global_generation else global_mode

    # The dark lattice becomes visual noise on the paper-like light palette.
    if effective_mode == "light":
        return

    geometry = data["new_geometry"]
    cell_width, cell_height = cell_size_for_window(window.os_window_id)
    tile_width = max(2, cell_width * 2)
    tile_height = max(2, cell_height)

    # Kitty tiles background images from the OS-window origin, while text starts
    # at geometry.left/top after borders, tab bars and padding. Shift the lines
    # inside the tile so its repetitions land exactly on text-cell boundaries.
    line_x = geometry.left % tile_width
    line_y = geometry.top % tile_height
    grid = (tile_width, tile_height, line_x, line_y)
    if _last_grid.get(window.os_window_id) == grid:
        return

    png = _grid_png(tile_width, tile_height, line_x, line_y)
    temporary = GRID_PATH.with_suffix(".tmp")
    temporary.write_bytes(png)
    temporary.replace(GRID_PATH)

    boss.set_background_image(
        str(GRID_PATH),
        (window.os_window_id,),
        False,
        "tiled",
        png,
        linear_interpolation=False,
        tint=0.0,
    )
    _last_grid[window.os_window_id] = grid
