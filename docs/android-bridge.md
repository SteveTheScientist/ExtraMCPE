# Android JNI Bridge & AppPlatform Guide

## 1. AppPlatform Virtual Functions & Overrides (`src/AppPlatform.h`)
| Virtual Function (`src/AppPlatform.h`) | Android (`AppPlatform_android.h`) | GLFW (`AppPlatform_glfw.h`) | Win32 (`AppPlatform_win32.h`) | iOS (`AppPlatform_iOS.mm`) |
| :--- | :---: | :---: | :---: | :---: |
| `virtual void saveScreenshot(const std::string&, int, int)` | Yes | Yes (stub) | Yes (stub) | No |
| `virtual TextureData loadTexture(const std::string&, bool)` | Yes | Yes | Yes | Yes |
| `virtual TextureData loadTextureFromMemory(const unsigned char*, size_t)` | Yes | Yes | Yes | Yes |
| `virtual void playSound(const std::string&, float, float)` | Yes | No | No | No |
| `virtual void hideCursor(bool)` | No | Yes | No | No |
| `virtual void showDialog(int)` | Yes | No | No | Yes |
| `virtual void createUserInput()` | Yes | No | No | No |
| `virtual int getUserInputStatus()` | Yes | No | No | Yes |
| `virtual StringVector getUserInput()` | Yes | No | No | Yes |
| `virtual std::string getDateString(int)` | Yes | Yes | Yes | Yes |
| `virtual void uploadPlatformDependentData(int, void*)` | Yes | No | No | No |
| `virtual BinaryBlob readAssetFile(const std::string&)` | Yes | Yes | Yes | Yes |
| `virtual int getScreenWidth()` | Yes | Yes | Yes | Yes |
| `virtual int getScreenHeight()` | Yes | Yes | Yes | Yes |
| `virtual float getPixelsPerMillimeter()` | Yes | Yes | Yes | Yes |
| `virtual bool isNetworkEnabled(bool)` | Yes | No | No | Yes |
| `virtual bool isPowerVR()` | Yes | No | No | Yes |
| `virtual int getKeyFromKeyCode(int, int, int)` | Yes | No | No | No |
| `virtual void openURL(const std::string&)` | Yes | Yes | Yes | No |
| `virtual void importSkinOrCape(int)` | Yes | No | No | Yes (stub) |
| `virtual void finish()` | Yes | No | No | No |
| `virtual bool supportsTouchscreen()` | Yes | Yes | Yes | Yes |
| `virtual void vibrate(int)` | Yes | No | No | Yes (stub) |
| `virtual std::string getPlatformStringVar(int)` | Yes | No | No | No |
| `virtual void showKeyboard()` / `hideKeyboard()` | Yes | Yes | No | Yes |

## 2. JNI Function Call Catalog

### Native (C++) -> Java Method Calls
Looked up in `AppPlatform_android::init` (`src/AppPlatform_android.h`) on `com/mojang/minecraftpe/MainActivity`:
* `getImageData`: `"(Ljava/lang/String;)[I"` (`_getImageData`)
* `getImageDataFromBytes`: `"([B)[I"` (`_getImageDataFromBytes`)
* `getFileDataBytes`: `"(Ljava/lang/String;)[B"` (`_readAssetFile`)
* `saveScreenshot`: `"(Ljava/lang/String;II[I)V"` (`_methodSaveScreenshot`, static)
* `displayDialog`: `"(I)V"` (`_showDialog`)
* `tick`: `"()V"` (`_methodTick`)
* `quit`: `"()V"` (`_methodFinish`)
* `initiateUserInput`: `"(I)V"` (`_methodUserInputInitiate`)
* `getUserInputStatus`: `"()I"` (`_methodUserInputStatus`)
* `getUserInputString`: `"()[Ljava/lang/String;"` (`_methodUserInputString`)
* `getDateString`: `"(I)Ljava/lang/String;"` (`_methodGetDateString`)
* `vibrate`: `"(I)V"` (`_methodVibrate`)
* `supportsTouchscreen`: `"()Z"` (`_methodSupportsTouchscreen`)
* `setIsPowerVR`: `"(Z)V"` (`_methodSetIsPowerVR`)
* `isNetworkEnabled`: `"(Z)Z"` (`_methodIsNetworkEnabled`)
* `getPixelsPerMillimeter`: `"()F"` (`_methodGetPixelsPerMillimeter`)
* `getPlatformStringVar`: `"(I)Ljava/lang/String;"` (`_methodGetPlatformStringVar`)
* `openURL`: `"(Ljava/lang/String;)V"` (`_methodOpenURL`)
* `importSkinOrCape`: `"(I)V"` (`_methodImportSkinOrCape`)
* `showSoftInput`: `"(Landroid/view/View;I)Z"` (`_methodShowSoftInput` on `InputMethodManager`)
* `hideSoftInputFromWindow`: `"(Landroid/os/IBinder;I)Z"` (`_methodHideSoftInput` on `InputMethodManager`)

