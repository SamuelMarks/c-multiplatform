# Architecture Plan: Runtime Defined Widgets & AoT Ejection

This plan details the implementation strategy for supporting runtime-defined widgets (e.g., for a form builder or remote UI delivery) that can be instantly rendered in the engine and subsequently "ejected" to raw C code for Ahead-of-Time (AoT) compilation.

## 1. Schema & Component Introspection
To dynamically construct widgets from a serialized format (JSON/XML) while adhering to the engine's strict memory and threading models, we need a lightweight introspection layer.

- [ ] **Define the UI Schema Format:** Create a formalized schema (e.g., JSON) representing the widget tree. This format must be governed by a formal, verifiable **JSON Schema definition** with strict `additionalProperties: false` (or equivalent) across all objects. Rejecting unknown fields prevents silent failures from typos (e.g., `bordr_radius`), simplifies the C-side parser, and guarantees structural 1:1 mapping with the target C structs. It must capture:
  - **Component Types & Layouts:** Standard widgets, plus layout primitives (`row`, `column`, `grid`) that map directly to the engine's CSS layout algorithms. *When generated from OpenAPI, these are driven by `x-ui-widget` extensions.*
  - **Static Properties:** (`props`) such as text, colors, and default sizes. *Can be mapped via `x-ui-props` extensions.*
  - **Reactive Bindings:** (`bind_signal`, `bind_cva`) mapping to the dynamic data context.
  - **Validation & Conditional Logic:** Visibility rules, regex patterns, and required fields. *These must be structurally compatible with OpenAPI 3.1 Schema Objects to allow deriving UI validation directly from API definitions.*
  - **Event Hooks & Workflows:** Action chains mapping to UI events (e.g., `on_click`). *The workflow structure should align with the Arazzo Specification to standardize how sequences of API calls, data extraction, and state mutations are defined.*
  - **Routing Definitions:** A manifest of application pages/screens and their hierarchical or flat relationships.
- [ ] **Component Registry (`ui_component_registry`):** Implement a global or thread-local registry that maps schema strings (e.g., `"ui_button_base"`) to their factory functions and binding vtables.
  - *Example:* Registering `"ui_input_base"` will store function pointers to `ui_input_base_create` and its CVA extraction logic.
- [ ] **Property Type Coercion:** Build utilities (leveraging `ui_coercion_utils.h`) to convert string-based schema values into native C types (enums, ints, floats, `ui_color`) required by the `bind_<property>` functions.

## 2. Dynamic Data Context, App State & Routing
Runtime widgets cannot use hardcoded C variables for their reactive state. We need a robust dictionary-based context and state manager to bridge string identifiers from the schema to actual native instances.

- [ ] **Global App State (`ui_app_state_registry`):** Create a global registry to hold top-level application state (global signals, user sessions, theme configurations) so widgets on different pages can communicate seamlessly.
- [ ] **Dynamic Context (`ui_dynamic_context`):** Create an arena-allocated dictionary for localized scope (page-level or component-level) that stores named signals, computed values, and form groups.
- [ ] **Form Control Resolution:** Implement logic to parse `bind_cva: "user.email"` from the schema, look up the `"user"` form group in the dynamic context, extract the `"email"` control, and bind it to the instantiated widget's CVA interface.
- [ ] **Signal Resolution & Widget Linking:** Parse `bind_text: "app.cart_count"`. This enables widget-to-widget linking (e.g., a "Add to Cart" button's workflow updates the global `app.cart_count` signal, automatically updating the Header's Badge component).
- [ ] **Dynamic Router (`ui_runtime_router`):** Implement a navigation controller that reads the schema's routing manifest, managing push/pop transitions between dynamically constructed DOM trees representing different screens.

## 3. Runtime Builder (Interpreter)
The interpreter consumes the schema and constructs the live DOM tree during the `ui_execution_context` tick loop.

