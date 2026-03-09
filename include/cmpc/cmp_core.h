#ifndef CMP_CORE_H
#define CMP_CORE_H

/**
 * @file cmp_core.h
 * @brief Core types, error codes, and ABI helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>
#include <stddef.h>

/**
 * @def CMP_SIZE_FORMAT
 * @brief Size formatter for printf.
 */
#if defined(_MSC_VER)
#define CMP_SIZE_FORMAT "%Iu"
#else
#define CMP_SIZE_FORMAT "%zu"
#endif

/**
 * @def CMP_SNPRINTF
 * @brief Safe snprintf macro.
 */
/**
 * @def CMP_STRNCPY
 * @brief Safe strncpy macro.
 */
#if defined(_MSC_VER)
#define CMP_SNPRINTF sprintf_s
#define CMP_STRNCPY strncpy_s
#else
#define CMP_SNPRINTF snprintf
#define CMP_STRNCPY strncpy
#endif

/** @brief Major version number. */
#define CMP_VERSION_MAJOR 0
/** @brief Minor version number. */
#define CMP_VERSION_MINOR 1
/** @brief Patch version number. */
#define CMP_VERSION_PATCH 0

/**
 * @def CMP_CALL
 * @brief Calling convention macro for public API functions.
 */
#ifndef CMP_CALL
#if defined(_WIN32) || defined(_WIN64)
#define CMP_CALL __cdecl
#else
#define CMP_CALL
#endif
#endif

/**
 * @def CMP_API
 * @brief Import/export annotation macro for public symbols.
 */
#ifndef CMP_API
#if defined(_WIN32) || defined(_WIN64)
#if defined(CMP_BUILD_DLL)
#define CMP_API __declspec(dllexport)
#elif defined(CMP_USE_DLL)
#define CMP_API __declspec(dllimport)
#else
#define CMP_API
#endif
#else
#define CMP_API
#endif
#endif

/** @brief Boolean type used across the ABI. */
typedef int CMPBool;
/** @brief Boolean true value. */
#define CMP_TRUE 1
/** @brief Boolean false value. */
#define CMP_FALSE 0

/** @brief Signed 8-bit integer. */
typedef signed char cmp_i8;
/** @brief Unsigned 8-bit integer. */
typedef unsigned char cmp_u8;
/** @brief Signed 16-bit integer. */
typedef signed short cmp_i16;
/** @brief Unsigned 16-bit integer. */
typedef unsigned short cmp_u16;
/** @brief Signed 32-bit integer. */
typedef signed int cmp_i32;
/** @brief Unsigned 32-bit integer. */
typedef unsigned int cmp_u32;
/** @brief Signed size type. */
typedef ptrdiff_t cmp_isize;
/** @brief Unsigned size type. */
typedef size_t cmp_usize;

/**
 * @def CMP_STATIC_ASSERT
 * @brief Compile-time assertion helper.
 */
#define CMP_STATIC_ASSERT(name, expr)                                          \
  typedef char cmp_static_assert_##name[(expr) ? 1 : -1]

#ifndef CMP_DOXYGEN
CMP_STATIC_ASSERT(char_is_8_bits, CHAR_BIT == 8);
CMP_STATIC_ASSERT(short_is_16_bits, sizeof(short) == 2);
CMP_STATIC_ASSERT(int_is_32_bits, sizeof(int) == 4);
CMP_STATIC_ASSERT(size_t_at_least_32_bits, sizeof(size_t) >= 4);
#endif

/** @brief Mark an unused variable or parameter. */
#define CMP_UNUSED(x) ((void)(x))
/** @brief Count elements in a fixed-size array. */
#define CMP_COUNTOF(x) (sizeof(x) / sizeof((x)[0]))

/** @brief Success result code. */
#define CMP_OK 0
/** @brief Unknown failure. */
#define CMP_ERR_UNKNOWN -1
/** @brief Invalid argument or null pointer. */
#define CMP_ERR_INVALID_ARGUMENT -2
/** @brief Allocation failed. */
#define CMP_ERR_OUT_OF_MEMORY -3
/** @brief Resource not found. */
#define CMP_ERR_NOT_FOUND -4
/** @brief Resource not ready yet. */
#define CMP_ERR_NOT_READY -5
/** @brief Operation is unsupported. */
#define CMP_ERR_UNSUPPORTED -6
/** @brief I/O failure. */
#define CMP_ERR_IO -7
/** @brief Invalid state for operation. */
#define CMP_ERR_STATE -8
/** @brief Value outside allowed range. */
#define CMP_ERR_RANGE -9
/** @brief Operation timed out. */
#define CMP_ERR_TIMEOUT -10
/** @brief Permission denied. */
#define CMP_ERR_PERMISSION -11
/** @brief Resource is busy. */
#define CMP_ERR_BUSY -12
/** @brief Data corruption detected. */
#define CMP_ERR_CORRUPT -13
/** @brief Arithmetic overflow. */
#define CMP_ERR_OVERFLOW -14
/** @brief Arithmetic underflow. */
#define CMP_ERR_UNDERFLOW -15

