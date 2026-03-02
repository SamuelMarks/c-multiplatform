USAGE
=====

This guide shows how to build LibCMPC and wire core widgets (including text fields) into a simple loop.
It also notes the current media decoding fallbacks and plugin helpers. Backends are opt-in and compiled behind CMake flags.

## System dependencies

- macOS: build developer tools (XCode); runtime (none)
- Windows: MSVC; runtime (none)
- Linux; gcc, clang, tcc, etc.; runtime (gtk4)
- Web: Emscripten

## Build

```sh
cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

## Backend Selection

Backends are enabled with CMake flags and may compile as stubs if their platform/dependencies are missing.

### FetchContent (CMake)

```cmake
include(FetchContent)

FetchContent_Declare(
    cmpc
    GIT_REPOSITORY https://github.com/SamuelMarks/c-multiplatform.git
    GIT_TAG master
)

set(CMP_ENABLE_SDL3 ON CACHE BOOL "" FORCE)
set(CMP_ENABLE_SDL3_TTF ON CACHE BOOL "" FORCE)
set(CMP_ENABLE_LIBCURL ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cmpc)

add_executable(app main.c)
# Core-only usage:
target_link_libraries(app PRIVATE cmp::cmpc)
# Material 3 widgets/styles:
# target_link_libraries(app PRIVATE m3::m3)
```

### Visual Documentation Pipeline

The project features an autonomous headless vector documentation generator (`cmp_docgen`) that emits SVG files for UI verification.

```bash
# Build the docgen tool
cmake -S . -B build -DCMP_BUILD_DOCGEN=ON
cmake --build build --target cmp_docgen

# Generate SVGs and JSON maps
./build/cmp_docgen --platform linux --theme material

# Render Markdown tables
python3 tools/docgen/build_docs.py

# Optional: Generate static HTML site with Doxygen
cd docs && npm install
cd ..
cmake -S . -B build-docs -DCMP_REQUIRE_DOXYGEN=ON
cmake --build build-docs --target cmp_docs
```

## Media Decoding (Fallback)

LibCMPC ships minimal fallback decoders for development and tests:

- **Images**: PPM (P6) and raw RGBA8 buffers.
- **Audio**: WAV (PCM16).
- **Video**: `M3V0` raw RGBA frame container.

Backends can override these via `CMPEnv` for platform-native codecs.

## Text Field Example

```c
#include "cmpc/cmp_text_field.h"

int setup_text_field(CMPGfx *gfx, CMPTextField *field) {
    CMPTextBackend backend;
    CMPTextFieldStyle style;
    int rc;

    rc = cmp_text_backend_from_gfx(gfx, &backend);
    if (rc != CMP_OK) return rc;

    rc = cmp_text_field_style_init(&style);
    if (rc != CMP_OK) return rc;

    style.text_style.utf8_family = "Sans";
    style.label_style.utf8_family = "Sans";

    rc = cmp_text_field_init(field, &backend, &style, NULL, "", 0);
    if (rc != CMP_OK) return rc;

    rc = cmp_text_field_set_label(field, "Email", 5);
    if (rc != CMP_OK) return rc;

    rc = cmp_text_field_set_placeholder(field, "name@example.com", 16);
    if (rc != CMP_OK) return rc;

    return CMP_OK;
}
```

## Camera Capture Example

The camera plugin exposes configuration for device selection, resolution, and pixel format. Use
`cmp_camera_config_init` to start from defaults, then override fields in `config.config`.

```c
#include "cmpc/cmp_camera.h"

int open_camera(CMPEnv *env, CMPCameraSession *session) {
    CMPCameraSessionConfig config;
    int rc;

    rc = cmp_camera_config_init(&config);
    if (rc != CMP_OK) return rc;

    config.env = env;
    config.config.facing = CMP_CAMERA_FACING_BACK;
    config.config.width = 1280u;
    config.config.height = 720u;
    config.config.format = CMP_CAMERA_FORMAT_NV12;

    rc = cmp_camera_init(session, &config);
    if (rc != CMP_OK) return rc;

    rc = cmp_camera_start(session);
    if (rc != CMP_OK) {
        cmp_camera_shutdown(session);
        return rc;
    }

    return CMP_OK;
}
```

## Network Request Example

Use the network plugin to issue HTTP requests through the active backend. Responses
must be released with `cmp_network_response_free` before shutting down the client.

```c
#include "cmpc/cmp_network.h"

int fetch_url(CMPEnv *env, const char *url) {
    CMPNetworkClient client;
    CMPNetworkConfig config;
    CMPNetworkRequest request;
    CMPNetworkResponse response;
    int rc;

    rc = cmp_network_config_init(&config);
    if (rc != CMP_OK) return rc;

    config.env = env;

    rc = cmp_network_init(&client, &config);
    if (rc != CMP_OK) return rc;

    rc = cmp_network_request_init(&request);
    if (rc != CMP_OK) {
        cmp_network_shutdown(&client);
        return rc;
    }

    request.method = "GET";
    request.url = url;
    request.timeout_ms = 5000u;

    rc = cmp_network_request(&client, &request, &response);
    if (rc != CMP_OK) {
        cmp_network_shutdown(&client);
        return rc;
    }

    /* consume response.body/response.body_size here */

    rc = cmp_network_response_free(&client, &response);
    if (rc != CMP_OK) {
        cmp_network_shutdown(&client);
        return rc;
    }

    return cmp_network_shutdown(&client);
}
```

## Storage Example

The storage helper provides a simple key/value store with optional file persistence via `CMPIO`.

```c
#include "cmpc/cmp_storage.h"

int save_prefs(CMPEnv *env) {
    CMPStorage storage;
    CMPStorageConfig config;
    CMPIO io;
    int rc;

    rc = cmp_storage_config_init(&config);
    if (rc != CMP_OK) return rc;

    rc = cmp_storage_init(&storage, &config);
    if (rc != CMP_OK) return rc;

    rc = cmp_storage_put(&storage, "theme", 5, "dark", 4, CMP_TRUE);
    if (rc != CMP_OK) {
        cmp_storage_shutdown(&storage);
        return rc;
    }

    rc = env->vtable->get_io(env->ctx, &io);
    if (rc != CMP_OK) {
        cmp_storage_shutdown(&storage);
        return rc;
    }

    rc = cmp_storage_save(&storage, &io, "prefs.m3s");
    if (rc != CMP_OK) {
        cmp_storage_shutdown(&storage);
        return rc;
    }

    return cmp_storage_shutdown(&storage);
}
```
