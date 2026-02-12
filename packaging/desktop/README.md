# Desktop Packaging

## CPack (DMG/MSI/TGZ)

```
cmake -S . -B build-desktop-package -DM3_ENABLE_PACKAGING=ON -DM3_BUILD_PACKAGING_STUB=ON
cmake --build build-desktop-package --target m3_package_desktop
```

CPack writes the installer into the build directory. Generator selection is based
on the host OS (DragNDrop on macOS, WIX on Windows, TGZ on Linux).

## Flatpak

```
flatpak-builder --force-clean build-flatpak packaging/desktop/flatpak/com.libm3c.LibM3C.json
```

This uses the packaging stub executable as the Flatpak command.
