# Web Packaging

1. Source the Emscripten environment so `emcmake` and `emcc` are available.
2. Run the packaging target:

```
cmake -S . -B build-web-package -DM3_ENABLE_PACKAGING=ON
cmake --build build-web-package --target m3_package_web
```

The PWA bundle is staged in `packaging/web/dist` and includes gzip/brotli
compression for the JS and WASM outputs.
