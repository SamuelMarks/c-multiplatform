# Asset Management & Asynchronous I/O

The UI Engine forbids blocking the main thread. All filesystem access, network requests, and heavy decoding (images, fonts, JSON) must occur in the background `ui_thread_pool`.

## 1. External Fetch Dependencies

The engine utilizes modular CMake `FetchContent` dependencies for platform-agnostic I/O:
*   `c-abstract-http`: For network requests (XHR/Fetch equivalents).
*   `c-fs`: For safe filesystem traversal and file handles.

## 2. Loading an Image Asynchronously

To display an image without freezing the UI, you must dispatch the decode task to the thread pool and return a `ui_promise`.

```c
#include "ui_engine.h"
#include "ui_image_decoder.h"
#include "ui_promise.h"

/* Structure to pass data between threads */
struct image_fetch_ctx {
    struct ui_engine *engine;
    struct ui_dom_node *img_node;
    const char *filepath;
};

/* 1. Executes in the background ui_thread_pool */
void background_decode_image(void *user_data, struct ui_promise_resolver *resolver) {
    struct image_fetch_ctx *ctx = (struct image_fetch_ctx*)user_data;
    struct ui_image *img = NULL;
    enum ui_error err = UI_ERROR_NONE;

    /* Perform heavy I/O and decoding (simulated via ui_image_decoder_load) */
    err = ui_image_decoder_load(ctx->engine, ctx->filepath, &img);

    if (err == UI_ERROR_NONE) {
        /* Success: Pass the decoded image pointer back */
        ui_promise_resolver_resolve(resolver, img);
    } else {
        /* Failure: Pass the error code */
        ui_promise_resolver_reject(resolver, (void*)(intptr_t)err);
    }
}

/* 2. Executes on the MAIN thread when the promise resolves */
void on_image_decoded(struct ui_engine *engine, void *result, void *user_data) {
    struct ui_image *img = (struct ui_image*)result;
    struct image_fetch_ctx *ctx = (struct image_fetch_ctx*)user_data;

    /* Safely assign the hardware texture to the DOM node */
    ui_dom_node_set_image(ctx->img_node, img);
    free(ctx);
}

/* 3. Dispatching the task from a component */
void load_profile_picture(struct ui_engine *engine, struct ui_dom_node *node) {
    struct image_fetch_ctx *ctx = malloc(sizeof(struct image_fetch_ctx));
    ctx->engine = engine;
    ctx->img_node = node;
    ctx->filepath = "assets/profile.png";

    /* Dispatch to the thread pool */
    struct ui_promise *promise = ui_thread_pool_dispatch(engine->pool, background_decode_image, ctx);

    /* Register the main-thread callback */
    ui_promise_then(promise, on_image_decoded, ctx);
}
```

## 3. Font Loading (`stb_truetype`)

Custom fonts follow the exact same asynchronous pattern.
1.  Read the `.ttf` or `.otf` file bytes using `c-fs` in the background.
2.  Pass the raw buffer to `ui_font_manager_load_from_memory` on the main thread.
3.  The text layout engine will automatically invalidate any dirty text nodes and recalculate glyph atlases on the next frame tick.

## 4. A/V Sync (Audio & Video)

Video playback requires synchronizing video frames to the audio output.
*   **Audio Clock:** The engine's internal monotonic clock is slaved to the Audio Sink's Presentation Timestamp (PTS).
*   **Video Decoder:** The hardware video pipeline (MediaFoundation, AVFoundation, etc.) decodes YUV frames in the background into lock-free ring buffers.
*   **Fragment Shader:** The main thread polls the ring buffer during the Render Pass, uploading the Y/U/V planes to separate OpenGL textures. A specialized fragment shader performs the YUV-to-RGB conversion entirely on the GPU.
