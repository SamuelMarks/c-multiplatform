/**
 * @file DOC_ARCHITECTURE.md
 * @brief DOC_ARCHITECTURE.md implementation.
 */
/**
 * @file DOC_ARCHITECTURE.md
 * @brief DOC_ARCHITECTURE.md implementation.
 */
# Core Architecture: Memory & Threading

## 1. Memory Ownership Model
The UI Engine adheres to a strict, hierarchical memory ownership model to prevent leaks without a garbage collector.

### Arena & Pool Allocators
- **Arenas (ui_arena):** Used for transient, per-frame data (like CSS layout passes, styling recalculations, and temporary DOM nodes). All memory allocated from an arena is freed instantly in a single ui_arena_reset call. Individual elements cannot be freed manually.
- **Object Pools:** Used for long-lived components (Windows, Image Textures, OpenAL Contexts) that need deterministic freeing out of order.
- **Reference Counting:** Extremely sparse usage. Only used for asynchronous ui_promise handles to prevent dangling pointers when a background thread and main thread both hold references.

### Error Percolation & goto cleanup
All functions that allocate memory check for NULL and use an explicit goto cleanup strategy upon failure. This ensures any partially constructed objects or locks acquired mid-function are safely released before returning an error enum to the caller.

## 2. Threading Model
The engine operates on a multi-threaded but thread-safe paradigm utilizing a Reactor pattern for I/O and an Event Loop for UI interactions.

### Main Execution Context (ui_execution_context)
- The Main Thread (or Tick Engine) is the sole owner of the DOM Tree and CSSOM.
- **No Mutexes in UI:** To maximize 60 FPS performance, we do not lock individual DOM nodes. Instead, state mutation must occur sequentially on the main thread.

### Background Workers (ui_thread_pool)
- File I/O, Image Decoding, and Network requests are dispatched to a background thread pool.
- Background threads cannot modify the DOM. They return results via ui_promise resolution, which queues a callback to execute safely within the ui_execution_context tick loop.

### Reactor (ui_reactor)
- Network sockets (via Epoll, Kqueue, or Select) are polled in a dedicated Reactor thread. Socket events are enqueued back to the main ui_execution_context.

## 3. Reactive State & Component Data Binding (Signals API)

The engine leverages a Signal-based reactive graph (`ui_signal`, `ui_computed`, `ui_effect`) as the single source of truth for component state. Every mountable widget in the Component Development Kit (CDK) is fully compatible with this engine.

For an end-to-end example of building a multi-step form utilizing these concepts, see [examples/signal_form_navigation/README.md](examples/signal_form_navigation/README.md).

### Component Categories
Widgets are broadly categorized into two architectural patterns regarding data binding:

#### A. Form Controls (Two-Way Binding & CVA)
Interactive input widgets (e.g., `ui_slider_base`, `ui_checkbox_base`, `ui_select_base`, `ui_radio_group_base`, `ui_input_base`) implement the **Control Value Accessor (CVA)** pattern.
- **Implementation:** Their creation functions populate a C-struct function pointer vtable, returning an optional `ui_control_value_accessor`.
- **Integration:** This CVA interface acts as a bridge, allowing the `ui_form_control`, `ui_form_group`, and `ui_form_array` primitives to automatically read, write, validate, and manage dirty/touched states without creating infinite bidirectional update loops.

**Example: Binding an Input**
```c
struct ui_input_base *input = NULL;
struct ui_control_value_accessor *cva = NULL;

/* 1. Create the widget and extract its CVA */
ui_input_base_create(&input, &cva);

/* 2. Retrieve the underlying reactive control from your form group */
struct ui_form_control *ctrl = ui_form_group_get_control(form, "email");

/* 3. Bind them. The engine now handles all value patching and event listening */
ui_form_control_bind_cva(ctrl, cva);
```

- **Guideline for Future Widget Developers:** Any new interactive widget that accepts user input *must* export a CVA interface to be compatible with the reactive forms engine.

#### B. Presentational Widgets (One-Way Binding)
Structural and presentational components (e.g., `ui_alert_base`, `ui_badge_base`, `ui_accordion_base`, `ui_button_base`) utilize explicit one-way property binding.
- **Implementation:** They expose explicit binding methods like `ui_alert_base_bind_open()`, `ui_badge_base_bind_text()`, or `ui_accordion_base_bind_data()`.
- **Integration:** These methods accept a `struct ui_signal*` or `struct ui_computed*`. The widget internally sets up a `ui_effect` to reactively update its DOM representation whenever the underlying signal changes.

**Example: Binding a Badge**
```c
struct ui_badge_base *badge = NULL;
ui_badge_base_create(&badge);

/* Bind the badge text to a computed signal derived from the cart contents */
ui_badge_base_bind_text(badge, cart_item_count_signal);
```

- **Guideline for Future Widget Developers:** Expose specific `bind_<property>` functions for any dynamically updatable state rather than forcing users to manually query and update the component.

### Internal Structures vs. Mountable Components
Note that not every struct in the `include/` directory is a mountable UI component. For instance, `ui_ripple_base` lacks signal integration because it is an internal animation state tracker (managed by parents like `ui_button_base`), not a standalone component in the reactive DOM tree.

## 4. Design System Implementation Guide (Material, Fluent, Cupertino)
When implementing a visual skin or design system over the CDK layer, implementors must follow strict separation of concerns:
1. **Never Re-implement State:** Rely entirely on the base component's CVA or signal bindings. The design system layer should *only* map internal computed values to visual CSS classes or CSSOM updates.
2. **Event Delegation:** Forward all pointer and keyboard events down to the base component's `_process_event` functions. Do not intercept and mutate state directly in the visual skin.
3. **Theming:** Use the exposed CSS variables (e.g., `--slider-track-bg` exposed by `ui_slider_base`) to adjust visuals. Let the `_base` component manage structural constraints and ARIA accessibility roles.
