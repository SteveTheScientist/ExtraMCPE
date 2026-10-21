# GUI & Player Model Developer Guide

## 1. GUI Component APIs

### `Screen` (`src/client/gui/Screen.h`)
* Pointer Access: `Screen::init(Minecraft* mc, int w, int h)` sets `this->minecraft = mc`, `width`, and `height`.
* Lifecycle & Event Signatures:
  * `virtual void init()` - Instantiate buttons and components.
  * `virtual void buttonClicked(Button* button)` - Handle button click actions.
  * `virtual void mouseClicked(int x, int y, int buttonNum)` - Dispatch touch/click events.
  * `virtual void keyPressed(int eventKey)` - Handle key presses (e.g., `Keyboard::KEY_ESCAPE`).
  * `virtual void render(int xm, int ym, float a)` - Draw screen elements and `buttons`.
  * `virtual void tick()` - Update component animations/timers.

### `Button` (`src/client/gui/components/Button.h`)
* `Button(int id, int x, int y, int w, int h, const std::string& msg)`
* `virtual bool clicked(Minecraft* mc, int x, int y)` - Hit test checking `x <= mx && mx < x + width`.
* `virtual void render(Minecraft* mc, int xm, int ym)` - Render button background texture and centered label text.

### `TextBox` (`src/client/gui/components/TextBox.h`)
* `TextBox(int id, int x, int y, int w, int h, const std::string& msg)`
* `virtual void mouseClicked(Minecraft* mc, int x, int y, int buttonNum)` - Focus handling.
* `virtual void keyPressed(Minecraft* mc, int key)` / `charPressed(Minecraft* mc, char c)` - Character input.
* `virtual void tick(Minecraft* mc)` - Cursor blink updates.
* **Text Storage:** Public string member `std::string text` (directly accessed for reading/writing text content).

### `OptionsScreen` Sub-menus (`src/client/gui/screens/OptionsScreen.cpp`)
* **Scrolling Grid:** Computes `m_maxScroll` from control rows and view area `height - 68`. Offsets `OptionControl` position `y = topY + 4 + row * 24 - m_scrollY` and clips using OpenGL scissor `glScissor`.
* **Done Button:** Positioned fixed at bottom `btnDone->y = height - 28`, checked first in `mouseClicked`.

## 2. Local Player Skin & Cape Pipeline
1. **File Loading:** `LocalPlayer::setSkinFromFile(path, err)` / `setCapeFromFile(path, err)` (`src/client/player/LocalPlayer.cpp`) validates file size (<100KB) and dimensions (64x32 or 64x64 skin, 64x32 cape) using `loadTextureFromMemory`, then writes PNG to `data/images/skins/<name>.png` or `capes/<name>.png`.
2. **Texture Reloading:** Calls `minecraft->textures->reloadTexture("skins/" + name + ".png")` (`Textures.cpp`), which deletes the old GL texture ID from `idMap` / `loadedImages` and re-allocates a new OpenGL texture ID.
3. **Model Selection:** `PlayerRenderer::setModernSkin` checks `isModernPlayerSkin` (`PlayerRenderer.cpp`), inspecting texture dimensions (64x64) and auto-detecting slim alpha transparency at $x \in [54, 55], y \in [20, 31]$. It picks `playerModel32`, `playerModel64`, or `playerModel64Slim`.
4. **Reaching Renderer from Screen:**
   ```cpp
   EntityRenderDispatcher* erd = EntityRenderDispatcher::getInstance();
   PlayerRenderer* rend = (PlayerRenderer*)erd->getRenderer(minecraft->player);
   ```

## 3. Remote Players' Skins
1. When a remote player joins, `LocalPlayer` / `CThread` spawns `fetchSkinForPlayer` (`LocalPlayer.cpp`).
2. Downloads skin from Mojang session servers via `HttpClient::download` to `data/images/skins/<username>.png`.
3. Calls `player->setTextureName("skins/" + username + ".png")`.
4. During rendering, `PlayerRenderer::render` automatically loads/binds `player->getTexture()` and detects slim/modern skin geometry.

## 4. Refreshing Model State Immediately After Option Changes
To apply skin model changes immediately when "Skin Model" option is toggled in GUI:
```cpp
// 1. Update option setting in Options
minecraft->options.set(OPTIONS_SKIN_MODEL, newSetting);
minecraft->options.save();

// 2. Fetch PlayerRenderer singleton and refresh model selection
if (minecraft->player && EntityRenderDispatcher::getInstance()) {
    PlayerRenderer* rend = (PlayerRenderer*)EntityRenderDispatcher::getInstance()->getRenderer(minecraft->player);
    if (rend) {
        rend->setModernSkin(minecraft->player);
    }
}
```

## 5. Developer Traps & Common Pitfalls
* **Permanent Geometry Destruction:** Do NOT use `vector::resize()` on `ModelPart::cubes` to toggle layers; resizing destroys `Cube` objects permanently. Use `ModelPart::renderCubeRange` to conditionally skip outer cubes instead.
* **CPU Texture Memory Lifetime:** `TextureData::data` pixel buffers in RAM are freed after uploading to OpenGL. Do NOT perform per-frame pixel inspection in render loops on `texData->data`; run skin checks once upon texture loading or reload.
* **`TextBox` Method Signatures:** `TextBox` virtual methods require `Minecraft* mc` as their first argument (e.g. `txt->tick(minecraft)`, `txt->mouseClicked(minecraft, x, y, button)`).
* **Option Controls:** `OptionString` does not auto-cycle on click. Implement string value cycling in `OptionControl::toggleValue` for multi-state string options like `"Auto" -> "Classic" -> "Slim"`.
