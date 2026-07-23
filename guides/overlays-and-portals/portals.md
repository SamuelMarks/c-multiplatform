# Portals

In a standard DOM hierarchy, a component's visual representation is restricted by its parent's CSS properties. For example, if a parent component has `overflow: hidden`, any child component that attempts to draw outside the parent's bounding box will be clipped.

Portals (`ui_portal.h`) provide a mechanism to conceptually keep a component in its original logical location (for data binding and event bubbling) but *visually* render it in an entirely different part of the DOM tree (usually at the root level).

## When to use Portals

You should use portals when a component needs to escape its container's visual constraints but must remain logically coupled to its origin. Common examples include:

*   Custom tooltips
*   Combobox dropdown lists
*   Drag-and-drop preview ghosts
*   Context menus

*Note: The `ui_overlay_director.h` uses Portals under the hood to hoist modals and popups to the root layer.*

## Creating a Portal

To use a portal, you define the component you want to hoist, and the target container where it should be visually injected.

```c
#include "ui_portal.h"

// 1. We have some deeply nested container with overflow:hidden
ui_component_t* nested_container = ui_container_create(arena);
ui_css_set_overflow(nested_container, UI_CSS_OVERFLOW_HIDDEN);

// 2. We create the tooltip that needs to break out
ui_component_t* tooltip = ui_tooltip_create(arena, "I am not clipped!");

// 3. We wrap the tooltip in a Portal.
// We tell it to render into the 'engine->document_root'.
ui_component_t* my_portal = ui_portal_create(arena, tooltip, engine->document_root);

// 4. We append the portal (NOT the tooltip directly) to the nested container
ui_dom_append_child(nested_container, my_portal);
```

## Event Bubbling

Even though the `tooltip` in the example above is visually drawn at the root of the screen, its DOM events still bubble logically up through the `my_portal` component into the `nested_container`.

This means if you attach an `on_click` listener to the `nested_container`, clicking the visually-hoisted tooltip will still trigger that listener.

## Coordinate Spaces

Because a portaled component is drawn in a different coordinate space than its logical parent, you must be careful when positioning it. You typically use the `ui_geometry_anchor.h` utilities to calculate the absolute screen coordinates of the logical parent and apply that transform to the portaled child.
