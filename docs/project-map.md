# Project Map & Developer Guide for Beginners

Welcome to the ExtraMCPE codebase! This guide is written to help you understand how the entire game works from start to finish—even if you have never written a line of C++ or game engine code before.

---

## 1. How the Game Engine Works (An Analogy for Beginners)

Imagine a Minecraft game engine like a digital Flip-Book:

```
+-----------------------------------------------------------------+
|                        THE GAME ENGINE                          |
|                                                                 |
|   1. INPUTS          2. LOGIC (TICK)         3. RENDER (FRAME)  |
|  +-----------+      +------------------+    +----------------+  |
|  | Keyboard  | ---> | Move player      | -> | Draw 3D Blocks |  |
|  | Mouse     |      | Grow plants      |    | Draw Monsters  |  |
|  | Touch     |      | Update physics   |    | Draw UI/Menus  |  |
|  +-----------+      +------------------+    +----------------+  |
+-----------------------------------------------------------------+
```

1. **Input (Event Loop):** Every millisecond, the computer checks if you pressed keys, moved the mouse, or touched the screen.
2. **Logic (Tick - 20 times per second):** The game recalculates physics (gravity, block collisions), mob movement, plant growth, and health. In Minecraft, this loop is called `tick()`.
3. **Graphics (Render - 60+ times per second):** The game draws 3D blocks, items, lighting, sky, clouds, and text on your monitor screen. In Minecraft, this loop is called `render()`.

---

## 2. Directory Structure Map

Here is where every part of the codebase lives in the project folder:

* `src/client/`: Everything running on the player's computer/device.
  * `Minecraft.cpp`: The "Brain" of the game that connects world rendering, sound, screen menus, and player input together.
  * `Options.cpp`: Stores settings (graphic quality, controls, volume, render distance) and saves them to `options.txt`.
* `src/client/gui/`: User Interfaces and Menus.
  * `Screen.h`: Base class for every menu (Pause menu, Inventory screen, Main menu).
  * `components/`: UI components like `Button`, `TextBox`, and `Slider`.
* `src/client/renderer/`: OpenGL 3D and 2D graphic rendering pipeline.
  * `Tessellator.cpp`: The fast 3D shape builder that draws triangles and rectangles on screen.
  * `Textures.cpp`: Loads PNG image assets (`terrain.png`, `items.png`) into graphics memory.
* `src/client/model/`: 3D Block and Mob models.
  * `HumanoidModel.cpp`: The 3D box model for player characters, Zombies, and Skeletons.
* `src/world/`: Game logic, physics, and world rules.
  * `level/tile/`: All blocks in the game (`Stone`, `Dirt`, `TNT`, `Glass`, `Chest`).
  * `entity/`: Entities moving around the world (`Player`, `Zombie`, `ItemEntity`, `Arrow`).
  * `item/`: Inventory items, tools, weapons, food, and crafting recipes.
* `src/platform/`: Hardware-specific tools for Android, iOS, Windows, Linux, and Web platforms.
* `src/commands/`: Chat text commands (`/give`, `/op`, `/ban`, `/kick`, `/help`).
* `project/`: Build setup files for compilers and platforms (`Android.mk`, Xcode `.pbxproj`, CMake).

---

## 3. "How To Add" Beginner Cookbook

### Example 1: Adding a Button to a Screen & Handling Clicks
When you open a screen (like PauseScreen), buttons are added inside `init()` and click actions are handled inside `buttonClicked()`:

```cpp
// 1. Inside PauseScreen::init():
// Button constructor: Button(ID, xPos, yPos, width, height, "Text Label")
btnOptions = new Button(2, width / 2 - 100, height / 2, 200, 20, "Options");
buttons.push_back(btnOptions); // Register button to screen

// 2. Inside PauseScreen::buttonClicked(Button* button):
if (button == btnOptions) {
    // Open the options screen when clicked
    minecraft->setScreen(new OptionsScreen());
}
```

