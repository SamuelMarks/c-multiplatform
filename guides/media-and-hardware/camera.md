# Camera Integration

Accessing hardware cameras for video conferencing, barcode scanning, or capturing profile pictures is handled via the `ui_camera_base.h` module. This provides a unified API over V4L2 (Linux), DirectShow/MediaFoundation (Windows), AVFoundation (macOS/iOS), and `getUserMedia` (Web/WASM).

## Requesting Permissions

Before accessing a camera, you must request permission from the user, as dictated by modern OS security models.

```c
#include "ui_camera_base.h"

void on_camera_permission(bool granted) {
    if (granted) {
        start_camera_feed();
    } else {
        show_error("Camera access denied.");
    }
}

// Prompt the user for permission
ui_camera_request_permission(on_camera_permission);
```

## Creating a Camera Viewfinder

Once permission is granted, you can create a UI component that automatically streams the camera feed into the rendering pipeline.

```c
void start_camera_feed() {
    // 1. Initialize the hardware camera (requesting 720p @ 30fps)
    ui_camera_device_t* cam_device = ui_camera_open_default(1280, 720, 30);

    // 2. Create the UI component that displays the video stream
    ui_component_t* viewfinder = ui_camera_view_create(arena, cam_device);

    // 3. Configure the visual fit (like object-fit: cover)
    ui_css_set_object_fit(viewfinder, UI_CSS_OBJECT_FIT_COVER);

    ui_dom_append_child(app_root, viewfinder);

    // 4. Start the hardware stream
    ui_camera_start(cam_device);
}
```

## Capturing a Still Frame

To take a picture, you capture the current frame buffer from the device. This returns a standard `ui_image_buffer_t` that you can encode to JPEG or process further.

```c
#include "stb_image_write.h" // Assuming STB is used for encoding

void take_photo(ui_camera_device_t* cam_device) {
    ui_image_buffer_t* photo = ui_camera_capture_frame(cam_device);

    if (photo) {
        // Save to disk
        stbi_write_jpg("output/snapshot.jpg", photo->width, photo->height, photo->channels, photo->pixels, 90);

        // Free the buffer
        ui_image_buffer_free(photo);
    }
}
```

## Processing Frames (Barcode Scanning)

If you need to analyze the video stream in real-time (e.g., scanning a QR code or running a face detection ML model), you can attach a frame callback. This callback fires on a background thread.

```c
void on_video_frame(ui_image_buffer_t* frame) {
    // Pass the raw pixels to a scanning library like zbar
    char* qr_data = scan_for_qr_codes(frame->pixels, frame->width, frame->height);

    if (qr_data) {
        printf("Found QR Code: %s\n", qr_data);
    }
}

ui_camera_on_frame(cam_device, on_video_frame);
```
