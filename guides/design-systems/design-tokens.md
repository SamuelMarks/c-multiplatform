# Design Tokens

To maintain visual consistency across a large application, the framework supports a comprehensive **Design Token** system (`ui_design_tokens.h`). Instead of hardcoding hex colors, font sizes, and padding values directly into components, you reference semantic tokens.

## What are Design Tokens?

Tokens are key-value pairs representing the atomic visual design decisions of your application. They are usually defined at the root of your application and can be updated at runtime to instantly change the theme (e.g., swapping from Light Mode to Dark Mode).

## Defining Tokens in CSS

The framework's CSS engine fully supports standard CSS custom properties (variables) as the primary mechanism for design tokens.

```css
/* Define base tokens on the root element */
:root {
    /* Color Palette */
    --color-primary: #6200EE;
    --color-primary-variant: #3700B3;
    --color-surface: #FFFFFF;
    --color-error: #B00020;

    /* Typography */
    --font-size-h1: 96px;
    --font-size-body: 16px;

    /* Spacing */
    --spacing-sm: 8px;
    --spacing-md: 16px;
    --spacing-lg: 24px;

    /* Animation */
    --transition-duration-fast: 150ms;
}

/* Consume tokens in your component styles */
.my-card {
    background-color: var(--color-surface);
    padding: var(--spacing-md);
    border-radius: var(--spacing-sm);
}
```

## Manipulating Tokens via C API

You can programmatically get and set tokens at runtime. This is the standard way to implement a "Dark Mode" toggle.

```c
#include "ui_design_tokens.h"
#include "ui_cssom_api.h"

void enable_dark_mode(ui_engine_t* engine) {
    ui_component_t* root = engine->document_root;

    // Mutate the token values dynamically
    ui_css_set_property(root, "--color-surface", "#121212");
    ui_css_set_property(root, "--color-on-surface", "#FFFFFF");
    ui_css_set_property(root, "--color-primary", "#BB86FC");

    // The engine will automatically invalidate and repaint all child components
    // that rely on these `var(--...)` tokens.
}
```

## Material Design Integrations

If you are building an application based on Material Design, the framework provides `ui_compositor_material_base.h`. This module includes pre-built token sets for Material 2 and Material 3 (Material You), as well as logic for generating dynamic color palettes based on a single seed color.