- [ ] **Strict Schema Validation:** Before instantiation, validate incoming payloads against the formal JSON Schema. Any payload containing unregistered components, mismatched types, or undeclared additional fields MUST be hard-rejected, safely bubbling an error enum up to the host application instead of attempting partial renders.
- [ ] **Schema Parser:** Integrate a lightweight JSON/XML parser to deserialize the UI Schema into an internal AST (Abstract Syntax Tree) allocated on a transient `ui_arena`.
- [ ] **Tree Instantiation (`ui_runtime_build`):**
  1. Recursively traverse the AST.
  2. Query the `ui_component_registry` to allocate the C structs (e.g., `ui_card_base`, `ui_row`, `ui_column`).
  3. Resolve properties and apply one-way bindings (Presentational Widgets).
  4. Resolve and attach CVAs (Form Controls) to the dynamic context.
  5. Assemble the internal DOM hierarchy ensuring flex/grid layout constraints are applied.
- [ ] **Multi-platform Canvas Previews (`ui_runtime_preview_viewport`):** Add support to render the interpreted DOM within a simulated sandbox. Expose an API to artificially constrain the `<html>`/`<body>` dimensions (e.g., 390x844 for iOS, 1920x1080 for Desktop) overriding the true OS window dimensions, enabling WYSIWYG editors to show responsive reflows live.
- [ ] **Event Bridging & Workflow Execution:** Map named event hooks in the schema (e.g., `"on_click"`) to a unified workflow engine. This engine executes sequential action blocks using **Arazzo Specification** semantics. Instead of proprietary action arrays, the engine processes standardized Arazzo `steps`—making HTTP requests, parsing responses via `outputs`, evaluating `successCriteria`, mutating the `ui_app_state_registry`, or triggering `ui_runtime_router` navigation—without needing recompiled C code.

## 4. AoT Code Generator (Ejector)
To achieve zero-overhead performance on subsequent runs, the runtime format must be ejectable to native C code. This process bypasses the schema parser, string lookups, and dynamic context entirely.

- [ ] **Ejector Traversal (`ui_runtime_eject`):** Write a code generator that traverses the loaded UI Schema AST.
- [ ] **C Code Emission:** For each node in the AST, output the exact C API calls as strings to a file (`.c` and `.h`).
  - *Example output for instantiation:*
    ```c
    struct ui_input_base *input_email = NULL;
    struct ui_control_value_accessor *cva_email = NULL;

    /* Engine standard goto cleanup enforcement */
    if (ui_input_base_create(&input_email, &cva_email) != UI_SUCCESS) goto cleanup;

    struct ui_form_control *ctrl_email = ui_form_group_get_control(form, "email");
    ui_form_control_bind_cva(ctrl_email, cva_email);
    ui_dom_append_child(parent_node, input_email->node);
    ```
  - *Example output for Conditional Logic (Skip Logic):*
    ```c
    /* Emitting reactive bindings for conditional visibility */
    struct ui_computed *show_q2 = ui_computed_create(engine, ...); /* logic extracted from schema */
    ui_component_bind_visibility(q2_container, show_q2);
    ```
  - *Example output for Validation:*
    ```c
    /* Emitting native validator mappings */
    ui_form_control_add_validator(ctrl_email, ui_validators_required());
    ui_form_control_add_validator(ctrl_email, ui_validators_pattern("^[\\w-\\.]+@([\\w-]+\\.)+[\\w-]{2,4}$"));
    ```
- [ ] **Identifier Generation:** Generate unique, safe C variable names based on the schema's IDs or hierarchy depth to prevent symbol collisions in the generated code.
- [ ] **Memory & Error Handling Emission:** Ensure the generated code strictly adheres to the engine's `goto cleanup` error percolation strategy. Every generated `_create` call must check for `NULL` or failure enum and jump to a generated cleanup block, ensuring safe memory management under the Arena/Pool models.
- [ ] **C89 & Quality Plan Compliance (Emission):** The generated `.c` and `.h` files must strictly adhere to the `PLAN_QUALITY.md` constraints. This includes emitting C89-compliant code, wrapping generated headers in `extern "C"` for C++ interop, utilizing MSVC Safe CRT fallbacks conditionally, injecting Doxygen-compliant comments for the exported mount functions, and wrapping generated `#include` blocks in `/* clang-format off */` / `/* clang-format on */`.

