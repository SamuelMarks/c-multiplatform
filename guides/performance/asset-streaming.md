# Resource Management & Asset Streaming

Loading high-resolution images, large custom fonts, and audio files synchronously blocks the main thread, causing the UI to freeze (jank). To solve this, the framework uses an asynchronous Asset Streamer (`ui_asset_streamer.h`).

## Asynchronous Loading

The asset streamer offloads file I/O and decoding (e.g., parsing a JPEG or decompressing an OGG file) to background worker threads.

```c
#include "ui_asset_streamer.h"
#include "ui_image_base.h"

// 1. Create the UI component
ui_component_t* profile_img = ui_image_create(arena);

// 2. Request the asset asynchronously
ui_asset_request_t request = {
    .url = "assets/images/high_res_profile.jpg",
    .priority = UI_ASSET_PRIORITY_HIGH,
    .target_component = profile_img,
    .on_complete = on_image_loaded
};

ui_asset_streamer_request(&request);
```

While the image is loading, the `ui_image_create` component will automatically display its placeholder color or fallback state.

## Callbacks and Main Thread

When the background thread finishes decoding the asset, the `on_complete` callback is safely dispatched back onto the **Main Rendering Thread** so you can mutate the DOM without mutex locks.

```c
void on_image_loaded(ui_asset_t* asset, void* user_data) {
    ui_component_t* img_component = (ui_component_t*)user_data;

    if (asset->status == UI_ASSET_STATUS_READY) {
        // Assign the decoded pixel buffer to the image component
        ui_image_set_pixels(img_component, asset->data.image);
    } else {
        // Handle failure (e.g., file not found, corrupt image)
        ui_image_set_error_state(img_component);
    }
}
```

## Caching Strategy

The `ui_asset_streamer.h` includes an LRU (Least Recently Used) cache. If you request `"assets/icon.png"` in five different components, the streamer will only load and decode it from disk once, instantly returning the cached `ui_asset_t*` for subsequent requests.

You can control the memory limit of the cache:

```c
// Limit asset cache to 256 MB
ui_asset_streamer_set_cache_limit(engine->streamer, 256 * 1024 * 1024);
```

When the limit is reached, the streamer automatically evicts the least recently used assets (destroying their textures) to free up memory. If an evicted asset is requested again later, it will be re-streamed from disk.
