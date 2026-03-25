# Execution Modalities and Concurrency Models

## Introduction and Tradeoffs

When building a cross-platform UI framework in C, providing flexibility in how the application executes and handles concurrency is paramount. Different applications have vastly different requirements. A simple utility might be fine with a single-threaded synchronous blocking loop, whereas a complex GUI with network requests, disk I/O, and background processing will benefit immensely from an asynchronous event loop, multithreading, or even multiprocess architecture.

Offering these modalities as an `enum` choice to the framework user allows them to tailor the runtime environment to their specific needs, much like how one might choose an execution policy in modern software design.

### Synchronous vs. Asynchronous
*   **Synchronous**: Execution flows sequentially. If a task blocks (e.g., waiting for file I/O or network), the entire thread blocks.
    *   *Tradeoffs*: Extremely simple to reason about and debug. State changes are predictable. However, it leads to unresponsive UIs if any operation takes longer than a frame (typically 16ms).
*   **Asynchronous**: Execution can be paused and resumed. The main thread continues to process events while waiting for operations to complete.
    *   *Tradeoffs*: Keeps the UI responsive. Ideal for network-heavy or I/O-bound applications (like the Node.js event loop model). The downside is increased complexity in state management (callback hell, state machines, or the need for promises/futures/coroutines).

### Single-threading vs. Multithreading
*   **Single-threading**: Only one sequence of instructions is executed at a time.
    *   *Tradeoffs*: No concurrency bugs like race conditions or deadlocks (mostly). Memory management is simpler. However, it cannot utilize multi-core processors for CPU-bound tasks, potentially leading to bottlenecks.
*   **Multithreading**: Multiple threads run concurrently, sharing the same memory space.
    *   *Tradeoffs*: Excellent for offloading heavy CPU computations (e.g., image processing, physics calculations) from the main UI thread. Utilizes multi-core CPUs efficiently. The major drawback is the extreme complexity introduced by shared state, requiring mutexes, semaphores, and atomic operations to prevent race conditions and deadlocks.

### Single-process vs. Multiprocess
*   **Single-process**: The entire application runs within a single operating system process.
    *   *Tradeoffs*: Low overhead for starting up and sharing data (since memory is shared). A crash in any part of the application brings down the whole application.
