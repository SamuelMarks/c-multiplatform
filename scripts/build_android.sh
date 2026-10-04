#!/bin/sh
set -e

# Usage instructions for generating a Keystore (if not exists):
# keytool -genkey -v -keystore release.keystore -alias androidreleasekey -keyalg RSA -keysize 2048 -validity 10000

echo "Building Android App Bundle and APK..."
cd android

# Generate release AAB and APK
./gradlew assembleRelease
./gradlew bundleRelease

echo "To sign the APK:"
echo "apksigner sign --ks release.keystore --out app-release-signed.apk app/build/outputs/apk/release/app-release-unsigned.apk"
echo "Build complete."
