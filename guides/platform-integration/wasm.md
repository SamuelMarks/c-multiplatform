# WebAssembly (WASM) Deployment

The framework is built in standard C, making it an excellent candidate for WebAssembly compilation via Emscripten. This allows you to run your native C application directly in a web browser using WebGL or WebGPU for rendering.

## The Emscripten Main Loop

Unlike native desktop platforms where you control the infinite `while(true)` event loop, the browser requires yielding execution back to the JavaScript main thread. Emscripten handles this via `emscripten_set_main_loop`.

```c
#include "ui_engine.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

ui_engine_t* g_engine = NULL;

void main_loop_step() {
    // Tick the engine once per frame
    ui_engine_tick(g_engine);
}

int main() {
    g_engine = ui_engine_create();

    // Mount the root component
    ui_engine_mount(g_engine, my_app_create(g_engine->arena));

#ifdef __EMSCRIPTEN__
    // 0 = requestAnimationFrame (sync to display refresh rate)
    // 1 = simulate infinite loop
    emscripten_set_main_loop(main_loop_step, 0, 1);
#else
    // Native desktop infinite loop
    while (!ui_engine_should_quit(g_engine)) {
        ui_engine_tick(g_engine);
    }
#endif

    ui_engine_destroy(g_engine);
    return 0;
}
```

## Build Configuration

When compiling with `emcc`, ensure you enable WebGL and pass your application assets (like fonts and images) so Emscripten can package them into a virtual file system.

```bash
emcc src/main.c \
  -o build_web/index.html \
  -s USE_WEBGL2=1 \
  -s FULL_ES3=1 \
  -s ASYNCIFY \
  --preload-file assets/ \
  -I include/
```

## Interacting with the Browser DOM

The C framework renders entirely onto an HTML5 `<canvas>`. However, you sometimes need to interact with the actual browser DOM (e.g., to open a native file picker or update the page URL). You can use Emscripten's `EM_JS` macros or the provided framework wrapper functions.

```c
// Example: Triggering a browser alert from C
#include <emscripten.h>

EM_JS(void, js_show_alert, (const char* str), {
    alert(UTF8ToString(str));
});

void on_button_click(ui_component_t* btn) {
    js_show_alert("Hello from C via WebAssembly!");
}
```
