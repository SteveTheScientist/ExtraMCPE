# Android JNI Bridge & AppPlatform Guide

This guide explains how the C++ game engine communicates with platform-specific hardware (Android Java, iOS Objective-C, Desktop GLFW) through the `AppPlatform` virtual abstraction interface and JNI bridge.

---

## 1. How Platform Abstraction Works (`AppPlatform`)

The game engine is written in portable C++ and must run identically across Windows, Linux, Android, iOS, and Web. To avoid placing platform-specific `#ifdef` checks everywhere in game code, all hardware operations pass through `AppPlatform` (`src/AppPlatform.h`).

```
+-------------------------------------------------------------------------+
|                      PLATFORM ABSTRACTION ARCHITECTURE                  |
|                                                                         |
|                          GAME ENGINE (C++)                              |
|                   Calls AppPlatform Virtual Methods                     |
|                                  |                                      |
|                  +---------------+---------------+                      |
|                  | AppPlatform Virtual Interface |                      |
|                  +---------------+---------------+                      |
|                                  |                                      |
|        +-------------------------+-------------------------+            |
|        |                         |                         |            |
| AppPlatform_android    AppPlatform_glfw           AppPlatform_iOS       |
| (Uses JNI to call      (Uses GLFW/C standard     (Uses Objective-C      |
|  Android Java APIs)     libraries)                UIPasteboard/UIKit)   |
+-------------------------------------------------------------------------+
```

---

## 2. Table of `AppPlatform` Methods & Platform Implementations

| Virtual Function (`src/AppPlatform.h`) | Description | Android | GLFW (Desktop) | iOS |
| :--- | :--- | :---: | :---: | :---: |
| `loadTexture(path, location)` | Loads PNG file to RGBA pixel array | JNI (`getImageData`) | `libpng` | UIImage |
| `showKeyboard()` / `hideKeyboard()` | Toggles virtual soft keyboard | JNI (`InputMethodManager`) | GLFW events | UIKit |
| `vibrate(int ms)` | Triggers device haptic feedback | JNI (`Vibrator`) | Stub | AudioServices |
| `getScreenWidth()` / `getScreenHeight()` | Returns display screen dimensions | JNI (`DisplayMetrics`) | GLFW window | UIScreen |
| `openURL(url)` | Opens web link in browser | JNI (`Intent.ACTION_VIEW`) | System shell | UIApplication |

---

## 3. Step-by-Step JNI Control Flows (Android)

### a. Loading a Texture Asset (`getImageData`)
```
[C++ Engine] Textures::loadTexture("mob/char.png")
     |
     v
[C++ AppPlatform_android] env->CallObjectMethod(..., _getImageData, "images/mob/char.png")
     |
     v
[Java MainActivity.java] getImageData() opens Asset, decodes Bitmap, extracts int[] pixels
     |
     v
[C++ AppPlatform_android] Receives RGBA int[], uploads pixels to OpenGL texture (glBindTexture)
```

### b. Soft Keyboard Input Handling
```
[C++ UI] Player taps TextBox widget -> calls platform()->showKeyboard()
     |
     v
[C++ AppPlatform_android] Calls JNI method _methodShowSoftInput on Android InputMethodManager
     |
     v
[Android OS] Soft keyboard pops up on screen
     |
     v
[Java GameInputConnection] Player types letters -> calls nativeCommitText() via JNI
     |
     v
[C++ Minecraft] handleCommitText() inserts typed text into active TextBox widget
```

---

## 4. Step-by-Step Guide: Adding a New JNI Bridge Method (e.g. `vibrate`)

If you want to add a platform feature (like triggering device vibration):

### Step 1: Add Java Method (`project/android/src/com/mojang/minecraftpe/MainActivity.java`)
```java
public void triggerVibration(int milliseconds) {
    Vibrator v = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
    if (v != null && v.hasVibrator()) {
        v.vibrate(milliseconds);
    }
}
```

### Step 2: Look Up JNI Method ID (`src/AppPlatform_android.h`)
In `AppPlatform_android::init()`, look up the Java method signature:
```cpp
_methodVibrate = env->GetMethodID(mainActivityClass, "triggerVibration", "(I)V");
```

### Step 3: Implement C++ Override (`src/AppPlatform_android.h`)
```cpp
virtual void vibrate(int milliseconds) override {
    JNIEnv* env = getJNIEnv();
    env->CallVoidMethod(mainActivityInstance, _methodVibrate, milliseconds);
}
```
