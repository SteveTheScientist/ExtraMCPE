# Java Edition Resource Pack Font System

This codebase loads and renders Minecraft: Java Edition resource pack bitmap fonts directly from `assets/minecraft/textures/font/` and `assets/minecraft/font/default.json`.

## How to Add Another Java Font Pack

1. Place the font texture images (`ascii.png`, `accented.png`, `nonlatin_european.png`, etc.) inside `assets/minecraft/textures/font/`.
2. If the resource pack provides a custom `default.json`, place it at `assets/minecraft/font/default.json`.
3. Ensure textures are decoded as RGBA8888 with alpha transparency preserved.

## Font Provider Specifications
- **Bitmap Provider**: Maps a grid of characters in an image file to unicode codepoints based on `chars` strings, `height`, `ascent`, and pixel alpha scanning.
- **Space Provider**: Maps space characters to exact pixel advance widths (e.g. advance 4 for `' '`).

## Root Cause of Previous Font Rendering Artifacts

1. **Fractional UV Coordinates & Non-Integer Coordinates**: The old system used arbitrary UV coordinate adjustments (`7.99f / 128.0f` and fractional half-texel offsets) which caused pixel stretching on quad boundaries.
2. **Metadata Pixel Bleeding**: The legacy texture included row 7 metadata marks at the bottom of 8x8 glyph cells. When quad height matched cell height, the bottom line was rendered on screen (causing `.` to render like an underscore at the baseline).
3. **Texture Filtering & Mipmapping**: Linear texture filtering and mipmaps caused adjacent empty or neighboring glyph pixels to bleed into character edges during scaling.
4. **Fractional GUI Scaling**: Non-integer scaling factors caused fractional screen pixel vertex positioning, resulting in stretched or doubled letter tops. The new system enforces `GL_NEAREST` filtering, `GL_CLAMP_TO_EDGE` wrap mode, integer pixel snapping (`floor`), and exact UV texture coordinates.
