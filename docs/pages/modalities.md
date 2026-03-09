\page modalities Execution Modalities & Concurrency

# Execution Modalities

LibCMPC uniquely separates **what** your application does from **how** it executes. By decoupling the execution model from the user interface logic, you can seamlessly scale your application from a single-threaded game loop to a highly concurrent multiprocess architecture without rewriting your UI code.

The execution model is determined at startup by configuring the `CMPModality` enum during application initialization.

```c
typedef enum {
    CMP_MODALITY_SYNC_SINGLE,         /**< Traditional blocking game loop */
    CMP_MODALITY_SYNC_MULTI,          /**< Main UI loop blocks, background thread pool */
    CMP_MODALITY_ASYNC_SINGLE,        /**< Node.js style, single threaded event loop */
    CMP_MODALITY_ASYNC_MULTI,         /**< Event loop main thread + worker threads */
    CMP_MODALITY_GREENTHREADS,        /**< Go-like execution with cooperative fibers */
    CMP_MODALITY_MULTIPROCESS_ACTOR   /**< Erlang/Chromium style isolated processes */
} CMPModality;
```

## Choosing a Modality

### 1. Synchronous Models (`SYNC_SINGLE`, `SYNC_MULTI`)
Ideal for simpler applications, embedded systems, or games.
- **SYNC_SINGLE:** The absolute simplest model. Everything executes sequentially. The UI blocks when you perform I/O.
- **SYNC_MULTI:** The main thread remains a simple blocking loop, but a thread pool is initialized to offload heavy CPU work.

### 2. Asynchronous Models (`ASYNC_SINGLE`, `ASYNC_MULTI`)
Ideal for network-heavy applications, desktop tools, and web-based (Emscripten) deployments.
- **ASYNC_SINGLE:** The "Node.js" model. All execution runs on one thread, but I/O and timers are non-blocking. Requires using callbacks (`cmp_fs_read_async`, `cmp_net_fetch_async`).
- **ASYNC_MULTI:** The sweet spot for modern desktop applications. Combines the non-blocking main event loop with a background thread pool for CPU-bound tasks.

### 3. Advanced Models (`GREENTHREADS`, `MULTIPROCESS_ACTOR`)
For highly complex, fault-tolerant, or massively concurrent systems.
- **GREENTHREADS:** Allows writing asynchronous code in a synchronous style using cooperative fibers/coroutines. High concurrency with low overhead.
- **MULTIPROCESS_ACTOR:** The "Chromium" architecture. Spawns isolated OS processes that communicate entirely via a unified message bus (`CMPMessageBus`) over IPC. Excellent fault tolerance (if a worker crashes, the UI remains alive and restarts it).

## Working with Async & Threads

### Thread Pool (Tasks)
When using a multithreaded modality, you can submit work to the background thread pool:

```c
/* Example of posting a background task */
state.tasks.vtable->task_post(state.tasks.ctx, my_background_task, my_data);
```

### The Message Bus
The framework utilizes a unified messaging bus that works identically whether communicating between local threads or across isolated OS processes.

```c
/* Sending a message */
CMPMessage msg;
msg.topic = 1;
msg.type = 100;
msg.payload = "hello";
msg.payload_size = 5;
cmp_message_bus_publish(bus, &msg);
```

For more practical demonstrations, explore the examples in the `examples/` directory:
- `node_js_clone.c`
- `heavy_compute.c`
- `resilient_actors.c`