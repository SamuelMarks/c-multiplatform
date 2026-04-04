# Material Catalog (C-Multiplatform Port)

| <br><img src="readme/material_catalog.png" alt="Material Catalog" width="240"></img>  <br><br>A catalog of Material Design components and features available in Jetpack Compose. See how to implement them and how they look and behave on real devices.<br><br>• Lives in AOSP—always up to date<br>• Uses the same samples as API reference docs<br>• Theme picker to change Material Theming values at runtime<br>• Links to guidelines, docs, source code, and issue tracker<br><br><a href="https://play.google.com/store/apps/details?id=androidx.compose.material.catalog"><img src="https://play.google.com/intl/en_us/badges/static/images/badges/en_badge_web_generic.png" height="70"></a><br>**[> Browse on AOSP](https://cs.android.com/androidx/platform/frameworks/support/+/androidx-main:compose/integration-tests/material-catalog)** <br><br>  | <img src="readme/screenshot_catalog.png" width="320" alt="Material Catalog sample demo">|

## Overview

This is a C89 port of the Android Jetpack Compose Material Catalog application. It uses the [`c-multiplatform`](https://github.com/SamuelMarks/c-multiplatform) (CMP) framework to demonstrate Material Design 3 components rendered natively across various operating systems using strict ISO C90.

## Building

This project is highly flexible. It is designed to be built either as an integrated component of the `c-multiplatform` repository or as a completely **standalone application template** detached from the core repo. 

It heavily utilizes CMake's `FetchContent` to automatically locate dependencies locally or dynamically download them from GitHub (including the [`c-multiplatform`](https://github.com/SamuelMarks/c-multiplatform), [`c-fs`](https://github.com/SamuelMarks/c-fs), [`c-abstract-http`](https://github.com/SamuelMarks/c-abstract-http), [`c-orm`](https://github.com/SamuelMarks/c-orm), and [`cdd-c`](https://github.com/SamuelMarks/cdd-c) repositories) if they are missing.

### Standalone Build

To use this as a starting point for your own CMP app, simply copy this directory to any place on your machine:

```bash
cp -r examples/google/material-catalog /tmp/catalog
# Navigate to the copied material-catalog directory
cd /tmp/catalog

# Generate the standalone project
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build the Material Catalog
cmake --build build --config Debug --target material_catalog --parallel
```

### Integrated Build

If you are building from the root of the `c-multiplatform` repository:

```bash
# Navigate to the root of c-multiplatform
cd c-multiplatform

# Configure the project with examples enabled
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMP_BUILD_EXAMPLES=ON

# Build the Material Catalog example via the main build tree
cmake --build build --config Debug --target material_catalog --parallel
```

## Running

Once built, you can execute the target. Note that the output binary location shifts depending on how you compiled it.

```bash
# If built standalone:
./build/Debug/material_catalog.exe

# If built integrated within the c-multiplatform root:
./build/bin/Debug/material_catalog.exe
```

## Testing

The project ensures 100% test coverage using the [`greatest`](https://github.com/silentbicycle/greatest) testing framework. The tests cleanly mock the windowing system and perform UI logic verification to prevent regressions.

```bash
# Run tests through CTest
cd build
ctest -C Debug -R test_material_catalog --output-on-failure
```

Alternatively, run the test executable directly:
```bash
# Standalone path
./Debug/test_material_catalog.exe

# Integrated path
./bin/Debug/test_material_catalog.exe
```

## Regenerating Screenshots

This example comes with an automated screenshot generation pipeline. It builds an offline renderer (`screenshot_generator`) that captures the UI tree's layout state and dumps it into an uncompressed `.bmp` file. A script then converts it to `.png` and organizes it into the `readme/` folder.

To regenerate the screenshots:

**On Windows (PowerShell):**
```powershell
cd examples/google/material-catalog
powershell -ExecutionPolicy Bypass -File .\generate_screenshots.ps1
```

**On macOS / Linux (POSIX Shell):**
```bash
cd examples/google/material-catalog
sh ./generate_screenshots.sh
```

This script will automatically:
1. Compile the `screenshot_generator` target via CMake.
2. Execute the generator to output the layout buffer.
3. Convert the `.bmp` file to `.png` and organize it without manual intervention.
