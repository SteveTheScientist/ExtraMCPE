# Bitmap Font Rendering & Text Controls

This document describes how bitmap fonts, section sign color codes, text width calculations, and text input box controls (`TextBox`) work in the game engine.

---

## 1. Bitmap Font System & Text Formatting

Fonts are loaded from $256 \times 256$ pixel PNG font sheets (such as `assets/minecraft/textures/font/ascii.png`).

```
+-------------------------------------------------------------------------+
|                        FONT RENDER PIPELINE                             |
|                                                                         |
|   1. TEXT STRING            2. COLOR CODES            3. GLYPH RENDER   |
|  +--------------------+    +--------------------+    +----------------+ |
|  | "§cHello §bWorld"  | -> | §c = Red Color     | -> | Draws textured | |
|  |                    |    | §b = Aqua Color    |    | character quad | |
|  +--------------------+    +--------------------+    +----------------+ |
+-------------------------------------------------------------------------+
```

### Color Codes (`§` Section Sign Codes)
In Minecraft text strings, color formatting uses the section sign character `§` followed by a color code digit:

| Code | Color Name | RGB Hex Value |
| :---: | :--- | :--- |
| `§0` | Black | `#000000` |
| `§1` | Dark Blue | `#0000AA` |
| `§2` | Dark Green | `#00AA00` |
| `§4` | Red | `#FF5555` |
| `§a` | Green | `#55FF55` |
| `§b` | Aqua | `#55FFFF` |
| `§c` | Light Red | `#FF5555` |
| `§e` | Yellow | `#FFFF55` |
| `§f` | White | `#FFFFFF` |
| `§r` | Reset Color | Resets to default text color |

---

## 2. Text Input Component (`TextBox`)

`TextBox` (`src/client/gui/components/TextBox.cpp`) is the unified component for all text entry fields in the game (Chat, Server Console, World Name, Sign Edit).

### Key Features
* **Focus & Cursor Navigation:** Clicking inside a `TextBox` sets focus and draws a blinking cursor (`|`).
* **Text Selection & Highlight:** Dragging mouse or holding Shift + Arrow keys selects text regions.
* **Clipboard Integration:** Supports Ctrl+C (Copy), Ctrl+V (Paste), and Ctrl+X (Cut) across Windows, Mac, Linux, Android, and iOS.
* **Format Filtering:** Strips formatting codes from user input when `isPlayerText` is true.

---

## 3. Unit Testing & Font Verification (`test_font`)

The `test_font` target tests string width calculations, character advance metrics, and `TextBox` character insertion:

```bash
# Compile and run font unit tests
cmake --build build --target test_font
./build/test_font
```
