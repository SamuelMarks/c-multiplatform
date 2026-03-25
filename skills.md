# C-Multiplatform Agent Skills & Mandates

This document provides explicit execution "skills" and workflow mandates for Large Language Models and AI agents working in the `c-multiplatform` repository. Adhere to these constraints flawlessly to ensure the codebase remains strictly compatible, leak-free, and idiomatically aligned with its C89 architecture.

---

## Skill 1: Strict C89 (ANSI C) Compliance
You are operating in an environment where modern C conventions will break legacy compilers (like MSVC 6 or ancient embedded GCC).
- **Rule**: Write ONLY standard C89.
- **Comments**: Only block comments (`/* comment */`). Absolutely no `//` inline comments.
- **Variable Declarations**: Variables MUST be declared at the very top of their enclosing block/scope. You cannot declare variables mid-function or inside a `for` loop initialization.
- **Headers**: Never use `<windows.h>` directly to avoid massive namespace pollution. Guard POSIX or C99 headers (like `<stdint.h>`) with compiler version checks.
- **Format**: Wrap include blocks in `/* clang-format off */` and `/* clang-format on */` to prevent auto-sorters from breaking include dependency orders. Wrap public headers in `extern "C"`.

## Skill 2: Memory Management & Leak Tracking
LibCMPC manages its own memory to guarantee zero GC pauses and catch leaks at runtime.
- **Rule**: NEVER use `malloc`, `calloc`, `realloc`, or `free` directly.
- **Implementation**: 
  - For standard dynamic heap allocations, use `CMP_MALLOC(size, &out_ptr)` and `CMP_FREE(ptr)`. These macros track allocations with file/line numbers.
  - For scoped or frame-based data, utilize the arena allocator: `cmp_arena_alloc(&arena, size, &out_ptr)`.
  - For high-frequency, fixed-size structs (like layout calculations or UI nodes), use the pool allocator: `cmp_pool_alloc(&pool, &out_ptr)`.
- **Validation**: Ensure all allocated memory has a clear ownership model and is explicitly freed. Tests run `cmp_mem_check_leaks()` at the end, and the CI will fail if it returns > 0.

## Skill 3: Error Handling & Function Signatures
LibCMPC uses a strict return-code contract to propagate errors safely without exceptions.
- **Rule**: All logic functions must return an `int` indicating success or failure.
- **Implementation**:
  - `0` or `CMP_SUCCESS` indicates success.
  - Non-zero values (`CMP_ERROR_OOM`, `CMP_ERROR_INVALID_ARG`, `CMP_ERROR_NOT_FOUND`) indicate failure.
  - Always return output data via pointer arguments.
  - **Example**: `int cmp_ui_box_create(cmp_ui_node_t **out_node)` NOT `cmp_ui_node_t* cmp_ui_box_create()`.

## Skill 4: UI & Layout Construction
The framework completely decouples the logical UI tree, the mathematical Layout tree, and the Graphical renderer.
- **Rule**: Never manually calculate or hardcode pixel coordinates for UI elements.
- **Implementation**:
  - Construct the UI tree using `cmp_ui_node_t` elements (`cmp_ui_box_create`, `cmp_ui_text_create`, `cmp_ui_button_create`).
  - Append children using `cmp_ui_node_add_child`.
  - Apply styling and dimensions via the Flexbox properties on the UI nodes.
  - The framework's `cmp_layout_node_t` engine will automatically calculate X/Y bounds during the tick phase.
  - Bind reactive data to `c-orm` observables using `cmp_ui_node_bind`.

## Skill 5: Modality & Concurrency Engine
LibCMPC does not assume the host OS threading model.
- **Rule**: Never block the main thread and never assume a single threading model.
- **Implementation**:
  - When writing asynchronous features (like network or disk I/O), dispatch work to the active `cmp_modality_t` instance using `cmp_modality_queue_task`.
  - Use `cmp_ring_buffer_t` for thread-safe cross-thread messaging without Mutex locking.
  - Utilize `cmp_tls.c` for Thread-Local Storage when managing context in `CMP_MODALITY_THREADED` mode.

## Skill 6: Using the Amalgamated API (`cmp.h`)
To maintain context-window scalability, the entire public API is consolidated.
- **Rule**: `include/cmp.h` is the ultimate source of truth.
- **Implementation**: When asked to implement a feature, always scan `include/cmp.h` first for existing interfaces, helper structs, and enums. Do not invent duplicate functionality if `cmp_string`, `cmp_timer`, or `cmp_vfs` already support the use case. If you need to modify the public API, you MUST update `include/cmp.h`.

## Skill 7: Testing & Verification
A change is not complete until it is verified.
- **Rule**: Always write or update tests using the `greatest` framework.
- **Implementation**: Tests are located in the `tests/` directory. Each system has a dedicated file (e.g., `test_cmp_memory.c`, `test_cmp_ui.c`). After implementing a feature, write a test case, run `ctest`, and ensure `cmp_mem_check_leaks()` reports 0 un-freed allocations.