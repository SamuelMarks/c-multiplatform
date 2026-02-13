# iOS Packaging

## XCFramework
Build a universal `LibCMPC.xcframework` using CMake + Xcode:

```
cmake -S . -B build-ios-package -DCMP_ENABLE_PACKAGING=ON
cmake --build build-ios-package --target cmp_package_ios_xcframework
```

The XCFramework is written to `packaging/ios/out/LibCMPC.xcframework`.

Note: `packaging/ios/build_xcframework.cmake` still uses legacy `M3_*` flags and builds the `m3` target. Update the script if you are standardizing on `CMP_*` option names.

## IPA Archive
`archive_app.cmake` wraps `xcodebuild archive` and `-exportArchive` for an existing
Xcode project or workspace. Configure the cache variables when generating:

```
cmake -S . -B build-ios-ipa -DCMP_ENABLE_PACKAGING=ON \
  -DCMP_IOS_PROJECT=/path/to/App.xcodeproj \
  -DCMP_IOS_SCHEME=App
cmake --build build-ios-ipa --target cmp_package_ios_ipa
```

Adjust `packaging/ios/exportOptions.plist` as needed.

Use `CMP_IOS_WORKSPACE` instead of `CMP_IOS_PROJECT` when building from an `.xcworkspace`, and override `CMP_IOS_EXPORT_OPTIONS` to point at a custom export plist.
