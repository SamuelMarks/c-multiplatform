# SDL3 Packaging

Builds a release library with SDL3 enabled (useful for desktop and console ports):

```
cmake -S . -B build-sdl3-package -DM3_ENABLE_PACKAGING=ON
cmake --build build-sdl3-package --target m3_package_sdl3
```

The build uses `M3_ENABLE_SDL3=ON` and produces a release library in
`build-sdl3-package`.
