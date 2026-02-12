# iOS Packaging

## XCFramework
Build a universal `LibM3C.xcframework` using CMake + Xcode:

```
cmake -S . -B build-ios-package -DM3_ENABLE_PACKAGING=ON
cmake --build build-ios-package --target m3_package_ios_xcframework
```

The XCFramework is written to `packaging/ios/out/LibM3C.xcframework`.

## IPA Archive
`archive_app.cmake` wraps `xcodebuild archive` and `-exportArchive` for an existing
Xcode project or workspace. Configure the cache variables when generating:

```
cmake -S . -B build-ios-ipa -DM3_ENABLE_PACKAGING=ON \
  -DM3_IOS_PROJECT=/path/to/App.xcodeproj \
  -DM3_IOS_SCHEME=App
cmake --build build-ios-ipa --target m3_package_ios_ipa
```

Adjust `packaging/ios/exportOptions.plist` as needed.
