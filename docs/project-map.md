# Project Map & Developer Guide

## 1. Directory Structure Map
* `src/client/`: Core game loop (`Minecraft.cpp`), settings (`Options.cpp`), and player input handling.
* `src/client/gui/`: UI framework (`Screen.h`, `GuiElement.h`) and screens (`src/client/gui/screens/`).
* `src/client/renderer/`: OpenGL rendering pipeline, textures (`Textures.cpp`), and entity renderers.
* `src/client/model/`: 3D box models (`HumanoidModel.cpp`, `ModelPart.cpp`).
* `src/world/`: Blocks (`Tile.cpp`), entities (`Player.cpp`, `Mob.cpp`), items, and chunk/level simulation.
* `src/platform/`: Operating system abstractions (logging, HTTP, PNG loading, threads).
* `src/commands/`: Chat console commands (`CommandManager.cpp`).
* `project/`: Native platform wrapper projects (`project/android/`, `project/iosproj/`, `project/raspberry/`, `project/win32/`).

## 2. "How To Add" Cookbook

### a. Add a Button to an Existing Screen & Handle Click
In `init()` of a screen (e.g., `src/client/gui/screens/PauseScreen.cpp`):
```cpp
btnOptions = new Button(2, 0, 0, 200, 20, "Options");
buttons.push_back(btnOptions);
```
In `buttonClicked(Button* button)`:
```cpp
if (button == btnOptions) {
    minecraft->setScreen(new OptionsScreen());
}
```

### b. Add a New Option Saved to `options.txt`
Declare option in `src/client/Options.h` and `src/client/Options.cpp`:
```cpp
// Options.h enum OptionId:
OPTIONS_CUSTOM_FLAG,
// Options.cpp:
OptionBool customFlag("customFlag", true);
m_options[OPTIONS_CUSTOM_FLAG] = &customFlag;
```
Read & use option:
```cpp
bool enabled = minecraft->options.getBooleanValue(OPTIONS_CUSTOM_FLAG);
```
Show as toggle in screen controls:
```cpp
m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_CUSTOM_FLAG, "Custom Option"));
```

### c. Create a New Sub-Menu Screen & Open It
Subclass `Screen` (e.g. `src/client/gui/screens/CloudsScreen.h` / `.cpp`):
```cpp
class CloudsScreen : public Screen {
public:
    virtual void init() override {
        btnDone = new Button(100, (width - 200) / 2, height - 28, 200, 20, "Done");
        buttons.push_back(btnDone);
    }
    virtual void buttonClicked(Button* button) override {
        if (button == btnDone) minecraft->setScreen(new CustomizationsScreen());
    }
};
```
Open screen from another screen:
```cpp
minecraft->setScreen(new CloudsScreen());
```

### d. Add a New `.cpp` File to Build Systems
Add your file (e.g., `src/client/gui/screens/NewScreen.cpp`) to:
1. `CMakeLists.txt` (or auto-included via glob/target sources).
2. `project/android/jni/Android.mk` & `project/android_java/jni/Android.mk`:
   ```makefile
   ../../../src/client/gui/screens/NewScreen.cpp \
   ```
3. `project/iosproj/minecraftpe.xcodeproj/project.pbxproj` (under `PBXBuildFile`, `PBXFileReference`, and `PBXSourcesBuildPhase`).

## 3. Main Game Loops & Frame Lifecycle
* **Desktop (GLFW):** `loop()` in `src/main_glfw.h` polls events, calls `g_app->update()`, and calls `glfwSwapBuffers()`.
* **Android:** `android_main` in `src/main_android.cpp` polls ALooper events and executes `app->update()`.
* **iOS:** `CADisplayLink` timer in `minecraftpeViewController.mm` calls native `app->update()`.
* **Web (Emscripten):** `emscripten_set_main_loop` in `src/main_glfw.h` invokes `loop()` continuously.

**Per-Frame Lifecycle:**
* **Update (`tick`):** Called via `Minecraft::tick()` -> updates level entities, player input, world tiles, and GUI screen timers.
* **Drawing (`render`):** Called via `Minecraft::render()` -> renders 3D world scene (`LevelRenderer`), entity models, particle engine, and 2D GUI overlay (`Screen::render`).

## 4. Android NDK r14b Compatibility Limits
The Android build uses an old NDK toolchain lacking C++11 `<cmath>` / `<string>` standard library helpers:
* **Avoid:** `std::to_string`, `std::stoi`, `std::round`, `std::lround`, `std::trunc`.
* **Use:** C math functions (e.g., `floorf(x + 0.5f)`) or `std::ostringstream` for string conversions:
```cpp
std::ostringstream ss;
ss << intValue;
std::string str = ss.str();
```