## 5. Isomorphic Execution Model (Runtime & AoT Parity)
A fundamental requirement of this architecture is strict behavioral equivalence between the interpreted and compiled states. The host application must work seamlessly in either mode without changing its core integration logic.

- [ ] **Unified App Loader (`ui_app_load`):** Expose a single abstraction for mounting a UI tree. In Runtime mode, this accepts a serialized schema payload. In AoT mode, it accepts the root function pointer of the generated C code. The host application's initialization code remains completely agnostic.
- [ ] **1:1 Native Mapping Guarantee:** The `ui_runtime_build` interpreter must strictly utilize the *exact same public C API functions* that the `ui_runtime_eject` generator emits. There are no "interpreter-only" layout algorithms or state shortcuts.
- [ ] **Unified Data Ingestion:** Supplying external data (e.g., API responses, local storage) to the UI must behave identically. Both Runtime and AoT modes rely on the same `ui_app_state_registry` and native `ui_signal_set()` calls to hydrate the UI.
- [ ] **Hybrid Operation (Remote Over-The-Air Updates):** An application compiled entirely AoT must retain the ability to mount dynamically downloaded Runtime schema snippets. For instance, a statically compiled C dashboard can fetch a JSON schema from a server to render a custom promotional banner, seamlessly integrating the dynamic nodes into the native DOM tree.

## 6. Integration & Workflow
- [ ] **Form Builder Example:** Create a new example application under `examples/form_builder/` demonstrating the full lifecycle:
  1. User builds a UI visually (which generates the JSON Schema).
  2. The application renders the schema live using `ui_runtime_build`.
  3. The application triggers `ui_runtime_eject` to write `generated_form.c`.
  4. The generated file is compiled and linked directly in the next build, providing native execution speed without the runtime parsing overhead.

## 7. Flagship Use Case 1: Survey Maker & Production Lifecycle
To validate the architecture, the engine will support a complete "Survey Maker" application lifecycle demonstrating the synergy between runtime interpretation and AoT compilation.

- **Phase 1: Design (Builder Mode)**
  - A visual drag-and-drop builder constructs a multi-page survey schema.
  - The schema encodes pagination rules, conditional skip logic (e.g., "If Q1=No, skip Q2"), and validation requirements.
  - The builder uses `ui_runtime_build` for instant previewing.
- **Phase 2: Beta Testing (Runtime Execution)**
  - The JSON schema is distributed directly to beta testers without recompiling the client app.
  - The beta application parses the schema on launch, evaluates conditional logic dynamically via the `ui_dynamic_context`, and submits serialized data that structurally matches the final payload.
  - This allows rapid, iteration loops (e.g., tweaking questions or adding options) purely via remote schema updates.
- **Phase 3: Production (AoT Ejection)**
  - Once the survey is finalized, the schema is run through `ui_runtime_eject`.
  - The code generator emits raw C code, directly wiring up native validation functions and creating native `ui_computed` nodes for the conditional skip logic.
  - This emitted C code is compiled into the production binary.
  - **Result:** The production survey achieves perfect zero-overhead parsing, minimal binary size, and native 60fps performance, completely shedding the JSON parser and dynamic resolution dictionary.

## 8. Flagship Use Case 2: Full Low-Code App Builder
Beyond single-page forms, the architecture supports building an entire visual studio (similar to FlutterFlow or Retool).

- **Phase 1: Visual Development (Canvas Mode)**
  - A comprehensive Web/Desktop IDE uses the C engine internally to render the canvas.
  - Users drag-and-drop structural components (`ui_row`, `ui_column`) and widgets.
  - The `ui_runtime_preview_viewport` instantly updates the layout engine for Mobile, Tablet, or Desktop form factors without recompiling.
  - Users map widget interactions (e.g., clicking a list item) to global `ui_app_state_registry` changes or page navigation via the Action/Workflow engine.
- **Phase 2: App Previewing (Interpreter Mode)**
  - Users can launch a "Live Test" which feeds the entire multi-page App Schema into `ui_runtime_router` and `ui_runtime_build`.
  - Global state signals sync in real-time. This provides an exact 1:1 behavioral representation of the final app natively.
