# C-Multiplatform Architecture & Standards

**Purpose:** This document details the high-level architecture, memory management, and engine design of the LibCMPC framework.
**Current State:** The repository contains a robust, monolithic core for building cross-platform GUI applications natively in C89, featuring a custom Flexbox layout engine and deep ecosystem integrations.
**Upcoming Features:** We are actively architecting the rendering pipeline to natively support **Google Material 3**, **Apple Cupertino / HIG**, and **Microsoft Fluent 2** design systems out-of-the-box.

LibCMPC has undergone a strategic rewrite to prioritize modality agnosticism, memory safety, and tight integration with a suite of C89 ecosystem libraries.

## Core Foundation & Memory
All memory allocations in LibCMPC are routed through custom allocators to ensure zero leaks.
- **Arena Allocator (`cmp_arena_t`)**: Used for per-frame or scoped allocations that can be discarded en-masse. 
- **Pool Allocator (`cmp_pool_t`)**: Fixed-size block allocation for high-frequency objects like UI nodes and layout calculations.
- **Leak Tracking**: In debug builds, `CMP_MALLOC` and `CMP_FREE` automatically track allocations via `cmp_mem_record_t`, enabling `cmp_mem_check_leaks()` to report exact file and line numbers of un-freed memory.

## Modality Engine (`cmp_modality_t`)
The core innovation of LibCMPC is its modality-agnostic event loop.
- **`CMP_MODALITY_SINGLE`**: A traditional blocking/polling loop suitable for simple games or legacy targets.   
- **`CMP_MODALITY_THREADED`**: Spawns a worker pool, safely dispatching tasks and UI events across threads using the lock-free `cmp_ring_buffer_t`.
- **`CMP_MODALITY_ASYNC`**: Integrates with OS-native asynchronous APIs (epoll/kqueue/IOCP).

## Ecosystem Integrations
Instead of reinventing the wheel, LibCMPC deeply embeds specialized C libraries:
1. **Virtual File System (`cmp_vfs_t` -> `c-fs`)**: Allows mounting archives (ZIPs) and native directories. `cmp_vfs_read_file_async` ties directly into the Modality engine to prevent blocking the main thread during asset loading.
2. **Network (`cmp_http` -> `c-abstract-http`)**: WebSockets, Server-Sent Events, and standard REST HTTP calls are dispatched and resolved within the `cmp_modality_t` loop.
3. **State Management (`cmp_orm` -> `c-orm`)**: UI nodes can be directly bound to ORM fields using `cmp_ui_node_bind(node, observable)`. When the local SQLite database updates, the UI invalidates and redraws automatically.  

## UI & Layout Pipeline
1. **UI Tree (`cmp_ui_node_t`)**: Developers construct a logical tree of widgets (`cmp_ui_box`, `cmp_ui_button`, `cmp_ui_text_input`).
2. **Layout Tree (`cmp_layout_node_t`)**: The UI tree generates a parallel Flexbox layout tree. `cmp_layout_calculate` resolves all absolute pixel coordinates based on the available window size.
3. **Window System (`cmp_window_t`)**: The calculated UI tree is bound to an OS window via `cmp_window_set_ui_tree`. Events (clicks, typing) are routed via `cmp_event_t` down the tree to focused nodes.

## Rendering Abstraction
Rendering is decoupled from the windowing system via `cmp_renderer_create`. This allows the same UI tree to be drawn using SDL3, Native Win32 GDI, Apple Metal, or WebGL without changing the UI code.
