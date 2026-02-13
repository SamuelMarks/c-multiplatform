# Web Packaging

1. Source the Emscripten environment so `emcmake` and `emcc` are available.
2. Run the packaging target:

```
cmake -S . -B build-web-package -DCMP_ENABLE_PACKAGING=ON -DCMP_ENABLE_WEB=ON -DCMP_BUILD_WEB_APP=ON
cmake --build build-web-package --target cmp_package_web
```

The PWA bundle is staged in `packaging/web/dist` and includes gzip/brotli
compression for the JS and WASM outputs.

Note: `packaging/web/build_web.cmake` still references legacy `m3_web_app`/`M3_*` naming. If it fails, update the script to `cmp_web_app` and `CMP_*` flags.
