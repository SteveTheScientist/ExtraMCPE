# GUI Screens & Player Architecture Guide

This guide explains how User Interfaces (UI screens, buttons, input text boxes), settings saving, and player 3D models/skins work in the game.

---

## 1. GUI Screen Subsystem (`src/client/gui/Screen.h`)

All game menus (Pause Menu, Options Screen, Inventory, Chat Screen) inherit from the base class `Screen`.

```
+-----------------------------------------------------------------------+
|                         GUI SCREEN HIERARCHY                          |
|                                                                       |
|                              Screen                                   |
|               (Base GUI Screen & Event Dispatcher)                    |
|                                 |                                     |
|         +-----------------------+-----------------------+             |
|         |                       |                       |             |
|    PauseScreen            OptionsScreen           InventoryScreen     |
|  (Resume, Options,       (Graphics, Sound,       (Player Hotbar &     |
|   Quit to Menu)           Controls, FOV)          Item Inventory)     |
|                                                                       |
|  Widgets inside Screens:                                              |
|  - Button: Clickable UI button                                        |
|  - TextBox: Editable text field for chat & names                      |
|  - Slider: Adjustable value bar (Volume, Sensitivity)                 |
+-----------------------------------------------------------------------+
```

### Screen Lifecycle Methods
1. `init()`: Called once when screen is opened. Used to position and create buttons.
2. `render(int mouseX, int mouseY, float partialTicks)`: Called every frame to draw background, title text, and buttons.
3. `tick()`: Called 20 times per second for cursor blinking or text input updates.
4. `buttonClicked(Button* button)`: Triggered automatically whenever player clicks a button on screen.

### Working Code Example: Custom Options Screen
```cpp
#include "client/gui/Screen.h"
#include "client/gui/Button.h"

class CustomOptionsScreen : public Screen {
private:
    Button* btnDone;

public:
    virtual void init() override {
        // Create button at center-bottom of screen: Button(ID, x, y, width, height, "Text")
        btnDone = new Button(1, width / 2 - 100, height - 30, 200, 20, "Done");
        buttons.push_back(btnDone); // Register button to screen list
    }

    virtual void buttonClicked(Button* button) override {
        if (button->id == 1) {
            // Close screen when "Done" is clicked
            minecraft->setScreen(NULL);
        }
    }

    virtual void render(int mouseX, int mouseY, float partialTicks) override {
        renderBackground(); // Draw dark semi-transparent background
        drawCenteredString(font, "Custom Settings", width / 2, 20, 0xFFFFFFFF);
        Screen::render(mouseX, mouseY, partialTicks); // Renders all registered buttons
    }
};
```

---

## 2. Options System (`options.txt`)

Game settings are saved in a simple text file called `options.txt` stored in the user's app data directory.

### File Format Example (`options.txt`)
```ini
username:Steve
music:1.00
sound:0.80
gfx_fancy:true
gfx_clouds:true
sensitivity:0.50
```

### How Options are Loaded & Saved (`src/client/Options.cpp`)
* **Loading (`Options::load()`):** Opens `options.txt` using `std::ifstream`, reads each line, splits by colon (`:`), and updates internal variables.
* **Saving (`Options::save()`):** Iterates over registered options and writes `key:value` lines using `std::ofstream`.

---

## 3. Player Skin & Cape Pipeline

```
+-----------------------------------------------------------------------+
|                     PLAYER SKIN & MODEL PIPELINE                      |
|                                                                       |
|  1. SKIN PNG TEXTURE        2. 3D HUMANOID MODEL     3. RENDERER      |
|  +--------------------+    +--------------------+  +-----------------+ |
|  | Default: char.png  | -> | Head, Torso, Arms, |->| Binds OpenGL    | |
|  | Custom: skin.png   |    | Legs, Outer Jacket |  | Texture & Draws | |
|  +--------------------+    +--------------------+  +-----------------+ |
+-----------------------------------------------------------------------+
```

1. **Texture Loading:** Default player skin is loaded from `data/images/mob/char.png` ($64 \times 32$ or $64 \times 64$ RGBA PNG image).
2. **3D Box Model (`HumanoidModel.cpp`):** Player shape is composed of 3D box parts (`head`, `body`, `rightArm`, `leftArm`, `rightLeg`, `leftLeg`).
3. **Limb Rotation Animations:** Walking animations calculate sine wave limb swings based on movement speed:
   ```cpp
   rightArm.xRot = Mth::cos(time * 0.6662f) * 1.4f * speed;
   leftArm.xRot = -Mth::cos(time * 0.6662f) * 1.4f * speed;
   ```
4. **Capes (`PlayerRenderer::renderCape`):** Capes render as a swinging 3D rectangle behind the player's back that sways outward when running.