- **Phase 3: Native Ejection (Compilation)**
  - The developer clicks "Export App".
  - The `ui_runtime_eject` system traverses the entire schema, generating a native `main.c` containing a hardcoded static routing table, statically allocated global `ui_signal` structures for app state, and native C functions for every screen's widget tree.
  - **Result:** An entire multi-screen application, designed purely visually, compiled into a highly performant, statically-linked, zero-dependency C binary for Windows/Mac/Linux/iOS/Android with a microscopic memory footprint.

## 9. Flagship Use Case 3: API-Driven Auto-Generated Interfaces
By aligning the engine's data context and workflow execution with OpenAPI and Arazzo specifications, the architecture unlocks powerful automated tooling and Data-Driven UI use cases.

- **Phase 1: OpenAPI Ingestion & Form Generation**
  - A developer points the CLI/Builder at an existing `openapi.yaml`.
  - The tooling reads an endpoint (e.g., `POST /users`) and automatically generates the UI JSON schema for a form.
  - **Widget Directives (UI Hints):** While the generator infers sensible default widgets (e.g., an `enum` becomes a `ui_select_base` dropdown, a `boolean` becomes a `ui_checkbox_base`), developers can explicitly direct the UI generation using OpenAPI vendor extensions. By annotating the API schema with fields like `x-ui-widget: "radio_group"` or `x-ui-layout: "row"`, the tooling maps the data directly to the desired presentation component without altering the API contract. For APIs outside the developer's control, a sidecar "UI-Schema" JSON overlay can be merged during generation.
  - Because our schema's validation nodes are compatible with OpenAPI 3.1 Schema Objects, the generated UI instantly enforces `maxLength`, `pattern` (regex), and `required` fields natively in the C engine without any manual data mapping.
- **Phase 2: Arazzo Workflow Orchestration**
  - The developer imports an `arazzo.yaml` specification defining a complex sequence of API calls (e.g., "Create User", extract `userId`, then "Assign Role").
  - The workflow engine automatically wires these steps to the submit button's event hook. It processes the sequence dynamically at runtime, using the Arazzo `successCriteria` to determine if it should navigate to the success screen or show an error `ui_alert_base`.
- **Phase 3: Interoperable Tooling**
  - Because the UI schema's data bindings and actions follow these standard specs, third-party API linters, documentation generators, and mock-servers work out-of-the-box with the UI's definitions.

## 10. Quality Assurance, Testing & Bindings Mandates
To satisfy the 100% coverage and architectural mandates outlined in `PLAN_QUALITY.md`, the runtime and AoT systems must implement rigorous verification pipelines:

- [ ] **Runtime vs. AoT Equivalence Assertions:** Using the Headless E2E Automation framework, implement automated test suites that mount a UI schema dynamically (`ui_runtime_build`) and simultaneously mount the compiled output of `ui_runtime_eject` for the exact same schema. The resulting DOM trees (box geometry, text styling, and reactive signal state) must mathematically match 100%.
- [ ] **Parser Error Path & Fuzzing (Zero-Leak):** The dynamic JSON/XML schema parser must undergo strict error path stress testing (mocking `malloc` failures during string parsing, supplying malformed JSON structures, providing invalid component names) to mathematically prove the Arena allocator and `goto cleanup` percolation prevents memory leaks (verified via ASAN and Valgrind).
- [ ] **Dynamic Context Memory Safety:** The `ui_dynamic_context` bridging dictionary must be exhaustively tested to ensure dynamically bound form controls and signals correctly sever their references during unmount, preventing dangling pointers when navigating between pages in Runtime mode.
- [ ] **FFI Bindings for Schema Injection:** The unified `ui_app_load` function must be safely exposed via automated FFI bindings (Rust's `sys` crate, C#'s `DllImport`, Python/Go extensions). This allows host applications written in managed languages to push JSON schema payloads or pre-compiled C function pointers down into the native engine seamlessly across the language boundary.
