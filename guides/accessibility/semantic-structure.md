# Semantic Structure & ARIA

Providing a semantic structure is crucial for assistive technologies (like screen readers) to understand and navigate your UI. The framework provides tools in `ui_aria.h` to map your custom drawn components to OS-level accessibility trees.

## The ARIA Node

Every UI component can optionally have an associated ARIA node. This node exposes properties such as:
* **Role:** The semantic purpose (e.g., Button, Checkbox, Slider, Dialog)
* **State:** The current condition (e.g., Checked, Disabled, Expanded)
* **Properties:** Relational or configurational data (e.g., ValueMin, ValueMax, Controls)

### Example: Making a Custom Button Accessible

When building custom components, it's essential to map the visual representation to the semantic one.

```c
#include "ui_aria.h"
#include "ui_component.h"

void my_button_init(ui_component_t* btn) {
    // ... setup visual rendering ...

    // Assign ARIA role
    ui_aria_set_role(btn, UI_ARIA_ROLE_BUTTON);

    // Provide an accessible label for screen readers
    ui_aria_set_label(btn, "Submit Form");
}

void my_button_set_disabled(ui_component_t* btn, bool disabled) {
    // Notify the accessibility tree of the state change
    ui_aria_set_disabled(btn, disabled);

    // ... update visual state ...
}
```

## Best Practices
1. **Prefer Native Roles:** Use the most specific ARIA role available rather than falling back to generic containers.
2. **Keep Labels Concise:** Ensure `aria-label` provides a clear, action-oriented description.
3. **Manage Focus:** If a component receives an action, ensure it can receive focus using the `ui_focus_manager.h` integration.
