# Desktop Packaging

## CPack (DMG/MSI/TGZ)

```
cmake -S . -B build-desktop-package -DCMP_ENABLE_PACKAGING=ON -DCMP_BUILD_PACKAGING_STUB=ON
cmake --build build-desktop-package --target cmp_package_desktop
```

CPack writes the installer into the build directory. Generator selection is based
on the host OS (DragNDrop on macOS, WIX on Windows, TGZ on Linux).

## Flatpak

```
flatpak-builder --force-clean build-flatpak packaging/desktop/flatpak/com.libcmpc.LibCMPC.json
```

This uses the packaging stub executable as the Flatpak command.

Note: `packaging/desktop/flatpak/com.libcmpc.LibCMPC.json` still references legacy `M3_*` flags. Update them if you standardize on `CMP_*` option names.