/**
 * @brief Allocation function signature.
 * @param ctx User-provided allocator context.
 * @param size Number of bytes to allocate.
 * @param out_ptr Receives the allocated block on success.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPAllocFn)(void *ctx, cmp_usize size, void **out_ptr);

/**
 * @brief Reallocation function signature.
 * @param ctx User-provided allocator context.
 * @param ptr Existing allocation to resize.
 * @param size New size in bytes.
 * @param out_ptr Receives the reallocated block on success.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPReallocFn)(void *ctx, void *ptr, cmp_usize size,
                                    void **out_ptr);

/**
 * @brief Free function signature.
 * @param ctx User-provided allocator context.
 * @param ptr Allocation to release.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPFreeFn)(void *ctx, void *ptr);

/**
 * @brief Allocator interface used throughout the ABI.
 */
typedef struct CMPAllocator {
  void *ctx;            /**< User-defined allocator context. */
  CMPAllocFn alloc;     /**< Allocation callback. */
  CMPReallocFn realloc; /**< Reallocation callback. */
  CMPFreeFn free;       /**< Free callback. */
} CMPAllocator;

/**
 * @brief Execution modalities for the framework's event loop and task
 * scheduler.
 */
typedef enum CMPModality {
  CMP_MODALITY_SYNC_SINGLE,  /**< Traditional blocking loop, single threaded. */
  CMP_MODALITY_SYNC_MULTI,   /**< Blocking main loop, thread pool for tasks. */
  CMP_MODALITY_ASYNC_SINGLE, /**< Non-blocking async event loop, single
                                threaded. */
  CMP_MODALITY_ASYNC_MULTI,  /**< Non-blocking async event loop, thread pool. */
  CMP_MODALITY_GREENTHREADS, /**< Cooperative user-space fibers/coroutines. */
  CMP_MODALITY_MULTIPROCESS_ACTOR /**< Isolated processes communicating via
                                     messages. */
} CMPModality;

/**
 * @brief Abstract event loop interface.
 */
typedef struct CMPEventLoop CMPEventLoop;

/**
 * @brief Represents an I/O handle polled by an async event loop.
 */
typedef void *CMPEventLoopIOHandle;

/**
 * @brief Represents a timer handle managed by an async event loop.
 */
typedef void *CMPEventLoopTimerHandle;

/**
 * @brief Type of I/O event.
 */
typedef enum CMPEventLoopIOFlags {
  CMP_EVENT_LOOP_IO_READ = 1,
  CMP_EVENT_LOOP_IO_WRITE = 2
} CMPEventLoopIOFlags;

/**
 * @brief Callback fired when an I/O handle is ready.
 * @param ctx User context.
 * @param handle The I/O handle.
 * @param flags Flags indicating readiness (read/write).
 * @return CMP_OK on success.
 */
typedef int(CMP_CALL *CMPEventLoopIOCallback)(void *ctx,
                                              CMPEventLoopIOHandle handle,
                                              cmp_u32 flags);

/**
 * @brief Callback fired when a timer elapses.
 * @param ctx User context.
 * @param handle The timer handle.
 * @return CMP_OK on success.
 */
typedef int(CMP_CALL *CMPEventLoopTimerCallback)(
    void *ctx, CMPEventLoopTimerHandle handle);

/**
 * @brief Callback to schedule a task into the event loop from a worker thread.
 * @param ctx User context.
 * @return CMP_OK on success.
 */
typedef int(CMP_CALL *CMPEventLoopWakeCallback)(void *ctx);

/**
 * @brief Event loop interface vtable.
 */