### Java -> Native (C++) Native Method Calls
Declared in `project/android/src/com/mojang/minecraftpe/MainActivity.java` and defined in `src/main_android.cpp`:
* `native void nativeRegisterThis()` -> `Java_com_mojang_minecraftpe_MainActivity_nativeRegisterThis`
* `native void nativeUnregisterThis()` -> `Java_com_mojang_minecraftpe_MainActivity_nativeUnregisterThis`
* `native void nativeStopThis()` -> `Java_com_mojang_minecraftpe_MainActivity_nativeStopThis`
* `native void nativeSetSkinResult(String, boolean, boolean, String)` -> `Java_com_mojang_minecraftpe_MainActivity_nativeSetSkinResult`

## 3. Step-by-Step Control Flows

### a. On-Screen Keyboard Show & Hide
1. `TextBox::setFocus` (`TextBox.cpp`) calls `minecraft->platform()->showKeyboard()`.
2. `AppPlatform_android::showKeyboard` (`AppPlatform_android.h`) uses `InputMethodManager` via JNI:
   ```cpp
   env->CallBooleanMethod(lInputMethodManager, _methodShowSoftInput, lDecorView, 0);
   ```
3. To hide, `TextBox::loseFocus` calls `hideKeyboard()`, running `_methodHideSoftInput` with `lBinder`.

### b. Texture Loading (`getImageData`)
1. Render code calls `Textures::loadTexture("mob/char.png")` (`Textures.cpp`).
2. `AppPlatform_android::loadTexture` calls Java `MainActivity.getImageData("images/mob/char.png")` via JNI:
   ```cpp
   jintArray arr = (jintArray)env->CallObjectMethod(instance, _getImageData, pathStr);
   ```
3. `MainActivity.getImageData` (`MainActivity.java`) opens the asset `InputStream`, decodes `Bitmap`, reads `getPixels`, and returns an int array `[width, height, pixel_0, ...]`.
4. `AppPlatform_android` converts BGR to RGB and returns 32-bit `TextureData`.

### c. Opening a Web URL
1. UI code calls `minecraft->platform()->openURL("https://...")`.
2. `AppPlatform_android::openURL` converts string to `jstring` and calls Java `MainActivity.openURL`.
3. `MainActivity.openURL` creates `Intent.ACTION_VIEW` with `Uri.parse(url)` and calls `startActivity(intent)`.

## 4. Android Writable Paths & Texture Loader
* **Writable Files Dir:** `setupExternalPath` in `src/main_android.cpp` calls Java `getExternalFilesDir(null)` (e.g. `/sdcard/Android/data/com.mojang.minecraftpe/files`).
* **Skins & Capes Paths:** Writable custom textures are stored under `data/images/skins/<name>.png` and `data/images/capes/<name>.png`.
* **Loader Disk Check:** `AppPlatform_android::loadTexture` first checks `std::ifstream("data/images/" + filename)` on disk. If found, it decodes the raw PNG bytes using `loadTextureFromMemory` / `getImageDataFromBytes`. If missing, it falls back to Java asset stream reading via `getImageData`.

## 5. Main Java Classes & Adding New Features
* `MainActivity.java`: Main `NativeActivity` entry point initializing JNI bindings, dialogs, asset loading, and system intents.
* `MainMenuOptionsActivity.java`: Preferences Activity managing native option string mappings.

### Adding a New Java Feature (e.g. File Picker)
1. **Declare Java Method in `MainActivity.java`:**
   ```java
   public void importSkinOrCape(final int type) {
       runOnUiThread(new Runnable() { public void run() {
           Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
           intent.setType("image/png");
           startActivityForResult(intent, REQUEST_CODE);
       }});
   }
   ```
2. **Handle `onActivityResult` in `MainActivity.java` & call Native JNI:**
   ```java
   Uri uri = data.getData();
   // Read bytes, write to app files dir...
   nativeSetSkinResult(targetFile.getAbsolutePath(), isSkin, true, "");
   ```
3. **Declare & Implement JNI Callback in `src/main_android.cpp`:**
   ```cpp
   JNIEXPORT void JNICALL Java_com_mojang_minecraftpe_MainActivity_nativeSetSkinResult(...)
   ```
4. **Call from C++ Platform Interface:** Look up `importSkinOrCape` JNI ID in `AppPlatform_android::init` and call `env->CallVoidMethod(instance, _methodImportSkinOrCape, type)`.
