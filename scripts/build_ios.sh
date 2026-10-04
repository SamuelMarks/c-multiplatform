#!/usr/bin/env bash
set -e

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <build_dir> <export_dir>"
    exit 1
fi

BUILD_DIR="$1"
EXPORT_DIR="$2"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory '$BUILD_DIR' not found."
    exit 1
fi

mkdir -p "$EXPORT_DIR"

cd "$BUILD_DIR"

# Ensure we have an exportOptions.plist, or create a default one
if [ ! -f "exportOptions.plist" ]; then
    cat <<EOF > exportOptions.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>method</key>
    <string>development</string>
    <key>compileBitcode</key>
    <false/>
</dict>
</plist>
EOF
fi

echo "Archiving..."
xcodebuild archive \
    -project ui_engine.xcodeproj \
    -scheme ui_engine \
    -configuration Release \
    -archivePath "build.xcarchive" \
    CODE_SIGN_IDENTITY="" \
    CODE_SIGNING_REQUIRED=NO \
    CODE_SIGNING_ALLOWED=NO

echo "Exporting Archive..."
xcodebuild -exportArchive \
    -archivePath "build.xcarchive" \
    -exportPath "$EXPORT_DIR" \
    -exportOptionsPlist "exportOptions.plist"

echo "iOS build complete in $EXPORT_DIR"
