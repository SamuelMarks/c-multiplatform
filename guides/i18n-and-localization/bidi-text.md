# Bidirectional (BiDi) Text & RTL Layouts

Building a truly global application means supporting languages that read Right-to-Left (RTL), such as Arabic, Hebrew, and Persian. The framework handles this through the `ui_bidi_manager.h` and the underlying logical CSS property engine.

## Setting Text Direction

The text direction can be set globally on the document root or locally on specific components. The framework uses the standard CSS `dir` attribute (or the C API equivalent).

```c
#include "ui_bidi_manager.h"
#include "ui_dom_node.h"

// Set the entire application to Right-to-Left
ui_dom_set_attribute(app_root, "dir", "rtl");

// Or set it programmatically via the BiDi manager
ui_bidi_set_base_direction(engine->bidi_manager, UI_BIDI_DIR_RTL);
```

When the direction is set to RTL, the text shaping engine automatically reverses the character rendering order where appropriate, while preserving the LTR rendering of embedded Latin text or numbers (true Bidirectional support).

## Logical CSS Properties

To prevent developers from needing to write separate stylesheets for LTR and RTL, the framework fully supports **CSS Logical Properties** (`ui_css_logical.h`).

Instead of physical directions (`left`, `right`), you use flow-relative directions (`start`, `end`).

```css
/* BAD: Hardcoded physical properties */
.chat-bubble {
    margin-left: 16px;
    padding-right: 8px;
    border-left: 2px solid blue;
}

/* GOOD: Logical properties adapt automatically */
.chat-bubble {
    margin-inline-start: 16px;
    padding-inline-end: 8px;
    border-inline-start: 2px solid blue;
}
```

When the layout engine calculates the bounding boxes, if `dir="rtl"`, `inline-start` maps to physical `right`, and `inline-end` maps to physical `left`.

## UI Flipping

Most UI components (like sidebars, navigation rails, and flexbox rows) will automatically mirror their layout in RTL mode. However, certain icons (e.g., a "back" arrow) need to be explicitly mirrored.

```c
// Using a modifier to automatically flip an icon in RTL mode
ui_icon_set_auto_mirror(back_arrow_icon, true);
```
