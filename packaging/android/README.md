# Android Packaging

1. Copy `keystore.properties.template` to `keystore.properties` and fill in your release signing values.
2. Ensure the Android SDK/NDK and Gradle are installed.
3. Run the packaging target:

```
cmake -S . -B build-android-package -DM3_ENABLE_PACKAGING=ON
cmake --build build-android-package --target m3_package_android
```

Release APK and AAB outputs land in `packaging/android/app/build/outputs`.
