# hyprhex

A small Hyprland 0.56.2 plugin that reveals opening windows cell-by-cell through a red hexagonal lattice. The first tiled window (and any window without a usable layout neighbour) keeps the original free diagonal field. Later tiled windows detect the side shared with the surrounding layout and reveal edge-to-edge toward that insertion boundary, following the old layout's squeeze direction: a new right-hand tile sweeps right-to-left, and a new lower tile sweeps bottom-to-top.

Closing windows use the same window-aligned cells as a directional dissolve. The plugin records the surrounding layout before removal, compares it with Hyprland's resulting target geometry, and sweeps a red hexagonal front from the opposite edge toward the boundary reached by the remaining windows. A lower tile expanding upward therefore dissolves the closing window from bottom to top and leaves its last cells at the top edge. Balanced fill dissolves inward from both edges and finishes at the centre. Floating, fullscreen, or otherwise unchanged layouts use a centred aspect-aware fallback.

Opening and closing have independent clocks: `duration` controls opening while `close_duration` controls closing. The supplied configuration uses 420 ms and 240 ms respectively, so closing remains complete and directional but feels substantially more immediate. The close shader continues returning a fully transparent snapshot after its clock ends, preventing a shortened duration from exposing a stale native fade frame.

Opening waits for the client's target geometry to remain quiet for `open_settle` milliseconds (capped internally so a noisy client cannot stall forever). While settling, the first frame stays fully hidden; during the reveal, oversized client buffers are clipped instead of stretched. The accompanying config uses `popin 100%` so Hyprland does not add a competing scale animation. This specifically avoids the visible resize twitch produced by clients such as Zathura and QQ's image viewer during their first configure sequence.

Fcitx/Fcitx5 candidate windows remain excluded from both plugin effects. Wofi is deliberately run as a centred normal floating window, so its translucent red hexagonal UI receives the same opening and closing lattice animation. The Super+R, Waybar, and wallpaper-picker launch paths all use this normal-window route; a manually forced layer-shell Wofi falls back to Hyprland's layer fade.

Kitty windows also receive a persistent hexagonal frost pattern arranged into alternating upright and inverted triangles. A dense 6-cell axial super-grid keeps the triangle edge at the lowest blur-matte level and assigns its inner cells to a deterministic three-phase sequence covering the other three levels. This produces tight circuit-like fields while every hexagon remains perfectly flat, with no random selection, internal grain, or color variation. Kitty's content and colors remain untouched. The path reuses the opening animation's exact `hexCell`, window-relative origin, and `cell_size`, so the opening red lattice and the final frost cells coincide. Configure it with `kitty_frost`, `cell_size`, and `kitty_frost_contrast` under `plugin:hyprhex`. Hyprland still uses one global blur kernel; the cells vary how strongly that blurred backdrop participates.

Build:

```sh
make
```

Load or unload manually:

```sh
hyprctl plugin load /home/xiaou0/.config/hypr/plugins/hyprhex/hyprhex.so
hyprctl plugin unload /home/xiaou0/.config/hypr/plugins/hyprhex/hyprhex.so
```

The plugin is ABI-bound to the installed Hyprland headers. Rebuild it after every Hyprland upgrade.