typedef struct CMPEventLoopVTable {
  /**
   * @brief Run the event loop.
   * @param ctx Event loop context.
   * @return CMP_OK on success or an error code.
   */
  int(CMP_CALL *run)(void *ctx);

  /**
   * @brief Stop the event loop.
   * @param ctx Event loop context.
   * @return CMP_OK on success or an error code.
   */
  int(CMP_CALL *stop)(void *ctx);

  /**
   * @brief Add an I/O file descriptor to the loop.
   * @param ctx Event loop context.
   * @param fd File descriptor/socket.
   * @param flags Read/Write flags.
   * @param cb Callback function.
   * @param cb_ctx Callback context.
   * @param out_handle Receives the I/O handle.
   * @return CMP_OK on success.
   */
  int(CMP_CALL *add_io)(void *ctx, int fd, cmp_u32 flags,
                        CMPEventLoopIOCallback cb, void *cb_ctx,
                        CMPEventLoopIOHandle *out_handle);

  /**
   * @brief Remove an I/O file descriptor from the loop.
   * @param ctx Event loop context.
   * @param handle Handle to remove.
   * @return CMP_OK on success.
   */
  int(CMP_CALL *remove_io)(void *ctx, CMPEventLoopIOHandle handle);

  /**
   * @brief Set a timer in the loop.
   * @param ctx Event loop context.
   * @param timeout_ms Timeout in milliseconds.
   * @param repeat If true, timer repeats.
   * @param cb Callback function.
   * @param cb_ctx Callback context.
   * @param out_handle Receives the timer handle.
   * @return CMP_OK on success.
   */
  int(CMP_CALL *set_timer)(void *ctx, cmp_u32 timeout_ms, CMPBool repeat,
                           CMPEventLoopTimerCallback cb, void *cb_ctx,
                           CMPEventLoopTimerHandle *out_handle);

  /**
   * @brief Clear a timer from the loop.
   * @param ctx Event loop context.
   * @param handle Timer handle.
   * @return CMP_OK on success.
   */
  int(CMP_CALL *clear_timer)(void *ctx, CMPEventLoopTimerHandle handle);

  /**
   * @brief Wake up the event loop from another thread.
   * @param ctx Event loop context.
   * @param cb Callback to run on the main event loop thread.
   * @param cb_ctx Callback context.
   * @return CMP_OK on success.
   */
  int(CMP_CALL *wake)(void *ctx, CMPEventLoopWakeCallback cb, void *cb_ctx);

} CMPEventLoopVTable;

/**
 * @brief Event loop abstraction.
 */
struct CMPEventLoop {
  void *ctx;                        /**< Implementation context. */
  const CMPEventLoopVTable *vtable; /**< Interface methods. */
};

/**
 * @brief Run the event loop.
 * @param loop The event loop.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_event_loop_run(CMPEventLoop *loop);

/**
 * @brief Stop the event loop.
 * @param loop The event loop.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_event_loop_stop(CMPEventLoop *loop);

/**
 * @brief Add an I/O descriptor to the event loop.
 * @param loop Event loop handle.
 * @param fd File descriptor to monitor.
 * @param flags Events to monitor for (e.g. CMP_EVENT_LOOP_IO_READ).
 * @param cb Callback function to invoke.
 * @param cb_ctx User context for the callback.
 * @param out_handle Receives the I/O handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_event_loop_add_io(CMPEventLoop *loop, int fd,
                                           cmp_u32 flags,
                                           CMPEventLoopIOCallback cb,
                                           void *cb_ctx,
                                           CMPEventLoopIOHandle *out_handle);

/**
 * @brief Remove an I/O descriptor from the event loop.
 * @param loop Event loop handle.
 * @param handle Handle returned by add_io.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_event_loop_remove_io(CMPEventLoop *loop,
                                              CMPEventLoopIOHandle handle);

/**
 * @brief Set a timer on the event loop.
 * @param loop Event loop handle.
 * @param timeout_ms Timeout in milliseconds.
 * @param repeat If true, the timer will repeat automatically.
 * @param cb Callback function to invoke.
 * @param cb_ctx User context for the callback.
 * @param out_handle Receives the timer handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL
cmp_event_loop_set_timer(CMPEventLoop *loop, cmp_u32 timeout_ms, CMPBool repeat,
                         CMPEventLoopTimerCallback cb, void *cb_ctx,
                         CMPEventLoopTimerHandle *out_handle);

/**
 * @brief Clear a timer from the event loop.
 * @param loop Event loop handle.
 * @param handle Handle returned by set_timer.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_event_loop_clear_timer(CMPEventLoop *loop,
                                                CMPEventLoopTimerHandle handle);

/**
 * @brief Wake the event loop from another thread to execute a callback.
 * @param loop Event loop handle.
 * @param cb Callback function to execute on the event loop thread.
 * @param cb_ctx User context for the callback.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_event_loop_wake(CMPEventLoop *loop,
                                         CMPEventLoopWakeCallback cb,
                                         void *cb_ctx);

/**
 * @brief Schedule a single-shot timer on the global event loop.
 * @param timeout_ms Delay in milliseconds.
 * @param cb Timer callback function.
 * @param user User data context passed to the callback.
 * @param out_handle Optional pointer to receive the timer handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_set_timeout(cmp_u32 timeout_ms,
                                     CMPEventLoopTimerCallback cb, void *user,
                                     CMPEventLoopTimerHandle *out_handle);

/**
 * @brief Schedule a repeating interval timer on the global event loop.
 * @param interval_ms Delay between executions in milliseconds.
 * @param cb Timer callback function.
 * @param user User data context passed to the callback.
 * @param out_handle Optional pointer to receive the timer handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_set_interval(cmp_u32 interval_ms,
                                      CMPEventLoopTimerCallback cb, void *user,
                                      CMPEventLoopTimerHandle *out_handle);

/**
 * @brief Clear a timer from the global event loop.
 * @param handle Handle of the timer to clear.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_clear_timeout(CMPEventLoopTimerHandle handle);

/**
 * @brief Configuration for a synchronous single-threaded event loop.
 */
