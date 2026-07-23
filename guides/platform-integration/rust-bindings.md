# Rust Bindings

The framework provides idiomatic, safe Rust bindings located in `bindings/rust/`. This allows you to build high-performance, memory-safe application logic in Rust while leveraging the C framework's ultra-fast rendering pipeline.

## Getting Started

To use the framework in a Rust project, add the local path to your `Cargo.toml`:

```toml
[dependencies]
ui_framework = { path = "../../bindings/rust/ui_framework" }
```

## Memory Management & Safety

The Rust bindings carefully map the C framework's `ui_arena.h` to Rust lifetimes. Components allocated in an arena are tied to that arena's lifetime, preventing use-after-free errors at compile time.

```rust
use ui_framework::{Engine, Arena, components::Button};

fn build_ui(arena: &Arena) {
    // The button's lifetime is tied to `arena`
    let btn = Button::new(arena);

    btn.set_text("Click Me");
    btn.on_click(|_event| {
        println!("Button was clicked!");
    });
}
```

## Constructing the DOM

The bindings use a builder pattern that mimics the C API but feels native to Rust.

```rust
use ui_framework::{Arena, components::{Container, Text, Button}};

fn create_card<'a>(arena: &'a Arena, title: &str) -> Container<'a> {
    let mut card = Container::new(arena);
    card.add_class("styled-card");

    let title_node = Text::new(arena, title);
    let action_btn = Button::new(arena).with_text("Action");

    card.append(title_node);
    card.append(action_btn);

    card
}
```

## Interoperating with C APIs

If you need to drop down to the raw C API (for example, to use a specialized header not yet fully wrapped in safe Rust), you can access the underlying raw pointers safely via `.as_raw()`:

```rust
use ui_framework::sys; // Raw FFI bindings

let raw_ptr: *mut sys::ui_component_t = my_rust_button.as_raw();

unsafe {
    // Calling a C function directly
    sys::ui_aria_set_role(raw_ptr, sys::UI_ARIA_ROLE_BUTTON);
}
```
