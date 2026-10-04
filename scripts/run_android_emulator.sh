#!/bin/sh
adb -e install -r android/app/build/outputs/apk/release/app-release-unsigned.apk
adb -e shell am start -n com.example.c_multiplatform/android.app.NativeActivity
