# Android Platform Integration Guide

This document explains the process of integrating, building, and deploying the C-Multiplatform application natively on Android.

## Architecture
The project targets Android purely via NativeActivity and the NDK.
It circumvents the JVM for rendering, relying entirely on the native C codebase, utilizing OpenGL ES via EGL for accelerated graphics.

### Event Loop
The main event loop is handled by android_main which bridges native AInputEvent elements.

### Resource Loading
The engine conditionally leverages AAssetManager when __ANDROID__ is defined to intercept and decode files.

## Build Requirements
1. Android Studio / SDK.
2. CMake Version 3.22.1 or newer.
3. Gradle.

## Building and Packaging
Run ./scripts/build_android.sh
