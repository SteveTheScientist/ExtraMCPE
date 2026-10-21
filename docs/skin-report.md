# Skin & Player Model Investigation Report

## 1. Skin Texture Loading & Binding
* **Path & File:** Default texture path `"mob/char.png"` (in `assets/images/mob/char.png`).
* **Format & Size:** 32-bit RGBA PNG, standard 64x32 size (or 64x64 dynamic texture).
* **Functions:** `Textures::loadTexture` (`src/client/renderer/Textures.cpp`) loads texture pixels via `AppPlatform::loadTexture`. `PlayerRenderer::bindTexture` (`src/client/renderer/entity/PlayerRenderer.cpp`) binds OpenGL texture ID before drawing `HumanoidModel`.

## 2. Supported Sizes & Second Layer
* **Sizes Handled:** Base `HumanoidModel` mapping assumes classic 64x32 UV coordinates. 64x64 textures are loaded but lower 64x64 jacket/sleeve/pants overlay layers are not mapped/drawn.
* **Hat/Helmet Layer:** Standard head overlay (hat layer, 32x16 UV region) is defined in `HumanoidModel::HumanoidModel()` (`src/client/model/HumanoidModel.cpp`) and drawn via `headoverlay.render(...)`.

## 3. Player & Mob Model Structure
* **Class Names:** `HumanoidModel` (`src/client/model/HumanoidModel.h`) inherits from `Model`. Uses `Cube` (`src/client/model/Cube.h`) for box definitions.
* **Entities Using Skin Model:** Local player (`LocalPlayer`), remote players (`Player`), Zombie (`Zombie`), Skeleton (`Skeleton`), PigZombie (`PigZombie`).
* **Slim Model Option:** No 3-pixel wide ("Alex" slim arm) option exists in `HumanoidModel`. Arm width is hardcoded to 4x4x12 pixels.

## 4. Cape Code Pipeline
* **Code Location:** `PlayerRenderer::renderCape` (`src/client/renderer/entity/PlayerRenderer.cpp`).
* **Texture & Model:** Uses `player->cape` texture path. Draws a 16x10 single-box cape plane attached to `torso` model origin, animating sway based on movement velocity vector (`player->getInterpolatedPosition()`).

## 5. Platform PNG Loading
* **Platforms:** Desktop (GLFW / Win32 / macOS), Android (`AppPlatform_android.h`), iOS (`AppPlatform_iOS.mm`).
* **Loader Mechanism:** Android uses JNI `MainActivity.getImageData` / `getImageDataFromBytes` via Android `BitmapFactory`. iOS uses `UIImage`/`CGImage`. Desktop uses `stb_image` or PNG decoders. Works with user-added files at runtime if path exists under application writable directory.

## 6. Options & Skin Customization Screen
* **Screen Base:** Sub-menu screens derive from `Screen` (`src/client/gui/Screen.h`), e.g., `OptionsScreen` (`src/client/gui/OptionsScreen.cpp`).
* **Persistence:** Saved in `options.txt` via `Options::save()` and loaded in `Options::load()` (`src/option/Options.cpp`).

## 7. Custom Skin Loading Obstacles
* **Hardcoded UVs:** `HumanoidModel` hardcodes classic 64x32 texture coordinates across `head`, `body`, `arm0`, `arm1`, `leg0`, `leg1`.
* **No Slim Arm Support:** Dynamic model mesh switching is not supported without extending `HumanoidModel`.
* **Texture Caching:** `Textures` caches loaded OpenGL texture IDs by path string; dynamically updated user skins require explicit dynamic texture cache invalidation (`Textures::reload()`).
