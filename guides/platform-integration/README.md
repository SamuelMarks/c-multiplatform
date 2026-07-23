# Platform Integration & Bindings

This framework is designed to be embedded and controlled from various host languages and deployed across multiple targets (Native Desktop, Web/WASM, Custom Compositors).

## Core Topics

*   **Language Bindings:**
    *   **[Using the UI framework from Rust](rust-bindings.md)** (`bindings/rust/`)
    *   **[Using the UI framework from Go](go-bindings.md)** (`bindings/go/`)
    *   **[Using the UI framework from C#](csharp-bindings.md)** (`bindings/csharp/`)
*   **[Wayland Compositors](wayland-compositors.md):** Specific architectural guidelines for embedding within Wayland environments.
*   **[WebAssembly (WASM)](wasm.md):** Compiling and deploying the framework to the browser via Emscripten.
*   **[Native OS Dialogs](native-dialogs.md):** Interfacing with native platform dialogs (File Open, Alerts) seamlessly (`ui_os_dialogs.h`).
