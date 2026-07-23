# The Component Lifecycle

Understanding the component lifecycle is essential for managing memory (via arenas), binding to data sources, and triggering efficient repaints. The lifecycle hooks are defined in `ui_component.h`.

## Lifecycle Phases

1. **Creation (`Init`)**: The component memory is allocated (usually from a `ui_arena_t`) and its initial state is populated. It is not yet attached to the DOM.
2. **Mount (`Attach`)**: The component is inserted into the active DOM tree. This is the ideal place to attach event listeners, mutation observers, or subscribe to data sources.
3. **Layout & Paint**: Handled automatically by the engine. The component's layout is calculated based on CSS rules, and it is subsequently painted to the screen.
4. **Update (`Invalidate`)**: Triggered when state changes. Instead of forcing a full repaint, you invalidate specific aspects to ensure high performance: `UI_INVALIDATE_LAYOUT`, `UI_INVALIDATE_PAINT`, or `UI_INVALIDATE_DATA`.
5. **Unmount (`Detach`)**: The component is removed from the DOM tree. **Crucial:** Cleanup external subscriptions and timers here to prevent leaks.
6. **Destruction (`Destroy`)**: Memory is reclaimed. If the application is using `ui_arena.h`, this might be an implicit no-op if the entire arena is being dropped at the end of a scene or frame.

### Lifecycle Hooks (C API)

When defining a custom component, you provide a vtable of lifecycle callbacks:

```c
typedef struct {
    void (*on_mount)(ui_component_t* self);
    void (*on_unmount)(ui_component_t* self);
    void (*on_update)(ui_component_t* self, ui_state_change_t change);
} ui_component_vtable_t;
```

### Example: Proper Unmounting

```c
void my_feed_component_on_mount(ui_component_t* self) {
    // Subscribe to live data when added to the screen
    data_source_subscribe(self->data_feed, self->on_data_received);
}

void my_feed_component_on_unmount(ui_component_t* self) {
    // Must unsubscribe to prevent callbacks to a detached component!
    data_source_unsubscribe(self->data_feed, self->on_data_received);
}
```
