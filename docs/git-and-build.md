# Git, Build Systems & CI/CD Guide

This guide explains how to build the game across all supported platforms (Linux, Windows, macOS, Android, iOS, and Web) and troubleshoot common compilation issues.

---

## 1. Multi-Platform Build Systems Overview

```
+--------------------------------------------------------------------------+
|                       MULTI-PLATFORM BUILD PIPELINE                      |
|                                                                          |
|   Target Platform        Build System / Project File     Output Binary   |
|  +---------------------+-------------------------------+---------------+ |
|  | Linux / macOS       | CMake (CMakeLists.txt)        | MinecraftPE   | |
|  | Windows             | CMake / MSBuild (.vcxproj)    | MinecraftPE   | |
|  | Android             | Android NDK (Android.mk)      | libmcpe.so/apk| |
|  | iOS                 | Xcode (.xcodeproj)            | minecraft.ipa | |
|  | Web (WebAssembly)   | Emscripten + CMake (Ninja)    | MinecraftPE.js| |
+--------------------------------------------------------------------------+
```

| Platform | Build System | Primary Command / File |
| :--- | :--- | :--- |
| **Linux / Desktop** | CMake | `cmake -B build -S .` |
| **Android (NDK r14b)** | Android NDK (`build.sh`) | `project/android/jni/Android.mk` |
| **iOS** | Xcode | `xcodebuild -scheme minecraftpe` |
| **Web** | Emscripten | `emscripten` toolchain file |

---

## 2. Desktop (Linux / Windows) CMake Build Instructions

### Basic Build Commands
```bash
# 1. Configure build directory (disabling Wayland/X11 headers for headless environments)
cmake -B build -S . -DGLFW_BUILD_WAYLAND=OFF -DGLFW_BUILD_X11=OFF

# 2. Compile server target
cmake --build build --target MinecraftPE-server -j$(nproc)
```

### Running Unit Tests
Unit test targets are compiled using CMake and executed from `./build/`:
```bash
# Compile and run font, fog, and cloud unit tests
cmake --build build --target test_clouds test_fog test_font
./build/test_clouds
./build/test_fog
./build/test_font
```

---

## 3. GitHub Actions CI/CD Pipeline (`.github/workflows/build.yml`)

The GitHub Actions CI/CD pipeline builds artifacts for all platforms automatically on every push:
1. **Linux Build (`ubuntu-latest`):** Builds Linux binary and server executable.
2. **Windows Build (`ubuntu-22.04`):** Cross-compiles Windows binaries using `llvm-mingw`.
3. **iOS Build (`macos-13`):** Uses x86_64 macOS 13 runner (`macos-13`) to avoid runner acquisition queue timeouts on arm64 runners. Builds signed/unsigned IPA packages.
4. **Android Build (`ubuntu-latest`):** Downloads NDK r14b and builds APKs for `arm64-v8a` and `armeabi-v7a`.
5. **Web Build (`ubuntu-latest`):** Uses Emscripten SDK to build WebAssembly `.js` and `.wasm` files.

---

## 4. Troubleshooting Common Build Failures

### Issue A: Android NDK r14b C++11 Function Errors
* **Error Message:** `error: 'to_string' is not a member of 'std'` or `error: 'round' is not a member of 'std'`.
* **Cause:** Android NDK r14b uses an older GCC/Clang stdlib configuration lacking C++11 `<string>`/`<cmath>` helper methods.
* **Fix:** Replace `std::to_string(n)` with `std::ostringstream ss; ss << n; return ss.str();`. Replace `std::round(f)` with `floorf(f + 0.5f)`.

### Issue B: Undefined Reference / Missing Symbol Linker Errors
* **Error Message:** `undefined reference to CommandGive::CommandGive()`.
* **Cause:** A new `.cpp` file was created in `src/` but was not added to platform build files.
* **Fix:** When adding any new `.cpp` file, add its path to:
  1. `CMakeLists.txt`
  2. `project/android/jni/Android.mk`
  3. `project/android_java/jni/Android.mk`
  4. `project/iosproj/minecraftpe.xcodeproj/project.pbxproj`
