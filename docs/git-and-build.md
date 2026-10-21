# Git, Build Systems & CI/CD Guide

## 1. Multi-Platform Build Systems Overview

| Platform | Build System / Project File | Source Manifest / Key File | Output Binary Target |
| :--- | :--- | :--- | :--- |
| **Desktop (Linux / macOS / Win32 GL)** | CMake (`CMakeLists.txt`) | `CMakeLists.txt` (`add_executable`) | `build/MinecraftPE-server` |
| **Android (Native JNI)** | Android NDK (`ndk-build`) | `project/android/jni/Android.mk` | `libminecraftpe.so` |
| **Android (Java App)** | Android Makefile / Gradle | `project/android_java/jni/Android.mk` | `MinecraftPE.apk` |
| **Raspberry Pi** | GNU Make (`Makefile`) | `project/raspberry/Makefile` | `minecraftpe` |
| **Windows Visual Studio** | MSBuild (`.vcxproj`) | `project/win32/MinecraftWin32.vcxproj` | `MinecraftWin32.exe` |
| **Apple iOS** | Xcode (`.xcodeproj`) | `project/iosproj/minecraftpe.xcodeproj` | `MinecraftPE.app` |

## 2. CMake Targets & Build Instructions

### Basic CMake Build Flow
```bash
# Configure build directory
cmake -B build -S . -DGLFW_BUILD_WAYLAND=OFF -DGLFW_BUILD_X11=OFF

# Compile primary game target
cmake --build build --target MinecraftPE-server -j$(nproc)
```

### Available CMake Unit Test Targets
* `test_fog`
* `test_clouds`
* `test_font`

Run test targets via:
```bash
cmake --build build --target test_font
./build/test_font
```

## 3. GitHub Actions CI/CD Pipeline (`.github/workflows/build.yml`)

### Active Matrix Jobs
1. **Linux Build (`ubuntu-latest`):** Configures CMake with Headless/GLFW options and compiles `MinecraftPE-server`.
2. **Android NDK Build (`ubuntu-latest`):** Downloads NDK r14b, runs `ndk-build` targeting `arm64-v8a` and `armeabi-v7a` (`minSdkVersion 21`, `targetSdkVersion 28`).
3. **macOS / iOS Build (`macos-13`):** Note: Uses `macos-13` (x86_64) runner explicitly to avoid arm64 runner capacity timeouts. Runs Xcode build for iOS simulator/device targets.
4. **Windows Build (`windows-latest`):** Builds MSBuild `.vcxproj` solution via MSBuild CLI.

## 4. Log Reading, Crash Diagnostics & Debugging

### Common Build Failures & Fixes

#### A. Headless Linux / Wayland Missing Headers
* **Symptom:** `EGL/egl.h` or Wayland display server header missing errors during CMake configuration.
* **Fix:** Pass `-DGLFW_BUILD_WAYLAND=OFF -DGLFW_BUILD_X11=OFF` flags to `cmake`.

#### B. Android NDK r14b C++11 Header Missing Functions
* **Symptom:** `std::to_string`, `std::round`, `std::lround`, or `std::trunc` undeclared in Android NDK builds.
* **Fix:** Do not call these C++11 `<cmath>`/`<string>` methods directly on Android NDK r14b. Replace `std::to_string(x)` with `std::ostringstream` and replace `std::round(f)` with `floorf(f + 0.5f)`.

#### C. Linker Error: Undefined Reference When Adding New Source Files
* **Symptom:** Unresolved external symbol during linking on Android, iOS, or Windows builds.
* **Fix:** When creating any new `.cpp` or `.h` file in `src/`, you **must** update ALL platform manifest files:
  1. `CMakeLists.txt`
  2. `project/android/jni/Android.mk`
  3. `project/android_java/jni/Android.mk`
  4. `project/raspberry/Makefile`
  5. `project/win32/MinecraftWin32.vcxproj`
  6. `project/win32_gl/MinecraftWin32_GL.vcxproj`
  7. `project/iosproj/minecraftpe.xcodeproj/project.pbxproj`

## 5. Pre-Commit Checklist & Pull Request Guidelines

1. **Verify No Code Modification:** Confirm `git status` shows edits restricted strictly to `docs/` files.
2. **Check Line Counts:** Ensure all documentation files in `docs/` remain under 300 lines each.
3. **Validate Code Symbol Accuracy:** Double check all file paths, class names, and method signatures match exact codebase definitions.
