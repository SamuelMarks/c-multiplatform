# CSS Object Model (CSSOM)

The framework parses standard W3C CSS into an internal, highly optimized C representation known as the CSS Object Model (CSSOM). This allows you to dynamically manipulate styles, query computed properties, and inject custom stylesheets at runtime (`ui_cssom.h`, `ui_cssom_api.h`).

## Understanding the CSSOM Tree

When a stylesheet is parsed, it generates a hierarchy:
1.  **`ui_cssom_stylesheet_t`**: Represents a single `.css` file or `<style>` block.
2.  **`ui_cssom_rule_t`**: Represents a rule (e.g., `@media`, or a standard style rule).
3.  **`ui_cssom_selector_t`**: Represents the parsed selector (e.g., `.btn:hover > span`).
4.  **`ui_cssom_declaration_t`**: Represents a single property-value pair (e.g., `color: red;`).

## Dynamically Injecting Styles

You can construct and inject CSSOM stylesheets dynamically at runtime. This is particularly useful for applying user themes or runtime-calculated design tokens.

```c
#include "ui_cssom_api.h"
#include "ui_engine.h"

void apply_dynamic_theme(ui_engine_t* engine, ui_color_t primary_brand) {
    // 1. Create a new stylesheet
    ui_cssom_stylesheet_t* sheet = ui_cssom_stylesheet_create();

    // 2. Format a CSS string dynamically (in a real app, use the arena)
    char css_buffer[256];
    snprintf(css_buffer, sizeof(css_buffer),
             ":root { --brand-primary: #%02x%02x%02x; }",
             primary_brand.r, primary_brand.g, primary_brand.b);

    // 3. Parse and append to the stylesheet
    ui_cssom_parse_and_append(sheet, css_buffer);

    // 4. Attach the stylesheet to the document
    ui_cssom_document_add_stylesheet(engine->document, sheet);
}
```

## Querying Computed Styles

Sometimes a component needs to know its actual, resolved styling (e.g., if a custom C-based canvas drawing routine needs to match the CSS background color). You use the `ui_cssom_view.h` API for this.

```c
#include "ui_cssom_view.h"

void my_custom_canvas_paint(ui_component_t* self, ui_canvas_t* canvas) {
    // Get the final computed styles for this specific component
    ui_computed_style_t* computed = ui_cssom_get_computed_style(self);

    // Retrieve a specific parsed property
    float border_radius = ui_computed_style_get_length(computed, UI_CSS_PROP_BORDER_RADIUS);

    // Draw using the CSS-provided value
    ui_canvas_draw_rounded_rect(canvas, self->bounds, border_radius);
}
```
