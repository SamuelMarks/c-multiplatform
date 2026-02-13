# SDL3 Packaging

Builds a release library with SDL3 enabled (useful for desktop and console ports):

```
cmake -S . -B build-sdl3-package -DCMP_ENABLE_PACKAGING=ON
cmake --build build-sdl3-package --target cmp_package_sdl3
```

The build uses `CMP_ENABLE_SDL3=ON` and produces a release library in
`build-sdl3-package`.

Note: `packaging/sdl3/build_sdl3.cmake` still uses legacy `M3_*` flags and builds the `m3` target. Update the script if you are standardizing on `CMP_*` option names.
