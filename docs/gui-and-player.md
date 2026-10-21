# GUI Screens & Player Architecture Guide

## 1. GUI Screen Subsystem (`src/client/gui/Screen.h`)

### Base Classes & Lifecycle
* **`Screen` (`src/client/gui/Screen.h`):** Base GUI screen class. Handles input events, widget management, and rendering.
  * `init()`: Called when screen opens to initialize buttons and text boxes.
  * `render(int mouseX, int mouseY, float partialTicks)`: Renders screen background, widgets, and labels.
  * `tick()`: Ticked every frame (or 20 TPS) for animations and input updates.
  * `buttonClicked(Button* button)`: Virtual callback when a button is clicked.
  * `keyPressed(int key)`: Called when keyboard key is pressed.
* **`Button` (`src/client/gui/Button.h`):** Interactive UI button with `id`, `x`, `y`, `width`, `height`, and `msg` string.
* **`TextBox` (`src/client/gui/TextBox.h`):** Single-line or multi-line text input field supporting focus, cursor, selection, and clipboard operations.

### Example: Adding a Custom GUI Sub-Menu Screen
```cpp
#include "client/gui/Screen.h"
#include "client/gui/Button.h"

class CustomOptionsScreen : public Screen {
private:
    Button* backButton;
public:
    virtual void init() override {
        this->buttons.push_back(this->backButton = new Button(1, this->width / 2 - 100, this->height - 30, "Done"));
    }
    virtual void buttonClicked(Button* button) override {
        if (button->id == 1) {
            this->minecraft->setScreen(nullptr);
        }
    }
    virtual void render(int mouseX, int mouseY, float partialTicks) override {
        this->renderBackground();
        this->drawCenteredString(this->font, "Custom Options", this->width / 2, 20, 0xFFFFFFFF);
        Screen::render(mouseX, mouseY, partialTicks);
    }
};
```

## 2. Options Persistence System (`src/option/Options.h`)

### File Format & Storage Path
* **File Name/Path:** `options.txt` stored in app data directory (e.g. `games/com.mojang/minecraftpe/options.txt` or platform storage).
* **Format:** Key-value plain text file (`key:value`).

### Code Integration
* **`Options` (`src/option/Options.h`):** Holds option settings (`music`, `sound`, `sensitivity`, `username`, `skin`).
* **Loading:** `Options::load()` reads `options.txt` line by line using `std::ifstream` and populates option fields.
* **Saving:** `Options::save()` writes all options back to `options.txt` using `std::ofstream`.
* **Adding Option:**
  1. Add member variable to `Options.h` (e.g. `bool customSetting`).
  2. In `Options::load()`, parse line `if (key == "mp_customSetting") customSetting = (val == "true");`.
  3. In `Options::save()`, write `file << "mp_customSetting:" << (customSetting ? "true" : "false") << "\n";`.

## 3. Player Skin & Cape Pipeline

### Skin Loading & Binding Pipeline
1. Default texture path: `"mob/char.png"` loaded by `Textures::loadTexture("mob/char.png")`.
2. Size & Format: Standard 64x32 or 64x64 32-bit RGBA PNG image.
3. Rendering Binding: `PlayerRenderer` (`src/client/renderer/entity/PlayerRenderer.cpp`) calls `bindTexture(player->texture)` before rendering `HumanoidModel`.

### Cape Loading & Binding Pipeline
1. Cape path stored on player entity (e.g. `player->cape`).
2. If custom cape texture is bound, `PlayerRenderer::renderCape` renders cape model plane behind player torso with dynamic movement swaying based on player velocity vector.

### Remote Player Skin Loading
* Skin downloaded via HTTP/HTTPS or JNI bridge (`importSkinOrCape`).
* Decoded to raw 32-bit RGBA pixel array in memory.
* Dynamic texture created via `Textures::loadTexture` with generated dynamic name or `AppPlatform::loadTextureFromMemory`.

### Dynamic Player Model Refresh
When a player changes their skin or model layout:
1. `player->texture = newSkinPath;`
2. `player->refreshSkin();` (or `Textures::reload()`) clears GL texture ID and re-binds new pixel data.
3. `PlayerRenderer` dynamically recalculates model UV coordinates or switches `HumanoidModel` dimensions if slim model is selected.