typedef struct CMPSyncEventLoopConfig {
  void *ctx; /**< Context passed to the tick function. */
  int(CMP_CALL *on_tick)(void *ctx,
                         CMPBool *out_continue); /**< Tick callback. */
} CMPSyncEventLoopConfig;

/**
 * @brief Initialize a synchronous event loop backend.
 * @param alloc Allocator to use for internal state.
 * @param config Configuration containing the tick callback.
 * @param out_loop Output abstract event loop.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_event_loop_sync_init(
    CMPAllocator *alloc, const CMPSyncEventLoopConfig *config,
    CMPEventLoop *out_loop);

/**
 * @brief Destroy a synchronous event loop backend.
 * @param alloc Allocator used during initialization.
 * @param loop Event loop to destroy.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_event_loop_sync_destroy(CMPAllocator *alloc,
                                                 CMPEventLoop *loop);

/**
 * @brief Configuration for an asynchronous event loop.
 */
typedef struct CMPAsyncEventLoopConfig {
  void *ctx; /**< Context passed to the poll function. */
  int(CMP_CALL *on_poll_ui)(
      void *ctx, CMPBool *out_continue); /**< Callback to poll UI events. */
} CMPAsyncEventLoopConfig;

/**
 * @brief Initialize an asynchronous event loop backend.
 * @param alloc Allocator to use for internal state.
 * @param config Configuration containing the UI poll callback.
 * @param out_loop Output abstract event loop.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_event_loop_async_init(
    CMPAllocator *alloc, const CMPAsyncEventLoopConfig *config,
    CMPEventLoop *out_loop);

/**
 * @brief Destroy an asynchronous event loop backend.
 * @param alloc Allocator used during initialization.
 * @param loop Event loop to destroy.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_event_loop_async_destroy(CMPAllocator *alloc,
                                                  CMPEventLoop *loop);

/**
 * @brief Application configuration.
 */
typedef struct CMPAppConfig {
  CMPModality modality; /**< Selected execution modality. */
} CMPAppConfig;

/**
 * @brief Initialize application configuration with default values.
 * @param config Pointer to configuration struct.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_app_config_init(CMPAppConfig *config);

/**
 * @brief Initialize the framework with a given configuration.
 * @param config Pointer to initialized configuration struct.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_app_init(const CMPAppConfig *config);

/**
 * @brief Destroy the framework global state.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_app_destroy(void);

/**
 * @brief Obtain the default system allocator.
 * @param out_alloc Receives the allocator interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_get_default_allocator(CMPAllocator *out_alloc);

#if defined(CMP_TESTING) && !defined(DOXYGEN_SHOULD_SKIP_THIS)
/**
 * @brief Force default allocator retrieval to fail when enabled (tests only).
 * @param fail CMP_TRUE to force failure, CMP_FALSE to restore normal behavior.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_core_test_set_default_allocator_fail(CMPBool fail);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_CORE_H */
