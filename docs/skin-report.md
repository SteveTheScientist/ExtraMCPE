# Skin and Player Model Architecture Report

### 1. Skin Texture Loading & Binding
* **Default Texture & Path:** `mob/char.png` (defined in `Player::Player`, `Player.cpp`). Local players fetch custom skins via `fetchSkinForPlayer` in `LocalPlayer.cpp` (downloads via `HttpClient::download` to `data/images/skins/<username>.png` and sets texture to `skins/<username>.png`).
* **Binding & Format:** Bound during rendering in `MobRenderer::render` calling `Textures::loadAndBindTexture` (`src/client/renderer/Textures.cpp`), which delegates to `AppPlatform::loadTexture` (`AppPlatform_glfw.h`, `AppPlatform_win32.h`). Decoded via `loadPngFromMemory` (`src/platform/PngLoader.cpp`) into 32-bit RGBA `TextureData`.
* **Dimensions:** 64x32 and 64x64 PNG files.

### 2. Supported Skin Sizes & Outer Layers
* **Sizes:** Handles 64x32 and 64x64 skins. Checked in `PlayerRenderer::isModernPlayerSkin` (`PlayerRenderer.cpp`) via `TextureData::w == 64 && h == 64`.
* **Outer Layers:** Rendered in `HumanoidModel::render` (`HumanoidModel.cpp`). 64x32 skins draw `hair` (head overlay). 64x64 skins additionally draw outer overlay layers for body, arms, and legs (`body`, `arm0`, `arm1`, `leg0`, `leg1`) initialized in `HumanoidModel::HumanoidModel`.

### 3. Player Model Construction & Drawing
* **Classes:** `HumanoidModel` (`src/client/model/HumanoidModel.cpp`), `PlayerRenderer` (`src/client/renderer/entity/PlayerRenderer.cpp`), and `HumanoidMobRenderer` (`src/client/renderer/entity/HumanoidMobRenderer.cpp`).
* **Cube Definitions:** `HumanoidModel::HumanoidModel` constructs boxes using `ModelPart::addBox` (`src/client/model/geom/ModelPart.cpp`) for head, hair, body, arm0, arm1, leg0, and leg1.
* **Slim Arms:** No Slim (3-pixel wide arm) option exists; arms are hardcoded to 4x12x4 pixels (`arm0.addBox(-3, -2, -2, 4, 12, 4, g)`).

### 4. Cape Code
* **Texture & Model:** Yes. `fetchCapeForPlayer` (`LocalPlayer.cpp`) downloads capes to `data/images/capes/<username>.png`.
* **Drawing Location:** Rendered in `HumanoidMobRenderer::additionalRendering` (`HumanoidMobRenderer.cpp`). Binds `Player::getCapeTexture()` and constructs/renders a 10x16x1 box mesh using `Tesselator::instance` with custom pitch/roll physics (`capeXRot`, `capeZRot`).

### 5. Runtime PNG Loading Across Platforms
* **Desktop (GLFW/Win32):** Uses `AppPlatform_glfw::loadTexture` / `AppPlatform_win32::loadTexture` and `loadPngFromMemory` (`src/platform/PngLoader.cpp`) using `libpng`. Supports loading files added/downloaded at runtime.
* **Android:** Uses `AppPlatform_android::loadTexture` (`AppPlatform_android.h`) calling JNI `MainActivity.getImageData`.
* **iOS:** Uses `AppPlatform_iOS::loadTexture` (`AppPlatform_iOS.mm`) via `UIImage` / `NSBundle mainBundle`.
* **Web:** Uses `AppPlatform_glfw` with Emscripten asset system.

### 6. Adding New Screens & Saving Options
* **New Screen:** Subclass `Screen` or `OptionsScreen` (e.g. `CloudsScreen` in `src/client/gui/screens/CloudsScreen.cpp`), initialized via `Minecraft::setScreen`.
* **Options Saving:** Options declared in `Options.cpp` (e.g., `OptionString skin`), stored in `Options::m_options`, and serialized to `options.txt` via `Options::save` calling `OptionsFile::save` (`src/client/OptionsFile.cpp`).

### 7. Difficulties for Custom Skin Loading
* **Texture Caching:** `Textures::loadTexture` (`Textures.cpp`) caches loaded texture IDs in `idMap` by name, preventing dynamic reloading without cache clearing.
* **Skin Format Detection:** `PlayerRenderer::isModernPlayerSkin` only inspects dimensions once on initial load.
* **Hardcoded UVs:** `HumanoidModel` has fixed 4-pixel arm UVs and geometry with no built-in support for slim (Alex) skin layouts.