### Example 2: Adding a Setting Saved to `options.txt`
1. Define your option ID in `src/client/Options.h`:
```cpp
enum OptionId {
    OPTIONS_MY_FEATURE,
    // ...
};
```
2. Create option object in `src/client/Options.cpp`:
```cpp
OptionBool myFeature("myFeature", true); // Default value is true
m_options[OPTIONS_MY_FEATURE] = &myFeature;
```
3. Read setting anywhere in game code:
```cpp
bool isEnabled = minecraft->options.getBooleanValue(OPTIONS_MY_FEATURE);
if (isEnabled) {
    // Feature code here
}
```

### Example 3: Adding a New C++ Source File to Build Systems
Whenever you create a new `.cpp` file (for example `src/commands/CommandMagic.cpp`), you must register it in all target build files:

1. **`CMakeLists.txt`:** Added automatically via file globbing `src/commands/*.cpp`.
2. **`project/android/jni/Android.mk` & `project/android_java/jni/Android.mk`:**
   ```makefile
   ../../../src/commands/CommandMagic.cpp \
   ```
3. **`project/iosproj/minecraftpe.xcodeproj/project.pbxproj`:** Add file reference under `PBXBuildFile`, `PBXFileReference`, and `PBXSourcesBuildPhase`.

### Example 4: How to Add a Game Rule
Game rules are registered globally using `GameRules::registerRule` and read per world from `level->getLevelData()->getGameRules()`:

1. **Register the rule:** (e.g. at startup or initialization)
```cpp
// Register a boolean game rule named "doDaylightCycle" with default value true
GameRules::registerRule("doDaylightCycle", GameRules::TYPE_BOOL, "true");

// Or register an integer game rule named "randomTickSpeed" with default value 3
GameRules::registerRule("randomTickSpeed", GameRules::TYPE_INT, "3");
```

2. **Read the rule value from game code:**
```cpp
if (level != NULL && level->getLevelData() != NULL) {
    bool doDaylight = level->getLevelData()->getGameRules().getBool("doDaylightCycle");
    if (doDaylight) {
        // Advance world time
    }
}
```

---

## 4. Main Game Loops & Frame Lifecycle

Different operating systems start the game engine differently, but all pass control to `Minecraft::update()`:

* **Windows / Linux / Web (GLFW & Emscripten):** `loop()` in `src/main_glfw.h` polls hardware events, updates frame timers, calls `Minecraft::update()`, and swaps display buffers.
* **Android (Native Activity):** `android_main` in `src/main_android.cpp` handles touch/sensor input and executes `app->update()`.
* **iOS (Metal/OpenGL View):** `CADisplayLink` timer in `minecraftpeViewController.mm` calls native `app->update()`.

### The 2 Main Parts of `Minecraft::update()`:
```
                      Minecraft::update()
                               |
            +------------------+------------------+
            |                                     |
    Minecraft::tick()                    Minecraft::render()
   (20 times per second)                (60+ times per second)
            |                                     |
  - Player input physics                - Draw 3D terrain blocks
  - Entity & Mob movement               - Draw player & mob models
  - Block updates & water flow          - Draw weather & clouds
  - Menu button updates                 - Draw 2D GUI HUD overlay
```

---

## 5. C++ Compatibility Guidelines for Android NDK r14b
The Android build uses Android NDK r14b. This compiler version lacks several standard modern C++11 library functions:

* ❌ **DO NOT USE:** `std::to_string()`, `std::stoi()`, `std::round()`, `std::lround()`, `std::trunc()`.
* ✅ **USE INSTEAD:** Standard C functions like `floorf(x + 0.5f)` or string streams:
```cpp
// Standard number-to-string conversion:
std::ostringstream ss;
ss << intValue;
std::string str = ss.str();

// Standard string-to-number conversion:
int intValue = std::atoi(str.c_str());
```
