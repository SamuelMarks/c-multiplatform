#!/bin/sh
adb -d install -r android/app/build/outputs/apk/release/app-release-unsigned.apk
adb logcat -s "UI_ENGINE"
