# Volumetric 3D Clouds System

This document describes how the volumetric 3D cloud rendering system and cloud customization options work in the game engine.

---

## 1. How Cloud Rendering Works

Clouds are rendered as a 3D volumetric layer above the world using OpenGL ES fixed-function geometry.

```
+-------------------------------------------------------------------------+
|                        CLOUD RENDER ARCHITECTURE                        |
|                                                                         |
|   1. SCROLLING POSITIONS        2. GRID MESH GENERATION     3. OPACITY  |
|  +-----------------------+     +------------------------+  +----------+ |
|  | xCell = floor(cameraX)| --> | Renders 6x6 Grid Cells |->| Distance | |
|  | zCell = floor(cameraZ)|     | Builds 3D Box Geometry |  | Fade     | |
|  +-----------------------+     +------------------------+  +----------+ |
+-------------------------------------------------------------------------+
```

* **Grid Scrolling:** As time passes or the player moves, cloud offsets ($xCell$, $zCell$) update smoothly using matrix translation (`glTranslatef`).
* **Mesh Caching:** To maximize performance, cloud 3D mesh geometry is rebuilt only when the camera crosses grid cell boundaries, eliminating per-frame allocations.
* **Inner Face Culling:** Hidden inner faces between adjacent cloud cells are omitted to prevent alpha blending overlap artifacts.

---

## 2. Cloud Customization Settings (`options.txt`)

All cloud options are configured in the **Clouds...** options sub-screen and saved to `options.txt`:

| Option Name | Default | Options Key | Description |
| :--- | :---: | :--- | :--- |
| **Cloud Height** | `108` | `options.cloud_y_level` | World Y height of the cloud layer ($32$ to $192$). |
| **Cloud Thickness** | `2.5x` | `options.cloud_thickness` | Vertical thickness multiplier of cloud geometry ($1.0\times$ to $4.0\times$). |
| **Cloud Fade** | `100%` | `options.cloud_fade` | View-dependent vertical gradient fade strength ($0\%$ to $100\%$). |
| **Cloud Opacity** | `80%` | `options.cloud_opacity` | Base opacity of clouds ($40\%$ to $100\%$). |
| **Flat Shading** | `ON` | `options.cloud_flat_shading` | Toggles uniform brightness (`ON`) vs darker bottom/side shading (`OFF`). |

---

## 3. Alpha Fade Formula & Unit Testing

Cloud opacity vertex alpha is computed using `cloudAlpha()` (`src/client/renderer/CloudUtils.h`):

$$rel = \text{layerBaseY} - \text{cameraY}$$
$$m = \frac{\text{clamp}(rel / 2.5, -1, 1) + 1}{2}$$
$$fade = (1 - ny) \times (1 - m) + ny \times m$$
$$\alpha_{faded} = \text{max}(0, \text{baseAlpha} \times (\text{opacity} - fade))$$

### Running Cloud Unit Tests
Run the standalone unit test binary:
```bash
cmake --build build --target test_clouds
./build/test_clouds
```
