# Cloud Customizations Documentation

This document describes the recreated cloud rendering system and its customization settings for ExtraMC (MCPE 0.6.1 structure).

---

## Technical Inspection Report (Rule 1)

* **Graphics API:** Fixed-function OpenGL ES 1.x pipeline (`gles.h`/`gles.cpp` wrapper). No GLSL shaders are used or added.
* **Original Cloud Rendering:**
  * **2D Fast Clouds (`renderClouds`):** Single plane rendered at $Y = 128 - \text{cameraY} + 0.33$, scaled $1/2048$, $32 \times 32$ grid cells, textured with `environment/clouds.png`.
  * **3D Advanced Clouds (`renderAdvancedClouds`):** $6 \times 6$ grid of $8 \times 8$ sub-cells ($D=8$, $radius=3$), horizontal scale $ss = 12.0f$, base height $yy = \text{cloudY} - \text{cameraY} + 0.33f$. Default base height $Y = 108.0$. Textured with `environment/clouds.png`. Darker shading applied to bottom ($0.7\times$) and side faces ($0.9\times$, $0.8\times$).
* **Scrolling:**
  * Calculated per frame using time ($\text{ticks} + \alpha$) offset:
    $$xo = \frac{\text{cameraX} + \text{cloudTime} \times 0.03}{ss}$$
    $$zo = \frac{\text{cameraZ}}{ss} + 0.33$$
  * Integer cell offsets ($xCell = \lfloor xo \rfloor$, $zCell = \lfloor zo \rfloor$) track cell boundaries.
* **Optimization & Performance:**
  * Geometry mesh is cached and rebuilt **only** when the camera crosses a grid cell boundary ($xCell$ or $zCell$) or when cloud options change.
  * Zero per-frame heap allocations are made during cloud rendering. Smooth scrolling and camera translation are handled via matrix transform `glTranslatef(-xOffs, yy, -zOffs)`.
  * Outer boundary faces touching empty texture cells are generated; hidden inner faces between adjacent cloud cells are omitted to prevent double blending.
  * Translucent depth pre-pass is preserved to prevent alpha sorting artifacts.

---

## Options Menu & Cloud Customization Sub-Screen

* **Options Menu Organization:** Options buttons and toggles are organized into clean, categorized sub-screens:
  * **Controls & Input...** (`ControlsOptionsScreen`)
  * **Graphics & Visuals...** (`GraphicsOptionsScreen`)
  * **Interface & Game...** (`InterfaceOptionsScreen`)
* **Customizations Menu:** Contains the **Clouds...** sub-screen button (`CloudsScreen`).
* **Beta Sky Interaction:** When **Tall Clouds** is enabled, the **Beta Sky** toggle in the Options menu is automatically disabled (`active = false`) and displays the disabled button texture.

All cloud settings are saved in `options.txt`:

| Option Name | Range | Default | Options-File Key | Description |
|---|---|---|---|---|
| **Tall Clouds** | `ON` / `OFF` | `ON` (`true`) | `options.tall_clouds` | Toggles volumetric tall clouds. When `OFF`, restores original cloud rendering and re-enables Beta Sky option. |
| **Cloud Height** | `32` to `192` | `108` (`108.0`) | `options.cloud_y_level` | Adjusts the Y world level of the cloud layer base height. |
| **Cloud Thickness** | `1.0x` to `4.0x` | `2.5x` (`2.5`) | `options.cloud_thickness` | Multiplier for vertical cloud thickness (`thicknessScale`). Clouds grow upward from the configured base height. |
| **Cloud Fade** | `0%` to `100%` | `100%` (`1.0`) | `options.cloud_fade` | Strength of view-dependent vertical alpha fade. 100% enables full vertical gradient, 0% applies uniform opacity. |
| **Cloud Opacity** | `40%` to `100%` | `80%` (`0.8`) | `options.cloud_opacity` | Cloud base opacity constant (represented as $0.8$ constant in formula). |
| **Flat Shading** | `ON` / `OFF` | `ON` (`true`) | `options.cloud_flat_shading` | Uses uniform $1.0\times$ brightness on top, bottom, and side faces when `ON`. Restores darker bottom/side shading when `OFF`. |
| **Reset Clouds** | Action Button | N/A | N/A | Resets all cloud customization options to their default values live. |

---

## Unit Testing & Verification

### Pure Function Formula
The pure function `cloudAlpha(ny, rel, baseAlpha, opacity, fadeStrength)` in `src/client/renderer/CloudUtils.h` computes vertex opacity according to:
$$rel = \text{layerBaseY} - \text{cameraY}$$
$$m = \frac{\text{clamp}(rel / 2.5, -1, 1) + 1}{2}$$
$$fade = (1 - ny) \times (1 - m) + ny \times m$$
$$\alpha_{faded} = \text{max}(0, \text{baseAlpha} \times (\text{opacity} - fade))$$

### Test Executable (`test_clouds`)
* Automated C++ unit tests (`test/test_clouds.cpp`) verify exact values:
  * `rel >= 2.5`: bottom $ny=0 \to 0.8$, top $ny=1 \to 0.0$
  * `rel <= -2.5`: bottom $ny=0 \to 0.0$, top $ny=1 \to 0.8$
  * `rel = 0.0`: bottom $ny=0 \to 0.3$, top $ny=1 \to 0.3$
  * `fadeStrength = 0.0`: constant $0.8$ opacity everywhere.
* Ran via `build/test_clouds`.

---

## What Could and Could Not Be Tested

### Tested
* **Mathematical Accuracy:** Verified all test cases in `cloudAlpha` via the unit test binary `test_clouds`.
* **Desktop Build & Compilation:** Compiled and linked the desktop C++ binary (`MinecraftPE`) using CMake with zero errors.
* **Option Persistence & UI Integration:** Verified `Options.cpp` serialization/parsing, slider label formatting, `CloudsScreen` menu navigation, categorized sub-screens, disabled Beta Sky state, cleaned Customizations menu, and build file updates.

### Untested
* **On-Device Physical Rendering:** Visual testing on an actual physical Android/iOS device with touch inputs or mobile GPU drivers (headless build environment without attached physical device).