*   **Multiprocess**: The application is split into multiple independent processes (e.g., a main UI process and a separate background daemon process, similar to Chromium's architecture).
    *   *Tradeoffs*: High fault tolerance; if a background process crashes, the UI survives and can restart it. Excellent security isolation. The downside is the high overhead of Inter-Process Communication (IPC) and memory overhead.

### Greenthreads (Fibers / Coroutines)
*   **Greenthreads**: User-space threads managed by a runtime library or virtual machine rather than the OS.
    *   *Tradeoffs*: Extremely lightweight context switching. You can spawn millions of them compared to thousands of OS threads. They are excellent for highly concurrent I/O-bound tasks. However, if a greenthread makes a blocking system call, it might block the underlying OS thread, stalling other greenthreads (unless the framework intercepts and makes these calls asynchronous).

### Message Passing
*   **Message Passing**: Threads or processes communicate by sending discrete messages rather than sharing memory (e.g., the Actor model, Go's channels).
    *   *Tradeoffs*: Drastically reduces the risk of race conditions and deadlocks since state is not shared. Forces a decoupled architecture. The downside is the overhead of copying or moving data into messages and the cognitive shift required to design around message queues.

## Architectural Modifications & `c-abstract-http` Integration

To support this diverse set of execution modalities via a simple configuration choice (e.g., `cmp_init(CMP_MODALITY_ASYNC_EVENT_LOOP)`), LibCMPC offloads this complexity to its sibling library, **`c-abstract-http`**. While primarily a network library, `c-abstract-http` provides the robust, cross-platform concurrency engines required by LibCMPC.

1.  **The Event Loop Abstraction**: The core of the framework is an abstract `EventDispatcher` deeply bridged 1:1 with `c-abstract-http`'s non-blocking event loop (mimicking Node.js `uv_run`).
    *   If `CMP_MODALITY_SYNC` is chosen, the dispatcher just polls the OS for UI events and blocks until one arrives.
    *   If `CMP_MODALITY_ASYNC_EVENT_LOOP` is chosen, the dispatcher integrates with `c-abstract-http`'s native asynchronous backends (epoll, kqueue, IOCP, or libuv fallback) to process UI events, I/O callbacks, and timers in a single unified loop.

2.  **Pluggable Schedulers**: We utilize `c-abstract-http`'s internal `Scheduler` interfaces.
    *   For multithreading, it utilizes the cross-platform thread pool architecture with robust Mutex and CondVar abstractions.
    *   For single-threading, tasks are queued to be run on the main event loop during idle time.
    *   For Greenthreads, it leverages Fibers (Windows) and `ucontext_t` (POSIX).

3.  **Unified Messaging Bus**: To support both Multithreading and Multiprocessing (and Message Passing), LibCMPC leverages `c-abstract-http`'s core `MessageBus`.
    *   Within the same process, the decentralized actor model and pub-sub bus route requests scalably.
    *   Across processes, it utilizes built-in C89 binary IPC serialization over anonymous pipes.

4.  **API Design**: The user configuration will look something like this:
    ```c
    typedef enum {
        CMP_MODALITY_SYNC_SINGLE,         // Traditional blocking game loop
        CMP_MODALITY_SYNC_MULTI,          // Main UI loop blocks, has a thread pool
        CMP_MODALITY_ASYNC_SINGLE,        // Node.js style, single threaded event loop
        CMP_MODALITY_ASYNC_MULTI,         // Event loop main thread + worker threads
        CMP_MODALITY_GREENTHREADS,        // Go-like execution with fibers
        CMP_MODALITY_MULTIPROCESS_ACTOR   // Erlang/Chromium style isolated processes
    } cmp_modality_t;

    cmp_app_config_t config = { .modality = CMP_MODALITY_ASYNC_SINGLE };
    cmp_app_init(&config);
    ```

## 50-Step Implementation Plan

- [ ] 1. **Research & Feasibility**: Evaluate libuv, libev, or native OS APIs (epoll/kqueue/IOCP) for the foundational asynchronous event loop.
- [ ] 2. **API Definition**: Define the `cmp_modality_t` enum in `cmp_core.h`.
- [ ] 3. **Config Struct**: Update `cmp_app_config_t` to include the execution modality enum.
- [ ] 4. **Core Loop Refactor**: Abstract the existing `while(running)` UI loop into a `cmp_run_loop()` function pointer or interface.
- [ ] 5. **Sync Single-Thread Impl**: Implement the `CMP_MODALITY_SYNC_SINGLE` backend (classic blocking poll loop).
- [ ] 6. **Task Queue Core**: Create a lock-free (or mutex-guarded) thread-safe FIFO queue for tasks/messages.
- [ ] 7. **Thread Pool API**: Define `cmp_thread_pool_init`, `_push`, `_destroy` in `cmp_tasks.h`.
- [ ] 8. **Worker Thread Impl**: Implement POSIX/Windows threads for the worker pool.
- [ ] 9. **Sync Multi-Thread Impl**: Wire up `CMP_MODALITY_SYNC_MULTI` to initialize the thread pool alongside the sync UI loop.
- [ ] 10. **Event Loop Abstraction**: Define an interface for `cmp_event_loop_t` (timers, file descriptors, UI events).
- [ ] 11. **libuv Integration (Optional/Core)**: Integrate a high-performance event loop library (or write a custom abstraction layer).
- [ ] 12. **Async API Design**: Create non-blocking APIs for file I/O (e.g., `cmp_fs_read_async(path, callback)`).
- [ ] 13. **Async Network API**: Create non-blocking APIs for networking (e.g., `cmp_net_fetch(url, callback)`).
- [ ] 14. **Timer Integration**: Map `cmp_set_timeout` and `cmp_set_interval` to the underlying event loop.
- [ ] 15. **UI Event to Async**: Ensure native OS UI events (mouse, keyboard, paint) are pushed into the async event loop safely.
- [ ] 16. **Async Single-Thread Impl**: Implement `CMP_MODALITY_ASYNC_SINGLE` to run the integrated event loop (the "Node.js" model).
- [ ] 17. **Cross-Thread Wakeup**: Implement a mechanism (e.g., `eventfd` or loopback socket) for worker threads to wake up the main async event loop.
- [ ] 18. **Async Multi-Thread Impl**: Combine the async event loop with the thread pool for `CMP_MODALITY_ASYNC_MULTI`.
- [ ] 19. **Message Structure**: Define a standard `cmp_msg_t` payload structure for message passing.
- [ ] 20. **Message Bus API**: Define `cmp_msg_subscribe` and `cmp_msg_publish`.
- [ ] 21. **In-Process Messaging**: Implement the message bus using shared memory and queues for inter-thread communication.
- [ ] 22. **Actor Model Prototyping**: Create an API for spawning "Actors" (stateful message handlers).
- [ ] 23. **Actor Supervision**: Implement basic supervision trees for Actors (restart on failure).
- [ ] 24. **Greenthread Research**: Evaluate C coroutine libraries (e.g., libco, boost::context ported to C, or ucontext.h).
- [ ] 25. **Fiber Context Switch**: Implement assembly or OS-level context switching for greenthreads.
- [ ] 26. **Greenthread Scheduler**: Build a round-robin scheduler to multiplex greenthreads over OS threads.
- [ ] 27. **Yielding API**: Implement `cmp_yield()` to pause a greenthread and return control to the scheduler.
- [ ] 28. **Async I/O to Yielding**: Hook async I/O callbacks to resume yielding greenthreads (making async look synchronous).
- [ ] 29. **Greenthread Modality Impl**: Integrate the scheduler into `CMP_MODALITY_GREENTHREADS`.
- [ ] 30. **Multiprocess IPC Research**: Evaluate named pipes, Unix domain sockets, and shared memory for IPC.
- [ ] 31. **Process Forking/Spawning**: Implement `cmp_process_spawn()` abstracting `fork()` or `CreateProcess()`.
- [ ] 32. **IPC Message Serialization**: Write a lightweight binary serializer for `cmp_msg_t` to send over IPC.
- [ ] 33. **IPC Channel Impl**: Implement the communication channels between the main UI process and worker processes.
- [ ] 34. **Multiprocess Modality Impl**: Wire up `CMP_MODALITY_MULTIPROCESS_ACTOR` to transparently route messages over IPC.
- [ ] 35. **Shared Memory Allocator**: Implement a specific allocator for sharing large data (e.g., images, buffers) between processes to avoid copy overhead.
- [ ] 36. **Synchronization Primitives**: Abstract Mutexes, Semaphores, and Condition Variables across all supported platforms.
- [ ] 37. **Atomic Operations**: Provide a wrapper for C11 stdatomic.h or compiler intrinsics.
- [ ] 38. **Memory Ownership Semantics**: Document and enforce rules for who owns memory passed between threads/processes/actors.
- [ ] 39. **Deadlock Detection**: (Optional Debug Tool) Add instrumentation to detect cyclic dependencies in mutex locks.
- [ ] 40. **Modality Switching (Runtime vs Startup)**: Enforce that modality is strictly a startup configuration and cannot be changed on the fly.
- [ ] 41. **Test Suite - Sync**: Write extensive unit tests asserting behavior in synchronous modes.
- [ ] 42. **Test Suite - Async**: Write tests ensuring callbacks fire correctly in the async event loop.
- [ ] 43. **Test Suite - Threading**: Write stress tests for the thread pool to catch race conditions.
- [ ] 44. **Test Suite - IPC**: Write tests validating message serialization and multiprocess communication.
- [ ] 45. **Example App - Node.js Clone**: Create a sample app demonstrating the asynchronous event loop serving HTTP requests while rendering UI.
- [ ] 46. **Example App - Heavy Compute**: Create a sample app demonstrating multithreading (e.g., real-time raytracing or fractal generation) keeping the UI smooth.
- [ ] 47. **Example App - Resilient Actors**: Create a multiprocess example where background workers intentionally crash and are revived by the main process.
- [ ] 48. **Documentation - Modalities**: Write detailed guides in `docs/` explaining when to use each modality.
- [ ] 49. **Documentation - API Ref**: Update API reference for all new concurrency, task, and messaging functions.
- [ ] 50. **Final Review & Profiling**: Audit the event loop and thread pool overhead to ensure performance regressions are not introduced in simple modalities.