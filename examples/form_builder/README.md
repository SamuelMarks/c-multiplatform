# Runtime Defined Form Builder & AoT Ejection Example

This example demonstrates the complete lifecycle of runtime-defined UI widgets and Ahead-of-Time (AoT) code generation in the engine:

1. **Design Phase (Schema Definition):** A form UI is described as a JSON Schema document (cards, inputs, buttons, validators, and reactive data bindings).
2. **Interpreter Phase (Live Preview):** The schema is dynamically validated and instantiated into live DOM elements via `ui_runtime_build_tree`.
3. **Ejection Phase (AoT Compilation):** The interpreted AST is ejected to native C89 source and header files (`generated_feedback_form.c` and `.h`) using `ui_runtime_eject`.
4. **Production Phase:** The ejected code executes with zero parsing overhead, shedding the JSON parser and dynamic resolution dictionary.
