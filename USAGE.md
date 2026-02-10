USAGE
=====

This guide shows how to build LibM3C and wire core widgets (including text fields) into a simple loop.

Build
-----

```
cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Text Field Example
------------------

```c
#include "m3/m3_text_field.h"

int setup_text_field(M3Gfx *gfx, M3TextField *field) {
    M3TextBackend backend;
    M3TextFieldStyle style;
    int rc;

    rc = m3_text_backend_from_gfx(gfx, &backend);
    if (rc != M3_OK) return rc;

    rc = m3_text_field_style_init(&style);
    if (rc != M3_OK) return rc;

    style.text_style.utf8_family = "Sans";
    style.label_style.utf8_family = "Sans";

    rc = m3_text_field_init(field, &backend, &style, NULL, "", 0);
    if (rc != M3_OK) return rc;

    rc = m3_text_field_set_label(field, "Email", 5);
    if (rc != M3_OK) return rc;

    rc = m3_text_field_set_placeholder(field, "name@example.com", 16);
    if (rc != M3_OK) return rc;

    return M3_OK;
}
```

Driving Animations
------------------

Text fields animate their floating labels and cursor blink. Call `m3_text_field_step` each frame:

```c
M3Bool changed;
rc = m3_text_field_step(&field, dt_seconds, &changed);
if (rc == M3_OK && changed) {
    /* trigger a repaint */
}
```

Input Routing
-------------

Use `m3_event_dispatch` to route `M3InputEvent` instances to widgets. Pointer and text input events update the
text field state automatically. For focus transitions managed outside the dispatcher, call
`m3_text_field_set_focus` explicitly.

Camera Capture Example
----------------------

The camera plugin exposes configuration for device selection, resolution, and pixel format. Use
`m3_camera_config_init` to start from defaults, then override fields in `config.config`.

```c
#include "m3/m3_camera.h"

int open_camera(M3Env *env, M3CameraSession *session) {
    M3CameraSessionConfig config;
    int rc;

    rc = m3_camera_config_init(&config);
    if (rc != M3_OK) return rc;

    config.env = env;
    config.config.facing = M3_CAMERA_FACING_BACK;
    config.config.width = 1280u;
    config.config.height = 720u;
    config.config.format = M3_CAMERA_FORMAT_NV12;

    rc = m3_camera_init(session, &config);
    if (rc != M3_OK) return rc;

    rc = m3_camera_start(session);
    if (rc != M3_OK) {
        m3_camera_shutdown(session);
        return rc;
    }

    return M3_OK;
}

int read_camera_frame(M3CameraSession *session, M3CameraFrame *frame, M3Bool *has_frame) {
    return m3_camera_read_frame(session, frame, has_frame);
}

int close_camera(M3CameraSession *session) {
    int rc;

    rc = m3_camera_stop(session);
    if (rc != M3_OK) return rc;

    return m3_camera_shutdown(session);
}
```

If a backend cannot satisfy the requested resolution or pixel format, it returns
`M3_ERR_UNSUPPORTED`. To accept a backend default, set `config.config.format` to
`M3_CAMERA_FORMAT_ANY` and leave `width`/`height` as `0`.

Network Request Example
-----------------------

Use the network plugin to issue HTTP requests through the active backend. Responses
must be released with `m3_network_response_free` before shutting down the client.

```c
#include "m3/m3_network.h"

int fetch_url(M3Env *env, const char *url) {
    M3NetworkClient client;
    M3NetworkConfig config;
    M3NetworkRequest request;
    M3NetworkResponse response;
    int rc;

    rc = m3_network_config_init(&config);
    if (rc != M3_OK) return rc;

    config.env = env;

    rc = m3_network_init(&client, &config);
    if (rc != M3_OK) return rc;

    rc = m3_network_request_init(&request);
    if (rc != M3_OK) {
        m3_network_shutdown(&client);
        return rc;
    }

    request.method = "GET";
    request.url = url;
    request.timeout_ms = 5000u;

    rc = m3_network_request(&client, &request, &response);
    if (rc != M3_OK) {
        m3_network_shutdown(&client);
        return rc;
    }

    /* consume response.body/response.body_size here */

    rc = m3_network_response_free(&client, &response);
    if (rc != M3_OK) {
        m3_network_shutdown(&client);
        return rc;
    }

    return m3_network_shutdown(&client);
}
```
