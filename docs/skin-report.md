# Skin & Player Model Investigation Report

This report summarizes how player skins, outer clothing layers, capes, dynamic model refreshing, and platform image decoders work in the ExtraMCPE engine.

---

## 1. Skin Texture Loading & OpenGL Binding
* **Texture File Path:** Default skin path is `"mob/char.png"` (`data/images/mob/char.png`).
* **Format & Size:** 32-bit RGBA PNG, standard $64 \times 32$ pixel layout (or $64 \times 64$ dynamic skin texture).
* **OpenGL Binding Pipeline:**
  1. `Textures::loadTexture("mob/char.png")` (`src/client/renderer/Textures.cpp`) loads raw pixels via `AppPlatform::loadTexture`.
  2. `PlayerRenderer::bindTexture` (`src/client/renderer/entity/PlayerRenderer.cpp`) binds the OpenGL texture handle `glBindTexture(GL_TEXTURE_2D, textureId)` before rendering `HumanoidModel`.

---

## 2. Supported Skin Layouts & Hat Layer
* **64x32 Layout:** The base `HumanoidModel` mapping assumes classic $64 \times 32$ UV coordinates across head, torso, arms, and legs.
* **Hat / Helmet Outer Layer:** Standard head overlay (hat layer, $32 \times 16$ UV region in the texture) is defined in `HumanoidModel::HumanoidModel()` (`src/client/model/HumanoidModel.cpp`) and drawn automatically via `headoverlay.render(...)`.
* **64x64 Modern Skins:** $64 \times 64$ texture files load successfully, but lower jacket/sleeve/pants overlay layers are ignored unless model UVs are expanded.

---

## 3. Player & Mob Model Architecture

```
+------------------------------------------------------------------------+
|                      HUMANOID MODEL ARCHITECTURE                       |
|                                                                        |
|  HumanoidModel (Inherits from Model)                                   |
|  +------------------------------------------------------------------+  |
|  | Parts: head, headoverlay, body, rightArm, leftArm, rightLeg, leftLeg|  |
|  | Render: Renders each ModelPart box mesh in 3D world space        |  |
|  +------------------------------------------------------------------+  |
|                                                                        |
|  Entities Using HumanoidModel:                                         |
|  - LocalPlayer & Player (Player characters)                            |
|  - Zombie & Skeleton (Hostile mobs)                                    |
|  - PigZombie (Nether mobs)                                             |
+------------------------------------------------------------------------+
```

---

## 4. Cape Rendering Pipeline
* **Code Location:** `PlayerRenderer::renderCape` (`src/client/renderer/entity/PlayerRenderer.cpp`).
* **Texture & Model:** Uses `player->cape` texture path. Draws a $16 \times 10$ single-box cape plane attached to the `torso` origin, animating sway based on player velocity vectors (`player->getInterpolatedPosition()`).

---

## 5. Platform PNG Decoders
* **Android (`AppPlatform_android.h`):** Uses JNI to call Java `MainActivity.getImageData` / `getImageDataFromBytes` via Android `BitmapFactory`.
* **iOS (`AppPlatform_iOS.mm`):** Uses native Apple `UIImage` / `CGImage` CoreGraphics framework.
* **Desktop (GLFW / Win32):** Uses `libpng` or `stb_image` C decoders.

---

## 6. Model Refreshing & Caching Constraints
* **Hardcoded UVs:** `HumanoidModel` hardcodes classic $64 \times 32$ texture coordinates across body parts.
* **Texture Cache Invalidation:** `Textures` caches OpenGL texture IDs by path string. Dynamically updating user skins at runtime requires calling `Textures::reload()` or binding a new dynamic texture ID.
