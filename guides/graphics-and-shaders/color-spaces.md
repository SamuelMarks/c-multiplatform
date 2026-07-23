# Color Spaces & Quantization

To ensure that your UI renders accurately across different displays—from standard SDR monitors to high-end HDR OLED screens—the framework provides deep control over Color Spaces (`ui_color_space.h`) and tools for palette optimization (`ui_color_quantization.h`).

## Working with Color Spaces

By default, the framework assumes standard sRGB for CSS colors. However, you can explicitly define colors in wider gamuts (like Display P3) to utilize the full vibrancy of modern screens.

```c
#include "ui_color_space.h"
#include "ui_cssom_api.h"

// Define a highly vibrant red in Display P3
ui_color_t vibrant_red = ui_color_p3(1.0f, 0.0f, 0.0f, 1.0f);

// If the target display does NOT support Display P3,
// the engine will automatically gamut-map this color down to sRGB during the render pass
// using a perceptual rendering intent.
ui_css_set_background_color(my_button, vibrant_red);
```

You can query the current display's capabilities to make runtime design decisions:

```c
if (ui_display_supports_color_space(engine->display, UI_COLOR_SPACE_DISPLAY_P3)) {
    // Load the wide-gamut version of the hero image
    ui_image_set_src(hero, "assets/hero_p3.png");
}
```

## Color Quantization

Quantization is the process of reducing a large color palette (e.g., millions of colors in a photograph) down to a smaller, representative set (e.g., 8 or 16 colors).

The framework provides an implementation of the **K-Means clustering** and **Median Cut** algorithms via `ui_color_quantization.h`.

### Use Case: Dynamic Theming

A common UX pattern is to extract the dominant colors from an image (like an album cover) and use them to tint the surrounding UI.

```c
#include "ui_color_quantization.h"
#include "ui_image_decoder.h"

void tint_ui_from_album_cover(ui_component_t* header, const char* image_path) {
    // 1. Decode the image pixels
    ui_image_buffer_t* img = ui_image_load(image_path);

    // 2. Extract the 3 most dominant colors using K-Means
    ui_color_t palette[3];
    ui_quantize_kmeans(img, palette, 3);

    // palette[0] is the most dominant color
    ui_color_t dominant = palette[0];

    // 3. Set the header background to a darkened version of the dominant color
    ui_color_t bg = ui_color_darken(dominant, 0.3f);
    ui_css_set_background_color(header, bg);

    // Ensure text is readable over the generated background
    ui_color_t text_color = ui_color_get_contrasting_text(bg);
    ui_css_set_color(header, text_color);

    ui_image_buffer_free(img);
}
```
