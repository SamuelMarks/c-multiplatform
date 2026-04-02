#ifndef CMP_H
#define CMP_H

/* Phase 12.1: C-Framework Bridging & Interoperability -> ABI Stability &
 * Interfaces */

/* If on Apple platforms, utilize clang/swift attributes to provide rich, safe
 * importing into Swift */
#if defined(__APPLE__) && defined(__has_feature)
#if __has_feature(assume_nonnull)
#define CMP_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define CMP_ASSUME_NONNULL_END _Pragma("clang assume_nonnull end")
#else
#define CMP_ASSUME_NONNULL_BEGIN
#define CMP_ASSUME_NONNULL_END
#endif

#if __has_feature(nullability)
#define CMP_NONNULL _Nonnull
#define CMP_NULLABLE _Nullable
#define CMP_NULL_UNSPECIFIED _Null_unspecified
#else
#define CMP_NONNULL
#define CMP_NULLABLE
#define CMP_NULL_UNSPECIFIED
#endif

/* CoreFoundation enum equivalents mapped to NS_ENUM/NS_CLOSED_ENUM for Swift
 * type-safety */
#if __has_attribute(enum_extensibility)
#define CMP_ENUM(_type, _name)                                                 \
  enum __attribute__((enum_extensibility(open))) _name : _type _name;          \
  enum _name : _type
#define CMP_CLOSED_ENUM(_type, _name)                                          \
  enum __attribute__((enum_extensibility(closed))) _name : _type _name;        \
  enum _name : _type
#else
#define CMP_ENUM(_type, _name)                                                 \
  _type _name;                                                                 \
  enum
#define CMP_CLOSED_ENUM(_type, _name)                                          \
  _type _name;                                                                 \
  enum
#endif

/* String typedef mapping to Swift Structs */
#if __has_attribute(swift_wrapper)
#define CMP_EXTENSIBLE_STRING_ENUM __attribute__((swift_wrapper(struct)))
#else
#define CMP_EXTENSIBLE_STRING_ENUM
#endif

/* Phase 12.2: Swift Refinements */
#if __has_attribute(swift_private)
#define CMP_REFINED_FOR_SWIFT __attribute__((swift_private))
#else
#define CMP_REFINED_FOR_SWIFT
#endif

#if __has_attribute(ns_returns_retained)
#define CMP_RETURNS_RETAINED __attribute__((ns_returns_retained))
#else
#define CMP_RETURNS_RETAINED
#endif

#else /* Non-Apple fallbacks */

#define CMP_ASSUME_NONNULL_BEGIN
#define CMP_ASSUME_NONNULL_END
#define CMP_NONNULL
#define CMP_NULLABLE
#define CMP_NULL_UNSPECIFIED
#define CMP_ENUM(_type, _name)                                                 \
  typedef _type _name;                                                         \
  enum
#define CMP_CLOSED_ENUM(_type, _name)                                          \
  typedef _type _name;                                                         \
  enum
#define CMP_EXTENSIBLE_STRING_ENUM
#define CMP_REFINED_FOR_SWIFT
#define CMP_RETURNS_RETAINED

#endif /* __APPLE__ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp_ffi.h"
#include <stddef.h>

#if defined(_MSC_VER) && _MSC_VER < 1600
/* MSVC older than 2010 might lack stdint.h */
typedef signed __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef signed __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef signed __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#if !defined(_WIN32)
#include <pthread.h>
#include <semaphore.h>
#endif

#include <cfs/cfs.h>
#include <c_abstract_http/c_abstract_http.h>
#include <c_abstract_http/http_ws.h>
#include <c_abstract_http/http_sse.h>
#include <c_orm_api.h>
#include <c_orm_sqlite.h>
/* clang-format on */

#if defined(_WIN32)
#define CMP_FMT_I64 "%I64d"
#define CMP_FMT_U64 "%I64u"
#else
#define CMP_FMT_I64 "%lld"
#define CMP_FMT_U64 "%llu"
#endif

/**
 * @brief Return codes for all CMP functions.
 */
typedef enum {
  CMP_SUCCESS = 0,             /**< Success */
  CMP_ERROR_OOM = 1,           /**< Out of memory */
  CMP_ERROR_INVALID_ARG = 2,   /**< Invalid argument provided */
  CMP_ERROR_NOT_FOUND = 3,     /**< Resource not found */
  CMP_ERROR_BOUNDS = 4,        /**< Out of bounds access */
  CMP_ERROR_IO = 5,            /**< I/O error */
  CMP_ERROR_INVALID_STATE = 6, /**< Object in invalid state */
  CMP_ERROR_GENERAL = 7        /**< General/Unknown error */
} cmp_error_t;

/**
 * @brief Memory arena structure
 */
typedef struct cmp_arena {
  uint8_t *buffer;
  size_t capacity;
  size_t offset;
} cmp_arena_t;

/**
 * @brief Initialize a memory arena.
 * @param arena Pointer to the arena to initialize.
 * @param size The size of the arena to allocate in bytes.
 * @return 0 on success, or an error code.
 */
int cmp_arena_init(cmp_arena_t *arena, size_t size);

/**
 * @brief Allocate memory from the arena.
 * @param arena Pointer to the arena.
 * @param size Number of bytes to allocate.
 * @param out_ptr Pointer to receive the allocated memory address.
 * @return 0 on success, or an error code.
 */
int cmp_arena_alloc(cmp_arena_t *arena, size_t size, void **out_ptr);

/**
 * @brief Free all memory in the arena.
 * @param arena Pointer to the arena.
 * @return 0 on success, or an error code.
 */
int cmp_arena_free(cmp_arena_t *arena);

/**
 * @brief Internal pool block structure
 */
typedef struct cmp_pool_block {
  struct cmp_pool_block *next;
} cmp_pool_block_t;

/**
 * @brief Fixed-size memory pool structure
 */
typedef struct cmp_pool {
  uint8_t *buffer;
  cmp_pool_block_t *free_list;
  size_t capacity;
  size_t block_size;
} cmp_pool_t;

/**
 * @brief Initialize a fixed-size memory pool.
 * @param pool Pointer to the pool to initialize.
 * @param block_size The size of each block in bytes.
 * @param block_count The maximum number of blocks the pool can hold.
 * @return 0 on success, or an error code.
 */
int cmp_pool_init(cmp_pool_t *pool, size_t block_size, size_t block_count);

/**
 * @brief Allocate a single block from the pool.
 * @param pool Pointer to the pool.
 * @param out_ptr Pointer to receive the allocated memory address.
 * @return 0 on success, or an error code.
 */
int cmp_pool_alloc(cmp_pool_t *pool, void **out_ptr);

/**
 * @brief Free a block back to the pool.
 * @param pool Pointer to the pool.
 * @param ptr The pointer to free.
 * @return 0 on success, or an error code.
 */
int cmp_pool_free(cmp_pool_t *pool, void *ptr);

/**
 * @brief Destroy the pool and release underlying memory.
 * @param pool Pointer to the pool.
 * @return 0 on success, or an error code.
 */
int cmp_pool_destroy(cmp_pool_t *pool);

/**
 * @brief Memory allocation record for leak tracking
 */
typedef struct cmp_mem_record {
  void *ptr;
  size_t size;
  const char *file;
  int line;
  struct cmp_mem_record *next;
} cmp_mem_record_t;

/**
 * @brief Allocate tracked memory
 * @param size Number of bytes to allocate
 * @param file Source file name (__FILE__)
 * @param line Source line number (__LINE__)
 * @param out_ptr Pointer to receive the allocated memory
 * @return 0 on success, or an error code.
 */
int cmp_mem_alloc_tracked(size_t size, const char *file, int line,
                          void **out_ptr);

/**
 * @brief Free tracked memory
 * @param ptr Pointer to free
 * @param file Source file name (__FILE__)
 * @param line Source line number (__LINE__)
 * @return 0 on success, or an error code.
 */
int cmp_mem_free_tracked(void *ptr, const char *file, int line);

/**
 * @brief Check for memory leaks and print report
 * @return Number of unfreed allocations
 */
int cmp_mem_check_leaks(void);

#define CMP_MALLOC(size, out_ptr)                                              \
  cmp_mem_alloc_tracked(size, __FILE__, __LINE__, out_ptr)
#define CMP_FREE(ptr) cmp_mem_free_tracked(ptr, __FILE__, __LINE__)

/**
 * @brief Initialize crash handler (registers SIGSEGV, SIGABRT, etc.)
 * @return 0 on success, or an error code.
 */
int cmp_crash_handler_init(void);

/**
 * @brief Dump the current stack trace to stderr
 */
void cmp_dump_stack_trace(void);

/**
 * @brief Internal assertion failure handler
 * @param condition Stringified condition that failed
 * @param file File name where the assertion occurred
 * @param line Line number where the assertion occurred
 */
void cmp_assert_fail(const char *condition, const char *file, int line);

/**
 * @brief Custom assertion macro
 */
#define CMP_ASSERT(condition)                                                  \
  do {                                                                         \
    if (!(condition)) {                                                        \
      cmp_assert_fail(#condition, __FILE__, __LINE__);                         \
    }                                                                          \
  } while (0)

/**
 * @brief Represents a dynamically allocated, UTF-8 string with length tracking.
 */
typedef struct cmp_string {
  char *data;
  size_t length;
  size_t capacity;
} cmp_string_t;

/**
 * @brief Initializes an empty string.
 * @param str Pointer to the string to initialize.
 * @return 0 on success, or an error code.
 */
int cmp_string_init(cmp_string_t *str);

/**
 * @brief Appends a null-terminated C string to the cmp_string.
 * @param str Pointer to the string to append to.
 * @param append The null-terminated C string to append.
 * @return 0 on success, or an error code.
 */
int cmp_string_append(cmp_string_t *str, const char *append);

/**
 * @brief Destroys the string, freeing its allocated memory.
 * @param str Pointer to the string to destroy.
 * @return 0 on success, or an error code.
 */
int cmp_string_destroy(cmp_string_t *str);

/**
 * @brief Thread-local storage key
 */
#if defined(_WIN32)
typedef unsigned long cmp_tls_key_t;
#else
#include <pthread.h>
typedef pthread_key_t cmp_tls_key_t;
#endif

/**
 * @brief Create a thread-local storage key
 * @param out_key Pointer to receive the key
 * @return 0 on success, or an error code.
 */
int cmp_tls_key_create(cmp_tls_key_t *out_key);

/**
 * @brief Delete a thread-local storage key
 * @param key The key to delete
 * @return 0 on success, or an error code.
 */
int cmp_tls_key_delete(cmp_tls_key_t key);

/**
 * @brief Set a thread-local value
 * @param key The key to set the value for
 * @param value The value to set
 * @return 0 on success, or an error code.
 */
int cmp_tls_set(cmp_tls_key_t key, void *value);

/**
 * @brief Get a thread-local value
 * @param key The key to get the value for
 * @param out_value Pointer to receive the value
 * @return 0 on success, or an error code.
 */
int cmp_tls_get(cmp_tls_key_t key, void **out_value);

/**
 * @brief Thread-safe ring buffer for cross-thread messaging
 */
typedef struct cmp_ring_buffer {
  void **buffer;
  size_t capacity;
#if defined(_WIN32)
  volatile long head;
  volatile long tail;
#else
  volatile size_t head;
  volatile size_t tail;
#endif
} cmp_ring_buffer_t;

/**
 * @brief Initialize a ring buffer
 * @param rb Pointer to the ring buffer
 * @param capacity Maximum number of items the buffer can hold
 * @return 0 on success, or an error code.
 */
int cmp_ring_buffer_init(cmp_ring_buffer_t *rb, size_t capacity);

/**
 * @brief Push an item into the ring buffer
 * @param rb Pointer to the ring buffer
 * @param item The item to push
 * @return 0 on success, or an error code (like CMP_ERROR_BOUNDS if full).
 */
int cmp_ring_buffer_push(cmp_ring_buffer_t *rb, void *item);

/**
 * @brief Pop an item from the ring buffer
 * @param rb Pointer to the ring buffer
 * @param out_item Pointer to receive the item
 * @return 0 on success, or an error code (like CMP_ERROR_NOT_FOUND if empty).
 */
int cmp_ring_buffer_pop(cmp_ring_buffer_t *rb, void **out_item);

/**
 * @brief Destroy a ring buffer
 * @param rb Pointer to the ring buffer
 * @return 0 on success, or an error code.
 */
int cmp_ring_buffer_destroy(cmp_ring_buffer_t *rb);

/**
 * @brief Modality execution paradigms
 */
typedef enum {
  CMP_MODALITY_SINGLE = 0,   /**< Single-threaded blocking/polling loop */
  CMP_MODALITY_THREADED = 1, /**< Multi-threaded worker pool */
  CMP_MODALITY_ASYNC = 2,    /**< Asynchronous event loop (epoll/IOCP) */
  CMP_MODALITY_EVENTLOOP =
      3 /**< External event loop integration (Node.js/Qt) */
} cmp_modality_type_t;

/**
 * @brief Represents the core context for an executing modality
 */
typedef struct cmp_modality {
  cmp_modality_type_t type;
  void *internal_state;
  int is_running;
} cmp_modality_t;

/**
 * @brief Function pointer signature for an executable task
 * @param arg User-provided argument
 */
typedef void (*cmp_task_fn_t)(void *arg);

/**
 * @brief OS Thread Handle
 */
#if defined(_WIN32)
typedef void *cmp_thread_t;
#else
typedef pthread_t cmp_thread_t;
#endif

/**
 * @brief Initialize an asynchronous event loop modality (epoll/IOCP)
 * @param mod Pointer to modality struct
 * @return 0 on success, or an error code.
 */
int cmp_modality_async_init(cmp_modality_t *mod);

/**
 * @brief Initialize an eventloop integration modality (Node.js/Qt)
 * @param mod Pointer to modality struct
 * @return 0 on success, or an error code.
 */
int cmp_modality_eventloop_init(cmp_modality_t *mod);

/**
 * @brief Initialize a single-threaded polling modality
 * @param mod Pointer to modality struct
 * @return 0 on success, or an error code.
 */
int cmp_modality_single_init(cmp_modality_t *mod);

/**
 * @brief Initialize a multi-threaded worker pool modality
 * @param mod Pointer to modality struct
 * @param num_workers Number of background worker threads to spawn
 * @return 0 on success, or an error code.
 */
int cmp_modality_threaded_init(cmp_modality_t *mod, int num_workers);

/**
 * @brief Queue a task into the modality execution loop
 * @param mod Pointer to modality struct
 * @param task Function to execute
 * @param arg Argument for the function
 * @return 0 on success, or an error code.
 */
int cmp_modality_queue_task(cmp_modality_t *mod, cmp_task_fn_t task, void *arg);

/**
 * @brief Run the modality loop until stopped
 * @param mod Pointer to modality struct
 * @return 0 on success, or an error code.
 */
int cmp_modality_run(cmp_modality_t *mod);

/**
 * @brief Signal the modality loop to stop
 * @param mod Pointer to modality struct
 * @return 0 on success, or an error code.
 */
int cmp_modality_stop(cmp_modality_t *mod);

/**
 * @brief Destroy a modality context
 * @param mod Pointer to modality struct
 * @return 0 on success, or an error code.
 */
int cmp_modality_destroy(cmp_modality_t *mod);

/**
 * @brief OS Mutex Handle
 */
#if defined(_WIN32)
typedef void *cmp_mutex_t;
typedef void *cmp_semaphore_t;
typedef void *cmp_cond_t;
#elif defined(__APPLE__)
#include <pthread.h>
typedef pthread_mutex_t cmp_mutex_t;
typedef void *cmp_semaphore_t;
typedef pthread_cond_t cmp_cond_t;
#else
#include <semaphore.h>
typedef pthread_mutex_t cmp_mutex_t;
typedef sem_t cmp_semaphore_t;
typedef pthread_cond_t cmp_cond_t;
#endif

/**
 * @brief Initialize a cross-platform Mutex
 * @param mutex Pointer to mutex
 * @return 0 on success, or an error code.
 */
int cmp_mutex_init(cmp_mutex_t *mutex);

/**
 * @brief Lock a cross-platform Mutex
 * @param mutex Pointer to mutex
 * @return 0 on success, or an error code.
 */
int cmp_mutex_lock(cmp_mutex_t *mutex);

/**
 * @brief Unlock a cross-platform Mutex
 * @param mutex Pointer to mutex
 * @return 0 on success, or an error code.
 */
int cmp_mutex_unlock(cmp_mutex_t *mutex);

/**
 * @brief Destroy a cross-platform Mutex
 * @param mutex Pointer to mutex
 * @return 0 on success, or an error code.
 */
int cmp_mutex_destroy(cmp_mutex_t *mutex);

/**
 * @brief Initialize a cross-platform Semaphore
 * @param sem Pointer to semaphore
 * @param initial_count Initial permit count
 * @return 0 on success, or an error code.
 */
int cmp_semaphore_init(cmp_semaphore_t *sem, int initial_count);

/**
 * @brief Wait on a Semaphore
 * @param sem Pointer to semaphore
 * @return 0 on success, or an error code.
 */
int cmp_semaphore_wait(cmp_semaphore_t *sem);

/**
 * @brief Post/Signal a Semaphore
 * @param sem Pointer to semaphore
 * @return 0 on success, or an error code.
 */
int cmp_semaphore_post(cmp_semaphore_t *sem);

/**
 * @brief Destroy a Semaphore
 * @param sem Pointer to semaphore
 * @return 0 on success, or an error code.
 */
int cmp_semaphore_destroy(cmp_semaphore_t *sem);

/**
 * @brief Initialize a Condition Variable
 * @param cond Pointer to condition variable
 * @return 0 on success, or an error code.
 */
int cmp_cond_init(cmp_cond_t *cond);

/**
 * @brief Wait on a Condition Variable
 * @param cond Pointer to condition variable
 * @param mutex Pointer to an already locked mutex
 * @return 0 on success, or an error code.
 */
int cmp_cond_wait(cmp_cond_t *cond, cmp_mutex_t *mutex);

/**
 * @brief Signal one thread waiting on a Condition Variable
 * @param cond Pointer to condition variable
 * @return 0 on success, or an error code.
 */
int cmp_cond_signal(cmp_cond_t *cond);

/**
 * @brief Broadcast to all threads waiting on a Condition Variable
 * @param cond Pointer to condition variable
 * @return 0 on success, or an error code.
 */
int cmp_cond_broadcast(cmp_cond_t *cond);

/**
 * @brief Destroy a Condition Variable
 * @param cond Pointer to condition variable
 * @return 0 on success, or an error code.
 */
int cmp_cond_destroy(cmp_cond_t *cond);

/**
 * @brief State of a coroutine execution
 */
typedef enum {
  CMP_CORO_READY = 0,
  CMP_CORO_RUNNING = 1,
  CMP_CORO_SUSPENDED = 2,
  CMP_CORO_FINISHED = 3
} cmp_coroutine_state_t;

/**
 * @brief Forward declaration of coroutine structure
 */
typedef struct cmp_coroutine cmp_coroutine_t;

/**
 * @brief Coroutine entry point function signature
 */
typedef void (*cmp_coroutine_fn_t)(cmp_coroutine_t *co, void *arg);

/**
 * @brief Coroutine structure containing context
 */
struct cmp_coroutine {
  cmp_coroutine_state_t state;
  cmp_coroutine_fn_t fn;
  void *arg;
  void *context; /* OS-specific context (e.g., Fiber handle on Windows, ucontext
                    on POSIX) */
  void *stack;   /* Allocated stack for POSIX */
  size_t stack_size;
  struct cmp_coroutine *caller;
};

/**
 * @brief Initialize the coroutine subsystem for the current thread.
 * MUST be called before creating any coroutines on this thread.
 * @return 0 on success, or an error code.
 */
int cmp_coroutine_system_init(void);

/**
 * @brief Create a new coroutine
 * @param out_co Pointer to receive the coroutine handle
 * @param stack_size Requested stack size (0 for default)
 * @param fn Entry point
 * @param arg Argument passed to entry point
 * @return 0 on success, or an error code.
 */
int cmp_coroutine_create(cmp_coroutine_t **out_co, size_t stack_size,
                         cmp_coroutine_fn_t fn, void *arg);

/**
 * @brief Resume execution of a suspended coroutine
 * @param co The coroutine to resume
 * @return 0 on success, or an error code.
 */
int cmp_coroutine_resume(cmp_coroutine_t *co);

/**
 * @brief Suspend execution of the current coroutine, returning control to
 * caller
 * @param co The current coroutine
 * @return 0 on success, or an error code.
 */
int cmp_coroutine_yield(cmp_coroutine_t *co);

/**
 * @brief Destroy a coroutine and free its resources
 * @param co The coroutine to destroy
 * @return 0 on success, or an error code.
 */
int cmp_coroutine_destroy(cmp_coroutine_t *co);

/**
 * @brief Represents a thread-safe timer
 */
typedef struct cmp_timer cmp_timer_t;

/**
 * @brief Initialize the global timer subsystem.
 * @return 0 on success, or an error code.
 */
int cmp_timer_system_init(void);

/**
 * @brief Shut down the global timer subsystem.
 * @return 0 on success, or an error code.
 */
int cmp_timer_system_shutdown(void);

/**
 * @brief Start a new timer.
 * @param out_timer Pointer to receive the timer handle
 * @param interval_ms Timer interval in milliseconds
 * @param repeat If non-zero, timer repeats continuously
 * @param fn Callback function to execute
 * @param arg Argument passed to callback
 * @return 0 on success, or an error code.
 */
int cmp_timer_start(cmp_timer_t **out_timer, unsigned int interval_ms,
                    int repeat, cmp_task_fn_t fn, void *arg);

/**
 * @brief Stop and destroy a timer.
 * @param timer The timer to stop
 * @return 0 on success, or an error code.
 */
int cmp_timer_stop(cmp_timer_t *timer);

/**
 * @brief Initialize the global VFS subsystem (wraps c-fs).
 * @return 0 on success, or an error code.
 */
int cmp_vfs_init(void);

/**
 * @brief Shut down the global VFS subsystem.
 * @return 0 on success, or an error code.
 */
int cmp_vfs_shutdown(void);

/**
 * @brief Mount an OS directory or ZIP file into the VFS.
 * @param mount_point The virtual path (e.g. "/assets")
 * @param real_path The physical OS path or ZIP file path
 * @return 0 on success, or an error code.
 */
int cmp_vfs_mount(const char *mount_point, const char *real_path);

/**
 * @brief Resolve a virtual path to a physical OS path.
 * @param virtual_path The path to resolve (e.g. "/assets/image.png")
 * @param out_path Pointer to a string to hold the resolved OS path. Must be
 * destroyed by cmp_string_destroy.
 * @return 0 on success, or an error code.
 */
int cmp_vfs_resolve_path(const char *virtual_path, cmp_string_t *out_path);

/**
 * @brief Read the full contents of a file from the VFS synchronously.
 * @param virtual_path The path to the file (e.g. "/assets/image.png")
 * @param out_buffer Pointer to receive the allocated file contents
 * @param out_size Pointer to receive the size of the contents
 * @return 0 on success, or an error code.
 */
int cmp_vfs_read_file_sync(const char *virtual_path, void **out_buffer,
                           size_t *out_size);

/**
 * @brief Callback for asynchronous VFS reads
 * @param error Error code (0 on success)
 * @param buffer Pointer to the allocated file contents (must be freed with
 * CMP_FREE)
 * @param size Size of the file contents
 * @param user_data User data passed to the async read
 */
typedef void (*cmp_vfs_read_cb_t)(int error, void *buffer, size_t size,
                                  void *user_data);

/**
 * @brief Read the full contents of a file from the VFS asynchronously using the
 * Modality Engine.
 * @param mod The modality to execute the read task on
 * @param virtual_path The path to the file
 * @param callback Callback to execute when the read is complete
 * @param user_data Optional user data to pass to the callback
 * @return 0 on success, or an error code.
 */
int cmp_vfs_read_file_async(cmp_modality_t *mod, const char *virtual_path,
                            cmp_vfs_read_cb_t callback, void *user_data);

/**
 * @brief Callback for file watching events
 * @param path The path of the file that changed
 * @param event_type Type of event (e.g., 1 for modified, 2 for created, 3 for
 * deleted)
 * @param user_data User data passed to the watch registration
 */
typedef void (*cmp_vfs_watch_cb_t)(const char *path, int event_type,
                                   void *user_data);

/**
 * @brief Represents a file system watch handle
 */
typedef struct cmp_vfs_watch cmp_vfs_watch_t;

/**
 * @brief Register a file system watch on a directory or file.
 * @param virtual_path The path to watch
 * @param callback Callback to execute when a change occurs
 * @param user_data Optional user data to pass to the callback
 * @param out_watch Pointer to receive the watch handle
 * @return 0 on success, or an error code.
 */
int cmp_vfs_watch_path(const char *virtual_path, cmp_vfs_watch_cb_t callback,
                       void *user_data, cmp_vfs_watch_t **out_watch);

/**
 * @brief Unregister a file system watch.
 * @param watch The watch handle to destroy
 * @return 0 on success, or an error code.
 */
int cmp_vfs_unwatch(cmp_vfs_watch_t *watch);
typedef struct cfs_file cmp_vfs_file_t;

/**
 * @brief Initialize global networking subsystem (wraps c-abstract-http).
 * @return 0 on success, or an error code.
 */
int cmp_http_init(void);

/**
 * @brief Shut down global networking subsystem.
 * @return 0 on success, or an error code.
 */
int cmp_http_shutdown(void);

/**
 * @brief Create an HTTP client configured for the provided modality.
 * @param mod The execution modality to bind the client to
 * @param out_client Pointer to receive the allocated client handle
 * @return 0 on success, or an error code.
 */
int cmp_http_client_create(cmp_modality_t *mod, struct HttpClient **out_client);

/**
 * @brief Destroy an HTTP client.
 * @param client The client to destroy
 * @return 0 on success, or an error code.
 */
int cmp_http_client_destroy(struct HttpClient *client);

/**
 * @brief Initialize an HTTP request.
 * @param req The request to initialize.
 * @return 0 on success, or an error code.
 */
int cmp_http_request_init(struct HttpRequest *req);

/**
 * @brief Free resources associated with an HTTP request.
 * @param req The request to free.
 */
void cmp_http_request_free(struct HttpRequest *req);

/**
 * @brief Execute a single HTTP request.
 * @param client The HTTP client
 * @param req The request to execute
 * @param out_res Pointer to receive the allocated HTTP response (must be freed
 * with cmp_http_response_free)
 * @return 0 on success, or an error code.
 */
int cmp_http_send(struct HttpClient *client, const struct HttpRequest *req,
                  struct HttpResponse **out_res);

/**
 * @brief Execute an HTTP request with progress callbacks.
 *
 * This uses the c-abstract-http streaming callbacks (`on_chunk` and
 * `read_chunk`) behind the scenes to provide a generic non-blocking progress
 * percentage update back to the UI or calling modality.
 *
 * @param client The HTTP client
 * @param req The request to execute
 * @param progress_cb Callback to receive progress updates (0 to 100)
 * @param user_data Data passed to the progress callback
 * @param out_res Pointer to receive the allocated HTTP response
 * @return 0 on success, or an error code.
 */
int cmp_http_send_with_progress(struct HttpClient *client,
                                struct HttpRequest *req,
                                int (*progress_cb)(float percentage,
                                                   void *user_data),
                                void *user_data, struct HttpResponse **out_res);

/**
 * @brief Execute multiple HTTP requests concurrently with progress callbacks.
 *
 * @param client The HTTP client
 * @param requests Array of requests to execute
 * @param num_requests Number of requests in the array
 * @param progress_cb Callback to receive global progress updates
 * @param user_data Data passed to the progress callback
 * @param out_responses Pre-allocated array to receive the responses (size must
 * be num_requests)
 * @return 0 on success, or an error code.
 */
int cmp_http_send_multi_with_progress(
    struct HttpClient *client, struct HttpRequest *const *requests,
    size_t num_requests, int (*progress_cb)(float percentage, void *user_data),
    void *user_data, struct HttpResponse **out_responses);

/**
 * @brief Download a file from a URL, saving it to a virtual or physical path
 * (via VFS).
 *
 * @param client The HTTP client
 * @param url The URL to download
 * @param save_virtual_path The path to save the downloaded file to
 * @param progress_cb Callback to receive progress updates (0 to 100)
 * @param user_data Data passed to the progress callback
 * @return 0 on success, or an error code.
 */
int cmp_http_download(struct HttpClient *client, const char *url,
                      const char *save_virtual_path,
                      int (*progress_cb)(float percentage, void *user_data),
                      void *user_data);

/**
 * @brief Free an HTTP response.
 * @param res The response to free.
 */
void cmp_http_response_free(struct HttpResponse *res);

/**
 * @brief Initialize a WebSocket connection request.
 * @param req The HTTP request.
 * @param config Optional WS config.
 * @return 0 on success, or an error code.
 */
int cmp_http_ws_init(struct HttpRequest *req,
                     const struct c_abstract_http_ws_config *config);

/**
 * @brief Read WebSocket events synchronously or queue to modality.
 * @param mod The modality to execute on (if CMP_MODALITY_ASYNC, it registers
 * it).
 * @param client The HTTP client.
 * @param req The HTTP request.
 * @param on_msg Message callback.
 * @param on_err Error callback.
 * @param on_close Close callback.
 * @param user_data Data for callbacks.
 * @param exit_flag Optional flag to exit the read loop.
 * @return 0 on success, or an error code.
 */
int cmp_http_ws_run(cmp_modality_t *mod, struct HttpClient *client,
                    struct HttpRequest *req,
                    c_abstract_http_ws_on_message on_msg,
                    c_abstract_http_ws_on_error on_err,
                    c_abstract_http_ws_on_close on_close, void *user_data,
                    volatile int *exit_flag);

/**
 * @brief Send a WebSocket frame.
 * @param req The HTTP request connection handle.
 * @param opcode The WebSocket opcode.
 * @param payload The payload to send.
 * @param len The length of the payload.
 * @return 0 on success, or an error code.
 */
int cmp_http_ws_send(struct HttpRequest *req,
                     enum c_abstract_http_ws_opcode opcode,
                     const unsigned char *payload, size_t len);

/**
 * @brief Close a WebSocket connection.
 * @param req The HTTP request connection handle.
 * @param status_code The closure status code.
 * @return 0 on success, or an error code.
 */
int cmp_http_ws_close(struct HttpRequest *req, int status_code);

/**
 * @brief Initialize an SSE stream request.
 * @param req The HTTP request.
 * @param config Optional SSE config.
 * @return 0 on success, or an error code.
 */
int cmp_http_sse_init(struct HttpRequest *req,
                      const struct c_abstract_http_sse_config *config);

/**
 * @brief Read SSE events synchronously or queue to modality.
 * @param mod The modality to execute on.
 * @param client The HTTP client.
 * @param req The HTTP request.
 * @param on_evt Event callback.
 * @param on_err Error callback.
 * @param on_close Close callback.
 * @param user_data Data for callbacks.
 * @param exit_flag Optional flag to exit the read loop.
 * @return 0 on success, or an error code.
 */
int cmp_http_sse_run(cmp_modality_t *mod, struct HttpClient *client,
                     struct HttpRequest *req,
                     c_abstract_http_sse_on_event on_evt,
                     c_abstract_http_sse_on_error on_err,
                     c_abstract_http_sse_on_close on_close, void *user_data,
                     volatile int *exit_flag);

/**
 * @brief Initialize the global database and state subsystem (wraps c-orm).
 * @return 0 on success, or an error code.
 */
int cmp_orm_init(void);

/**
 * @brief Shut down the global database and state subsystem.
 * @return 0 on success, or an error code.
 */
int cmp_orm_shutdown(void);

/**
 * @brief Open a database connection mapped via the VFS.
 * @param virtual_path Path to the database file (e.g. "virt:/cache/db.sqlite")
 * @param out_db Pointer to receive the database handle
 * @return 0 on success, or an error code.
 */
int cmp_orm_connect(const char *virtual_path, c_orm_db_t **out_db);

/**
 * @brief Close a database connection.
 * @param db Database handle to close
 * @return 0 on success, or an error code.
 */
int cmp_orm_disconnect(c_orm_db_t *db);

/**
 * @brief Execute a raw SQL statement that modifies state.
 * @param db Database connection
 * @param sql Raw SQL statement
 * @return 0 on success, or an error code.
 */
int cmp_orm_execute(c_orm_db_t *db, const char *sql);

/**
 * @brief Run pending database migrations automatically.
 * @param db Database handle
 * @param migrations_dir Virtual path to the directory containing .sql migration
 * scripts
 * @return 0 on success, or an error code.
 */
int cmp_orm_migrate(c_orm_db_t *db, const char *migrations_dir);

/**
 * @brief Provide the encryption key for the secure enclave (e.g. SQLCipher
 * backend).
 * @param db Database handle
 * @param key The encryption key
 * @return 0 on success, or an error code.
 */
int cmp_orm_set_encryption_key(c_orm_db_t *db, const char *key);

/**
 * @brief Opaque Data Binding Observable
 */
typedef struct cmp_orm_observable cmp_orm_observable_t;

/**
 * @brief Initialize an observable data binding to a database query.
 * @param db The database handle
 * @param query The SQL query representing the observable state
 * @param out_obs Pointer to receive the observable handle
 * @return 0 on success, or an error code.
 */
int cmp_orm_observable_create(c_orm_db_t *db, const char *query,
                              cmp_orm_observable_t **out_obs);

typedef struct cmp_ui_node cmp_ui_node_t;
/**
 * @brief Opaque forward declaration for the theme Virtual Table.
 * @thread_safety This pointer type provides read-only function maps for the
 * rendering cycle. Calls to `cmp_resolve_vtable` are thread-safe and lock-free.
 */
typedef struct cmp_theme_vtable_s cmp_theme_vtable_t;
typedef struct cmp_theme cmp_theme_t;

/**
 * @brief Bind a UI node to an observable property.
 * @param node The UI node to bind
 * @param obs The observable to bind to
 * @param property_name Name of the UI property (e.g. "text", "value")
 * @return 0 on success, or an error code.
 */
int cmp_ui_node_bind(cmp_ui_node_t *node, cmp_orm_observable_t *obs,
                     const char *property_name);

/**
 * @brief Destroy an observable binding.
 * @param obs The observable to destroy
 * @return 0 on success, or an error code.
 */
int cmp_orm_observable_destroy(cmp_orm_observable_t *obs);

/**
 * @brief Map OS standard paths (App Data, Temp, Cache, Documents, Executable
 * Directory).
 * @param type The type of standard path to resolve (1=AppData, 2=Temp, 3=Cache,
 * 4=Documents, 5=Executable Dir)
 * @param out_path Pointer to a string to hold the resolved OS path. Must be
 * destroyed by cmp_string_destroy.
 * @return 0 on success, or an error code.
 */
int cmp_vfs_get_standard_path(int type, cmp_string_t *out_path);

/**
 * @brief Unified Action Enums for Inputs
 */
typedef enum cmp_action {
  CMP_ACTION_DOWN = 1,
  CMP_ACTION_UP = 2,
  CMP_ACTION_MOVE = 3,
  CMP_ACTION_CANCEL = 4
} cmp_action_t;

/**
 * @brief Normalized Multi-modal Input Event Structure
 */
typedef struct cmp_event {
  uint32_t type; /* e.g. MOUSE, TOUCH, KEYBOARD */
  cmp_action_t action;
  int x;
  int y;
  int source_id;      /* Pointer ID or Key Code */
  float pressure;     /* For stylus/wacom */
  uint32_t modifiers; /* Shift, Ctrl, Alt */
} cmp_event_t;

/**
 * @brief Pointer lock mode for dragging / FPS camera
 */
typedef enum cmp_pointer_lock {
  CMP_POINTER_UNLOCKED = 0,
  CMP_POINTER_LOCKED = 1,
  CMP_POINTER_LOCKED_HIDDEN = 2
} cmp_pointer_lock_t;

/**
 * @brief Initialize the global Event normalization subsystem
 * @return 0 on success, or an error code.
 */
int cmp_event_system_init(void);

/**
 * @brief Evaluation mode for pointer events
 */
typedef enum cmp_pointer_events {
  CMP_POINTER_EVENTS_AUTO = 0,
  CMP_POINTER_EVENTS_NONE = 1
} cmp_pointer_events_t;

/**
 * @brief Set the pointer events state of a node
 * @param node The UI node to modify
 * @param state The pointer events state
 * @return 0 on success, or an error code.
 */
int cmp_ui_node_set_pointer_events(cmp_ui_node_t *node,
                                   cmp_pointer_events_t state);

/**
 * @brief Get the pointer events state of a node
 * @param node The UI node to inspect
 * @return The pointer events state (cast to int), or an error/fallback state if
 * invalid.
 */
int cmp_ui_node_get_pointer_events(cmp_ui_node_t *node);

#ifndef CMP_TEXTURE_T_DEFINED
#define CMP_TEXTURE_T_DEFINED
typedef struct cmp_texture cmp_texture_t;
#endif

/**
 * @brief Shutdown the global Event subsystem
 * @return 0 on success, or an error code.
 */
int cmp_event_system_shutdown(void);

/**
 * @brief Push a raw event into the Producer-Consumer normalization queue
 * @param event The event to push
 * @return 0 on success, or an error code.
 */
int cmp_event_push(const cmp_event_t *event);

/**
 * @brief Pop a normalized event from the queue
 * @param out_event Pointer to receive the event
 * @return 0 on success, or an error code.
 */
int cmp_event_pop(cmp_event_t *out_event);

/**
 * @brief Execute a hit test mapping coordinates to a UI tree node (stub)
 * @param x X coordinate
 * @param y Y coordinate
 * @return Node ID or pointer (stubbed returning int)
 */
int cmp_event_hit_test(int x, int y);

/**
 * @brief Event Phase indicating the propagation state
 */
typedef enum cmp_event_phase {
  CMP_EVENT_PHASE_NONE = 0,
  CMP_EVENT_PHASE_CAPTURING = 1,
  CMP_EVENT_PHASE_AT_TARGET = 2,
  CMP_EVENT_PHASE_BUBBLING = 3
} cmp_event_phase_t;

/**
 * @brief Opaque structure for an Event Dispatch Context
 */
typedef struct cmp_event_dispatch cmp_event_dispatch_t;

/**
 * @brief Dispatch a normalized event through the full Capture -> Target ->
 * Bubble sequence
 * @param tree The UI tree
 * @param target_node The identified DOM hit target
 * @param event The event payload
 * @return 0 on success, or an error code.
 */
int cmp_event_dispatch_run(cmp_ui_node_t *tree, cmp_ui_node_t *target_node,
                           cmp_event_t *event);

/**
 * @brief Initialize the passive event listener subsystem
 * @return 0 on success, or an error code.
 */
int cmp_passive_event_init(void);

/**
 * @brief Register a passive event listener
 * @param node The target UI node
 * @param event_type The numeric type of the event (e.g. scroll, touchstart)
 * @param callback The function to invoke
 * @param user_data Opaque user pointer
 * @return 0 on success, or an error code.
 */
int cmp_ui_node_add_passive_listener(cmp_ui_node_t *node, uint32_t event_type,
                                     void (*callback)(cmp_event_t *,
                                                      cmp_ui_node_t *, void *),
                                     void *user_data);

/**
 * @brief Prevent the default action for an event. Will return an error if
 * called from a passive listener.
 * @param event The event context to modify
 * @return 0 on success, CMP_ERROR_INVALID_ARG if invalid, or CMP_ERROR_BOUNDS
 * if called from a passive listener.
 */
int cmp_event_prevent_default(cmp_event_t *event);

/**
 * @brief Check if preventDefault was called on the event
 * @param event The event context
 * @return 1 if default is prevented, 0 otherwise
 */
int cmp_event_is_default_prevented(const cmp_event_t *event);

/**
 * @brief Set the exclusive pointer capture node
 * @param node The UI node to receive exclusive capture, or NULL to release
 * @param pointer_id The OS-level pointer identifier (e.g. from touch events)
 * @return 0 on success, or an error code.
 */
int cmp_ui_node_set_pointer_capture(cmp_ui_node_t *node, int pointer_id);

/**
 * @brief Release pointer capture for a specific pointer ID
 * @param node The UI node that currently holds capture
 * @param pointer_id The OS-level pointer identifier
 * @return 0 on success, CMP_ERROR_NOT_FOUND if the node does not hold capture
 * for the given ID, or an error code.
 */
int cmp_ui_node_release_pointer_capture(cmp_ui_node_t *node, int pointer_id);

/**
 * @brief Check if a node currently holds pointer capture for a given pointer ID
 * @param node The UI node to check
 * @param pointer_id The OS-level pointer identifier
 * @return 1 if the node holds capture for the given ID, 0 otherwise
 */
int cmp_ui_node_has_pointer_capture(const cmp_ui_node_t *node, int pointer_id);

/**
 * @brief Register an event listener on a specific UI node
 * @param node The UI node
 * @param event_type The numeric type of event to listen for
 * @param capture If non-zero, triggers during the capture phase rather than
 * bubbling
 * @param callback The function pointer to invoke
 * @param user_data Opaque pointer passed back into the callback
 * @return 0 on success, or an error code.
 */
int cmp_ui_node_add_event_listener(
    cmp_ui_node_t *node, uint32_t event_type, int capture,
    void (*callback)(cmp_event_t *, cmp_ui_node_t *, void *), void *user_data);

/**
 * @brief Initialize the pointer capture tracking subsystem.
 * @return 0 on success, or an error code.
 */
int cmp_pointer_capture_init(void);

/**
 * @brief Free resources associated with pointer capture tracking.
 * @return 0 on success, or an error code.
 */
int cmp_pointer_capture_shutdown(void);

/**
 * @brief Discrete States for State-Machine Gesture Recognizers
 */
typedef enum cmp_gesture_state {
  CMP_GESTURE_STATE_POSSIBLE = 0,
  CMP_GESTURE_STATE_BEGAN = 1,
  CMP_GESTURE_STATE_CHANGED = 2,
  CMP_GESTURE_STATE_ENDED = 3,
  CMP_GESTURE_STATE_CANCELLED = 4
} cmp_gesture_state_t;

/**
 * @brief Opaque Gesture Recognizer Context
 */
typedef struct cmp_gesture cmp_gesture_t;

/**
 * @brief Callback signature for gesture events
 */
typedef void (*cmp_gesture_cb_t)(cmp_gesture_t *gesture, cmp_ui_node_t *node,
                                 void *user_data);

/**
 * @brief Create a generic gesture recognizer
 * @param out_gesture Pointer to receive the allocated recognizer
 * @return 0 on success, or an error code.
 */
int cmp_gesture_create(cmp_gesture_t **out_gesture);

/**
 * @brief Destroy a gesture recognizer
 * @param gesture The gesture to destroy
 * @return 0 on success, or an error code.
 */
int cmp_gesture_destroy(cmp_gesture_t *gesture);

/**
 * @brief Get the current deterministic state of the gesture
 * @param gesture The gesture
 * @return The current cmp_gesture_state_t (cast to int)
 */
int cmp_gesture_get_state(const cmp_gesture_t *gesture);

/**
 * @brief Feed a raw pointer event into the gesture state machine
 * @param gesture The gesture recognizer
 * @param event The incoming multi-touch/mouse event
 * @return 0 on success, or an error code.
 */
int cmp_gesture_process_event(cmp_gesture_t *gesture, const cmp_event_t *event);

/**
 * @brief Attach a callback to the gesture recognizer
 * @param gesture The gesture recognizer
 * @param callback The function to execute on state changes
 * @param user_data Opaque pointer passed to the callback
 * @return 0 on success, or an error code.
 */
int cmp_gesture_set_callback(cmp_gesture_t *gesture, cmp_gesture_cb_t callback,
                             void *user_data);

/**
 * @brief Opaque Complex Gesture Recognizer Context (Pan, Pinch, Rotate)
 */
typedef struct cmp_complex_gesture cmp_complex_gesture_t;

/**
 * @brief Initialize a complex multi-touch gesture recognizer
 * @param out_gesture Pointer to receive the allocated recognizer
 * @return 0 on success, or an error code.
 */
int cmp_complex_gesture_create(cmp_complex_gesture_t **out_gesture);

/**
 * @brief Destroy a complex gesture recognizer
 * @param gesture The gesture to destroy
 * @return 0 on success, or an error code.
 */
int cmp_complex_gesture_destroy(cmp_complex_gesture_t *gesture);

/**
 * @brief Process an incoming event through the complex gesture state machine
 * @param gesture The complex gesture recognizer
 * @param event The incoming multi-touch event
 * @return 0 on success, or an error code.
 */
int cmp_complex_gesture_process_event(cmp_complex_gesture_t *gesture,
                                      const cmp_event_t *event);

/**
 * @brief Get the current deterministic state of the complex gesture
 * @param gesture The complex gesture recognizer
 * @return The current state (cast to int)
 */
int cmp_complex_gesture_get_state(const cmp_complex_gesture_t *gesture);

/**
 * @brief Retrieve the computed transformation deltas from a complex gesture
 * @param gesture The gesture recognizer
 * @param out_pan_x Pointer to receive X panning delta
 * @param out_pan_y Pointer to receive Y panning delta
 * @param out_scale Pointer to receive pinch scale delta (1.0 = no change)
 * @param out_rotation Pointer to receive rotation delta in radians
 * @return 0 on success, or an error code.
 */
int cmp_complex_gesture_get_deltas(const cmp_complex_gesture_t *gesture,
                                   float *out_pan_x, float *out_pan_y,
                                   float *out_scale, float *out_rotation);

/**
 * @brief Opaque Stylus / Pointer Pressure Context
 */
typedef struct cmp_pointer_pressure cmp_pointer_pressure_t;

/**
 * @brief Represents the raw stylus data received from the OS
 */
typedef struct cmp_stylus_data {
  float pressure; /**< 0.0 (no pressure) to 1.0 (max pressure) */
  float tilt_x;   /**< Altitude/Azimuth X representation (-90 to 90 degrees) */
  float tilt_y;   /**< Altitude/Azimuth Y representation (-90 to 90 degrees) */
  int button_pressed; /**< Barrel button state (1 if pressed, 0 otherwise) */
  int is_eraser;      /**< 1 if the eraser end is active, 0 for the tip */
} cmp_stylus_data_t;

/**
 * @brief Initialize a pointer pressure tracking context
 * @param out_pressure Pointer to receive the tracking context
 * @return 0 on success, or an error code.
 */
int cmp_pointer_pressure_create(cmp_pointer_pressure_t **out_pressure);

/**
 * @brief Destroy a pointer pressure tracking context
 * @param pressure The tracking context
 * @return 0 on success, or an error code.
 */
int cmp_pointer_pressure_destroy(cmp_pointer_pressure_t *pressure);

/**
 * @brief Process an incoming stylus event, updating internal pressure
 * algorithms
 * @param pressure The tracking context
 * @param event The event payload, which must contain valid pointer extensions
 * @return 0 on success, or an error code.
 */
int cmp_pointer_pressure_process_event(cmp_pointer_pressure_t *pressure,
                                       const cmp_event_t *event);

/**
 * @brief Get the smoothed / computed stylus data from the tracker
 * @param pressure The tracking context
 * @param out_data Pointer to receive the computed stylus data
 * @return 0 on success, or an error code.
 */
int cmp_pointer_pressure_get_data(const cmp_pointer_pressure_t *pressure,
                                  cmp_stylus_data_t *out_data);

/**
 * @brief Touch Action bitflags evaluating native vs custom gesture routing
 */
typedef enum cmp_touch_action {
  CMP_TOUCH_ACTION_AUTO =
      0, /**< OS handles all gestures (scrolling, zooming) */
  CMP_TOUCH_ACTION_NONE =
      1, /**< App handles all touches, OS native gestures disabled */
  CMP_TOUCH_ACTION_PAN_X = 2, /**< OS handles horizontal scrolling only */
  CMP_TOUCH_ACTION_PAN_Y = 4, /**< OS handles vertical scrolling only */
  CMP_TOUCH_ACTION_MANIPULATION =
      6, /**< OS handles both X and Y scrolling (PAN_X | PAN_Y) */
  CMP_TOUCH_ACTION_PINCH_ZOOM = 8 /**< OS handles pinch-to-zoom */
} cmp_touch_action_t;

/**
 * @brief Set the touch-action policy for a given UI node
 * @param node The UI node to modify
 * @param action The requested touch action mask (can be combined with bitwise
 * OR)
 * @return 0 on success, or an error code.
 */
int cmp_ui_node_set_touch_action(cmp_ui_node_t *node, uint32_t action);

/**
 * @brief Retrieve the computed touch-action policy for a given UI node
 * @param node The UI node to inspect
 * @return The active touch action mask (cast to int), or CMP_TOUCH_ACTION_AUTO
 * on failure
 */
int cmp_ui_node_get_touch_action(const cmp_ui_node_t *node);

/**
 * @brief Opaque Context Menu Tracking State
 */
typedef struct cmp_context_menu cmp_context_menu_t;

/**
 * @brief Callback signature for context menus (triggered by right-click or
 * long-press)
 */
typedef void (*cmp_context_menu_cb_t)(cmp_context_menu_t *menu_ctx,
                                      cmp_ui_node_t *node, float x, float y,
                                      void *user_data);

/**
 * @brief Initialize a context menu tracker
 * @param out_menu Pointer to receive the allocated context menu tracker
 * @return 0 on success, or an error code.
 */
int cmp_context_menu_create(cmp_context_menu_t **out_menu);

/**
 * @brief Destroy a context menu tracker
 * @param menu The tracker to destroy
 * @return 0 on success, or an error code.
 */
int cmp_context_menu_destroy(cmp_context_menu_t *menu);

/**
 * @brief Attach a callback to fire when a context menu should open
 * @param menu The context menu tracker
 * @param callback The callback to fire
 * @param user_data Opaque user data
 * @return 0 on success, or an error code.
 */
int cmp_context_menu_set_callback(cmp_context_menu_t *menu,
                                  cmp_context_menu_cb_t callback,
                                  void *user_data);

/**
 * @brief Process an event to determine if it triggers the context menu (long
 * press or right click)
 * @param menu The context menu tracker
 * @param event The pointer event to process
 * @return 0 on success (event handled/progressed), or an error code.
 */
int cmp_context_menu_process_event(cmp_context_menu_t *menu,
                                   const cmp_event_t *event);

/**
 * @brief Opaque Hover Intent / Hysteresis Tracking Context
 */
typedef struct cmp_hover_intent cmp_hover_intent_t;

/**
 * @brief Initialize a hover intent tracker
 * @param out_intent Pointer to receive the allocated tracker
 * @return 0 on success, or an error code.
 */
int cmp_hover_intent_create(cmp_hover_intent_t **out_intent);

/**
 * @brief Destroy a hover intent tracker
 * @param intent The tracker to destroy
 * @return 0 on success, or an error code.
 */
int cmp_hover_intent_destroy(cmp_hover_intent_t *intent);

/**
 * @brief Process pointer coordinates through the hysteresis algorithm
 * @param intent The hover intent tracker
 * @param event The pointer event to process
 * @param dt_ms Time elapsed since last evaluation in milliseconds
 * @return 1 if hover intent is confirmed, 0 if still deliberating/cancelled, or
 * an error code.
 */
int cmp_hover_intent_process(cmp_hover_intent_t *intent,
                             const cmp_event_t *event, float dt_ms);

/**
 * @brief Independent Scroll Context decoupled from Layout step
 */
typedef struct cmp_scroll_ctx cmp_scroll_ctx_t;

/**
 * @brief Initialize a new independent scroll context
 * @param out_ctx Pointer to receive the allocated context
 * @return 0 on success, or an error code.
 */
int cmp_scroll_ctx_create(cmp_scroll_ctx_t **out_ctx);

/**
 * @brief Destroy a scroll context
 * @param ctx The context to destroy
 * @return 0 on success, or an error code.
 */
int cmp_scroll_ctx_destroy(cmp_scroll_ctx_t *ctx);

/**
 * @brief Get the current scrollTop and scrollLeft values
 * @param ctx The scroll context
 * @param out_x Pointer to receive X scroll offset
 * @param out_y Pointer to receive Y scroll offset
 * @return 0 on success, or an error code.
 */
int cmp_scroll_ctx_get_offset(const cmp_scroll_ctx_t *ctx, float *out_x,
                              float *out_y);

/**
 * @brief Set the scroll offsets programmatically
 * @param ctx The scroll context
 * @param x X scroll offset
 * @param y Y scroll offset
 * @return 0 on success, or an error code.
 */
int cmp_scroll_ctx_set_offset(cmp_scroll_ctx_t *ctx, float x, float y);

#ifndef CMP_UBO_T_DEFINED
#define CMP_UBO_T_DEFINED
typedef struct cmp_ubo cmp_ubo_t;
#endif

/**
 * @brief Hardware-Accelerated Scroll Offset Injection
 *
 * Binds the scroll context offset directly to a GPU uniform buffer
 * without triggering CPU Layout/Paint.
 *
 * @param ctx The scroll context containing values
 * @param ubo The Uniform Buffer Object mapping to the active UI layer shader
 * @return 0 on success, or an error code.
 */
int cmp_scroll_ctx_inject_to_gpu(const cmp_scroll_ctx_t *ctx, cmp_ubo_t *ubo);

/**
 * @brief Opaque Scroll Velocity Tracker
 */
typedef struct cmp_scroll_velocity cmp_scroll_velocity_t;

/**
 * @brief Initialize a new scroll velocity tracker
 * @param out_tracker Pointer to receive the allocated tracker
 * @return 0 on success, or an error code.
 */
int cmp_scroll_velocity_create(cmp_scroll_velocity_t **out_tracker);

/**
 * @brief Destroy a scroll velocity tracker
 * @param tracker The tracker to destroy
 * @return 0 on success, or an error code.
 */
int cmp_scroll_velocity_destroy(cmp_scroll_velocity_t *tracker);

/**
 * @brief Process a touch/pointer event to update velocity history
 * @param tracker The velocity tracker
 * @param event The event containing coordinates
 * @param dt_ms Time elapsed in milliseconds
 * @return 0 on success, or an error code.
 */
int cmp_scroll_velocity_push(cmp_scroll_velocity_t *tracker,
                             const cmp_event_t *event, float dt_ms);

/**
 * @brief Get the computed rolling average velocity upon release
 * @param tracker The velocity tracker
 * @param out_vx Receives X velocity in pixels/second
 * @param out_vy Receives Y velocity in pixels/second
 * @return 0 on success, or an error code.
 */
int cmp_scroll_velocity_get(const cmp_scroll_velocity_t *tracker, float *out_vx,
                            float *out_vy);

/**
 * @brief Momentum Friction Decay Calculation
 * @param initial_velocity The starting velocity
 * @param elapsed_time_ms Time elapsed since scroll release
 * @param platform_decay_rate Friction factor (e.g. 0.998 Apple, 0.990 Material)
 * @param out_offset Resulting scroll delta offset to apply this frame
 * @param out_current_velocity Resulting current velocity
 * @return 0 on success, or an error code.
 */
int cmp_scroll_momentum_calculate(float initial_velocity, float elapsed_time_ms,
                                  float platform_decay_rate, float *out_offset,
                                  float *out_current_velocity);

/**
 * @brief Overscroll behavior modes
 */
typedef enum cmp_overscroll {
  CMP_OVERSCROLL_AUTO = 0,
  CMP_OVERSCROLL_CONTAIN = 1,
  CMP_OVERSCROLL_NONE = 2
} cmp_overscroll_t;

/**
 * @brief Evaluate overscroll chaining prevention
 * @param mode The defined overscroll behavior for the boundary
 * @param boundary_hit 1 if a scroll boundary was hit, 0 otherwise
 * @return 1 if scroll chaining is allowed (event should bubble), 0 if trapped
 */
int cmp_overscroll_evaluate(cmp_overscroll_t mode, int boundary_hit);

/**
 * @brief Opaque Rubber Band Physics State
 */
typedef struct cmp_rubber_band cmp_rubber_band_t;

/**
 * @brief Initialize a rubber band oscillator
 * @param out_band Pointer to receive the allocated state
 * @return 0 on success, or an error code.
 */
int cmp_rubber_band_create(cmp_rubber_band_t **out_band);

/**
 * @brief Destroy a rubber band oscillator
 * @param band The state to destroy
 * @return 0 on success, or an error code.
 */
int cmp_rubber_band_destroy(cmp_rubber_band_t *band);

/**
 * @brief Step the rubber band physics simulation
 * @param band The oscillator state
 * @param dt_ms Time elapsed in milliseconds
 * @param target_rest_position The boundary offset to snap back to
 * @param out_current_position Receives the new calculated position
 * @param out_is_resting Receives 1 if the animation is fully complete, 0
 * otherwise
 * @return 0 on success, or an error code.
 */
int cmp_rubber_band_step(cmp_rubber_band_t *band, float dt_ms,
                         float target_rest_position,
                         float *out_current_position, int *out_is_resting);

/**
 * @brief Manually inject kinetic energy (velocity) into the rubber band
 * @param band The oscillator state
 * @param initial_velocity The velocity in pixels/second
 * @param current_position The starting position when released
 * @return 0 on success, or an error code.
 */
int cmp_rubber_band_start(cmp_rubber_band_t *band, float initial_velocity,
                          float current_position);

/**
 * @brief Opaque Smooth Scroll Interpolator
 */
typedef struct cmp_scroll_smooth cmp_scroll_smooth_t;

/**
 * @brief Initialize a smooth scroll interpolator
 * @param out_smooth Pointer to receive the allocated state
 * @return 0 on success, or an error code.
 */
int cmp_scroll_smooth_create(cmp_scroll_smooth_t **out_smooth);

/**
 * @brief Destroy a smooth scroll interpolator
 * @param smooth The state to destroy
 * @return 0 on success, or an error code.
 */
int cmp_scroll_smooth_destroy(cmp_scroll_smooth_t *smooth);

/**
 * @brief Initiate a smooth scroll to a target
 * @param smooth The interpolator state
 * @param current_pos Starting offset
 * @param target_pos Desired final offset
 * @return 0 on success, or an error code.
 */
int cmp_scroll_smooth_start(cmp_scroll_smooth_t *smooth, float current_pos,
                            float target_pos);

/**
 * @brief Step the cubic-bezier scroll interpolation
 * @param smooth The interpolator state
 * @param dt_ms Time elapsed in milliseconds
 * @param out_current_position Receives the new calculated position
 * @param out_is_complete Receives 1 if the scroll reached target, 0 otherwise
 * @return 0 on success, or an error code.
 */
int cmp_scroll_smooth_step(cmp_scroll_smooth_t *smooth, float dt_ms,
                           float *out_current_position, int *out_is_complete);

/**
 * @brief Represents the `scroll-snap-stop` CSS property behavior
 */
typedef enum cmp_scroll_snap_stop {
  CMP_SCROLL_SNAP_STOP_NORMAL =
      0, /**< Allows scrolling past multiple snap points if velocity is high */
  CMP_SCROLL_SNAP_STOP_ALWAYS = 1 /**< Forces stopping at the very next snap
                                     point regardless of velocity */
} cmp_scroll_snap_stop_t;

/**
 * @brief Evaluate if a fast kinetic scroll should be hard-stopped at the next
 * snap boundary
 * @param mode The `scroll-snap-stop` mode of the approaching snap point
 * @param current_velocity The current scroll velocity
 * @return 1 if the scroll must stop, 0 if it can continue past
 */
int cmp_scroll_snap_stop_evaluate(cmp_scroll_snap_stop_t mode,
                                  float current_velocity);

/**
 * @brief Represents scroll-padding/scroll-margin constraints for a viewport
 */
typedef struct cmp_scroll_padding {
  float top;
  float right;
  float bottom;
  float left;
} cmp_scroll_padding_t;

#ifndef CMP_RECT_T_DEFINED
#define CMP_RECT_T_DEFINED
typedef struct cmp_rect cmp_rect_t;
#endif

/**
 * @brief Apply scroll padding to an element's bounds to calculate the true
 * scroll offset needed
 * @param element_bounds The raw visual bounds of the target element
 * @param padding The computed `scroll-padding` of the scroll container
 * @param margin The computed `scroll-margin` of the target element
 * @param out_target_scroll_y Receives the final calculated Y scroll offset
 * @return 0 on success, or an error code.
 */
int cmp_scroll_padding_apply(const cmp_rect_t *element_bounds,
                             const cmp_scroll_padding_t *padding,
                             const cmp_scroll_padding_t *margin,
                             float *out_target_scroll_y);

/**
 * @brief Scrollbar Gutter Property
 */
typedef enum cmp_scrollbar_gutter {
  CMP_SCROLLBAR_GUTTER_AUTO = 0,
  CMP_SCROLLBAR_GUTTER_STABLE = 1,
  CMP_SCROLLBAR_GUTTER_STABLE_BOTH_EDGES = 2
} cmp_scrollbar_gutter_t;

/**
 * @brief Calculate the reserved space for the scrollbar gutter
 *
 * @param mode The defined scrollbar-gutter property
 * @param is_overflowing 1 if the content is actually overflowing, 0 otherwise
 * @param scrollbar_thickness The physical thickness of the scrollbar if it were
 * rendered
 * @param out_reserved_left Pointer to receive reserved pixels on the left edge
 * @param out_reserved_right Pointer to receive reserved pixels on the right
 * edge
 *
 * @return 0 on success, or an error code
 */
int cmp_scrollbar_gutter_calculate(cmp_scrollbar_gutter_t mode,
                                   int is_overflowing,
                                   float scrollbar_thickness,
                                   float *out_reserved_left,
                                   float *out_reserved_right);

/**
 * @brief Opaque Scroll Anchor Context
 */
typedef struct cmp_scroll_anchor cmp_scroll_anchor_t;

/**
 * @brief Initialize a scroll anchor context
 */
int cmp_scroll_anchor_create(cmp_scroll_anchor_t **out_anchor);

/**
 * @brief Destroy a scroll anchor context
 */
int cmp_scroll_anchor_destroy(cmp_scroll_anchor_t *anchor);

/**
 * @brief Save the current visual position of an anchor element before layout
 * mutation
 *
 * @param anchor The scroll anchor context
 * @param element_id An opaque identifier for the anchored element
 * @param visual_offset_y The element's current visual offset from the viewport
 * top
 * @return 0 on success
 */
int cmp_scroll_anchor_save(cmp_scroll_anchor_t *anchor, unsigned int element_id,
                           float visual_offset_y);

/**
 * @brief Calculate the necessary scroll delta to restore the anchor's visual
 * position after layout mutation
 *
 * @param anchor The scroll anchor context
 * @param element_id The identifier for the anchored element
 * @param new_visual_offset_y The element's new visual offset from the viewport
 * top
 * @param out_scroll_delta_y Pointer to receive the scroll adjustment needed to
 * restore visual position
 * @return 0 on success
 */
int cmp_scroll_anchor_restore(const cmp_scroll_anchor_t *anchor,
                              unsigned int element_id,
                              float new_visual_offset_y,
                              float *out_scroll_delta_y);

/**
 * @brief Pull-to-Refresh State
 */
typedef enum cmp_ptr_state {
  CMP_PTR_STATE_IDLE = 0,
  CMP_PTR_STATE_PULLING = 1,
  CMP_PTR_STATE_READY_TO_REFRESH = 2,
  CMP_PTR_STATE_REFRESHING = 3
} cmp_ptr_state_t;

/**
 * @brief Opaque Pull-to-Refresh Context
 */
typedef struct cmp_ptr cmp_ptr_t;

/**
 * @brief Initialize a pull-to-refresh context
 */
int cmp_ptr_create(cmp_ptr_t **out_ptr);

/**
 * @brief Destroy a pull-to-refresh context
 */
int cmp_ptr_destroy(cmp_ptr_t *ptr);

/**
 * @brief Update the pull-to-refresh state with current overscroll amount
 *
 * @param ptr The pull-to-refresh context
 * @param overscroll_y The current negative overscroll distance (pixels pulled
 * past the top edge)
 * @param threshold_y The distance required to transition to READY_TO_REFRESH
 * @param out_state Pointer to receive the updated state
 * @param out_progress Pointer to receive a normalized 0.0 to 1.0 progress value
 * @return 0 on success
 */
int cmp_ptr_update(cmp_ptr_t *ptr, float overscroll_y, float threshold_y,
                   cmp_ptr_state_t *out_state, float *out_progress);

/**
 * @brief Notify the pull-to-refresh context that a refresh action has started
 * or ended
 *
 * @param ptr The pull-to-refresh context
 * @param is_refreshing 1 to enter refreshing state, 0 to return to idle
 * @return 0 on success
 */
int cmp_ptr_set_refreshing(cmp_ptr_t *ptr, int is_refreshing);

/* ========================================================================= */
/* Phase 19: Motion: Transitions, Keyframes & View Morphing                  */
/* ========================================================================= */

/**
 * @brief Opaque render tick state
 */
typedef struct cmp_tick cmp_tick_t;

/**
 * @brief Create a main render tick controller
 */
int cmp_tick_create(cmp_tick_t **out_tick);

/**
 * @brief Destroy a main render tick controller
 */
int cmp_tick_destroy(cmp_tick_t *tick);

/**
 * @brief Simulate an OS VSync signal and tick the animation engine
 *
 * @param tick The tick controller
 * @param os_timestamp_ms The high-resolution OS timestamp in milliseconds
 * @param out_dt_ms Output for the calculated delta time since last tick
 * @return 0 on success
 */
int cmp_tick_fire(cmp_tick_t *tick, double os_timestamp_ms, double *out_dt_ms);

/**
 * @brief Delta Time measurement context
 */
typedef struct cmp_dt {
  double current_time_ms;
  double last_time_ms;
  double delta_time_ms;
} cmp_dt_t;

/**
 * @brief Update delta time metrics
 */
int cmp_dt_update(cmp_dt_t *dt, double current_time_ms);

/**
 * @brief Compositor Animation Property Target
 */
typedef enum cmp_compositor_prop {
  CMP_COMPOSITOR_PROP_OPACITY = 0,
  CMP_COMPOSITOR_PROP_TRANSFORM = 1
} cmp_compositor_prop_t;

/**
 * @brief Opaque Compositor-Only Animation state
 */
typedef struct cmp_compositor_anim cmp_compositor_anim_t;

/**
 * @brief Value for a compositor property
 */
typedef union cmp_compositor_val {
  float opacity;
  float transform_matrix[16];
} cmp_compositor_val_t;

/**
 * @brief Initialize a new compositor-only animation
 *
 * @param property The property to animate (opacity or transform)
 * @param out_anim The created animation context
 */
int cmp_compositor_anim_create(cmp_compositor_prop_t property,
                               cmp_compositor_anim_t **out_anim);

/**
 * @brief Destroy a compositor-only animation
 */
int cmp_compositor_anim_destroy(cmp_compositor_anim_t *anim);

/**
 * @brief Set the start and end values for the animation
 */
int cmp_compositor_anim_set_range(cmp_compositor_anim_t *anim,
                                  const cmp_compositor_val_t *start_val,
                                  const cmp_compositor_val_t *end_val);

/**
 * @brief Advance the compositor-only animation by delta time
 *
 * @param anim The animation context
 * @param dt_ms Delta time in milliseconds
 * @param duration_ms Total animation duration
 * @param out_val Computed output value (to be uploaded as GPU uniform)
 * @param out_finished Set to 1 if animation has completed, 0 otherwise
 */
int cmp_compositor_anim_step(cmp_compositor_anim_t *anim, double dt_ms,
                             double duration_ms, cmp_compositor_val_t *out_val,
                             int *out_finished);

/**
 * @brief Opaque CSS Transition state
 */
typedef struct cmp_transition cmp_transition_t;

/**
 * @brief CSS Transition Behavior
 */
typedef enum cmp_transition_behavior {
  CMP_TRANSITION_BEHAVIOR_NORMAL = 0,
  CMP_TRANSITION_BEHAVIOR_ALLOW_DISCRETE = 1
} cmp_transition_behavior_t;

/**
 * @brief Initialize a new implicit CSS transition
 *
 * @param duration_ms The duration of the transition
 * @param delay_ms The delay before starting
 * @param behavior Normal or allow-discrete
 * @param out_transition The created transition context
 */
int cmp_transition_create(double duration_ms, double delay_ms,
                          cmp_transition_behavior_t behavior,
                          cmp_transition_t **out_transition);

/**
 * @brief Destroy a CSS transition
 */
int cmp_transition_destroy(cmp_transition_t *transition);

/**
 * @brief Step a CSS transition forward in time
 *
 * @param transition The transition context
 * @param dt_ms Time elapsed since last frame
 * @param out_progress Output normalized progress (0.0 to 1.0)
 */
int cmp_transition_step(cmp_transition_t *transition, double dt_ms,
                        float *out_progress);

/**
 * @brief Opaque Keyframe Animation Context
 */
typedef struct cmp_keyframe cmp_keyframe_t;

/**
 * @brief CSS Animation Play State
 */
typedef enum cmp_animation_play_state {
  CMP_ANIMATION_PLAY_STATE_RUNNING = 0,
  CMP_ANIMATION_PLAY_STATE_PAUSED = 1
} cmp_animation_play_state_t;

/**
 * @brief CSS Animation Fill Mode
 */
typedef enum cmp_animation_fill_mode {
  CMP_ANIMATION_FILL_MODE_NONE = 0,
  CMP_ANIMATION_FILL_MODE_FORWARDS = 1,
  CMP_ANIMATION_FILL_MODE_BACKWARDS = 2,
  CMP_ANIMATION_FILL_MODE_BOTH = 3
} cmp_animation_fill_mode_t;

/**
 * @brief Create a keyframe animation context
 */
int cmp_keyframe_create(cmp_keyframe_t **out_keyframe);

/**
 * @brief Destroy a keyframe animation context
 */
int cmp_keyframe_destroy(cmp_keyframe_t *keyframe);

/**
 * @brief Add a percentage waypoint to the keyframe animation
 *
 * @param keyframe The keyframe context
 * @param percentage The waypoint (0.0 to 1.0)
 */
int cmp_keyframe_add_stop(cmp_keyframe_t *keyframe, float percentage);

/**
 * @brief Step the keyframe animation forward in time
 *
 * @param keyframe The keyframe context
 * @param dt_ms The time elapsed since last tick
 * @param play_state The current play state (running or paused)
 * @param out_current_percentage Output the current animation percentage mapped
 * to the timeline
 */
int cmp_keyframe_step(cmp_keyframe_t *keyframe, double dt_ms,
                      cmp_animation_play_state_t play_state,
                      float *out_current_percentage);

/**
 * @brief CSS Animation Composition Operator
 */
typedef enum cmp_anim_compose_op {
  CMP_ANIM_COMPOSE_REPLACE = 0,
  CMP_ANIM_COMPOSE_ADD = 1,
  CMP_ANIM_COMPOSE_ACCUMULATE = 2
} cmp_anim_compose_op_t;

/**
 * @brief Compose two numerical animation values based on the operator
 *
 * @param base_value The underlying value
 * @param anim_value The animation layer value
 * @param op The composition operator
 * @param out_value Output computed value
 */
int cmp_anim_compose_numerical(float base_value, float anim_value,
                               cmp_anim_compose_op_t op, float *out_value);

/**
 * @brief Opaque Spring Physics Interpolator Context
 */
typedef struct cmp_spring_ease cmp_spring_ease_t;

/**
 * @brief Create a spring ease context
 *
 * @param mass The mass of the spring
 * @param stiffness The stiffness of the spring
 * @param damping The damping factor
 * @param initial_velocity The initial velocity
 * @param out_spring Output spring context
 */
int cmp_spring_ease_create(float mass, float stiffness, float damping,
                           float initial_velocity,
                           cmp_spring_ease_t **out_spring);

/**
 * @brief Destroy a spring ease context
 */
int cmp_spring_ease_destroy(cmp_spring_ease_t *spring);

/**
 * @brief Calculate the current value of the spring physics simulation
 *
 * @param spring The spring context
 * @param t Time elapsed
 * @param out_value Evaluated progress
 */
int cmp_spring_ease_evaluate(cmp_spring_ease_t *spring, double t,
                             float *out_value);

/**
 * @brief Opaque Cubic Bezier Interpolator Context
 */
typedef struct cmp_bezier_ease cmp_bezier_ease_t;

/**
 * @brief Create a cubic bezier ease context
 *
 * @param x1 Control point 1 X
 * @param y1 Control point 1 Y
 * @param x2 Control point 2 X
 * @param y2 Control point 2 Y
 * @param out_bezier Output bezier context
 */
int cmp_bezier_ease_create(float x1, float y1, float x2, float y2,
                           cmp_bezier_ease_t **out_bezier);

/**
 * @brief Destroy a cubic bezier ease context
 */
int cmp_bezier_ease_destroy(cmp_bezier_ease_t *bezier);

/**
 * @brief Evaluate the cubic bezier curve at time t (0.0 to 1.0)
 *
 * @param bezier The bezier context
 * @param t Normalized time
 * @param out_value Evaluated progress
 */
int cmp_bezier_ease_evaluate(cmp_bezier_ease_t *bezier, float t,
                             float *out_value);

/**
 * @brief Opaque Step Interpolator Context
 */
typedef struct cmp_step_ease cmp_step_ease_t;

/**
 * @brief CSS Steps Position
 */
typedef enum cmp_step_position {
  CMP_STEP_POSITION_START = 0,
  CMP_STEP_POSITION_END = 1
} cmp_step_position_t;

/**
 * @brief Create a step ease context
 *
 * @param steps Number of steps
 * @param position Step position (start or end)
 * @param out_step Output step context
 */
int cmp_step_ease_create(int steps, cmp_step_position_t position,
                         cmp_step_ease_t **out_step);

/**
 * @brief Destroy a step ease context
 */
int cmp_step_ease_destroy(cmp_step_ease_t *step);

/**
 * @brief Evaluate the step function at time t (0.0 to 1.0)
 *
 * @param step The step context
 * @param t Normalized time
 * @param out_value Evaluated progress
 */
int cmp_step_ease_evaluate(cmp_step_ease_t *step, float t, float *out_value);

/**
 * @brief Opaque Motion Path Animation Context
 */
typedef struct cmp_motion_path cmp_motion_path_t;

/**
 * @brief Create a motion path context
 */
int cmp_motion_path_create(cmp_motion_path_t **out_path);

/**
 * @brief Destroy a motion path context
 */
int cmp_motion_path_destroy(cmp_motion_path_t *path);

/**
 * @brief Evaluate position and rotation along the motion path
 *
 * @param path The motion path context
 * @param distance The offset distance (0.0 to 1.0)
 * @param offset_rotate Additional rotation offset in degrees
 * @param out_x Evaluated X position
 * @param out_y Evaluated Y position
 * @param out_angle Evaluated rotation angle
 */
int cmp_motion_path_evaluate(cmp_motion_path_t *path, float distance,
                             float offset_rotate, float *out_x, float *out_y,
                             float *out_angle);

/**
 * @brief Opaque Scroll-Driven Animation Timeline Context
 */
typedef struct cmp_scroll_timeline cmp_scroll_timeline_t;

/**
 * @brief Create a scroll timeline context
 */
int cmp_scroll_timeline_create(cmp_scroll_timeline_t **out_timeline);

/**
 * @brief Destroy a scroll timeline context
 */
int cmp_scroll_timeline_destroy(cmp_scroll_timeline_t *timeline);

/**
 * @brief Evaluate the progress of a scroll timeline based on scroll offset
 *
 * @param timeline The scroll timeline context
 * @param scroll_offset Current scroll offset
 * @param max_scroll_offset Maximum scroll offset
 * @param out_progress Evaluated progress (0.0 to 1.0)
 */
int cmp_scroll_timeline_evaluate(cmp_scroll_timeline_t *timeline,
                                 float scroll_offset, float max_scroll_offset,
                                 float *out_progress);

/**
 * @brief Opaque View Transition Context
 */
typedef struct cmp_view_transition cmp_view_transition_t;

/**
 * @brief Create a view transition context
 */
int cmp_view_transition_create(cmp_view_transition_t **out_transition);

/**
 * @brief Destroy a view transition context
 */
int cmp_view_transition_destroy(cmp_view_transition_t *transition);

/**
 * @brief Start a view transition between old and new state
 */
int cmp_view_transition_start(cmp_view_transition_t *transition);

/**
 * @brief Opaque View Transition Shared Element Context
 */
typedef struct cmp_vt_shared cmp_vt_shared_t;

/**
 * @brief Create a view transition shared element context
 */
int cmp_vt_shared_create(cmp_vt_shared_t **out_shared);

/**
 * @brief Destroy a view transition shared element context
 */
int cmp_vt_shared_destroy(cmp_vt_shared_t *shared);

/**
 * @brief Calculate transform matrix to morph from old rect to new rect
 */
int cmp_vt_shared_calculate_morph(cmp_vt_shared_t *shared,
                                  const cmp_rect_t *old_rect,
                                  const cmp_rect_t *new_rect, float progress,
                                  cmp_rect_t *out_rect);

/**
 * @brief Opaque Discrete Property Transition Context
 */
typedef struct cmp_discrete_transition cmp_discrete_transition_t;

/**
 * @brief Create a discrete transition context
 */
int cmp_discrete_transition_create(cmp_discrete_transition_t **out_transition);

/**
 * @brief Destroy a discrete transition context
 */
int cmp_discrete_transition_destroy(cmp_discrete_transition_t *transition);

/**
 * @brief Evaluate discrete transition state
 */
int cmp_discrete_transition_evaluate(cmp_discrete_transition_t *transition,
                                     float progress, int *out_is_visible);

/**
 * @brief Opaque FLIP Animation Helper Context
 */
typedef struct cmp_flip cmp_flip_t;

/**
 * @brief Create a FLIP animation context
 */
int cmp_flip_create(cmp_flip_t **out_flip);

/**
 * @brief Destroy a FLIP animation context
 */
int cmp_flip_destroy(cmp_flip_t *flip);

/**
 * @brief Perform the 'First' step of FLIP (record initial bounds)
 */
int cmp_flip_first(cmp_flip_t *flip, const cmp_rect_t *initial_bounds);

/**
 * @brief Perform the 'Last' and 'Invert' steps of FLIP (record final bounds,
 * calculate inverse transform)
 */
int cmp_flip_last_and_invert(cmp_flip_t *flip, const cmp_rect_t *final_bounds,
                             float *out_translate_x, float *out_translate_y,
                             float *out_scale_x, float *out_scale_y);

/* ========================================================================= */
/* Phase 20: Forms, Text Editing & Input Methods (IME)                       */
/* ========================================================================= */

/**
 * @brief Opaque Form Controls State Context
 */
typedef struct cmp_form_controls cmp_form_controls_t;

/**
 * @brief Create a form controls context
 */
int cmp_form_controls_create(cmp_form_controls_t **out_controls);

/**
 * @brief Destroy a form controls context
 */
int cmp_form_controls_destroy(cmp_form_controls_t *controls);

/**
 * @brief Opaque Validation State Context
 */
typedef struct cmp_validation cmp_validation_t;

/**
 * @brief Create a validation context
 */
int cmp_validation_create(cmp_validation_t **out_validation);

/**
 * @brief Destroy a validation context
 */
int cmp_validation_destroy(cmp_validation_t *validation);

/**
 * @brief Validate an input string against HTML5 constraints
 */
int cmp_validation_check(cmp_validation_t *validation, const char *input_value,
                         int *out_is_valid);

/**
 * @brief Opaque Input Mask Context
 */
typedef struct cmp_input_mask cmp_input_mask_t;

/**
 * @brief Create an input mask context
 */
int cmp_input_mask_create(const char *mask_pattern,
                          cmp_input_mask_t **out_mask);

/**
 * @brief Destroy an input mask context
 */
int cmp_input_mask_destroy(cmp_input_mask_t *mask);

/**
 * @brief Apply mask formatting to a raw input string
 */
int cmp_input_mask_apply(cmp_input_mask_t *mask, const char *raw_input,
                         char *out_buffer, size_t out_capacity);

/**
 * @brief Opaque Indeterminate State Context
 */
typedef struct cmp_indeterminate cmp_indeterminate_t;

/**
 * @brief Create an indeterminate state context
 */
int cmp_indeterminate_create(cmp_indeterminate_t **out_state);

/**
 * @brief Destroy an indeterminate state context
 */
int cmp_indeterminate_destroy(cmp_indeterminate_t *state);

/**
 * @brief Set the indeterminate state (0=false, 1=true)
 */
int cmp_indeterminate_set(cmp_indeterminate_t *state, int is_indeterminate);

/**
 * @brief Opaque Select UI Context
 */
typedef struct cmp_select_ui cmp_select_ui_t;

/**
 * @brief Create a select UI context
 */
int cmp_select_ui_create(cmp_select_ui_t **out_select);

/**
 * @brief Destroy a select UI context
 */
int cmp_select_ui_destroy(cmp_select_ui_t *select);

/**
 * @brief Open the select dropdown or native picker
 */
int cmp_select_ui_open(cmp_select_ui_t *select);

/**
 * @brief Opaque Datalist Context
 */
typedef struct cmp_datalist cmp_datalist_t;

/**
 * @brief Create a datalist context
 */
int cmp_datalist_create(cmp_datalist_t **out_datalist);

/**
 * @brief Destroy a datalist context
 */
int cmp_datalist_destroy(cmp_datalist_t *datalist);

/**
 * @brief Filter datalist options based on input string
 */
int cmp_datalist_filter(cmp_datalist_t *datalist, const char *input_string);

/**
 * @brief Opaque Range Slider Context
 */
typedef struct cmp_range_slider cmp_range_slider_t;

/**
 * @brief Create a range slider context
 */
int cmp_range_slider_create(cmp_range_slider_t **out_slider);

/**
 * @brief Destroy a range slider context
 */
int cmp_range_slider_destroy(cmp_range_slider_t *slider);

/**
 * @brief Update the value of a specific thumb on the slider
 */
int cmp_range_slider_set_value(cmp_range_slider_t *slider, int thumb_index,
                               float value);

/**
 * @brief Opaque Color Picker Context
 */
typedef struct cmp_color_picker cmp_color_picker_t;

/**
 * @brief Create a color picker context
 */
int cmp_color_picker_create(cmp_color_picker_t **out_picker);

/**
 * @brief Destroy a color picker context
 */
int cmp_color_picker_destroy(cmp_color_picker_t *picker);

/**
 * @brief Opaque Date Picker Context
 */
typedef struct cmp_date_picker cmp_date_picker_t;

/**
 * @brief Create a date picker context
 */
int cmp_date_picker_create(cmp_date_picker_t **out_picker);

/**
 * @brief Destroy a date picker context
 */
int cmp_date_picker_destroy(cmp_date_picker_t *picker);

/**
 * @brief Opaque Caret Rendering Context
 */
typedef struct cmp_caret cmp_caret_t;

/**
 * @brief Create a caret context
 */
int cmp_caret_create(cmp_caret_t **out_caret);

/**
 * @brief Destroy a caret context
 */
int cmp_caret_destroy(cmp_caret_t *caret);

/**
 * @brief Update the caret blink state based on delta time
 */
int cmp_caret_update_blink(cmp_caret_t *caret, double dt_ms,
                           int *out_is_visible);

/**
 * @brief Opaque Text Selection Context
 */
typedef struct cmp_selection cmp_selection_t;

/**
 * @brief Create a selection context
 */
int cmp_selection_create(cmp_selection_t **out_selection);

/**
 * @brief Destroy a selection context
 */
int cmp_selection_destroy(cmp_selection_t *selection);

/**
 * @brief Opaque Content Editable Context
 */
typedef struct cmp_editable cmp_editable_t;

/**
 * @brief Create an editable context
 */
int cmp_editable_create(cmp_editable_t **out_editable);

/**
 * @brief Destroy an editable context
 */
int cmp_editable_destroy(cmp_editable_t *editable);

/**
 * @brief Insert text at the current cursor position
 */
int cmp_editable_insert_text(cmp_editable_t *editable, const char *text);

/**
 * @brief Opaque IME Context
 */
typedef struct cmp_ime cmp_ime_t;

/**
 * @brief Create an IME context
 */
int cmp_ime_create(cmp_ime_t **out_ime);

/**
 * @brief Destroy an IME context
 */
int cmp_ime_destroy(cmp_ime_t *ime);

/**
 * @brief Handle an incoming IME composition string
 */
int cmp_ime_update_composition(cmp_ime_t *ime, const char *composition_string);

/**
 * @brief Opaque Spellcheck Context
 */
typedef struct cmp_spellcheck cmp_spellcheck_t;

/**
 * @brief Create a spellcheck context
 */
int cmp_spellcheck_create(cmp_spellcheck_t **out_spellcheck);

/**
 * @brief Destroy a spellcheck context
 */
int cmp_spellcheck_destroy(cmp_spellcheck_t *spellcheck);

/**
 * @brief Check a word for spelling errors
 */
int cmp_spellcheck_verify_word(cmp_spellcheck_t *spellcheck, const char *word,
                               int *out_is_correct);

/**
 * @brief Opaque Undo/Redo Stack Context
 */
typedef struct cmp_undo_redo cmp_undo_redo_t;

/**
 * @brief Create an undo/redo stack context
 */
int cmp_undo_redo_create(cmp_undo_redo_t **out_stack);

/**
 * @brief Destroy an undo/redo stack context
 */
int cmp_undo_redo_destroy(cmp_undo_redo_t *stack);

/**
 * @brief Push a new state onto the undo stack
 */
int cmp_undo_redo_push(cmp_undo_redo_t *stack, const char *state);

/**
 * @brief Perform an undo operation
 */
int cmp_undo_redo_undo(cmp_undo_redo_t *stack, char *out_buffer,
                       size_t out_capacity);

#ifndef CMP_TEXTURE_T_DEFINED
#define CMP_TEXTURE_T_DEFINED
typedef struct cmp_texture cmp_texture_t;
#endif

#ifndef CMP_TEXTURE_T_DEFINED
#define CMP_TEXTURE_T_DEFINED
typedef struct cmp_texture cmp_texture_t;
#endif

/**
 * @brief Abstract representation of a compiled Audio buffer
 */
typedef struct cmp_audio_buffer {
  void *internal_handle;
  int channels;
  int sample_rate;
} cmp_audio_buffer_t;

/**
 * @brief Abstract representation of an active Audio source (playback instance)
 */
typedef struct cmp_audio_source {
  void *internal_handle;
  float volume;
  float pan_x;
  float pan_y;
  float pan_z;
  int is_playing;
  int is_looping;
} cmp_audio_source_t;

/**
 * @brief Abstract representation of a Video Decoder
 */
typedef struct cmp_video_decoder {
  void *internal_handle;
  int width;
  int height;
  float framerate;
  int is_playing;
} cmp_video_decoder_t;

/**
 * @brief Initialize the global Audio subsystem
 * @return 0 on success, or an error code.
 */
int cmp_audio_init(void);

/**
 * @brief Shutdown the global Audio subsystem
 * @return 0 on success, or an error code.
 */
int cmp_audio_shutdown(void);

/**
 * @brief Load an audio file (WAV/MP3/OGG) into memory from the VFS
 * @param virtual_path VFS path to the audio file
 * @param out_buffer Pointer to receive the allocated audio buffer
 * @return 0 on success, or an error code.
 */
int cmp_audio_buffer_load(const char *virtual_path,
                          cmp_audio_buffer_t **out_buffer);

/**
 * @brief Destroy an audio buffer
 * @param buffer The buffer to destroy
 * @return 0 on success, or an error code.
 */
int cmp_audio_buffer_destroy(cmp_audio_buffer_t *buffer);

/**
 * @brief Create an audio playback source from a buffer
 * @param buffer The loaded audio buffer
 * @param out_source Pointer to receive the allocated audio source
 * @return 0 on success, or an error code.
 */
int cmp_audio_source_create(cmp_audio_buffer_t *buffer,
                            cmp_audio_source_t **out_source);

/**
 * @brief Start playing an audio source
 * @param source The audio source to play
 * @return 0 on success, or an error code.
 */
int cmp_audio_source_play(cmp_audio_source_t *source);

/**
 * @brief Set the spatial panning position of an audio source
 * @param source The audio source
 * @param x X coordinate relative to listener
 * @param y Y coordinate relative to listener
 * @param z Z coordinate relative to listener
 * @return 0 on success, or an error code.
 */
int cmp_audio_source_set_position(cmp_audio_source_t *source, float x, float y,
                                  float z);

/**
 * @brief Destroy an audio playback source
 * @param source The audio source to destroy
 * @return 0 on success, or an error code.
 */
int cmp_audio_source_destroy(cmp_audio_source_t *source);

/**
 * @brief Open a video file via the OS native decoder
 * (MediaFoundation/AVFoundation/FFmpeg)
 * @param virtual_path VFS path to the video file
 * @param out_decoder Pointer to receive the allocated video decoder
 * @return 0 on success, or an error code.
 */
int cmp_video_decoder_open(const char *virtual_path,
                           cmp_video_decoder_t **out_decoder);

/**
 * @brief Read the next available frame from the video decoder into a texture
 * @param decoder The video decoder
 * @param target_texture The texture to update with the new frame data
 * @return 0 on success, or an error code.
 */
int cmp_video_decoder_read_frame(cmp_video_decoder_t *decoder,
                                 cmp_texture_t *target_texture);

/**
 * @brief Destroy a video decoder
 * @param decoder The video decoder to destroy
 * @return 0 on success, or an error code.
 */
int cmp_video_decoder_destroy(cmp_video_decoder_t *decoder);

#ifndef CMP_TEXTURE_T_DEFINED
#define CMP_TEXTURE_T_DEFINED
typedef struct cmp_texture cmp_texture_t;
#endif

/**
 * @brief Window abstraction handle
 */
#ifndef CMP_WINDOW_T_DEFINED
#define CMP_WINDOW_T_DEFINED
typedef struct cmp_window cmp_window_t;
#endif

/**
 * @brief Hardware Peripherals & Sensors
 */

typedef struct cmp_gamepad {
  int id;
  int is_connected;
  float axes[6];
  int buttons[16];
} cmp_gamepad_t;

typedef struct cmp_camera {
  void *internal_handle;
  int width;
  int height;
  int is_capturing;
} cmp_camera_t;

typedef struct cmp_sensor_data {
  float accel_x, accel_y, accel_z;
  float gyro_x, gyro_y, gyro_z;
  float mag_x, mag_y, mag_z;
} cmp_sensor_data_t;

/**
 * @brief Poll the current state of a connected Gamepad
 * @param index Gamepad index (e.g. 0-3)
 * @param out_gamepad Pointer to receive state
 * @return 0 on success, or an error code.
 */
int cmp_hardware_poll_gamepad(int index, cmp_gamepad_t *out_gamepad);

/**
 * @brief Trigger a haptic vibration motor
 * @param index Gamepad/Device index
 * @param low_frequency Intensity for low-frequency motor (0.0 to 1.0)
 * @param high_frequency Intensity for high-frequency motor (0.0 to 1.0)
 * @param duration_ms Duration in milliseconds
 * @return 0 on success, or an error code.
 */
int cmp_hardware_trigger_haptic(int index, float low_frequency,
                                float high_frequency, int duration_ms);

/**
 * @brief Start capturing frames from a physical camera
 * @param device_index Camera index
 * @param out_camera Pointer to receive camera handle
 * @return 0 on success, or an error code.
 */
int cmp_hardware_camera_start(int device_index, cmp_camera_t **out_camera);

/**
 * @brief Read the latest frame from an active camera into a texture
 * @param camera Camera handle
 * @param target_texture Texture to receive the frame
 * @return 0 on success, or an error code.
 */
int cmp_hardware_camera_read_frame(cmp_camera_t *camera,
                                   cmp_texture_t *target_texture);

/**
 * @brief Stop capturing frames and destroy camera handle
 * @param camera Camera handle
 * @return 0 on success, or an error code.
 */
int cmp_hardware_camera_stop(cmp_camera_t *camera);

/**
 * @brief Poll current orientation sensors (accelerometer, gyroscope,
 * magnetometer)
 * @param out_data Pointer to receive sensor data
 * @return 0 on success, or an error code.
 */
int cmp_hardware_poll_sensors(cmp_sensor_data_t *out_data);

/**
 * @brief Poll OS location services
 * @param out_latitude Pointer to receive latitude
 * @param out_longitude Pointer to receive longitude
 * @return 0 on success, or an error code.
 */
int cmp_hardware_poll_geolocation(double *out_latitude, double *out_longitude);

/**
 * @brief Request pointer capture/lock on the given window
 * @param window The window requesting capture
 * @param lock_mode The requested lock mode
 * @return 0 on success, or an error code.
 */
int cmp_window_set_pointer_lock(cmp_window_t *window,
                                cmp_pointer_lock_t lock_mode);

/**
 * @brief Set logical UI focus to a specific element ID
 * @param element_id The ID of the UI node to focus
 * @return 0 on success, or an error code.
 */
int cmp_event_set_focus(int element_id);

/**
 * @brief Get the ID of the currently focused UI node
 * @return Node ID or -1 if no focus
 */
int cmp_event_get_focus(void);

/**
 * @brief Check if the accessibility/keyboard focus ring should be visibly drawn
 * @return 1 if visible, 0 if hidden (e.g. user clicked with a mouse instead of
 * tabbing)
 */
int cmp_event_is_focus_ring_visible(void);

/**
 * @brief Clear the current logical focus and hide the focus ring
 * @return 0 on success, or an error code.
 */
int cmp_event_clear_focus(void);

/**
 * @brief Trigger the semantic focus traversal algorithm (Tab / Shift+Tab)
 * @param reverse 1 to traverse backwards (Shift+Tab), 0 to traverse forwards
 * (Tab)
 * @return 0 on success, or an error code.
 */
int cmp_event_handle_tab_targeting(int reverse);

/**
 * @brief Abstract Router Context
 */
typedef struct cmp_router cmp_router_t;

/**
 * @brief Router View Constructor Callback
 */
typedef void *(*cmp_route_builder_cb)(const char *route_params);

/**
 * @brief Route Guard Middleware Callback
 * @return 1 to allow route transition, 0 to block
 */
typedef int (*cmp_route_guard_cb)(const char *to_route, void *user_data);

/**
 * @brief Initialize a hierarchical stack-based router
 * @param out_router Pointer to receive the router handle
 * @return 0 on success, or an error code.
 */
int cmp_router_create(cmp_router_t **out_router);

/**
 * @brief Destroy a router instance
 * @param router The router to destroy
 * @return 0 on success, or an error code.
 */
int cmp_router_destroy(cmp_router_t *router);

/**
 * @brief Register a route path with a view builder and optional guard
 * @param router The router instance
 * @param path The route path (e.g. "/settings/profile/:id")
 * @param builder The callback to construct the view
 * @param guard Optional guard middleware (can be NULL)
 * @param guard_data Data passed to the guard
 * @return 0 on success, or an error code.
 */
int cmp_router_register(cmp_router_t *router, const char *path,
                        cmp_route_builder_cb builder, cmp_route_guard_cb guard,
                        void *guard_data);

/**
 * @brief Push a new route onto the navigation stack
 * @param router The router instance
 * @param uri The URI to navigate to (e.g. "/settings/profile/42")
 * @return 0 on success, or an error code.
 */
int cmp_router_push(cmp_router_t *router, const char *uri);

/**
 * @brief Replace the current route without growing the stack
 * @param router The router instance
 * @param uri The URI to navigate to
 * @return 0 on success, or an error code.
 */
int cmp_router_replace(cmp_router_t *router, const char *uri);

/**
 * @brief Initialize route transition animations
 * @param router The router instance
 * @param enable 1 to enable slide/fade transitions, 0 to disable
 * @return 0 on success, or an error code.
 */
int cmp_router_set_transitions(cmp_router_t *router, int enable);

/**
 * @brief Hook into WebAssembly History API
 * @param router The router instance to bind
 * @return 0 on success, or an error code.
 */
int cmp_router_wasm_bind_history(cmp_router_t *router);
int cmp_router_pop(cmp_router_t *router);

/**
 * @brief Get the current active route URI
 * @param router The router instance
 * @param out_uri Pointer to string to hold the URI. Must be destroyed with
 * cmp_string_destroy.
 * @return 0 on success, or an error code.
 */
int cmp_router_get_current(cmp_router_t *router, cmp_string_t *out_uri);

/**
 * @brief Register an OS-level URI scheme (e.g. myapp://)
 * @param scheme The custom scheme (e.g. "myapp")
 * @return 0 on success, or an error code.
 */
int cmp_os_register_uri_scheme(const char *scheme);

/**
 * @brief Core layout dimensions and bounds
 */
struct cmp_rect {
  float x;
  float y;
  float width;
  float height;
};

/**
 * @brief Flexbox Alignment Options
 */
typedef enum cmp_flex_align {
  CMP_FLEX_ALIGN_START = 0,
  CMP_FLEX_ALIGN_CENTER = 1,
  CMP_FLEX_ALIGN_END = 2,
  CMP_FLEX_ALIGN_STRETCH = 3,
  CMP_FLEX_ALIGN_SPACE_BETWEEN = 4,
  CMP_FLEX_ALIGN_SPACE_AROUND = 5,
  CMP_FLEX_ALIGN_BASELINE = 6
} cmp_flex_align_t;

/**
 * @brief Flexbox Direction
 */
typedef enum cmp_flex_direction {
  CMP_FLEX_ROW = 0,
  CMP_FLEX_COLUMN = 1
} cmp_flex_direction_t;

/**
 * @brief Flexbox Wrap Options
 */
typedef enum cmp_flex_wrap {
  CMP_FLEX_NOWRAP = 0,
  CMP_FLEX_WRAP = 1,
  CMP_FLEX_WRAP_REVERSE = 2
} cmp_flex_wrap_t;

/**
 * @brief Positioning Mode
 */
typedef enum cmp_position_type {
  CMP_POSITION_RELATIVE = 0,
  CMP_POSITION_ABSOLUTE = 1,
  CMP_POSITION_FIXED = 2,
  CMP_POSITION_STICKY = 3
} cmp_position_type_t;

/**
 * @brief Node definition for the generic UI Tree layout engine
 */
typedef struct cmp_layout_node {
  int id;
  cmp_flex_direction_t direction;
  cmp_flex_wrap_t flex_wrap;
  cmp_flex_align_t justify_content;
  cmp_flex_align_t align_items;
  cmp_position_type_t position_type;

  /* Style constraints */
  float width;
  float height;
  float min_width;
  float min_height;
  float max_width;
  float max_height;
  float aspect_ratio; /* Width / Height ratio for locking, <=0 to disable */
  float flex_grow;
  float flex_shrink;
  float margin[4];   /* Top, Right, Bottom, Left */
  float padding[4];  /* Top, Right, Bottom, Left */
  float position[4]; /* Top, Right, Bottom, Left (for absolute positioning) */
  int z_index;

  /* Multi-column layout */
  int column_count;
  float column_gap;
  float column_width;

  /* Scroll View Constraints */
  int overflow_x; /* 0=Visible, 1=Scroll, 2=Hidden */
  int overflow_y; /* 0=Visible, 1=Scroll, 2=Hidden */
  float scroll_offset_x;
  float scroll_offset_y;
  cmp_rect_t scroll_content_size;

  /* Computed layout */
  cmp_rect_t computed_rect;

  /* Hierarchy */
  struct cmp_layout_node *parent;
  struct cmp_layout_node **children;
  size_t child_count;
  size_t child_capacity;
} cmp_layout_node_t;

/**
 * @brief Create a new layout node
 * @param out_node Pointer to receive the allocated node
 * @return 0 on success, or an error code.
 */
int cmp_layout_node_create(cmp_layout_node_t **out_node);

/**
 * @brief Destroy a layout node and its children
 * @param node Node to destroy
 * @return 0 on success, or an error code.
 */
int cmp_layout_node_destroy(cmp_layout_node_t *node);

/**
 * @brief Add a child to a layout node
 * @param parent Parent node
 * @param child Child node
 * @return 0 on success, or an error code.
 */
int cmp_layout_node_add_child(cmp_layout_node_t *parent,
                              cmp_layout_node_t *child);

/**
 * @brief Execute the Measure & Layout pass on a tree
 * @param root The root node of the tree
 * @param available_width The total available width
 * @param available_height The total available height
 * @return 0 on success, or an error code.
 */
int cmp_layout_calculate(cmp_layout_node_t *root, float available_width,
                         float available_height);

/**
 * @brief Calculate the Block Formatting Context for a node.
 * @param node The layout node.
 * @param available_width The available width for the BFC.
 * @return 0 on success, or an error code.
 */
int cmp_bfc_calculate(cmp_layout_node_t *node, float available_width);

/**
 * @brief Calculate the Inline Formatting Context for a node.
 * @param node The layout node.
 * @param available_width The available width for the IFC.
 * @return 0 on success, or an error code.
 */
int cmp_ifc_calculate(cmp_layout_node_t *node, float available_width);

/**
 * @brief Evaluate float displacement and clearing for a node.
 * @param node The layout node.
 * @param is_float Non-zero if the node is a float.
 * @param clear Non-zero if the node clears previous floats.
 * @param out_x Pointer to receive the calculated X displacement.
 * @param out_y Pointer to receive the calculated Y displacement.
 * @return 0 on success, or an error code.
 */
int cmp_float_evaluate(cmp_layout_node_t *node, int is_float, int clear,
                       float *out_x, float *out_y);

/**
 * @brief Evaluate the shape-outside property for a floating node.
 * @param node The layout node.
 * @param float_rect The bounding box of the float.
 * @param shape_radius The radius for circular shapes, or 0.
 * @param margin The margin to apply around the shape.
 * @return 0 on success, or an error code.
 */
int cmp_shape_outside_evaluate(cmp_layout_node_t *node, cmp_rect_t float_rect,
                               float shape_radius, float margin);

int cmp_table_evaluate(cmp_layout_node_t *node, int is_fixed);

/**
 * @brief Perform table border collapse resolution.
 * @param table The table layout node.
 * @return 0 on success, or an error code.
 */
int cmp_table_border_collapse(cmp_layout_node_t *table);

/**
 * @brief Abstract representation of a UI component node mapped to the layout
 * engine
 */
struct cmp_ui_node {
  cmp_layout_node_t *layout;
  int type; /* 1=Box, 2=Text, 3=Button, 4=TextInput, 5=Checkbox, 6=ImageView,
               7=Slider */

  /* Specific properties depending on type */
  void *properties;

  struct cmp_ui_node *parent;
  struct cmp_ui_node **children;
  size_t child_count;
  size_t child_capacity;

  uint8_t design_language_override : 3; /* 0=Inherit, 1=Material3, 2=Fluent2,
                                           3=Cupertino, 4=Unstyled */
  uint8_t
      density_override : 2; /* 0=Inherit, 1=Compact, 2=Standard, 3=Relaxed */
};

/**
 * @brief Initialize a base UI box component (container)
 * @param out_node Pointer to receive the allocated node
 * @return 0 on success, or an error code.
 */
int cmp_ui_box_create(cmp_ui_node_t **out_node);

/**
 * @brief Initialize a UI text label component
 * @param out_node Pointer to receive the allocated node
 * @param text Initial text
 * @param text_len Length of the text in bytes, or -1 if null-terminated
 * @return 0 on success, or an error code.
 */
int cmp_ui_text_create(cmp_ui_node_t **out_node, const char *text,
                       int text_len);

/**
 * @brief Initialize a UI button component
 * @param out_node Pointer to receive the allocated node
 * @param label Text to display inside the button
 * @param label_len Length of the label in bytes, or -1 if null-terminated
 * @return 0 on success, or an error code.
 */
int cmp_ui_button_create(cmp_ui_node_t **out_node, const char *label,
                         int label_len);

/**
 * @brief Initialize a UI text input component
 * @param out_node Pointer to receive the allocated node
 * @return 0 on success, or an error code.
 */
int cmp_ui_text_input_create(cmp_ui_node_t **out_node);

/**
 * @brief Initialize a UI checkbox component
 * @param out_node Pointer to receive the allocated node
 * @param label Text to display next to the checkbox
 * @return 0 on success, or an error code.
 */
int cmp_ui_checkbox_create(cmp_ui_node_t **out_node, const char *label);

/**
 * @brief Initialize a UI radio button component
 * @param out_node Pointer to receive the allocated node
 * @param group_id ID associating radio buttons together
 * @return 0 on success, or an error code.
 */
int cmp_ui_radio_create(cmp_ui_node_t **out_node, int group_id);

/**
 * @brief Initialize a UI image view component
 * @param out_node Pointer to receive the allocated node
 * @param image_path Path to the image asset
 * @return 0 on success, or an error code.
 */
int cmp_ui_image_view_create(cmp_ui_node_t **out_node, const char *image_path);

/**
 * @brief Initialize a UI slider component
 * @param out_node Pointer to receive the allocated node
 * @param min Minimum bounds
 * @param max Maximum bounds
 * @return 0 on success, or an error code.
 */
int cmp_ui_slider_create(cmp_ui_node_t **out_node, float min, float max);

/**
 * @brief Initialize a UI list view component (virtualized)
 * @param out_node Pointer to receive the allocated node
 * @return 0 on success, or an error code.
 */
int cmp_ui_list_view_create(cmp_ui_node_t **out_node);

/**
 * @brief Initialize a UI grid view component (virtualized)
 * @param out_node Pointer to receive the allocated node
 * @param columns Number of columns in the grid
 * @return 0 on success, or an error code.
 */
int cmp_ui_grid_view_create(cmp_ui_node_t **out_node, int columns);

/**
 * @brief Initialize a UI dropdown component
 * @param out_node Pointer to receive the allocated node
 * @return 0 on success, or an error code.
 */
int cmp_ui_dropdown_create(cmp_ui_node_t **out_node);

/**
 * @brief Initialize a UI modal overlay component
 * @param out_node Pointer to receive the allocated node
 * @return 0 on success, or an error code.
 */
int cmp_ui_modal_create(cmp_ui_node_t **out_node);

/**
 * @brief Initialize a UI 2D drawing canvas component
 * @param out_node Pointer to receive the allocated node
 * @return 0 on success, or an error code.
 */
int cmp_ui_canvas_create(cmp_ui_node_t **out_node);

/**
 * @brief Initialize a UI rich text editor component
 * @param out_node Pointer to receive the allocated node
 * @return 0 on success, or an error code.
 */
int cmp_ui_rich_text_create(cmp_ui_node_t **out_node);

/**
 * @brief Add a UI child to a UI parent
 * @param parent Parent node
 * @param child Child node
 * @return 0 on success, or an error code.
 */
int cmp_ui_node_add_child(cmp_ui_node_t *parent, cmp_ui_node_t *child);

/**
 * @brief Destroy a UI node and its children
 * @param node Node to destroy
 * @return 0 on success, or an error code.
 */
int cmp_ui_node_destroy(cmp_ui_node_t *node);

#ifndef CMP_WINDOW_T_DEFINED
#define CMP_WINDOW_T_DEFINED
typedef struct cmp_window cmp_window_t;
#endif

/**
 * @brief Window configuration options
 */
typedef struct cmp_window_config {
  const char *title;
  int width;
  int height;
  int x;
  int y;
  int hidden;
  int frameless;
  int use_legacy_backend;
} cmp_window_config_t;

/**
 * @brief Callback fired when a file payload is dropped into the window
 * @param path The absolute path to the dropped file
 * @param user_data Optional user pointer
 */
typedef void (*cmp_window_drop_cb_t)(const char *path, void *user_data);

/**
 * @brief Register a callback to receive drag and drop events for the window
 * @param window The window instance
 * @param drop_cb The callback function to receive file paths
 * @param user_data Data passed directly into the callback
 * @return 0 on success, or an error code.
 */
int cmp_window_set_drop_callback(cmp_window_t *window,
                                 cmp_window_drop_cb_t drop_cb, void *user_data);

/**
 * @brief Rendering Backend Type
 */
typedef enum cmp_render_backend {
  CMP_RENDER_BACKEND_DEFAULT = 0,
  CMP_RENDER_BACKEND_D3D11 = 1,
  CMP_RENDER_BACKEND_OPENGL = 2,
  CMP_RENDER_BACKEND_METAL = 3,
  CMP_RENDER_BACKEND_VULKAN = 4,
  CMP_RENDER_BACKEND_SOFTWARE = 5
} cmp_render_backend_t;

/**
 * @brief Abstract representation of a compiled Texture
 */
struct cmp_texture {
  void *internal_handle;
  int width;
  int height;
  int format;
};

/**
 * @brief Font alignment and shaping definitions
 */
typedef struct cmp_font {
  void *internal_handle;
  float default_size;
} cmp_font_t;

/**
 * @brief Initialize the global Typography subsystem
 * @return 0 on success, or an error code.
 */
int cmp_typography_init(void);

/**
 * @brief Shutdown the global Typography subsystem
 * @return 0 on success, or an error code.
 */
int cmp_typography_shutdown(void);

/**
 * @brief Load a font into memory from the VFS
 * @param virtual_path VFS path to a .ttf or .otf file
 * @param default_size Base pixel size to rasterize
 * @param out_font Pointer to receive the allocated font structure
 * @return 0 on success, or an error code.
 */
int cmp_font_load(const char *virtual_path, float default_size,
                  cmp_font_t **out_font);

/**
 * @brief Load a font from a memory buffer
 * @param buffer Raw TTF/OTF bytes
 * @param size Size of the buffer
 * @param default_size Base pixel size to rasterize
 * @param out_font Pointer to receive the allocated font structure
 * @return 0 on success, or an error code.
 */
int cmp_font_load_memory(const void *buffer, size_t size, float default_size,
                         cmp_font_t **out_font);

/**
 * @brief Add a fallback font to an existing font's chain
 * @param primary Primary font to bind to
 * @param fallback Fallback font (e.g. for emojis or missing language glyphs)
 * @return 0 on success, or an error code.
 */
int cmp_font_add_fallback(cmp_font_t *primary, cmp_font_t *fallback);

/**
 * @brief Generate a Signed Distance Field (SDF) representation of a glyph
 * @param font The source font
 * @param codepoint The unicode character
 * @param out_texture Pointer to receive the generated SDF texture
 * @return 0 on success, or an error code.
 */
int cmp_font_generate_sdf(cmp_font_t *font, uint32_t codepoint,
                          cmp_texture_t **out_texture);

/**
 * @brief Destroy an instantiated font
 * @param font The font to destroy
 * @return 0 on success, or an error code.
 */
int cmp_font_destroy(cmp_font_t *font);

/**
 * @brief Shape a string using HarfBuzz or a fallback algorithm
 * @param font The font configuration to use
 * @param text The UTF-8 string to shape
 * @param out_width Pointer to receive the total advance width
 * @param out_height Pointer to receive the max layout height
 * @return 0 on success, or an error code.
 */
int cmp_text_shape(cmp_font_t *font, const char *text, float *out_width,
                   float *out_height);

typedef enum cmp_color_space {
  CMP_COLOR_SPACE_SRGB = 0,
  CMP_COLOR_SPACE_DISPLAY_P3 = 1,
  CMP_COLOR_SPACE_OKLCH = 2
} cmp_color_space_t;

/**
 * @brief Color structure (RGBA)
 */
typedef struct cmp_color {
  float r;
  float g;
  float b;
  float a;
  cmp_color_space_t space;
} cmp_color_t;

/**
 * @brief Dynamic Color Palette (Material You / Fluent equivalent)
 */
typedef struct cmp_palette {
  cmp_color_t primary;
  cmp_color_t on_primary;
  cmp_color_t primary_container;
  cmp_color_t on_primary_container;
  cmp_color_t secondary;
  cmp_color_t on_secondary;
  cmp_color_t secondary_container;
  cmp_color_t on_secondary_container;
  cmp_color_t background;
  cmp_color_t on_background;
  cmp_color_t surface;
  cmp_color_t on_surface;
  cmp_color_t error;
  cmp_color_t on_error;
} cmp_palette_t;

/**
 * @brief Design Language Options
 */
typedef enum cmp_design_language {
  CMP_DESIGN_MATERIAL_3 = 1,
  CMP_DESIGN_FLUENT_2 = 2,
  CMP_DESIGN_CUPERTINO = 3,
  CMP_DESIGN_UNSTYLED = 4
} cmp_design_language_t;

/**
 * @brief Animation State Layer (Ripples, Hovers)
 */
typedef struct cmp_state_layer {
  float hover_opacity;
  float press_opacity;
  float focus_opacity;
  float drag_opacity;

  /* Ripple timing specifics */
  float current_radius;
  float max_radius;
  float current_opacity;
  int is_active;
} cmp_state_layer_t;

/**
 * @brief Semantic Focus Traversal Node
 */
typedef struct cmp_a11y_node {
  int id;
  int next_id;
  int prev_id;
  int up_id;
  int down_id;
  const char *aria_label;
  const char *aria_role;
} cmp_a11y_node_t;

/**
 * @brief Global UI Theme Context
 */
typedef struct cmp_theme {
  cmp_design_language_t language;
  cmp_palette_t light_palette;
  cmp_palette_t dark_palette;
  cmp_palette_t high_contrast_palette;
  int is_dark_mode;
  int high_contrast_mode;
  int reduce_motion;
  int increase_contrast;
  int differentiate_without_color;
  float global_scale;
  float density_multiplier;
  float base_font_size;
  float optical_sizing_modifier;
  float line_height_multiplier;
} cmp_theme_t;

/**
 * @brief Initialize the global theming/design subsystem
 * @return 0 on success, or an error code.
 */
int cmp_theme_init(void);

/**
 * @brief Shutdown the global theming subsystem
 * @return 0 on success, or an error code.
 */
int cmp_theme_shutdown(void);

/**
 * @brief Enable Developer Tools overlay rendering
 * @param window The window to attach the overlay to
 * @param enable 1 to enable, 0 to disable
 * @return 0 on success, or an error code.
 */
int cmp_devtools_set_enabled(cmp_window_t *window, int enable);

/**
 * @brief Force the framework into headless mode for testing
 * @return 0 on success, or an error code.
 */
int cmp_test_enable_headless(void);

/**
 * @brief Simulate a raw normalized input event synthetically
 * @param event The event to dispatch
 * @return 0 on success, or an error code.
 */
int cmp_test_simulate_input(const cmp_event_t *event);

/**
 * @brief Capture a snapshot of the current window framebuffer
 * @param window The window to snapshot
 * @param out_pixels Pointer to receive raw RGBA pixel data
 * @param out_width Pointer to receive width
 * @param out_height Pointer to receive height
 * @return 0 on success, or an error code.
 */
int cmp_test_capture_snapshot(cmp_window_t *window, void **out_pixels,
                              int *out_width, int *out_height);

/**
 * @brief Initialize a new Theme context. Memory is owned by the caller/FFI
 * boundary until destroyed.
 * @param out_theme Pointer to receive the dynamically allocated theme.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_theme_create(cmp_theme_t **out_theme);

/**
 * @brief Destroys a Theme context to prevent memory leaks across FFI
 * boundaries.
 * @param theme The theme to destroy.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_theme_destroy(cmp_theme_t *theme);

/**
 * @brief Generate a dynamic palette from a seed color (e.g. Material You
 * extraction)
 * @param seed The base color
 * @param out_palette Pointer to receive the generated palette
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_theme_generate_palette(cmp_color_t seed,
                                       cmp_palette_t *out_palette);

/**
 * @brief Apply a specific design language engine to the active window
 * @param window The window to style
 * @param theme The theme configuration to apply
 * @return 0 on success, or an error code.
 */
int cmp_window_set_theme(cmp_window_t *window, const cmp_theme_t *theme);

/**
 * @brief Text directionality (LTR/RTL)
 */
typedef enum cmp_text_direction {
  CMP_TEXT_DIR_LTR = 0,
  CMP_TEXT_DIR_RTL = 1,
  CMP_TEXT_DIR_AUTO = 2
} cmp_text_direction_t;

/**
 * @brief Initialize the global i18n subsystem
 * @return 0 on success, or an error code.
 */
int cmp_i18n_init(void);

/**
 * @brief Shutdown the global i18n subsystem
 * @return 0 on success, or an error code.
 */
int cmp_i18n_shutdown(void);

/**
 * @brief Detect the host OS locale (e.g. "en-US", "he-IL")
 * @param out_locale Pointer to string to receive the locale. Must be destroyed.
 * @return 0 on success, or an error code.
 */
int cmp_i18n_detect_os_locale(cmp_string_t *out_locale);

/**
 * @brief Load a translation catalog from the VFS (e.g. .mo or .po JSON mapped
 * file)
 * @param virtual_path VFS path to the catalog
 * @param locale Locale string (e.g. "fr-FR")
 * @return 0 on success, or an error code.
 */
int cmp_i18n_load_catalog(const char *virtual_path, const char *locale);

/**
 * @brief Translate a string using the currently loaded active catalog
 * @param key The translation key
 * @param out_translated Pointer to receive the translation string. Must be
 * destroyed.
 * @return 0 on success, or an error code.
 */
int cmp_i18n_translate(const char *key, cmp_string_t *out_translated);

/**
 * @brief Translate a string taking into account CLDR pluralization forms
 * @param key The translation key
 * @param count The number determining the plural rule
 * @param out_translated Pointer to receive the translation string. Must be
 * destroyed.
 * @return 0 on success, or an error code.
 */
int cmp_i18n_translate_plural(const char *key, int count,
                              cmp_string_t *out_translated);

/**
 * @brief Bidi Layout Configuration applied at the UI hierarchy level
 */
typedef struct cmp_i18n_bidi_state {
  cmp_text_direction_t root_direction;
} cmp_i18n_bidi_state_t;

/**
 * @brief Set the global UI Bidi Layout Direction
 * @param dir The requested direction
 * @return 0 on success, or an error code.
 */
int cmp_i18n_set_bidi_direction(cmp_text_direction_t dir);

/**
 * @brief Get the currently active UI Bidi Layout Direction
 * @return The active direction (cast to int)
 */
int cmp_i18n_get_bidi_direction(void);

/**
 * @brief Abstract Representation of a custom Shader Pipeline
 */
typedef struct cmp_shader {
  void *internal_handle;
} cmp_shader_t;

/**
 * @brief Initialize a specialized shader for rounded rectangles
 * @param shader Pointer to the shader object to initialize
 * @return 0 on success, or an error code.
 */
int cmp_shader_init_rounded_rect(cmp_shader_t *shader);

/**
 * @brief Initialize a specialized shader for gradients
 * @param shader Pointer to the shader object to initialize
 * @return 0 on success, or an error code.
 */
int cmp_shader_init_gradient(cmp_shader_t *shader);

/**
 * @brief Initialize a specialized shader for SDF text
 * @param shader Pointer to the shader object to initialize
 * @return 0 on success, or an error code.
 */
int cmp_shader_init_sdf_text(cmp_shader_t *shader);

/**
 * @brief Destroy a shader pipeline and free its resources
 * @param shader The shader object to destroy
 * @return 0 on success, or an error code.
 */
int cmp_shader_destroy(cmp_shader_t *shader);

/**
 * @brief Opaque Shader Cache Structure
 */
typedef struct cmp_shader_cache cmp_shader_cache_t;

/**
 * @brief Initialize a shader cache
 * @param out_cache Pointer to the cache pointer
 * @return 0 on success, or an error code.
 */
int cmp_shader_cache_create(cmp_shader_cache_t **out_cache);

/**
 * @brief Destroy a shader cache and free resources
 * @param cache The cache to destroy
 * @return 0 on success, or an error code.
 */
int cmp_shader_cache_destroy(cmp_shader_cache_t *cache);

/**
 * @brief Store a pre-compiled shader in the cache
 * @param cache The cache
 * @param key The unique key (e.g., shader name or hash)
 * @param shader The compiled shader to store
 * @return 0 on success, or an error code.
 */
int cmp_shader_cache_store(cmp_shader_cache_t *cache, const char *key,
                           cmp_shader_t *shader);

/**
 * @brief Retrieve a shader from the cache
 * @param cache The cache
 * @param key The unique key
 * @param out_shader Pointer to the shader object to be populated
 * @return 0 on success (found), CMP_ERROR_NOT_FOUND if not present, or an error
 * code.
 */
int cmp_shader_cache_retrieve(cmp_shader_cache_t *cache, const char *key,
                              cmp_shader_t **out_shader);

/**
 * @brief Save the cache to a file on disk to avoid jank on next startup
 * @param cache The cache
 * @param filepath Path to the file
 * @return 0 on success, or an error code.
 */
int cmp_shader_cache_save_to_disk(cmp_shader_cache_t *cache,
                                  const char *filepath);

/**
 * @brief Load the cache from a file on disk
 * @param cache The cache
 * @param filepath Path to the file
 * @return 0 on success, or an error code.
 */
int cmp_shader_cache_load_from_disk(cmp_shader_cache_t *cache,
                                    const char *filepath);

/**
 * @brief Opaque MSAA Context
 */
typedef struct cmp_msaa cmp_msaa_t;

/**
 * @brief Initialize an MSAA render target
 * @param sample_count Number of samples (e.g., 2, 4, 8, 16)
 * @param width Width of the render target
 * @param height Height of the render target
 * @param out_msaa Pointer to receive the MSAA context
 * @return 0 on success, or an error code.
 */
int cmp_msaa_create(uint8_t sample_count, uint32_t width, uint32_t height,
                    cmp_msaa_t **out_msaa);

/**
 * @brief Destroy an MSAA context
 * @param msaa The MSAA context to destroy
 * @return 0 on success, or an error code.
 */
int cmp_msaa_destroy(cmp_msaa_t *msaa);

/**
 * @brief Resolve the multisampled buffer into a standard single-sampled texture
 * @param msaa The MSAA context
 * @param target_texture The destination texture to receive the resolved pixels
 * @return 0 on success, or an error code.
 */
int cmp_msaa_resolve(cmp_msaa_t *msaa, cmp_texture_t *target_texture);

/**
 * @brief Opaque Linear sRGB Blending Context
 */
typedef struct cmp_linear_blend cmp_linear_blend_t;

/**
 * @brief Initialize a linear blending context
 * @param gamma Display gamma (typically 2.2f)
 * @param out_blend Pointer to receive the blend context
 * @return 0 on success, or an error code.
 */
int cmp_linear_blend_create(float gamma, cmp_linear_blend_t **out_blend);

/**
 * @brief Destroy a linear blending context
 * @param blend The blend context to destroy
 * @return 0 on success, or an error code.
 */
int cmp_linear_blend_destroy(cmp_linear_blend_t *blend);

/**
 * @brief Convert an sRGB color to linear space
 * @param blend The blend context
 * @param srgb The input sRGB color
 * @param out_linear The output linear color
 * @return 0 on success, or an error code.
 */
int cmp_linear_blend_srgb_to_linear(cmp_linear_blend_t *blend,
                                    const cmp_color_t *srgb,
                                    cmp_color_t *out_linear);

/**
 * @brief Convert a linear color to sRGB space
 * @param blend The blend context
 * @param linear The input linear color
 * @param out_srgb The output sRGB color
 * @return 0 on success, or an error code.
 */
int cmp_linear_blend_linear_to_srgb(cmp_linear_blend_t *blend,
                                    const cmp_color_t *linear,
                                    cmp_color_t *out_srgb);

/**
 * @brief Blend two sRGB colors correctly in linear space
 * @param blend The blend context
 * @param bg Background color (sRGB)
 * @param fg Foreground color (sRGB)
 * @param alpha Global alpha factor [0.0, 1.0] applied to fg
 * @param out_blended The resulting blended color (sRGB)
 * @return 0 on success, or an error code.
 */
int cmp_linear_blend_mix(cmp_linear_blend_t *blend, const cmp_color_t *bg,
                         const cmp_color_t *fg, float alpha,
                         cmp_color_t *out_blended);

/**
 * @brief Supported texture compression formats
 */
typedef enum cmp_tex_compression_type {
  CMP_TEX_COMPRESSION_NONE = 0,
  CMP_TEX_COMPRESSION_ASTC = 1,
  CMP_TEX_COMPRESSION_BC7 = 2,
  CMP_TEX_COMPRESSION_ETC2 = 3
} cmp_tex_compression_type_t;

/**
 * @brief Opaque Texture Compression Context
 */
typedef struct cmp_tex_compression cmp_tex_compression_t;

/**
 * @brief Initialize a texture compression context
 * @param type The compression format type
 * @param width The width of the image
 * @param height The height of the image
 * @param data The compressed binary payload
 * @param data_size The size of the payload in bytes
 * @param out_tex_comp Pointer to receive the texture compression context
 * @return 0 on success, or an error code.
 */
int cmp_tex_compression_create(cmp_tex_compression_type_t type, uint32_t width,
                               uint32_t height, const void *data,
                               size_t data_size,
                               cmp_tex_compression_t **out_tex_comp);

/**
 * @brief Destroy a texture compression context
 * @param tex_comp The texture compression context
 * @return 0 on success, or an error code.
 */
int cmp_tex_compression_destroy(cmp_tex_compression_t *tex_comp);

/**
 * @brief Mount compressed data into a GPU texture directly
 * @param tex_comp The compression context
 * @param target_texture The GPU texture structure to bind into
 * @return 0 on success, or an error code.
 */
int cmp_tex_compression_mount(cmp_tex_compression_t *tex_comp,
                              cmp_texture_t *target_texture);

/**
 * @brief Opaque Mipmap Generator Context
 */
typedef struct cmp_mipmap cmp_mipmap_t;

/**
 * @brief Initialize a mipmap generator context
 * @param out_mipmap Pointer to receive the context
 * @return 0 on success, or an error code.
 */
int cmp_mipmap_create(cmp_mipmap_t **out_mipmap);

/**
 * @brief Destroy a mipmap generator context
 * @param mipmap The context to destroy
 * @return 0 on success, or an error code.
 */
int cmp_mipmap_destroy(cmp_mipmap_t *mipmap);

/**
 * @brief Generate mipmap levels for a given target texture
 * @param mipmap The mipmap context
 * @param target_texture The texture to generate mipmaps for (must have valid
 * bounds and power-of-two dimensions usually)
 * @return 0 on success, or an error code.
 */
int cmp_mipmap_generate(cmp_mipmap_t *mipmap, cmp_texture_t *target_texture);

/**
 * @brief Apply Anisotropic Filtering settings to a texture
 * @param target_texture The texture to apply the filter to
 * @param max_anisotropy Maximum anisotropy level (e.g. 1.0f for
 * none, 4.0f, 8.0f, 16.0f)
 * @return 0 on success, or an error code.
 */
int cmp_mipmap_set_anisotropy(cmp_texture_t *target_texture,
                              float max_anisotropy);

/**
 * @brief Swapchain Presentation Modes
 */
typedef enum cmp_swapchain_mode {
  CMP_SWAPCHAIN_FIFO = 0, /**< Standard VSync (double/triple buffering) */
  CMP_SWAPCHAIN_MAILBOX =
      1, /**< Lowest latency VSync (triple buffering, replaces old frames) */
  CMP_SWAPCHAIN_IMMEDIATE = 2 /**< No VSync (tearing may occur) */
} cmp_swapchain_mode_t;

/**
 * @brief Opaque Swapchain Context
 */
typedef struct cmp_swapchain cmp_swapchain_t;

/**
 * @brief Initialize a hardware swapchain for a window
 * @param window The window to bind to
 * @param mode The requested presentation mode
 * @param out_swapchain Pointer to receive the swapchain context
 * @return 0 on success, or an error code.
 */
int cmp_swapchain_create(cmp_window_t *window, cmp_swapchain_mode_t mode,
                         cmp_swapchain_t **out_swapchain);

/**
 * @brief Destroy a swapchain
 * @param swapchain The swapchain to destroy
 * @return 0 on success, or an error code.
 */
int cmp_swapchain_destroy(cmp_swapchain_t *swapchain);

/**
 * @brief Acquire the next renderable image/texture from the swapchain
 * @param swapchain The swapchain context
 * @param out_texture Pointer to receive the texture handle for the next frame
 * @return 0 on success, or an error code.
 */
int cmp_swapchain_acquire_next_image(cmp_swapchain_t *swapchain,
                                     cmp_texture_t **out_texture);

/**
 * @brief Present the active image to the screen/OS compositor
 * @param swapchain The swapchain context
 * @return 0 on success, or an error code.
 */
int cmp_swapchain_present(cmp_swapchain_t *swapchain);

/**
 * @brief Opaque Overdraw Visualizer Context
 */
typedef struct cmp_overdraw cmp_overdraw_t;

/**
 * @brief Initialize an overdraw visualizer
 * @param out_overdraw Pointer to receive the visualizer context
 * @return 0 on success, or an error code.
 */
int cmp_overdraw_create(cmp_overdraw_t **out_overdraw);

/**
 * @brief Destroy an overdraw visualizer
 * @param overdraw The visualizer context
 * @return 0 on success, or an error code.
 */
int cmp_overdraw_destroy(cmp_overdraw_t *overdraw);

/**
 * @brief Opaque Renderer Context
 */
typedef struct cmp_renderer cmp_renderer_t;

/**
 * @brief Enable or disable overdraw visualization on a specific renderer
 * @param overdraw The overdraw visualizer context
 * @param renderer The target renderer
 * @param enable Non-zero to enable, 0 to disable
 * @return 0 on success, or an error code.
 */
int cmp_overdraw_set_enabled(cmp_overdraw_t *overdraw, cmp_renderer_t *renderer,
                             int enable);

/**
 * @brief Opaque hardware layer tiling engine
 */
typedef struct cmp_layer_tiling cmp_layer_tiling_t;

/**
 * @brief Initialize a hardware layer tiling engine for extremely tall layers
 * @param tile_size The size of each VRAM tile chunk (e.g., 512, 1024)
 * @param out_tiling Pointer to receive the tiling context
 * @return 0 on success, or an error code.
 */
int cmp_layer_tiling_create(uint32_t tile_size,
                            cmp_layer_tiling_t **out_tiling);

/**
 * @brief Destroy a hardware layer tiling engine
 * @param tiling The tiling context
 * @return 0 on success, or an error code.
 */
int cmp_layer_tiling_destroy(cmp_layer_tiling_t *tiling);

/**
 * @brief Chunk a large logical bounding box into multiple VRAM tiles
 * @param tiling The tiling context
 * @param width The logical width of the layer
 * @param height The logical height of the layer
 * @param out_tile_count Pointer to receive the number of tiles generated
 * @return 0 on success, or an error code.
 */
int cmp_layer_tiling_calculate(cmp_layer_tiling_t *tiling, uint32_t width,
                               uint32_t height, uint32_t *out_tile_count);

/**
 * @brief Retrieve the bounds of a specific tile chunk
 * @param tiling The tiling context
 * @param tile_index The index of the tile (0 to out_tile_count - 1)
 * @param out_rect Pointer to receive the tile bounding box
 * @return 0 on success, or an error code.
 */
int cmp_layer_tiling_get_bounds(cmp_layer_tiling_t *tiling, uint32_t tile_index,
                                cmp_rect_t *out_rect);

/**
 * @brief Opaque Hit-Testing Context
 */
typedef struct cmp_hit_test cmp_hit_test_t;

/**
 * @brief Create a hit-testing context attached to a layout tree
 * @param tree The component tree to perform hit tests against
 * @param out_hit_test Pointer to receive the context
 * @return 0 on success, or an error code.
 */
int cmp_hit_test_create(cmp_ui_node_t *tree, cmp_hit_test_t **out_hit_test);

/**
 * @brief Destroy a hit-testing context
 * @param hit_test The context to destroy
 * @return 0 on success, or an error code.
 */
int cmp_hit_test_destroy(cmp_hit_test_t *hit_test);

/**
 * @brief Find the topmost DOM node intersecting with a given 2D screen
 * coordinate
 * @param hit_test The hit-testing context
 * @param x The physical screen X coordinate
 * @param y The physical screen Y coordinate
 * @param out_node Pointer to receive the targeted node
 * @return 0 on success (hit found), CMP_ERROR_NOT_FOUND (no hit), or an error
 * code.
 */
int cmp_hit_test_query(cmp_hit_test_t *hit_test, float x, float y,
                       cmp_ui_node_t **out_node);
/**
 * @brief Initialize a renderer instance
 * @param window The window to bind the renderer to
 * @param backend Requested backend, or DEFAULT for automatic selection
 * @param out_renderer Pointer to receive the allocated renderer context
 * @return 0 on success, or an error code.
 */
int cmp_renderer_create(cmp_window_t *window, cmp_render_backend_t backend,
                        cmp_renderer_t **out_renderer);

/**
 * @brief Destroy a renderer instance
 * @param renderer The renderer to destroy
 * @return 0 on success, or an error code.
 */
int cmp_renderer_destroy(cmp_renderer_t *renderer);

/**
 * @brief Begin a new rendering frame (clears backbuffer)
 * @param renderer The renderer context
 * @param clear_color Background clear color
 * @return 0 on success, or an error code.
 */
int cmp_renderer_begin_frame(cmp_renderer_t *renderer, cmp_color_t clear_color);

/**
 * @brief End the current rendering frame (swaps buffers)
 * @param renderer The renderer context
 * @return 0 on success, or an error code.
 */
int cmp_renderer_end_frame(cmp_renderer_t *renderer);

/**
 * @brief Push a sprite into the batching engine
 * @param renderer The renderer context
 * @param texture Texture to draw (NULL for solid color rect)
 * @param dest Destination bounds
 * @param src Source bounds within texture (NULL for full texture)
 * @param color Tint color / blend color
 * @return 0 on success, or an error code.
 */
int cmp_renderer_draw_sprite(cmp_renderer_t *renderer, cmp_texture_t *texture,
                             cmp_rect_t dest, cmp_rect_t *src,
                             cmp_color_t color);

/**
 * @brief Set the current active custom shader pipeline
 * @param renderer The renderer context
 * @param shader The custom shader (NULL to revert to default batching shader)
 * @return 0 on success, or an error code.
 */
int cmp_renderer_set_shader(cmp_renderer_t *renderer, cmp_shader_t *shader);

/**
 * @brief Change the rendering target (e.g. for Render-to-Texture post
 * processing)
 * @param renderer The renderer context
 * @param texture The texture to render into, or NULL to render to the
 * backbuffer.
 * @return 0 on success, or an error code.
 */
int cmp_renderer_set_render_target(cmp_renderer_t *renderer,
                                   cmp_texture_t *texture);

/**
 * @brief Create a texture from raw RGBA pixel data
 * @param renderer The renderer context
 * @param width Width in pixels
 * @param height Height in pixels
 * @param pixels Raw RGBA8 pixel data
 * @param out_texture Pointer to receive the texture handle
 * @return 0 on success, or an error code.
 */
int cmp_texture_create(cmp_renderer_t *renderer, int width, int height,
                       const void *pixels, cmp_texture_t **out_texture);

/**
 * @brief Destroy a compiled texture
 * @param texture The texture to destroy
 * @return 0 on success, or an error code.
 */
int cmp_texture_destroy(cmp_texture_t *texture);

#ifndef CMP_WINDOW_MANAGER_T_DEFINED
#define CMP_WINDOW_MANAGER_T_DEFINED
typedef struct cmp_window_manager cmp_window_manager_t;
#endif

/**
 * @brief Create a new window manager instance
 * @param out_manager Pointer to receive the allocated manager
 * @return 0 on success, or an error code
 */
int cmp_window_manager_create(cmp_window_manager_t **out_manager);

/**
 * @brief Destroy a window manager and all managed windows
 * @param manager The manager to destroy
 * @return 0 on success, or an error code
 */
int cmp_window_manager_destroy(cmp_window_manager_t *manager);

/**
 * @brief Register a created window with the manager
 * @param manager The manager instance
 * @param window The window to add
 * @return 0 on success, or an error code
 */
int cmp_window_manager_add_window(cmp_window_manager_t *manager,
                                  cmp_window_t *window);

/**
 * @brief Remove a window from the manager (does not destroy the window)
 * @param manager The manager instance
 * @param window The window to remove
 * @return 0 on success, or an error code
 */
int cmp_window_manager_remove_window(cmp_window_manager_t *manager,
                                     cmp_window_t *window);

/**
 * @brief Get the number of currently managed windows
 * @param manager The manager instance
 * @param out_count Pointer to receive the window count
 * @return 0 on success, or an error code
 */
int cmp_window_manager_get_window_count(const cmp_window_manager_t *manager,
                                        unsigned int *out_count);

/**
 * @brief Retrieve a managed window by index
 * @param manager The manager instance
 * @param index The index of the window (0 to count - 1)
 * @param out_window Pointer to receive the window handle
 * @return 0 on success, or an error code
 */
int cmp_window_manager_get_window_at(const cmp_window_manager_t *manager,
                                     unsigned int index,
                                     cmp_window_t **out_window);

/**
 * @brief Poll events for all managed windows
 * @param manager The manager instance
 * @return 0 on success, or an error code
 */
int cmp_window_manager_poll_events(cmp_window_manager_t *manager);

/**
 * @brief Check if all windows in the manager are requesting to close
 * @param manager The manager instance
 * @return 1 if all windows should close or if manager is empty, 0 otherwise
 */
int cmp_window_manager_should_close(const cmp_window_manager_t *manager);

/**
 * @brief Initialize the global window subsystem
 * @return 0 on success, or an error code.
 */
int cmp_window_system_init(void);

/**
 * @brief Shutdown the global window subsystem
 * @return 0 on success, or an error code.
 */
int cmp_window_system_shutdown(void);

/**
 * @brief Create a new window instance
 * @param config Pointer to window configuration
 * @param out_window Pointer to receive the window handle
 * @return 0 on success, or an error code.
 */
int cmp_window_create(const cmp_window_config_t *config,
                      cmp_window_t **out_window);

/**
 * @brief Show a created window
 * @param window The window to show
 * @return 0 on success, or an error code.
 */
int cmp_window_show(cmp_window_t *window);

/**
 * @brief Poll OS window events
 * @param window The window to poll events for
 * @return 0 on success, or an error code.
 */
int cmp_window_poll_events(cmp_window_t *window);

/**
 * @brief Check if the window is requesting to close
 * @param window The window to check
 * @return 1 if closing, 0 otherwise
 */
int cmp_window_should_close(cmp_window_t *window);

/**
 * @brief Initialize native macOS menu bar.
 * Note: Only implemented on macOS targets.
 * @return 0 on success, or an error code.
 */
int cmp_window_mac_init_menu_bar(void);

/**
 * @brief Add a native macOS menu item.
 * @param title The text for the menu item
 * @param key_equiv The keyboard shortcut (e.g. "q" for Cmd+Q)
 * @param callback Callback to trigger when selected
 * @return 0 on success, or an error code.
 */
int cmp_window_mac_add_menu_item(const char *title, const char *key_equiv,
                                 void (*callback)(void));

/**
 * @brief Initialize display link synchronization (CADisplayLink /
 * CVDisplayLink) for smooth frame pacing.
 * @param window The window to attach the display link to
 * @param refresh_rate The target refresh rate (e.g. 60, 120), or 0 for native
 * maximum
 * @return 0 on success, or an error code.
 */
int cmp_window_apple_init_display_link(cmp_window_t *window, int refresh_rate);

/**
 * @brief Register specific multi-touch/trackpad gesture recognition hooks
 * @param window The window to bind to
 * @param enable_pinch Enable pinch-to-zoom mapping
 * @param enable_rotation Enable multi-touch rotation mapping
 * @param enable_swipe Enable two-finger swipe mapping
 * @return 0 on success, or an error code.
 */
int cmp_window_apple_enable_gestures(cmp_window_t *window, int enable_pinch,
                                     int enable_rotation, int enable_swipe);

/**
 * @brief Show a native OS desktop notification (e.g. via DBus on Linux).
 * @param title The title of the notification
 * @param body The body text of the notification
 * @return 0 on success, or an error code.
 */
int cmp_window_os_notify(const char *title, const char *body);

/**
 * @brief Enum identifying clipboard targets
 */
typedef enum {
  CMP_CLIPBOARD_PRIMARY = 0, /* Standard Ctrl+C / Ctrl+V clipboard */
  CMP_CLIPBOARD_SELECTION =
      1 /* Middle-click / X11 active selection clipboard */
} cmp_clipboard_type_t;

/**
 * @brief Set the contents of the OS clipboard
 * @param window The window handle
 * @param type The type of clipboard to set (Primary vs Selection)
 * @param text The text to copy
 * @return 0 on success, or an error code.
 */
int cmp_window_set_clipboard_text(cmp_window_t *window,
                                  cmp_clipboard_type_t type, const char *text);

/**
 * @brief Get the contents of the OS clipboard
 * @param window The window handle
 * @param type The type of clipboard to retrieve
 * @param out_text Pointer to a string to receive the text. Must be destroyed.
 * @return 0 on success, or an error code.
 */
int cmp_window_get_clipboard_text(cmp_window_t *window,
                                  cmp_clipboard_type_t type,
                                  cmp_string_t *out_text);

/**
 * @brief Bind a UI tree root to the window for rendering
 * @param window The window instance
 * @param tree The root node of the UI layout
 * @return 0 on success, or an error code.
 */
int cmp_window_set_ui_tree(cmp_window_t *window, cmp_ui_node_t *tree);

/**
 * @brief Initialize raw evdev input capturing for headless / embedded Linux
 * environments.
 * @param event_device_path The path to the evdev node (e.g.,
 * "/dev/input/event0")
 * @return 0 on success, or an error code.
 */
int cmp_window_linux_init_evdev(const char *event_device_path);

/**
 * @brief Initialize Android JNI/Asset Manager hooks.
 * @param app_state Pointer to the Android android_app state structure (from
 * android_native_app_glue)
 * @return 0 on success, or an error code.
 */
int cmp_window_android_init_hooks(void *app_state);

/**
 * @brief Summon or hide the Android software keyboard (IME).
 * @param window The window instance
 * @param show 1 to show the keyboard, 0 to hide
 * @return 0 on success, or an error code.
 */
int cmp_window_android_show_keyboard(cmp_window_t *window, int show);

/**
 * @brief Retrieve the Android safe area (notch, status bar) inset bounds.
 * @param window The window instance
 * @param out_top Pointer to receive top inset
 * @param out_bottom Pointer to receive bottom inset
 * @param out_left Pointer to receive left inset
 * @param out_right Pointer to receive right inset
 * @return 0 on success, or an error code.
 */
int cmp_window_android_get_safe_area(cmp_window_t *window, int *out_top,
                                     int *out_bottom, int *out_left,
                                     int *out_right);

/**
 * @brief Render a basic test payload (e.g. "Hello World" or clear color) to
 * the window
 * @param window The window to render to
 * @return 0 on success, or an error code.
 */
int cmp_window_render_test_frame(cmp_window_t *window);

/**
 * @brief Initialize SDL3 Universal Fallback explicit Gamepad/Audio hooks.
 * @return 0 on success, or an error code.
 */
int cmp_sdl3_fallback_init_subsystems(void);

/**
 * @brief Initialize embedded Lua scripting runtime within the UI framework
 * @return 0 on success, or an error code.
 */
int cmp_scripting_lua_init(void);

/**
 * @brief Load and execute a Lua UI script
 * @param script_path Virtual path to the lua script
 * @return 0 on success, or an error code.
 */
int cmp_scripting_lua_execute_file(const char *script_path);

/**
 * @brief Bridge initialization for Node.js N-API modules
 * @param env N-API environment
 * @param exports N-API exports object
 * @return 0 on success, or an error code.
 */
int cmp_scripting_napi_init(void *env, void *exports);

/**
 * @brief Generator for Python ctypes/cffi bindings
 * @param output_path Virtual path to write the python binding definitions
 * @return 0 on success, or an error code.
 */
int cmp_scripting_python_generate_bindings(const char *output_path);
int cmp_sdl3_fallback_bridge_audio(void);

/**
 * @brief Initialize Emscripten bindings (Web Audio, DOM Events, WebGL/Canvas).
 * @param canvas_selector The DOM ID selector for the target canvas (e.g.
 * "#canvas")
 * @return 0 on success, or an error code.
 */
int cmp_window_wasm_init(const char *canvas_selector);

/**
 * @brief Resume Web Audio Context. (Browsers require user interaction before
 * playing audio).
 * @return 0 on success, or an error code.
 */
int cmp_window_wasm_resume_audio(void);

/**
 * @brief Register the main loop callback for Emscripten's asynchronous
 * execution model.
 * @param mod The execution modality
 * @param main_loop The function to call each frame
 * @param arg Data to pass to the function
 * @return 0 on success, or an error code.
 */
int cmp_window_wasm_set_main_loop(cmp_modality_t *mod,
                                  void (*main_loop)(void *), void *arg);

/**
 * @brief Destroy a window instance
 * @param window The window to destroy
 * @return 0 on success, or an error code.
 */
int cmp_window_destroy(cmp_window_t *window);

/**
 * @brief Missing types from previous implementation
 */

typedef struct cmp_stack_ctx {
  cmp_layout_node_t *node;
  int z_index;
  struct cmp_stack_ctx *parent;
  struct cmp_stack_ctx **children;
  size_t child_count;
  size_t child_capacity;
} cmp_stack_ctx_t;

typedef enum cmp_popover_state {
  CMP_POPOVER_HIDDEN = 0,
  CMP_POPOVER_SHOWING = 1
} cmp_popover_state_t;

struct cmp_layer {
  cmp_layout_node_t *node;
  struct cmp_layer **children;
  size_t child_count;
  size_t child_capacity;
};
typedef struct cmp_layer cmp_layer_t;

/**
 * @brief Evaluate absolute and relative positioning for a node.
 * @param node The layout node.
 * @param parent_rect The bounding box of the containing block.
 * @return 0 on success, or an error code.
 */
int cmp_pos_absolute_relative(cmp_layout_node_t *node,
                              const cmp_rect_t *parent_rect);

/**
 * @brief Evaluate fixed positioning for a node.
 * @param node The layout node.
 * @param viewport_rect The bounding box of the viewport.
 * @return 0 on success, or an error code.
 */
int cmp_pos_fixed(cmp_layout_node_t *node, const cmp_rect_t *viewport_rect);

/**
 * @brief Evaluate sticky positioning for a node.
 * @param node The layout node.
 * @param scroll_offset The current scroll displacement.
 * @param container_rect The bounding box of the sticky container.
 * @return 0 on success, or an error code.
 */
int cmp_pos_sticky(cmp_layout_node_t *node, float scroll_offset,
                   const cmp_rect_t *container_rect);

/**
 * @brief Position a floating node relative to an anchor element.
 * @param floating_node The node to position.
 * @param anchor_rect The bounding box of the anchor element.
 * @return 0 on success, or an error code.
 */
int cmp_anchor_position(cmp_layout_node_t *floating_node,
                        const cmp_rect_t *anchor_rect);

/**
 * @brief Handle fallback positioning for anchored elements.
 * @param floating_node The node to position.
 * @param anchor_rect The bounding box of the anchor.
 * @param viewport_rect The bounding box of the viewport.
 * @return 0 on success, or an error code.
 */
int cmp_anchor_fallback(cmp_layout_node_t *floating_node,
                        const cmp_rect_t *anchor_rect,
                        const cmp_rect_t *viewport_rect);

/**
 * @brief Calculate the size of a floating node based on its anchor.
 * @param floating_node The node to size.
 * @param anchor_rect The bounding box of the anchor.
 * @return 0 on success, or an error code.
 */
int cmp_anchor_size(cmp_layout_node_t *floating_node,
                    const cmp_rect_t *anchor_rect);

/**
 * @brief Create a new stacking context.
 * @param out_ctx Pointer to receive the allocated stacking context.
 * @param node The node that establishes the stacking context.
 * @return 0 on success, or an error code.
 */
int cmp_stack_ctx_create(cmp_stack_ctx_t **out_ctx, cmp_layout_node_t *node);

/**
 * @brief Destroy a stacking context.
 * @param ctx Stacking context to destroy.
 * @return 0 on success, or an error code.
 */
int cmp_stack_ctx_destroy(cmp_stack_ctx_t *ctx);

/**
 * @brief Add a child stacking context to a parent.
 * @param parent Parent stacking context.
 * @param child Child stacking context.
 * @return 0 on success, or an error code.
 */
int cmp_stack_ctx_add_child(cmp_stack_ctx_t *parent, cmp_stack_ctx_t *child);

/**
 * @brief Sort child stacking contexts by z-index.
 * @param ctx The stacking context to sort.
 * @return 0 on success, or an error code.
 */
int cmp_z_index_sort(cmp_stack_ctx_t *ctx);

/**
 * @brief Evaluate 3D transformations for a node.
 * @param node The layout node.
 * @param preserve_3d Non-zero to preserve 3D context for children.
 * @return 0 on success, or an error code.
 */
int cmp_transform_3d_evaluate(cmp_layout_node_t *node, int preserve_3d);

/**
 * @brief Evaluate backface visibility for a node.
 * @param node The layout node.
 * @param is_hidden Non-zero if backface-visibility is hidden.
 * @param rotation_y The current Y-axis rotation.
 * @param out_visible Pointer to receive visibility result.
 * @return 0 on success, or an error code.
 */
int cmp_backface_visibility_evaluate(cmp_layout_node_t *node, int is_hidden,
                                     float rotation_y, int *out_visible);

/**
 * @brief Promote a node to the top layer.
 * @param node The layout node.
 * @return 0 on success, or an error code.
 */
int cmp_top_layer_promote(cmp_layout_node_t *node);

/**
 * @brief Toggle popover state for a node.
 * @param node The layout node.
 * @param state The target popover state.
 * @return 0 on success, or an error code.
 */
int cmp_popover_toggle(cmp_layout_node_t *node, cmp_popover_state_t state);

/**
 * @brief Build a layer tree starting from a root node.
 * @param root_node The root layout node.
 * @param out_layer_root Pointer to receive the layer tree root.
 * @return 0 on success, or an error code.
 */
int cmp_layer_tree_build(cmp_layout_node_t *root_node,
                         cmp_layer_t **out_layer_root);

/**
 * @brief Destroy a layer tree.
 * @param layer_root The root of the layer tree.
 * @return 0 on success, or an error code.
 */
int cmp_layer_tree_destroy(cmp_layer_t *layer_root);

typedef enum cmp_grid_track_type {
  CMP_GRID_TRACK_FIXED = 0,
  CMP_GRID_TRACK_PERCENTAGE = 1,
  CMP_GRID_TRACK_FR = 2,
  CMP_GRID_TRACK_AUTO = 3,
  CMP_GRID_TRACK_MIN_CONTENT = 4,
  CMP_GRID_TRACK_MAX_CONTENT = 5,
  CMP_GRID_TRACK_FIT_CONTENT = 6,
  CMP_GRID_TRACK_MINMAX = 7
} cmp_grid_track_type_t;

typedef struct cmp_grid_track_size {
  cmp_grid_track_type_t type;
  float value;
  float min_value;
  float max_value;
} cmp_grid_track_size_t;

typedef struct cmp_grid_placement {
  int is_auto;
  int span;
  int line;
  const char *name;
} cmp_grid_placement_t;

typedef struct cmp_grid_area {
  const char *name;
  int row_start;
  int col_start;
  int row_end;
  int col_end;
} cmp_grid_area_t;

typedef struct cmp_grid_item {
  cmp_layout_node_t *node;
  cmp_grid_placement_t row_start;
  cmp_grid_placement_t row_end;
  cmp_grid_placement_t col_start;
  cmp_grid_placement_t col_end;
  int resolved_row_start;
  int resolved_row_end;
  int resolved_col_start;
  int resolved_col_end;
  float computed_x;
  float computed_y;
  float computed_width;
  float computed_height;
} cmp_grid_item_t;

typedef struct cmp_grid_ctx {
  cmp_grid_track_size_t *template_columns;
  size_t template_columns_count;
  cmp_grid_track_size_t *template_rows;
  size_t template_rows_count;
  cmp_grid_track_size_t *auto_columns;
  size_t auto_columns_count;
  cmp_grid_track_size_t *auto_rows;
  size_t auto_rows_count;
  cmp_grid_area_t *template_areas;
  size_t template_areas_count;
  cmp_grid_item_t *items;
  size_t item_count;
  size_t item_capacity;
  float *computed_row_sizes;
  size_t computed_row_count;
  float *computed_col_sizes;
  size_t computed_col_count;
  float row_gap;
  float column_gap;
} cmp_grid_ctx_t;

/**
 * @brief Create a new grid context.
 * @param out_ctx Pointer to receive the allocated grid context.
 * @return 0 on success, or an error code.
 */
int cmp_grid_ctx_create(cmp_grid_ctx_t **out_ctx);

/**
 * @brief Destroy a grid context.
 * @param ctx The grid context to destroy.
 * @return 0 on success, or an error code.
 */
int cmp_grid_ctx_destroy(cmp_grid_ctx_t *ctx);

/**
 * @brief Add a layout node as an item to the grid.
 * @param ctx The grid context.
 * @param node The layout node to add.
 * @param out_item Optional pointer to receive the created grid item.
 * @return 0 on success, or an error code.
 */
int cmp_grid_ctx_add_item(cmp_grid_ctx_t *ctx, cmp_layout_node_t *node,
                          cmp_grid_item_t **out_item);

/**
 * @brief Evaluate the size of a grid track.
 * @param track The track size definition.
 * @param container_size The size of the grid container in the relevant axis.
 * @param out_size Pointer to receive the calculated size.
 * @return 0 on success, or an error code.
 */
int cmp_grid_track_evaluate(cmp_grid_track_size_t *track, float container_size,
                            float *out_size);

/**
 * @brief Distribute available space among flexible (fr) grid tracks.
 * @param ctx The grid context.
 * @param available_width The available width for columns.
 * @param available_height The available height for rows.
 * @return 0 on success, or an error code.
 */
int cmp_grid_fr_distribute(cmp_grid_ctx_t *ctx, float available_width,
                           float available_height);

/**
 * @brief Resolve minmax() track size constraints.
 * @param track The track size definition.
 * @param container_size The container size in the relevant axis.
 * @param out_size Pointer to receive the resolved size.
 * @return 0 on success, or an error code.
 */
int cmp_grid_minmax_resolve(cmp_grid_track_size_t *track, float container_size,
                            float *out_size);

/**
 * @brief Expand repeat(auto-fill/auto-fit) track definitions.
 * @param track The track size definition containing the repeat rule.
 * @param auto_fit Non-zero if auto-fit is used, 0 for auto-fill.
 * @param container_size The container size in the relevant axis.
 * @param out_count Pointer to receive the number of tracks generated.
 * @return 0 on success, or an error code.
 */
int cmp_grid_repeat_expand(cmp_grid_track_size_t *track, int auto_fit,
                           float container_size, int *out_count);

/**
 * @brief Resolve start and end lines for grid placement.
 * @param start The start placement definition.
 * @param end The end placement definition.
 * @param track_count Total number of tracks in the axis.
 * @param out_start Pointer to receive the resolved start line.
 * @param out_end Pointer to receive the resolved end line.
 * @return 0 on success, or an error code.
 */
int cmp_grid_placement_resolve(cmp_grid_placement_t *start,
                               cmp_grid_placement_t *end, int track_count,
                               int *out_start, int *out_end);

/**
 * @brief Resolve a named grid area to its line coordinates.
 * @param ctx The grid context.
 * @param name The name of the grid area.
 * @param out_row_start Pointer to receive the start row line.
 * @param out_col_start Pointer to receive the start column line.
 * @param out_row_end Pointer to receive the end row line.
 * @param out_col_end Pointer to receive the end column line.
 * @return 0 on success, or an error code.
 */
int cmp_grid_area_resolve(cmp_grid_ctx_t *ctx, const char *name,
                          int *out_row_start, int *out_col_start,
                          int *out_row_end, int *out_col_end);

/**
 * @brief Perform automatic placement of items using the 'dense' packing
 * algorithm.
 * @param ctx The grid context.
 * @return 0 on success, or an error code.
 */
int cmp_grid_auto_dense_place(cmp_grid_ctx_t *ctx);

/**
 * @brief Perform automatic placement of items using the 'sparse' packing
 * algorithm.
 * @param ctx The grid context.
 * @return 0 on success, or an error code.
 */
int cmp_grid_auto_sparse_place(cmp_grid_ctx_t *ctx);

/**
 * @brief Generate implicit tracks based on item placement.
 * @param ctx The grid context.
 * @return 0 on success, or an error code.
 */
int cmp_grid_implicit_tracks_generate(cmp_grid_ctx_t *ctx);

/**
 * @brief Synchronize subgrid tracks with the parent grid.
 * @param parent The parent grid context.
 * @param child The subgrid context.
 * @return 0 on success, or an error code.
 */
int cmp_subgrid_sync(cmp_grid_ctx_t *parent, cmp_grid_ctx_t *child);

int cmp_masonry_layout(cmp_grid_ctx_t *ctx);

/**
 * @brief Apply grid gaps to the computed layout.
 * @param ctx The grid context.
 * @param out_row_gaps Pointer to receive the row gap value.
 * @param out_col_gaps Pointer to receive the column gap value.
 * @return 0 on success, or an error code.
 */
int cmp_grid_gap_apply(cmp_grid_ctx_t *ctx, float *out_row_gaps,
                       float *out_col_gaps);

typedef enum cmp_grid_align {
  CMP_GRID_ALIGN_START = 0,
  CMP_GRID_ALIGN_END = 1,
  CMP_GRID_ALIGN_CENTER = 2,
  CMP_GRID_ALIGN_STRETCH = 3
} cmp_grid_align_t;

typedef enum cmp_column_fill {
  CMP_COLUMN_FILL_BALANCE = 0,
  CMP_COLUMN_FILL_AUTO = 1
} cmp_column_fill_t;

typedef enum cmp_container_type {
  CMP_CONTAINER_TYPE_NORMAL = 0,
  CMP_CONTAINER_TYPE_SIZE = 1,
  CMP_CONTAINER_TYPE_INLINE_SIZE = 2
} cmp_container_type_t;

typedef enum cmp_contain {
  CMP_CONTAIN_NONE = 0,
  CMP_CONTAIN_STRICT = 1,
  CMP_CONTAIN_CONTENT = 2,
  CMP_CONTAIN_SIZE = 4,
  CMP_CONTAIN_LAYOUT = 8,
  CMP_CONTAIN_STYLE = 16,
  CMP_CONTAIN_PAINT = 32
} cmp_contain_t;

/**
 * @brief Evaluate CSS containment effects.
 * @param contain The containment bitmask.
 * @param out_isolates_layout Pointer to receive if layout is isolated.
 * @param out_isolates_paint Pointer to receive if paint is isolated.
 * @return 0 on success, or an error code.
 */
int cmp_contain_evaluate(cmp_contain_t contain, int *out_isolates_layout,
                         int *out_isolates_paint);

typedef enum cmp_gradient_type {
  CMP_GRADIENT_LINEAR = 0,
  CMP_GRADIENT_RADIAL = 1,
  CMP_GRADIENT_CONIC = 2
} cmp_gradient_type_t;

typedef struct cmp_gradient_stop {
  cmp_color_t color;
  float position;
} cmp_gradient_stop_t;

typedef struct cmp_gradient {
  cmp_gradient_type_t type;
  cmp_gradient_stop_t *stops;
  size_t stop_count;
} cmp_gradient_t;

/**
 * @brief Create a new gradient structure.
 */
int cmp_gradient_create(cmp_gradient_t **out_gradient,
                        cmp_gradient_type_t type);

/**
 * @brief Destroy a gradient structure.
 */
int cmp_gradient_destroy(cmp_gradient_t *gradient);

/**
 * @brief Add a color stop to a gradient.
 */
int cmp_gradient_add_stop(cmp_gradient_t *gradient, cmp_color_t color,
                          float position);

/**
 * @brief Parse a Display P3 color string.
 */
int cmp_color_parse_p3(const char *color_str, cmp_color_t *out_color);

/**
 * @brief Convert OKLCH color to sRGB.
 */
int cmp_color_oklch_to_srgb(const cmp_color_t *in_color,
                            cmp_color_t *out_color);

/**
 * @brief Mix two colors in a specified color space.
 */
int cmp_color_mix(const cmp_color_t *c1, const cmp_color_t *c2, float weight,
                  cmp_color_space_t space, cmp_color_t *out_color);

/**
 * @brief Calculate relative luminance of a color.
 */
int cmp_color_luminance(const cmp_color_t *color, float *out_luminance);

/**
 * @brief Calculate contrast ratio between two colors.
 */
int cmp_color_contrast_ratio(const cmp_color_t *c1, const cmp_color_t *c2,
                             float *out_ratio);

/**
 * @brief Parse an ICC profile from an image buffer.
 */
int cmp_icc_profile_parse(const void *image_buffer, size_t size,
                          void **out_profile_handle);

typedef enum cmp_corner_shape {
  CMP_CORNER_ROUND = 0,
  CMP_CORNER_SQUIRCLE = 1,
  CMP_CORNER_CUT = 2
} cmp_corner_shape_t;

typedef struct cmp_radius {
  float top_left_x;
  float top_left_y;
  float top_right_x;
  float top_right_y;
  float bottom_right_x;
  float bottom_right_y;
  float bottom_left_x;
  float bottom_left_y;
  cmp_corner_shape_t corner_shape;
} cmp_radius_t;

/**
 * @brief Initialize a radius structure.
 */
int cmp_radius_init(cmp_radius_t *out_radius);

/**
 * @brief Set all radius corners to a uniform value.
 */
int cmp_radius_set_uniform(cmp_radius_t *radius, float r);

/**
 * @brief Perform a hit test against a rounded rectangle.
 */
int cmp_radius_hit_test(const cmp_radius_t *radius, float width, float height,
                        float x, float y, int *out_inside);

typedef struct cmp_box_shadow {
  float offset_x;
  float offset_y;
  float blur;
  float spread;
  cmp_color_t color;
  int is_inset;
  struct cmp_box_shadow *next;
} cmp_box_shadow_t;

/**
 * @brief Create a new box shadow structure.
 */
int cmp_box_shadow_create(cmp_box_shadow_t **out_shadow);

/**
 * @brief Destroy a box shadow structure.
 */
int cmp_box_shadow_destroy(cmp_box_shadow_t *shadow);

/**
 * @brief Append a box shadow to a shadow chain.
 */
int cmp_box_shadow_append(cmp_box_shadow_t *root, cmp_box_shadow_t *next);

typedef struct cmp_shadow_9patch {
  float elevation;
  cmp_texture_t *base_texture;
} cmp_shadow_9patch_t;

/**
 * @brief Generate a 9-patch shadow for a given elevation.
 */
int cmp_shadow_9patch_generate(float elevation,
                               cmp_shadow_9patch_t *out_shadow);

typedef enum cmp_filter_op {
  CMP_FILTER_BLUR = 0,
  CMP_FILTER_BRIGHTNESS,
  CMP_FILTER_CONTRAST,
  CMP_FILTER_DROP_SHADOW,
  CMP_FILTER_GRAYSCALE,
  CMP_FILTER_HUE_ROTATE,
  CMP_FILTER_INVERT,
  CMP_FILTER_SATURATE,
  CMP_FILTER_BACKDROP_BLUR
} cmp_filter_op_t;

typedef struct cmp_filter {
  cmp_filter_op_t op;
  float amount;
  struct cmp_filter *next;
} cmp_filter_t;

/**
 * @brief Create a new filter structure.
 */
int cmp_filter_create(cmp_filter_t **out_filter, cmp_filter_op_t op,
                      float amount);

/**
 * @brief Destroy a filter structure.
 */
int cmp_filter_destroy(cmp_filter_t *filter);

/**
 * @brief Append a filter to a filter chain.
 */
int cmp_filter_append(cmp_filter_t *root, cmp_filter_t *next);

/**
 * @brief Utility to mirror coordinates at image edges for backdrop filters.
 */
int cmp_backdrop_edge_mirror(int image_width, int x, int *out_clamped_x);

typedef struct cmp_media_query_env {
  float viewport_width;
  float viewport_height;
  float resolution_dpi;
  int is_landscape;
  int has_hover;
  int is_pointer_coarse;
  int update_frequency;
  int light_level;
} cmp_media_query_env_t;

typedef struct cmp_media_query {
  float min_width;
  float max_width;
  float min_height;
  float max_height;
  float min_aspect_ratio;
  float max_aspect_ratio;
  float min_resolution;
  float max_resolution;
  int orientation;
  int hover;
  int pointer;
  int update;
  int light_level;
} cmp_media_query_t;

/**
 * @brief Evaluate a media query against the current environment.
 * @param query The media query to evaluate.
 * @param env The current environment state.
 * @param out_matches Pointer to receive the evaluation result (1 for match, 0
 * for no match).
 * @return 0 on success, or an error code.
 */
int cmp_media_query_evaluate(const cmp_media_query_t *query,
                             const cmp_media_query_env_t *env,
                             int *out_matches);

/**
 * @brief Evaluate the update frequency media feature.
 * @param query The media query definition.
 * @param env The current environment state.
 * @param out_matches Pointer to receive the evaluation result.
 * @return 0 on success, or an error code.
 */
int cmp_update_media_evaluate(const cmp_media_query_t *query,
                              const cmp_media_query_env_t *env,
                              int *out_matches);

/**
 * @brief Evaluate the light-level media feature.
 * @param query The media query definition.
 * @param env The current environment state.
 * @param out_matches Pointer to receive the evaluation result.
 * @return 0 on success, or an error code.
 */
int cmp_light_level_evaluate(const cmp_media_query_t *query,
                             const cmp_media_query_env_t *env,
                             int *out_matches);

typedef struct cmp_container_ctx {
  const char *name;
  cmp_container_type_t type;
  float inline_size;
  float block_size;
} cmp_container_ctx_t;

/**
 * @brief Create a new container context for container queries.
 * @param out_ctx Pointer to receive the allocated container context.
 * @param type The type of container (size, inline-size, etc.).
 * @param name Optional name for the container.
 * @return 0 on success, or an error code.
 */
int cmp_container_ctx_create(cmp_container_ctx_t **out_ctx,
                             cmp_container_type_t type, const char *name);

/**
 * @brief Destroy a container context.
 * @param ctx The container context to destroy.
 * @return 0 on success, or an error code.
 */
int cmp_container_ctx_destroy(cmp_container_ctx_t *ctx);

typedef struct cmp_container_query {
  const char *name;
  float min_width;
  float max_width;
  float min_height;
  float max_height;
} cmp_container_query_t;

typedef struct cmp_container_style {
  const char *property_name;
  const char *property_value;
} cmp_container_style_t;

typedef struct cmp_style_query {
  const char *property_name;
  const char *property_value;
} cmp_style_query_t;

/**
 * @brief Evaluate a style query against a container's computed styles.
 * @param query The style query definition.
 * @param container_styles Array of styles from the container.
 * @param num_styles Number of styles in the array.
 * @param out_matches Pointer to receive the evaluation result.
 * @return 0 on success, or an error code.
 */
int cmp_style_query_evaluate(const cmp_style_query_t *query,
                             const cmp_style_query_t *container_styles,
                             int num_styles, int *out_matches);

typedef enum cmp_content_visibility {
  CMP_CONTENT_VISIBILITY_VISIBLE = 0,
  CMP_CONTENT_VISIBILITY_HIDDEN = 1,
  CMP_CONTENT_VISIBILITY_AUTO = 2
} cmp_content_visibility_t;

/**
 * @brief Evaluate content visibility for a node.
 * @param visibility The content-visibility property value.
 * @param viewport The current viewport rectangle.
 * @param node_rect The layout rectangle of the node.
 * @param out_is_visible Pointer to receive the visibility result.
 * @return 0 on success, or an error code.
 */
int cmp_content_visibility_evaluate(cmp_content_visibility_t visibility,
                                    const cmp_rect_t *viewport,
                                    const cmp_rect_t *node_rect,
                                    int *out_is_visible);

struct cmp_resize_observer {
  void (*on_resize)(struct cmp_resize_observer *, cmp_layout_node_t *, float,
                    float);
  void *user_data;
};
typedef struct cmp_resize_observer cmp_resize_observer_t;

typedef enum cmp_svg_path_type {
  CMP_SVG_PATH_POLYGON = 0,
  CMP_SVG_PATH_BEZIER = 1,
  CMP_SVG_PATH_ARC = 2
} cmp_svg_path_type_t;

typedef enum cmp_svg_fill_rule {
  CMP_SVG_FILL_NONZERO = 0,
  CMP_SVG_FILL_EVENODD = 1
} cmp_svg_fill_rule_t;

/**
 * @brief SVG Fill Rule Configuration
 */
typedef struct cmp_svg_fill {
  cmp_svg_fill_rule_t rule; /**< Winding rule (nonzero or evenodd) */
  cmp_color_t color;        /**< Fill color */
} cmp_svg_fill_t;

/**
 * @brief Evaluates an SVG fill path by generating a triangle fan for GPU
 * stencil buffering.
 * @param fill The fill configuration.
 * @param in_vertices The input path vertices (x,y pairs).
 * @param in_count The number of input vertices.
 * @param out_fill_vertices Pointer to receive the generated triangle vertices.
 * @param out_fill_count Pointer to receive the number of generated vertices.
 * @return 0 on success, or an error code.
 */
int cmp_svg_fill_evaluate(const cmp_svg_fill_t *fill, const float *in_vertices,
                          size_t in_count, float **out_fill_vertices,
                          size_t *out_fill_count);

typedef struct cmp_svg_dash {
  float *array;
  size_t count;
  float offset;
} cmp_svg_dash_t;

typedef struct cmp_svg_stroke {
  float width;
  int linecap;  /* 0=butt, 1=round, 2=square */
  int linejoin; /* 0=miter, 1=round, 2=bevel */
  float miter_limit;
  cmp_svg_dash_t dash;
} cmp_svg_stroke_t;

int cmp_svg_dash_evaluate(const cmp_svg_dash_t *dash, const float *in_vertices,
                          size_t in_count, float ***out_subpaths,
                          size_t **out_subpath_counts,
                          size_t *out_subpath_count);

typedef enum cmp_svg_filter_type {
  CMP_SVG_FE_COLOR_MATRIX = 0,
  CMP_SVG_FE_DISPLACEMENT_MAP = 1,
  CMP_SVG_FE_TURBULENCE = 2
} cmp_svg_filter_type_t;

typedef struct cmp_svg_filter_node {
  cmp_svg_filter_type_t type;
  void *config;
  struct cmp_svg_filter_node *next;
} cmp_svg_filter_node_t;

typedef struct cmp_svg_smil {
  int is_transform;
  float duration;
  float current_time;
  float start_value;
  float end_value;
  void *target_node;
} cmp_svg_smil_t;

/**
 * @brief SVG CSS Integration context
 */
typedef struct cmp_svg_css {
  const char *element_name; /**< SVG element tag name (e.g. "rect", "path") */
  const char *class_names;  /**< CSS class names */
  const char *id_name;      /**< CSS ID name */
  void *computed_style;     /**< Resolved CSS cascade style pointer */
  int is_hovered;           /**< Hover state flag */
  int is_active;            /**< Active state flag */
} cmp_svg_css_t;

typedef struct cmp_svg_node {
  int id;
  int is_foreign_object;
  cmp_layout_node_t *dom_mapping;
  cmp_svg_css_t *css;
  cmp_svg_stroke_t *stroke;
  cmp_svg_fill_t *fill;
  cmp_svg_filter_node_t *filter;
  cmp_svg_smil_t *animations;
  struct cmp_svg_node *parent;
  struct cmp_svg_node **children;
  size_t child_count;
  size_t child_capacity;
} cmp_svg_node_t;

/**
 * @brief SVG Preserve Aspect Ratio Align
 */
typedef enum cmp_svg_preserve_aspect_ratio_align {
  CMP_SVG_ASPECT_RATIO_NONE = 0,
  CMP_SVG_ASPECT_RATIO_XMIN_YMIN = 1,
  CMP_SVG_ASPECT_RATIO_XMID_YMIN = 2,
  CMP_SVG_ASPECT_RATIO_XMAX_YMIN = 3,
  CMP_SVG_ASPECT_RATIO_XMIN_YMID = 4,
  CMP_SVG_ASPECT_RATIO_XMID_YMID = 5,
  CMP_SVG_ASPECT_RATIO_XMAX_YMID = 6,
  CMP_SVG_ASPECT_RATIO_XMIN_YMAX = 7,
  CMP_SVG_ASPECT_RATIO_XMID_YMAX = 8,
  CMP_SVG_ASPECT_RATIO_XMAX_YMAX = 9
} cmp_svg_preserve_aspect_ratio_align_t;

/**
 * @brief SVG Preserve Aspect Ratio Meet or Slice
 */
typedef enum cmp_svg_preserve_aspect_ratio_meet_slice {
  CMP_SVG_ASPECT_RATIO_MEET = 0,
  CMP_SVG_ASPECT_RATIO_SLICE = 1
} cmp_svg_preserve_aspect_ratio_meet_slice_t;

/**
 * @brief SVG Preserve Aspect Ratio Strategy
 */
typedef struct cmp_svg_preserve_aspect_ratio {
  cmp_svg_preserve_aspect_ratio_align_t align;
  cmp_svg_preserve_aspect_ratio_meet_slice_t meet_or_slice;
} cmp_svg_preserve_aspect_ratio_t;

/**
 * @brief SVG ViewBox
 */
typedef struct cmp_svg_viewbox {
  float x;
  float y;
  float width;
  float height;
  cmp_svg_preserve_aspect_ratio_t aspect_ratio;
} cmp_svg_viewbox_t;

int cmp_svg_viewbox_evaluate(const cmp_svg_viewbox_t *viewbox,
                             float layout_width, float layout_height,
                             float *out_offset_x, float *out_offset_y,
                             float *out_scale_x, float *out_scale_y);

/**
 * @brief SVG Path Renderer for tessellation
 */
typedef struct cmp_svg_renderer {
  float *vertices;
  size_t vertex_count;
  size_t vertex_capacity;
  float tolerance;
  float current_x;
  float current_y;
  float start_x;
  float start_y;
} cmp_svg_renderer_t;

int cmp_svg_renderer_create(cmp_svg_renderer_t **out_renderer, float tolerance);
int cmp_svg_renderer_destroy(cmp_svg_renderer_t *renderer);
int cmp_svg_renderer_move_to(cmp_svg_renderer_t *renderer, float x, float y);
int cmp_svg_renderer_line_to(cmp_svg_renderer_t *renderer, float x, float y);
int cmp_svg_renderer_quad_to(cmp_svg_renderer_t *renderer, float cx, float cy,
                             float x, float y);
int cmp_svg_renderer_cubic_to(cmp_svg_renderer_t *renderer, float cx1,
                              float cy1, float cx2, float cy2, float x,
                              float y);
int cmp_svg_renderer_arc_to(cmp_svg_renderer_t *renderer, float rx, float ry,
                            float x_axis_rotation, int large_arc_flag,
                            int sweep_flag, float x, float y);
int cmp_svg_renderer_close(cmp_svg_renderer_t *renderer);

int cmp_svg_path_tessellate(cmp_svg_path_type_t path_type, const float *data,
                            size_t data_len, float **out_vertices,
                            size_t *out_vertex_count);
int cmp_svg_stroke_evaluate(const cmp_svg_stroke_t *stroke,
                            const float *in_vertices, size_t in_count,
                            float **out_stroke_vertices,
                            size_t *out_stroke_count);
int cmp_svg_css_bind(cmp_svg_node_t *svg_node, void *css_style);
int cmp_svg_use_instantiate(cmp_svg_node_t *source_node,
                            cmp_svg_node_t **out_cloned_node);
int cmp_svg_smil_tick(cmp_svg_node_t *node, float dt_ms);
int cmp_svg_foreign_bind(cmp_svg_node_t *svg_node, cmp_layout_node_t *dom_node);
int cmp_svg_filter_evaluate(const cmp_svg_filter_node_t *filter,
                            const unsigned char *in_pixels, int width,
                            int height, unsigned char **out_pixels);
int cmp_svg_node_create(cmp_svg_node_t **out_node);
int cmp_svg_node_destroy(cmp_svg_node_t *node);
int cmp_svg_node_add_child(cmp_svg_node_t *parent, cmp_svg_node_t *child);

/* Phase 16: Rendering Pipelines (GPU & CPU Fallback) */
typedef enum cmp_gpu_backend_type {
  CMP_BACKEND_NONE = 0,
  CMP_BACKEND_VULKAN = 1,
  CMP_BACKEND_METAL = 2,
  CMP_BACKEND_DX12 = 3,
  CMP_BACKEND_GLES3 = 4,
  CMP_BACKEND_CPU_SOFTWARE = 5 /* Windows 98 / Legacy fallback */
} cmp_gpu_backend_type_t;

typedef struct cmp_gpu {
  cmp_gpu_backend_type_t backend;
  void *context;
} cmp_gpu_t;

typedef struct cmp_vbo {
  float *data;
  size_t count;
  size_t capacity;
} cmp_vbo_t;

struct cmp_ubo {
  void *data;
  size_t size;
};

typedef struct cmp_draw_call {
  int texture_id;
  int shader_id;
  int blend_mode;
  size_t vertex_offset;
  size_t vertex_count;
} cmp_draw_call_t;

typedef struct cmp_draw_call_optimizer {
  cmp_draw_call_t *calls;
  size_t count;
  size_t capacity;
} cmp_draw_call_optimizer_t;

int cmp_gpu_create(cmp_gpu_backend_type_t preferred_backend,
                   cmp_gpu_t **out_gpu);
int cmp_gpu_destroy(cmp_gpu_t *gpu);
int cmp_vbo_create(cmp_vbo_t **out_vbo);
int cmp_vbo_append(cmp_vbo_t *vbo, const float *vertices, size_t count);
int cmp_vbo_destroy(cmp_vbo_t *vbo);
int cmp_ubo_create(size_t size, cmp_ubo_t **out_ubo);
int cmp_ubo_update(cmp_ubo_t *ubo, const void *data, size_t size);
int cmp_ubo_destroy(cmp_ubo_t *ubo);
int cmp_draw_call_optimizer_create(cmp_draw_call_optimizer_t **out_opt);
int cmp_draw_call_optimizer_add(cmp_draw_call_optimizer_t *opt,
                                const cmp_draw_call_t *call);
int cmp_draw_call_optimizer_optimize(cmp_draw_call_optimizer_t *opt);
int cmp_draw_call_optimizer_destroy(cmp_draw_call_optimizer_t *opt);

/* ========================================================================= */
/* Phase 21: Accessibility (A11y) & Screen Readers                           */
/* ========================================================================= */

/**
 * @brief Opaque A11y Tree Context
 */
typedef struct cmp_a11y_tree cmp_a11y_tree_t;

/**
 * @brief Create an a11y tree context
 */
int cmp_a11y_tree_create(cmp_a11y_tree_t **out_tree);

/**
 * @brief Destroy an a11y tree context
 */
int cmp_a11y_tree_destroy(cmp_a11y_tree_t *tree);

/**
 * @brief Add a node to the a11y tree
 */
int cmp_a11y_tree_add_node(cmp_a11y_tree_t *tree, int node_id, const char *role,
                           const char *name);

/**
 * @brief Serializes the state of an A11y Tree mapping to a specific UI Node for
 * debugging and validation.
 * @param tree The active a11y tree graph context.
 * @param node The UI Node to query mapping constraints against.
 * @param out_buffer String buffer to write into.
 * @param buffer_size The size of the provided string buffer.
 * @return 0 on success, or an error code.
 */
int cmp_a11y_tree_serialize(cmp_a11y_tree_t *tree, cmp_ui_node_t *node,
                            char *out_buffer, size_t buffer_size);

/**
 * @brief Define standard a11y traits
 */
typedef enum cmp_a11y_trait {
  CMP_A11Y_TRAIT_NONE = 0,
  CMP_A11Y_TRAIT_BUTTON = 1 << 0,
  CMP_A11Y_TRAIT_HEADER = 1 << 1,
  CMP_A11Y_TRAIT_IMAGE = 1 << 2,
  CMP_A11Y_TRAIT_LINK = 1 << 3,
  CMP_A11Y_TRAIT_SEARCH_FIELD = 1 << 4,
  CMP_A11Y_TRAIT_PLAYS_SOUND = 1 << 5,
  CMP_A11Y_TRAIT_SELECTED = 1 << 6,
  CMP_A11Y_TRAIT_NOT_ENABLED = 1 << 7
} cmp_a11y_trait_t;

/**
 * @brief Set the a11y VoiceOver label of a node
 */
int cmp_a11y_tree_set_node_label(cmp_a11y_tree_t *tree, int node_id,
                                 const char *label);

/**
 * @brief Set the a11y VoiceOver hint of a node
 */
int cmp_a11y_tree_set_node_hint(cmp_a11y_tree_t *tree, int node_id,
                                const char *hint);

/**
 * @brief Set the a11y VoiceOver value of a node
 */
int cmp_a11y_tree_set_node_value(cmp_a11y_tree_t *tree, int node_id,
                                 const char *value);

/**
 * @brief Set the a11y VoiceOver traits of a node
 */
int cmp_a11y_tree_set_node_traits(cmp_a11y_tree_t *tree, int node_id,
                                  uint32_t traits);

/**
 * @brief Group logically related elements into a single focusable VoiceOver
 * node
 */
int cmp_a11y_tree_set_node_grouped(cmp_a11y_tree_t *tree, int node_id,
                                   int is_grouped);

/**
 * @brief Add a custom VoiceOver action to expose swipe-actions or hidden
 * buttons
 */
int cmp_a11y_tree_add_node_custom_action(cmp_a11y_tree_t *tree, int node_id,
                                         const char *action_name);

/**
 * @brief Add a custom VoiceOver rotor to filter navigation by specific element
 * types
 */
int cmp_a11y_tree_add_node_custom_rotor(cmp_a11y_tree_t *tree, int node_id,
                                        const char *rotor_name);

/**
 * @brief Set the strict logical focus order of a node
 */
int cmp_a11y_tree_set_node_focus_order(cmp_a11y_tree_t *tree, int node_id,
                                       int focus_order);

/**
 * @brief Enable braille screen input for text fields
 */
int cmp_a11y_tree_set_node_braille_input(cmp_a11y_tree_t *tree, int node_id,
                                         int enabled);

/**
 * @brief Provide secondary audio descriptive tracks
 */
int cmp_a11y_tree_set_node_audio_description(cmp_a11y_tree_t *tree, int node_id,
                                             const char *description_url);

/**
 * @brief Provide phonetic pronunciation guides for unusual words
 */
int cmp_a11y_tree_set_node_pronunciation(cmp_a11y_tree_t *tree, int node_id,
                                         const char *pronunciation);

/**
 * @brief Allow direct interaction to bypass VoiceOver gestures
 */
int cmp_a11y_tree_set_node_direct_touch(cmp_a11y_tree_t *tree, int node_id,
                                        int enabled);

/**
 * @brief Post an asynchronous a11y announcement
 */
int cmp_a11y_post_announcement(cmp_a11y_tree_t *tree, const char *announcement);

/**
 * @brief Dynamic Type Size Categories
 */
typedef enum cmp_a11y_content_size_category {
  CMP_A11Y_CONTENT_SIZE_EXTRA_SMALL = 0,
  CMP_A11Y_CONTENT_SIZE_SMALL,
  CMP_A11Y_CONTENT_SIZE_MEDIUM,
  CMP_A11Y_CONTENT_SIZE_LARGE, /* Default */
  CMP_A11Y_CONTENT_SIZE_EXTRA_LARGE,
  CMP_A11Y_CONTENT_SIZE_EXTRA_EXTRA_LARGE,
  CMP_A11Y_CONTENT_SIZE_EXTRA_EXTRA_EXTRA_LARGE,
  CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_MEDIUM,
  CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_LARGE,
  CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_EXTRA_LARGE,
  CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_EXTRA_EXTRA_LARGE,
  CMP_A11Y_CONTENT_SIZE_ACCESSIBILITY_EXTRA_EXTRA_EXTRA_LARGE
} cmp_a11y_content_size_category_t;

/**
 * @brief Opaque Dynamic Type configuration structure
 */
typedef struct cmp_dynamic_type cmp_dynamic_type_t;

/**
 * @brief Create a Dynamic Type context
 */
int cmp_dynamic_type_create(cmp_dynamic_type_t **out_dynamic_type);

/**
 * @brief Destroy a Dynamic Type context
 */
int cmp_dynamic_type_destroy(cmp_dynamic_type_t *dynamic_type);

/**
 * @brief Set the current OS-level content size category
 */
int cmp_dynamic_type_set_category(cmp_dynamic_type_t *dynamic_type,
                                  cmp_a11y_content_size_category_t category);

/**
 * @brief Apply dynamic scaling to a root font size based on category
 */
int cmp_dynamic_type_apply_scale(cmp_dynamic_type_t *dynamic_type,
                                 float base_size, float *out_scaled_size);

/**
 * @brief Query if layout should reflow to vertical stacks (Accessibility sizes)
 */
int cmp_dynamic_type_should_reflow(cmp_dynamic_type_t *dynamic_type,
                                   int *out_should_reflow);

/**
 * @brief Opaque Bold Text configuration structure
 */
typedef struct cmp_a11y_bold_text cmp_a11y_bold_text_t;

/**
 * @brief Create a Bold Text context
 */
int cmp_a11y_bold_text_create(cmp_a11y_bold_text_t **out_bold_text);

/**
 * @brief Destroy a Bold Text context
 */
int cmp_a11y_bold_text_destroy(cmp_a11y_bold_text_t *bold_text);

/**
 * @brief Set the current OS-level Bold Text setting
 */
int cmp_a11y_bold_text_set(cmp_a11y_bold_text_t *bold_text, int enabled);

/**
 * @brief Apply bold text weight adjustments
 */
int cmp_a11y_bold_text_apply(cmp_a11y_bold_text_t *bold_text, int base_weight,
                             int *out_weight);

/**
 * @brief Opaque Button Shapes configuration structure
 */
typedef struct cmp_a11y_button_shapes cmp_a11y_button_shapes_t;

/**
 * @brief Create a Button Shapes context
 */
int cmp_a11y_button_shapes_create(cmp_a11y_button_shapes_t **out_button_shapes);

/**
 * @brief Destroy a Button Shapes context
 */
int cmp_a11y_button_shapes_destroy(cmp_a11y_button_shapes_t *button_shapes);

/**
 * @brief Set the current OS-level Button Shapes setting
 */
int cmp_a11y_button_shapes_set(cmp_a11y_button_shapes_t *button_shapes,
                               int enabled);

/**
 * @brief Query if a button should draw explicit background shapes/underlines
 */
int cmp_a11y_button_shapes_should_draw(cmp_a11y_button_shapes_t *button_shapes,
                                       int *out_should_draw);

/**
 * @brief Opaque Increase Contrast configuration structure
 */
typedef struct cmp_a11y_increase_contrast cmp_a11y_increase_contrast_t;

/**
 * @brief Create an Increase Contrast context
 */
int cmp_a11y_increase_contrast_create(
    cmp_a11y_increase_contrast_t **out_increase_contrast);

/**
 * @brief Destroy an Increase Contrast context
 */
int cmp_a11y_increase_contrast_destroy(
    cmp_a11y_increase_contrast_t *increase_contrast);

/**
 * @brief Set the current OS-level Increase Contrast setting
 */
int cmp_a11y_increase_contrast_set(
    cmp_a11y_increase_contrast_t *increase_contrast, int enabled);

/**
 * @brief Apply increased contrast color adjustments (simulated via an
 * opacity/darkening factor)
 */
int cmp_a11y_increase_contrast_apply(
    cmp_a11y_increase_contrast_t *increase_contrast, float *out_opacity_factor);

/**
 * @brief Prevent smart invert colors acting as negatives for specific nodes
 */
int cmp_a11y_tree_set_node_ignores_invert(cmp_a11y_tree_t *tree, int node_id,
                                          int ignores_invert);

/**
 * @brief Differentiate without color - use icons or distinct shapes for
 * critical states
 */
int cmp_a11y_tree_set_node_differentiate_without_color(cmp_a11y_tree_t *tree,
                                                       int node_id,
                                                       int enabled);

/**
 * @brief Opaque macOS/iPadOS Hover Text Bubble structure
 */
typedef struct cmp_a11y_hover_text cmp_a11y_hover_text_t;

/**
 * @brief Create a Hover Text Bubble context
 */
int cmp_a11y_hover_text_create(cmp_a11y_hover_text_t **out_hover_text);

/**
 * @brief Destroy a Hover Text Bubble context
 */
int cmp_a11y_hover_text_destroy(cmp_a11y_hover_text_t *hover_text);

/**
 * @brief Retrieve High Contrast Text Bubble String for pointer hovers
 */
int cmp_a11y_hover_text_get_bubble(cmp_a11y_hover_text_t *hover_text,
                                   int node_id, char *out_text,
                                   size_t capacity);

/**
 * @brief Set the text for a node when pointer hover high contrast bubbles are
 * enabled
 */
int cmp_a11y_tree_set_node_hover_text(cmp_a11y_tree_t *tree, int node_id,
                                      const char *hover_text_string);

/**
 * @brief Enforce strict minimum contrast ratio of 4.5:1 against its background
 * (or 3:1 for large text)
 */
int cmp_color_verify_contrast_ratio(uint32_t foreground_rgba,
                                    uint32_t background_rgba, int is_large_text,
                                    int *out_passes_wcag);

/**
 * @brief Get the a11y semantic description of a node
 */
int cmp_a11y_tree_get_node_desc(cmp_a11y_tree_t *tree, int node_id,
                                char *out_desc, size_t out_capacity);

/**
 * @brief Opaque Screen Reader OS Integration Context
 */
typedef struct cmp_screen_reader cmp_screen_reader_t;

/**
 * @brief Create a screen reader integration context
 */
int cmp_screen_reader_create(cmp_a11y_tree_t *tree,
                             cmp_screen_reader_t **out_reader);

/**
 * @brief Destroy a screen reader integration context
 */
int cmp_screen_reader_destroy(cmp_screen_reader_t *reader);

/**
 * @brief Announce a message to the screen reader
 */
int cmp_screen_reader_announce(cmp_screen_reader_t *reader,
                               const char *message);

/**
 * @brief Map a specific a11y tree node to a native screen reader node
 */
int cmp_screen_reader_map_node(cmp_screen_reader_t *reader, int node_id,
                               void *native_node);

/**
 * @brief Opaque ARIA State Mapping Context
 */
typedef struct cmp_aria cmp_aria_t;

/**
 * @brief WAI-ARIA standard roles
 */
typedef enum {
  CMP_ARIA_ROLE_NONE = 0,
  CMP_ARIA_ROLE_BUTTON,
  CMP_ARIA_ROLE_CHECKBOX,
  CMP_ARIA_ROLE_DIALOG,
  CMP_ARIA_ROLE_HEADING,
  CMP_ARIA_ROLE_LINK,
  CMP_ARIA_ROLE_TABLIST,
  CMP_ARIA_ROLE_TAB,
  CMP_ARIA_ROLE_TABPANEL,
  CMP_ARIA_ROLE_TOOLTIP
} cmp_aria_role_t;

/**
 * @brief Create an ARIA mapping context
 */
int cmp_aria_create(cmp_a11y_tree_t *tree, cmp_aria_t **out_aria);

/**
 * @brief Destroy an ARIA mapping context
 */
int cmp_aria_destroy(cmp_aria_t *aria);

/**
 * @brief Assign an ARIA role to an a11y node
 */
int cmp_aria_set_role(cmp_aria_t *aria, int node_id, cmp_aria_role_t role);

/**
 * @brief Set a boolean ARIA state (e.g. aria-expanded, aria-checked)
 */
int cmp_aria_set_state_bool(cmp_aria_t *aria, int node_id,
                            const char *state_name, int value);

/**
 * @brief Sync ARIA roles and states to the underlying a11y tree
 */
int cmp_aria_sync(cmp_aria_t *aria);

/**
 * @brief Opaque ARIA Relationship Graph Context
 */
typedef struct cmp_aria_relations cmp_aria_relations_t;

/**
 * @brief Type of ARIA relationship
 */
typedef enum {
  CMP_ARIA_RELATION_OWNS = 1,
  CMP_ARIA_RELATION_CONTROLS,
  CMP_ARIA_RELATION_DESCRIBEDBY
} cmp_aria_relation_type_t;

/**
 * @brief Create an ARIA relationship graph context
 */
int cmp_aria_relations_create(cmp_a11y_tree_t *tree,
                              cmp_aria_relations_t **out_rels);

/**
 * @brief Destroy an ARIA relationship graph context
 */
int cmp_aria_relations_destroy(cmp_aria_relations_t *rels);

/**
 * @brief Add an ARIA relationship
 */
int cmp_aria_relations_add(cmp_aria_relations_t *rels, int source_id,
                           int target_id, cmp_aria_relation_type_t rel_type);

/**
 * @brief Sync relationships to the underlying a11y tree, overriding natural
 * hierarchy
 */
int cmp_aria_relations_sync(cmp_aria_relations_t *rels);

/**
 * @brief Opaque ARIA Live Regions Context
 */
typedef struct cmp_aria_live cmp_aria_live_t;

/**
 * @brief ARIA Live mode types
 */
typedef enum {
  CMP_ARIA_LIVE_OFF = 0,
  CMP_ARIA_LIVE_POLITE,
  CMP_ARIA_LIVE_ASSERTIVE
} cmp_aria_live_mode_t;

/**
 * @brief Create an ARIA Live regions context
 */
int cmp_aria_live_create(cmp_a11y_tree_t *tree, cmp_aria_live_t **out_live);

/**
 * @brief Destroy an ARIA Live regions context
 */
int cmp_aria_live_destroy(cmp_aria_live_t *live);

/**
 * @brief Set the live region mode for a specific node
 */
int cmp_aria_live_set_mode(cmp_aria_live_t *live, int node_id,
                           cmp_aria_live_mode_t mode);

/**
 * @brief Proactively announce a dynamic UI change (e.g. toast notification)
 */
int cmp_aria_live_announce(cmp_aria_live_t *live, int node_id,
                           const char *message);

/**
 * @brief Opaque Focus Manager Context
 */
typedef struct cmp_focus_manager cmp_focus_manager_t;

/**
 * @brief Create a focus manager
 */
int cmp_focus_manager_create(cmp_a11y_tree_t *tree,
                             cmp_focus_manager_t **out_focus_manager);

/**
 * @brief Destroy a focus manager
 */
int cmp_focus_manager_destroy(cmp_focus_manager_t *focus_manager);

/**
 * @brief Set keyboard focus state for a specific node
 */
int cmp_focus_manager_set_focus(cmp_focus_manager_t *focus_manager, int node_id,
                                int has_focus);

/**
 * @brief Computes directional (Up/Down/Left/Right) spatial navigation paths
 * using 2D intersection heuristics
 *
 * @param focus_manager The focus manager.
 * @param current_node_id The currently focused node ID.
 * @param direction 0=Up, 1=Down, 2=Left, 3=Right.
 * @param out_next_node_id The ID of the node to move focus to.
 */
int cmp_focus_manager_navigate(cmp_focus_manager_t *focus_manager,
                               int current_node_id, int direction,
                               int *out_next_node_id);

/**
 * @brief Opaque Focus Ring Context
 */
typedef struct cmp_focus_ring cmp_focus_ring_t;

/**
 * @brief Create a focus ring context
 */
int cmp_focus_ring_create(cmp_a11y_tree_t *tree, cmp_focus_ring_t **out_ring);

/**
 * @brief Destroy a focus ring context
 */
int cmp_focus_ring_destroy(cmp_focus_ring_t *ring);

/**
 * @brief Set whether the user is interacting via keyboard (shows rings) or
 * pointer (hides rings)
 */
int cmp_focus_ring_set_keyboard_mode(cmp_focus_ring_t *ring, int is_keyboard);

/**
 * @brief Notify the focus ring system that a node gained focus
 */
int cmp_focus_ring_node_focused(cmp_focus_ring_t *ring, int node_id);

/**
 * @brief Opaque Accessibility Rotor Hooks Context
 */
typedef struct cmp_a11y_rotor cmp_a11y_rotor_t;

/**
 * @brief Built-in rotor item categories
 */
typedef enum {
  CMP_A11Y_ROTOR_HEADING = 1,
  CMP_A11Y_ROTOR_LINK,
  CMP_A11Y_ROTOR_FORM_ELEMENT,
  CMP_A11Y_ROTOR_LANDMARK
} cmp_a11y_rotor_category_t;

/**
 * @brief Create a rotor context
 */
int cmp_a11y_rotor_create(cmp_a11y_tree_t *tree, cmp_a11y_rotor_t **out_rotor);

/**
 * @brief Destroy a rotor context
 */
int cmp_a11y_rotor_destroy(cmp_a11y_rotor_t *rotor);

/**
 * @brief Register a node into a specific rotor category for fast scanning
 */
int cmp_a11y_rotor_register_node(cmp_a11y_rotor_t *rotor, int node_id,
                                 cmp_a11y_rotor_category_t category);

/**
 * @brief Returns an array of node IDs matching the given category.
 *
 * @param rotor The rotor context.
 * @param category The category to query.
 * @param out_node_ids Buffer to store matching node IDs.
 * @param max_nodes Maximum number of nodes the buffer can hold.
 * @param out_count Actual number of nodes found.
 */
int cmp_a11y_rotor_get_nodes(cmp_a11y_rotor_t *rotor,
                             cmp_a11y_rotor_category_t category,
                             int *out_node_ids, int max_nodes, int *out_count);

/**
 * @brief Opaque Accessibility Action Context
 */
typedef struct cmp_a11y_action cmp_a11y_action_t;

/**
 * @brief Types of accessibility actions triggered by screen readers
 */
typedef enum {
  CMP_A11Y_ACTION_CLICK = 1,
  CMP_A11Y_ACTION_SCROLL_FORWARD,
  CMP_A11Y_ACTION_SCROLL_BACKWARD,
  CMP_A11Y_ACTION_FOCUS,
  CMP_A11Y_ACTION_BLUR
} cmp_a11y_action_type_t;

/**
 * @brief Create an accessibility action context
 */
int cmp_a11y_action_create(cmp_a11y_tree_t *tree,
                           cmp_a11y_action_t **out_action);

/**
 * @brief Destroy an accessibility action context
 */
int cmp_a11y_action_destroy(cmp_a11y_action_t *action);

/**
 * @brief Execute a screen reader action on a specific node
 */
int cmp_a11y_action_execute(cmp_a11y_action_t *action, int node_id,
                            cmp_a11y_action_type_t action_type);

/**
 * @brief Opaque Dynamic Type Scaling Context
 */
typedef struct cmp_dynamic_type cmp_dynamic_type_t;

/**
 * @brief Create a dynamic type context
 */
int cmp_dynamic_type_create(cmp_dynamic_type_t **out_dyn_type);

/**
 * @brief Destroy a dynamic type context
 */
int cmp_dynamic_type_destroy(cmp_dynamic_type_t *dyn_type);

/**
 * @brief Set the OS-level text size scale factor (1.0 = normal, 1.5 = 150%,
 * etc.)
 */
int cmp_dynamic_type_set_scale(cmp_dynamic_type_t *dyn_type, float scale);

/**
 * @brief Apply the dynamic type scale to a root style tree
 */
int cmp_dynamic_type_apply(cmp_dynamic_type_t *dyn_type, void *root_style_tree);

/**
 * @brief Opaque Reduced Motion Context
 */
typedef struct cmp_prefers_reduced_motion cmp_prefers_reduced_motion_t;

/**
 * @brief Create a reduced motion context
 */
int cmp_prefers_reduced_motion_create(cmp_prefers_reduced_motion_t **out_rm);

/**
 * @brief Destroy a reduced motion context
 */
int cmp_prefers_reduced_motion_destroy(cmp_prefers_reduced_motion_t *rm);

/**
 * @brief Enable or disable the reduced motion preference
 */
int cmp_prefers_reduced_motion_set(cmp_prefers_reduced_motion_t *rm,
                                   int enabled);

/**
 * @brief Apply reduced motion preferences to a timeline duration (may zero it
 * out)
 */
int cmp_prefers_reduced_motion_apply(cmp_prefers_reduced_motion_t *rm,
                                     float *duration_ms);

/**
 * @brief Opaque Auto-Play Video Avoidance Context
 */
typedef struct cmp_a11y_autoplay_avoidance cmp_a11y_autoplay_avoidance_t;

/**
 * @brief Create an Auto-Play Avoidance Context
 */
int cmp_a11y_autoplay_avoidance_create(cmp_a11y_autoplay_avoidance_t **out_ctx);

/**
 * @brief Destroy an Auto-Play Avoidance Context
 */
int cmp_a11y_autoplay_avoidance_destroy(cmp_a11y_autoplay_avoidance_t *ctx);

/**
 * @brief Set the OS-level Auto-Play Video setting
 */
int cmp_a11y_autoplay_avoidance_set(cmp_a11y_autoplay_avoidance_t *ctx,
                                    int enabled);

/**
 * @brief Query if videos/animations should autoplay
 */
int cmp_a11y_autoplay_should_play(cmp_a11y_autoplay_avoidance_t *ctx,
                                  int *out_should_play);

/**
 * @brief Set a node as a Switch Control explicit focus anchor
 */
int cmp_a11y_tree_set_node_switch_control_anchor(cmp_a11y_tree_t *tree,
                                                 int node_id, int is_anchor);

/**
 * @brief Map an element to the Voice Control grid, exposing numeric tags and
 * phonetic labels
 */
int cmp_a11y_tree_set_node_voice_control_tag(cmp_a11y_tree_t *tree, int node_id,
                                             const char *phonetic_label,
                                             int numeric_grid_id);

/**
 * @brief Disable elements to conform with Guided Access
 */
int cmp_a11y_tree_set_node_guided_access_disabled(cmp_a11y_tree_t *tree,
                                                  int node_id, int disabled);

/**
 * @brief Configure Time Limits / Cognitive Extensions
 */
int cmp_a11y_tree_set_node_cognitive_time_limit(cmp_a11y_tree_t *tree,
                                                int node_id,
                                                float time_extension_ms);

/**
 * @brief Attach visual alternatives for Sound Recognition/Captions
 */
int cmp_a11y_tree_set_node_sound_caption(cmp_a11y_tree_t *tree, int node_id,
                                         const char *caption);

/**
 * @brief Opaque Reduced Transparency Context
 */
typedef struct cmp_a11y_transparency cmp_a11y_transparency_t;

/**
 * @brief Create an accessibility transparency context
 */
int cmp_a11y_transparency_create(cmp_a11y_transparency_t **out_trans);

/**
 * @brief Destroy an accessibility transparency context
 */
int cmp_a11y_transparency_destroy(cmp_a11y_transparency_t *trans);

/**
 * @brief Enable or disable reduced transparency preference
 */
int cmp_a11y_transparency_set(cmp_a11y_transparency_t *trans, int enabled);

/**
 * @brief Apply reduced transparency preferences (override opacity with
 * fallback)
 */
int cmp_a11y_transparency_apply(cmp_a11y_transparency_t *trans,
                                float *out_opacity, float fallback_opacity);

/**
 * @brief Opaque High Contrast & Forced Colors Mode Context
 */
typedef struct cmp_forced_colors cmp_forced_colors_t;

/**
 * @brief Create a forced colors context
 */
int cmp_forced_colors_create(cmp_forced_colors_t **out_ctx);

/**
 * @brief Destroy a forced colors context
 */
int cmp_forced_colors_destroy(cmp_forced_colors_t *ctx);

/**
 * @brief Enable or disable forced colors mode
 */
int cmp_forced_colors_set(cmp_forced_colors_t *ctx, int active);

/**
 * @brief Apply forced colors striping for a background image or gradient
 *
 * @param ctx Context
 * @param out_strip 1 if background should be stripped, 0 otherwise
 */
int cmp_forced_colors_strip_background(const cmp_forced_colors_t *ctx,
                                       int *out_strip);

/**
 * @brief Apply forced colors striping for box shadows
 *
 * @param ctx Context
 * @param out_strip 1 if box shadow should be stripped, 0 otherwise
 */
int cmp_forced_colors_strip_box_shadow(const cmp_forced_colors_t *ctx,
                                       int *out_strip);

/**
 * @brief System Color Keywords
 */
typedef enum cmp_sys_color_keyword {
  CMP_SYS_COLOR_ACCENT_COLOR,
  CMP_SYS_COLOR_ACCENT_COLOR_TEXT,
  CMP_SYS_COLOR_ACTIVE_TEXT,
  CMP_SYS_COLOR_BUTTON_BORDER,
  CMP_SYS_COLOR_BUTTON_FACE,
  CMP_SYS_COLOR_BUTTON_TEXT,
  CMP_SYS_COLOR_CANVAS,
  CMP_SYS_COLOR_CANVAS_TEXT,
  CMP_SYS_COLOR_FIELD,
  CMP_SYS_COLOR_FIELD_TEXT,
  CMP_SYS_COLOR_GRAY_TEXT,
  CMP_SYS_COLOR_HIGHLIGHT,
  CMP_SYS_COLOR_HIGHLIGHT_TEXT,
  CMP_SYS_COLOR_LINK_TEXT,
  CMP_SYS_COLOR_MARK,
  CMP_SYS_COLOR_MARK_TEXT,
  CMP_SYS_COLOR_VISITED_TEXT,
  CMP_SYS_COLOR_MAX
} cmp_sys_color_keyword_t;

/**
 * @brief Opaque System Color Context
 */
typedef struct cmp_sys_colors cmp_sys_colors_t;

/**
 * @brief Create a system colors context
 */
int cmp_sys_colors_create(cmp_sys_colors_t **out_ctx);

/**
 * @brief Destroy a system colors context
 */
int cmp_sys_colors_destroy(cmp_sys_colors_t *ctx);

/**
 * @brief Set a system color mapping
 */
int cmp_sys_colors_set(cmp_sys_colors_t *ctx, cmp_sys_color_keyword_t keyword,
                       const cmp_color_t *color);

/**
 * @brief Resolve a system color keyword to a concrete color
 */
int cmp_sys_colors_resolve(const cmp_sys_colors_t *ctx,
                           cmp_sys_color_keyword_t keyword,
                           cmp_color_t *out_color);

/**
 * @brief DPI Awareness per Monitor
 * Subscribes to OS display events, dynamically rescaling UI trees
 * when a window is dragged between a 1x 1080p monitor and a 2x 4K monitor.
 */
#ifndef CMP_DPI_T_DEFINED
#define CMP_DPI_T_DEFINED
typedef struct cmp_dpi cmp_dpi_t;
#endif

/**
 * @brief Create a DPI manager context
 */
int cmp_dpi_create(cmp_dpi_t **out_dpi);

/**
 * @brief Destroy a DPI manager context
 */
int cmp_dpi_destroy(cmp_dpi_t *dpi);

/**
 * @brief Set the scale factor for a specific monitor
 */
int cmp_dpi_set_monitor_scale(cmp_dpi_t *dpi, int monitor_id, float scale);

/**
 * @brief Get the scale factor for a specific monitor
 */
int cmp_dpi_get_monitor_scale(const cmp_dpi_t *dpi, int monitor_id,
                              float *out_scale);

/**
 * @brief Update a window's scale factor based on the monitor it resides on
 */
int cmp_dpi_update_window_scale(cmp_dpi_t *dpi, cmp_window_t *window,
                                int monitor_id);

/**
 * @brief System Safe Areas
 * Injects dimensions for physical bezels, Apple Watch circles, TV overscan,
 * and smartphone notches into the `env()` variable system.
 */
#ifndef CMP_SAFE_AREAS_T_DEFINED
#define CMP_SAFE_AREAS_T_DEFINED
typedef struct cmp_safe_areas cmp_safe_areas_t;
#endif

/**
 * @brief Create a safe areas context
 */
int cmp_safe_areas_create(cmp_safe_areas_t **out_safe_areas);

/**
 * @brief Destroy a safe areas context
 */
int cmp_safe_areas_destroy(cmp_safe_areas_t *safe_areas);

/**
 * @brief Set the safe area inset for a specific edge
 * @param edge 0: top, 1: right, 2: bottom, 3: left
 */
int cmp_safe_areas_set_inset(cmp_safe_areas_t *safe_areas, int edge,
                             float inset);

/**
 * @brief Get the safe area inset for a specific edge
 */
int cmp_safe_areas_get_inset(const cmp_safe_areas_t *safe_areas, int edge,
                             float *out_inset);

/**
 * @brief Translucent OS Windows
 * Hooks into Desktop Window Manager (DWM/Mica on Windows,
 * NSVisualEffectView on macOS) to render the UI on a transparent,
 * blurred OS backdrop.
 */
#ifndef CMP_WINDOW_BLUR_T_DEFINED
#define CMP_WINDOW_BLUR_T_DEFINED
typedef struct cmp_window_blur cmp_window_blur_t;
#endif

/**
 * @brief Create a window blur context
 */
int cmp_window_blur_create(cmp_window_blur_t **out_blur);

/**
 * @brief Destroy a window blur context
 */
int cmp_window_blur_destroy(cmp_window_blur_t *blur);

/**
 * @brief Enable or disable background blur for a specific window
 */
int cmp_window_blur_set_enabled(cmp_window_blur_t *blur, cmp_window_t *window,
                                int enabled);

/**
 * @brief Check if background blur is enabled for a specific window
 */
int cmp_window_blur_is_enabled(const cmp_window_blur_t *blur,
                               const cmp_window_t *window, int *out_enabled);

/**
 * @brief Frameless Window Drag Regions
 * Evaluates -webkit-app-region: drag, intercepting mouse events within the UI
 * and passing them to the OS window manager to move the application.
 */
typedef enum {
  CMP_APP_REGION_NONE,
  CMP_APP_REGION_DRAG,
  CMP_APP_REGION_NO_DRAG
} cmp_app_region_type_t;

#ifndef CMP_APP_REGION_T_DEFINED
#define CMP_APP_REGION_T_DEFINED
typedef struct cmp_app_region cmp_app_region_t;
#endif

/**
 * @brief Create a new app region context.
 */
int cmp_app_region_create(cmp_app_region_t **out_region);

/**
 * @brief Destroy an app region context.
 */
int cmp_app_region_destroy(cmp_app_region_t *region);

/**
 * @brief Add a rectangle to an app region.
 */
int cmp_app_region_add_rect(cmp_app_region_t *region, float x, float y,
                            float width, float height,
                            cmp_app_region_type_t type);

/**
 * @brief Clear app region rectangles.
 */
int cmp_app_region_clear(cmp_app_region_t *region);

/**
 * @brief Hit test against app region.
 */
int cmp_app_region_hit_test(const cmp_app_region_t *region, float x, float y,
                            cmp_app_region_type_t *out_type);

/**
 * @brief System Titlebar Overlay
 */
#ifndef CMP_TITLEBAR_ENV_T_DEFINED
#define CMP_TITLEBAR_ENV_T_DEFINED
typedef struct cmp_titlebar_env cmp_titlebar_env_t;
#endif

/**
 * @brief Create a new titlebar env context.
 */
int cmp_titlebar_env_create(cmp_titlebar_env_t **out_env);

/**
 * @brief Destroy a titlebar env context.
 */
int cmp_titlebar_env_destroy(cmp_titlebar_env_t *env);

/**
 * @brief Set the titlebar area.
 */
int cmp_titlebar_env_set_area(cmp_titlebar_env_t *env, float x, float y,
                              float width, float height);

/**
 * @brief Get the titlebar area.
 */
int cmp_titlebar_env_get_area(const cmp_titlebar_env_t *env, float *out_x,
                              float *out_y, float *out_width,
                              float *out_height);

/**
 * @brief Prefers Color Scheme
 */
typedef enum {
  CMP_COLOR_SCHEME_LIGHT,
  CMP_COLOR_SCHEME_DARK
} cmp_color_scheme_t;

#ifndef CMP_PREFERS_COLOR_SCHEME_T_DEFINED
#define CMP_PREFERS_COLOR_SCHEME_T_DEFINED
typedef struct cmp_prefers_color_scheme cmp_prefers_color_scheme_t;
#endif

/**
 * @brief Create a new prefers color scheme context.
 */
int cmp_prefers_color_scheme_create(cmp_prefers_color_scheme_t **out_pcs);

/**
 * @brief Destroy a prefers color scheme context.
 */
int cmp_prefers_color_scheme_destroy(cmp_prefers_color_scheme_t *pcs);

/**
 * @brief Set the current color scheme.
 */
int cmp_prefers_color_scheme_set(cmp_prefers_color_scheme_t *pcs,
                                 cmp_color_scheme_t scheme);

/**
 * @brief Get the current color scheme.
 */
int cmp_prefers_color_scheme_get(const cmp_prefers_color_scheme_t *pcs,
                                 cmp_color_scheme_t *out_scheme);

/**
 * @brief Clipboard API
 */
#ifndef CMP_CLIPBOARD_T_DEFINED
#define CMP_CLIPBOARD_T_DEFINED
typedef struct cmp_clipboard cmp_clipboard_t;
#endif

/**
 * @brief Create a new clipboard context.
 */
int cmp_clipboard_create(cmp_clipboard_t **out_clipboard);

/**
 * @brief Destroy a clipboard context.
 */
int cmp_clipboard_destroy(cmp_clipboard_t *clipboard);

/**
 * @brief Set clipboard text content.
 */
int cmp_clipboard_set_text(cmp_clipboard_t *clipboard, const char *text);

/**
 * @brief Get clipboard text content.
 */
int cmp_clipboard_get_text(const cmp_clipboard_t *clipboard, char **out_text);

/**
 * @brief Clear clipboard content.
 */
int cmp_clipboard_clear(cmp_clipboard_t *clipboard);

/**
 * @brief Drag and Drop (DND) API
 */
typedef enum {
  CMP_DND_OP_NONE = 0,
  CMP_DND_OP_COPY = 1,
  CMP_DND_OP_MOVE = 2,
  CMP_DND_OP_LINK = 3
} cmp_dnd_op_t;

#ifndef CMP_DND_T_DEFINED
#define CMP_DND_T_DEFINED
typedef struct cmp_dnd cmp_dnd_t;
#endif

/**
 * @brief Create a new DND context.
 */
int cmp_dnd_create(cmp_dnd_t **out_dnd);

/**
 * @brief Destroy a DND context.
 */
int cmp_dnd_destroy(cmp_dnd_t *dnd);

/**
 * @brief Set DND operation.
 */
int cmp_dnd_set_operation(cmp_dnd_t *dnd, cmp_dnd_op_t op);

/**
 * @brief Get DND operation.
 */
int cmp_dnd_get_operation(const cmp_dnd_t *dnd, cmp_dnd_op_t *out_op);

/**
 * @brief Set DND payload text.
 */
int cmp_dnd_set_payload_text(cmp_dnd_t *dnd, const char *text);

/**
 * @brief Get DND payload text.
 */
int cmp_dnd_get_payload_text(const cmp_dnd_t *dnd, char **out_text);

/**
 * @brief Native Dialog API
 */
typedef enum {
  CMP_DIALOG_TYPE_ALERT,
  CMP_DIALOG_TYPE_CONFIRM,
  CMP_DIALOG_TYPE_PROMPT,
  CMP_DIALOG_TYPE_FILE_OPEN,
  CMP_DIALOG_TYPE_FILE_SAVE
} cmp_dialog_type_t;

#ifndef CMP_NATIVE_DIALOG_T_DEFINED
#define CMP_NATIVE_DIALOG_T_DEFINED
typedef struct cmp_native_dialog cmp_native_dialog_t;
#endif

/**
 * @brief Create a new native dialog.
 */
int cmp_native_dialog_create(cmp_native_dialog_t **out_dialog);

/**
 * @brief Destroy a native dialog.
 */
int cmp_native_dialog_destroy(cmp_native_dialog_t *dialog);

/**
 * @brief Show a native dialog.
 */
int cmp_native_dialog_show(cmp_native_dialog_t *dialog, cmp_dialog_type_t type);

/**
 * @brief Get the result string from a native dialog.
 */
int cmp_native_dialog_get_result_string(const cmp_native_dialog_t *dialog,
                                        char **out_result);

/**
 * @brief Set the result string for a native dialog (internal/mock).
 */
int cmp_native_dialog_set_result_string(cmp_native_dialog_t *dialog,
                                        const char *result);

/**
 * @brief System Menu API
 */
#ifndef CMP_SYSTEM_MENU_T_DEFINED
#define CMP_SYSTEM_MENU_T_DEFINED
typedef struct cmp_system_menu cmp_system_menu_t;
#endif

/**
 * @brief Create a new system menu.
 */
int cmp_system_menu_create(cmp_system_menu_t **out_menu);

/**
 * @brief Destroy a system menu.
 */
int cmp_system_menu_destroy(cmp_system_menu_t *menu);

/**
 * @brief Add an item to the system menu.
 */
int cmp_system_menu_add_item(cmp_system_menu_t *menu, const char *label,
                             int id);

/**
 * @brief Show the system menu.
 */
int cmp_system_menu_show(cmp_system_menu_t *menu);

/**
 * @brief Haptics API
 */
typedef enum {
  CMP_HAPTICS_TYPE_LIGHT_IMPACT,
  CMP_HAPTICS_TYPE_MEDIUM_IMPACT,
  CMP_HAPTICS_TYPE_HEAVY_IMPACT,
  CMP_HAPTICS_TYPE_SELECTION_CHANGE,
  CMP_HAPTICS_TYPE_SUCCESS,
  CMP_HAPTICS_TYPE_WARNING,
  CMP_HAPTICS_TYPE_ERROR
} cmp_haptics_type_t;

#ifndef CMP_HAPTICS_T_DEFINED
#define CMP_HAPTICS_T_DEFINED
typedef struct cmp_haptics cmp_haptics_t;
#endif

/**
 * @brief Create a new haptics context.
 */
int cmp_haptics_create(cmp_haptics_t **out_haptics);

/**
 * @brief Destroy a haptics context.
 */
int cmp_haptics_destroy(cmp_haptics_t *haptics);

/**
 * @brief Trigger haptic feedback.
 */
int cmp_haptics_trigger(cmp_haptics_t *haptics, cmp_haptics_type_t type);

/* Phase 4.3: Haptics (UIFeedbackGenerator) */

/**
 * @brief Preemptively "warm up" the haptic engine before an anticipated
 * interaction to eliminate latency
 */
int cmp_haptics_prepare(cmp_haptics_t *haptics);

/**
 * @brief Synchronize haptic feedback with associated system UI sounds
 */
int cmp_haptics_trigger_with_audio_sync(cmp_haptics_t *haptics,
                                        cmp_haptics_type_t type,
                                        const char *audio_file_path);

/**
 * @brief Extra Impact Types for iOS 13+ (Rigid, Soft)
 */
int cmp_haptics_trigger_rigid(cmp_haptics_t *haptics);
int cmp_haptics_trigger_soft(cmp_haptics_t *haptics);

/**
 * @brief Picture-in-Picture (PiP) API
 */
#ifndef CMP_PIP_T_DEFINED
#define CMP_PIP_T_DEFINED
typedef struct cmp_pip cmp_pip_t;
#endif

/**
 * @brief Create a new PiP context.
 */
int cmp_pip_create(cmp_pip_t **out_pip);

/**
 * @brief Destroy a PiP context.
 */
int cmp_pip_destroy(cmp_pip_t *pip);

/**
 * @brief Enable PiP mode.
 */
int cmp_pip_enable(cmp_pip_t *pip, void *ui_node_or_video_stream);

/**
 * @brief Disable PiP mode.
 */
int cmp_pip_disable(cmp_pip_t *pip);

/**
 * @brief Check if PiP mode is active.
 */
int cmp_pip_is_active(const cmp_pip_t *pip, int *out_is_active);

/**
 * @brief Print Context API
 */
#ifndef CMP_PRINT_CTX_T_DEFINED
#define CMP_PRINT_CTX_T_DEFINED
typedef struct cmp_print_ctx cmp_print_ctx_t;
#endif

/**
 * @brief Create a new print context.
 */
int cmp_print_ctx_create(cmp_print_ctx_t **out_pc);

/**
 * @brief Destroy a print context.
 */
int cmp_print_ctx_destroy(cmp_print_ctx_t *pc);

/**
 * @brief Begin a new page in print context.
 */
int cmp_print_ctx_begin_page(cmp_print_ctx_t *ctx);

/**
 * @brief End current page in print context.
 */
int cmp_print_ctx_end_page(cmp_print_ctx_t *ctx);

/**
 * @brief Save printed content to a PDF file.
 */
int cmp_print_ctx_save_pdf(cmp_print_ctx_t *ctx, const char *file_path);

/**
 * @brief Network Status API
 */
typedef enum {
  CMP_NETWORK_STATUS_OFFLINE,
  CMP_NETWORK_STATUS_ONLINE,
  CMP_NETWORK_STATUS_METERED
} cmp_network_status_t;

#ifndef CMP_NETWORK_T_DEFINED
#define CMP_NETWORK_T_DEFINED
typedef struct cmp_network cmp_network_t;
#endif

/**
 * @brief Create a new network context.
 */
int cmp_network_create(cmp_network_t **out_network);

/**
 * @brief Destroy a network context.
 */
int cmp_network_destroy(cmp_network_t *network);

/**
 * @brief Get the current network status.
 */
int cmp_network_get_status(const cmp_network_t *network,
                           cmp_network_status_t *out_status);

/**
 * @brief Set the network status (internal/mock).
 */
int cmp_network_set_status(cmp_network_t *network, cmp_network_status_t status);

/**
 * @brief I18n API
 */
#ifndef CMP_I18N_T_DEFINED
#define CMP_I18N_T_DEFINED
typedef struct cmp_i18n cmp_i18n_t;
#endif

/**
 * @brief Create a new i18n context.
 */
int cmp_i18n_create(cmp_i18n_t **out_i18n);

/**
 * @brief Destroy an i18n context.
 */
int cmp_i18n_destroy(cmp_i18n_t *i18n);

/**
 * @brief Add a localized string.
 */
int cmp_i18n_add_string(cmp_i18n_t *i18n, const char *locale, const char *key,
                        const char *value);

/**
 * @brief Get a localized string.
 */
int cmp_i18n_get_string(const cmp_i18n_t *i18n, const char *locale,
                        const char *key, char **out_value);

/**
 * @brief Content Security Policy (CSP) API
 */
typedef enum {
  CMP_CSP_RESOURCE_IMAGE,
  CMP_CSP_RESOURCE_FONT,
  CMP_CSP_RESOURCE_STYLE,
  CMP_CSP_RESOURCE_SCRIPT
} cmp_csp_resource_type_t;

#ifndef CMP_CSP_T_DEFINED
#define CMP_CSP_T_DEFINED
typedef struct cmp_csp cmp_csp_t;
#endif

/**
 * @brief Create a new CSP context.
 */
int cmp_csp_create(cmp_csp_t **out_csp);

/**
 * @brief Destroy a CSP context.
 */
int cmp_csp_destroy(cmp_csp_t *csp);

/**
 * @brief Add a domain to the CSP whitelist.
 */
int cmp_csp_add_domain(cmp_csp_t *csp, const char *domain);

/**
 * @brief Check if a domain is allowed by CSP.
 */
int cmp_csp_check_domain(const cmp_csp_t *csp, const char *domain,
                         cmp_csp_resource_type_t type, int *out_allowed);

/**
 * @brief Multi-column evaluation
 */
int cmp_multicolumn_evaluate(cmp_layout_node_t *node,
                             cmp_column_fill_t fill_mode);

/**
 * @brief Grid alignment evaluation
 */
int cmp_grid_align_evaluate(cmp_grid_align_t align, float track_size,
                            float item_size, float *out_offset);

/**
 * @brief Container query evaluation
 */
int cmp_container_query_evaluate(const cmp_container_query_t *query,
                                 const cmp_container_ctx_t *ctx,
                                 int *out_matches);

/**
 * @brief Create a new resize observer.
 */
int cmp_resize_observer_create(cmp_resize_observer_t **out_observer,
                               void (*on_resize)(cmp_resize_observer_t *,
                                                 cmp_layout_node_t *, float,
                                                 float),
                               void *user_data);

/**
 * @brief Destroy a resize observer.
 */
int cmp_resize_observer_destroy(cmp_resize_observer_t *observer);

/**
 * @brief Notify a resize observer of a change.
 */
int cmp_resize_observer_notify(cmp_resize_observer_t *observer,
                               cmp_layout_node_t *node, float width,
                               float height);

/**
 * @brief Evaluate flex alignment for a node.
 */
int cmp_flex_align_evaluate(cmp_flex_align_t align_val, float cross_size,
                            float item_cross_size, float item_baseline,
                            float max_baseline, float *out_position,
                            float *out_cross_size);

/**
 * @brief Pointer media query evaluation
 */
int cmp_pointer_media_evaluate(const cmp_media_query_t *query,
                               const cmp_media_query_env_t *env,
                               int *out_matches);

/* Phase 23: Privacy, Security & Permissions */

/**
 * @brief Opaque Permissions Context
 */
typedef struct cmp_permissions cmp_permissions_t;

/**
 * @brief Hardware Permission Types
 */
typedef enum cmp_permission_type {
  CMP_PERMISSION_LOCATION = 0,
  CMP_PERMISSION_CAMERA,
  CMP_PERMISSION_MICROPHONE,
  CMP_PERMISSION_PHOTO_LIBRARY,
  CMP_PERMISSION_APP_TRACKING
} cmp_permission_type_t;

/**
 * @brief Permission Status
 */
typedef enum cmp_permission_status {
  CMP_PERMISSION_STATUS_NOT_DETERMINED = 0,
  CMP_PERMISSION_STATUS_RESTRICTED,
  CMP_PERMISSION_STATUS_DENIED,
  CMP_PERMISSION_STATUS_AUTHORIZED,
  CMP_PERMISSION_STATUS_LIMITED,               /* iOS 14+ Limited Photos */
  CMP_PERMISSION_STATUS_AUTHORIZED_APPROXIMATE /* iOS 14+ Approximate Location
                                                */
} cmp_permission_status_t;

/**
 * @brief Create a Permissions Context
 */
int cmp_permissions_create(cmp_permissions_t **out_ctx);

/**
 * @brief Destroy a Permissions Context
 */
int cmp_permissions_destroy(cmp_permissions_t *ctx);

/**
 * @brief Check current permission status
 */
int cmp_permissions_get_status(cmp_permissions_t *ctx,
                               cmp_permission_type_t type,
                               cmp_permission_status_t *out_status);

/**
 * @brief Request permission contextually (with purpose string verification
 * internally)
 */
int cmp_permissions_request(cmp_permissions_t *ctx, cmp_permission_type_t type);

/**
 * @brief Force Screen Recording Prevention (Blackout) for a specific node tree
 */
int cmp_tree_set_screen_recording_prevention(cmp_a11y_tree_t *tree, int node_id,
                                             int enabled);

/**
 * @brief Opaque System Privacy Indicators Context (Microphone/Camera
 * orange/green dots)
 */
typedef struct cmp_privacy_indicators cmp_privacy_indicators_t;

/**
 * @brief Create a Privacy Indicators Context
 */
int cmp_privacy_indicators_create(cmp_privacy_indicators_t **out_indicators);

/**
 * @brief Destroy a Privacy Indicators Context
 */
int cmp_privacy_indicators_destroy(cmp_privacy_indicators_t *indicators);

/**
 * @brief Verify if the custom UI obscures the system privacy indicators
 * (returns error if it does)
 */
int cmp_privacy_indicators_verify_layout(cmp_privacy_indicators_t *indicators,
                                         const cmp_rect_t *ui_bounds,
                                         int *out_is_obscured);

/* Phase 23 (Continued): Authentication & Security Types */

/**
 * @brief Opaque Sign in with Apple (SIWA) Context
 */
typedef struct cmp_siwa_ctx cmp_siwa_ctx_t;

/**
 * @brief Initialize SIWA (Checking OS capability)
 */
int cmp_siwa_create(cmp_siwa_ctx_t **out_ctx);

/**
 * @brief Destroy SIWA context
 */
int cmp_siwa_destroy(cmp_siwa_ctx_t *ctx);

/**
 * @brief Request SIWA authorization
 * @param req_hidden_email Will request the private relay proxy email if 1
 */
int cmp_siwa_request(cmp_siwa_ctx_t *ctx, int req_hidden_email,
                     char *out_token_buf, size_t token_cap);

/**
 * @brief Enforce precise Apple branding heuristics on a button node
 */
int cmp_tree_validate_siwa_branding(cmp_a11y_tree_t *tree, int node_id,
                                    int *out_is_valid);

/**
 * @brief Opaque LocalAuthentication Context (Face ID / Touch ID)
 */
typedef struct cmp_local_auth cmp_local_auth_t;

/**
 * @brief Create Biometric Context
 */
int cmp_local_auth_create(cmp_local_auth_t **out_auth);

/**
 * @brief Destroy Biometric Context
 */
int cmp_local_auth_destroy(cmp_local_auth_t *auth);

/**
 * @brief Request Biometric Authentication (falling back to Passcode)
 */
int cmp_local_auth_request(cmp_local_auth_t *auth, const char *reason,
                           int *out_success);

/**
 * @brief Opaque Secure Enclave / Keychain Context
 */
typedef struct cmp_keychain cmp_keychain_t;

/**
 * @brief Create Keychain Context
 */
int cmp_keychain_create(cmp_keychain_t **out_keychain);

/**
 * @brief Destroy Keychain Context
 */
int cmp_keychain_destroy(cmp_keychain_t *keychain);

/**
 * @brief Save secure credential to the OS keychain/enclave
 */
int cmp_keychain_save(cmp_keychain_t *keychain, const char *key,
                      const char *secret);

/**
 * @brief Load secure credential from the OS keychain/enclave
 */
int cmp_keychain_load(cmp_keychain_t *keychain, const char *key,
                      char *out_secret, size_t secret_cap);

/**
 * @brief Text Content Types for System AutoFill
 */
typedef enum cmp_text_content_type {
  CMP_TEXT_CONTENT_TYPE_NONE = 0,
  CMP_TEXT_CONTENT_TYPE_USERNAME,
  CMP_TEXT_CONTENT_TYPE_PASSWORD,
  CMP_TEXT_CONTENT_TYPE_NEW_PASSWORD,
  CMP_TEXT_CONTENT_TYPE_ONE_TIME_CODE
} cmp_text_content_type_t;

/**
 * @brief Tag a text field to trigger OS password managers
 */
int cmp_a11y_tree_set_node_text_content_type(cmp_a11y_tree_t *tree, int node_id,
                                             cmp_text_content_type_t type);

/**
 * @brief Instruct the OS to blur the window entirely during the app
 * switcher/backgrounding
 */
int cmp_window_set_secure_background_obscure(struct cmp_window *window,
                                             int enabled);

/**
 * @brief Check OS APIs to see if a loaded image/video payload is flagged as
 * explicit content
 */
int cmp_visuals_check_sensitive_content(const void *payload, size_t size,
                                        int *out_is_sensitive);

/* Phase 3: Visual Design & Rendering Architecture */

/**
 * @brief Opaque System Colors Context for mapping semantic colors
 */
typedef struct cmp_semantic_colors cmp_semantic_colors_t;

/**
 * @brief Create a Semantic Colors Context
 */
int cmp_semantic_colors_create(cmp_semantic_colors_t **out_ctx);

/**
 * @brief Destroy a Semantic Colors Context
 */
int cmp_semantic_colors_destroy(cmp_semantic_colors_t *ctx);

/**
 * @brief Resolve a semantic OS color (e.g., systemBlue,
 * secondarySystemBackground) into 32-bit RGBA
 */
int cmp_semantic_colors_resolve(cmp_semantic_colors_t *ctx,
                                const char *semantic_name, int is_dark_mode,
                                uint32_t *out_rgba);

/**
 * @brief Set the global application tint/accent color
 */
int cmp_semantic_colors_set_tint_color(cmp_semantic_colors_t *ctx,
                                       uint32_t tint_rgba);

/**
 * @brief Get the active tint color, handling fallback defaults
 */
int cmp_semantic_colors_get_tint_color(cmp_semantic_colors_t *ctx,
                                       uint32_t *out_tint_rgba);

/**
 * @brief Resolve elevation background color for iOS dark mode (lighter grays
 * instead of shadows)
 */
int cmp_semantic_colors_resolve_elevation(cmp_semantic_colors_t *ctx,
                                          int elevation_level, int is_dark_mode,
                                          uint32_t *out_rgba);

/**
 * @brief Opaque Color Profiling and P3 Context
 */
typedef struct cmp_color_pipeline cmp_color_pipeline_t;

/**
 * @brief Initialize the P3 16-bit / EDR rendering pipeline context
 */
int cmp_color_pipeline_create(cmp_color_pipeline_t **out_pipeline);

/**
 * @brief Destroy the color pipeline context
 */
int cmp_color_pipeline_destroy(cmp_color_pipeline_t *pipeline);

/**
 * @brief Check if the current display hardware natively supports Wide Color
 * (Display P3)
 */
int cmp_color_pipeline_supports_p3(cmp_color_pipeline_t *pipeline,
                                   int *out_supports_p3);

/**
 * @brief Check if the current display hardware natively supports Extended
 * Dynamic Range (EDR/HDR)
 */
int cmp_color_pipeline_supports_edr(cmp_color_pipeline_t *pipeline,
                                    int *out_supports_edr);

/**
 * @brief Convert an sRGB color to Display P3 color space
 */
int cmp_color_pipeline_srgb_to_p3(cmp_color_pipeline_t *pipeline, float r,
                                  float g, float b, float *out_p3_r,
                                  float *out_p3_g, float *out_p3_b);

/* Phase 3.2: Typography & Text Rendering (Apple HIG specific) */

/**
 * @brief System Font Type Ecosystem
 */
typedef enum cmp_system_font_type {
  CMP_SYSTEM_FONT_SF_PRO = 0,    /* Default iOS/macOS */
  CMP_SYSTEM_FONT_SF_COMPACT,    /* watchOS */
  CMP_SYSTEM_FONT_SF_MONO,       /* Code/Tabular */
  CMP_SYSTEM_FONT_NEW_YORK,      /* Serif */
  CMP_SYSTEM_FONT_SEGOE_UI,      /* Windows Fluent 2 Default */
  CMP_SYSTEM_FONT_SEGOE_UI_VAR,  /* Windows Fluent 2 Variable */
  CMP_SYSTEM_FONT_SYSTEM_DEFAULT /* Fallback */
} cmp_system_font_type_t;

/**
 * @brief System Text Styles (Dynamic Type sizing)
 */
typedef enum cmp_semantic_text_style {
  CMP_TEXT_STYLE_LARGE_TITLE = 0,
  CMP_TEXT_STYLE_TITLE_1,
  CMP_TEXT_STYLE_TITLE_2,
  CMP_TEXT_STYLE_TITLE_3,
  CMP_TEXT_STYLE_HEADLINE,
  CMP_TEXT_STYLE_BODY,
  CMP_TEXT_STYLE_CALLOUT,
  CMP_TEXT_STYLE_SUBHEAD,
  CMP_TEXT_STYLE_FOOTNOTE,
  CMP_TEXT_STYLE_CAPTION_1,
  CMP_TEXT_STYLE_CAPTION_2
} cmp_semantic_text_style_t;

/**
 * @brief Opaque System Font Resolution Context
 */
typedef struct cmp_system_fonts cmp_system_fonts_t;

/**
 * @brief Create a System Fonts Context
 */
int cmp_system_fonts_create(cmp_system_fonts_t **out_ctx);

/**
 * @brief Destroy a System Fonts Context
 */
int cmp_system_fonts_destroy(cmp_system_fonts_t *ctx);

/**
 * @brief Request an Apple ecosystem font (SF Pro, Compact, Mono, NY) by
 * semantic style
 */
int cmp_system_fonts_request(cmp_system_fonts_t *ctx,
                             cmp_system_font_type_t type,
                             cmp_semantic_text_style_t style, int weight,
                             cmp_font_t **out_font);

/**
 * @brief Automatically calculate optical size (Text vs Display variant) and
 * dynamic tracking (letter-spacing) based on the requested point size.
 */
int cmp_system_fonts_get_tracking_and_optical(cmp_system_fonts_t *ctx,
                                              float point_size,
                                              int *out_is_display_variant,
                                              float *out_tracking);

/**
 * @brief Set standard OpenType features (Kerning, Ligatures, Tabular Figures)
 */
int cmp_font_set_opentype_features(cmp_font_t *font, int enable_kerning,
                                   int enable_ligatures,
                                   int enable_tabular_figures);

/**
 * @brief Set variable font axes (Weight, Width)
 */
int cmp_font_set_variable_axes(cmp_font_t *font, float weight, float width);

/**
 * @brief Get the standard Apple line-height and leading metrics for a specific
 * text style
 */
int cmp_system_fonts_get_metrics(cmp_system_fonts_t *ctx,
                                 cmp_semantic_text_style_t style,
                                 float *out_line_height, float *out_leading,
                                 float *out_baseline_offset);

/**
 * @brief UIBlurEffect standard thickness styles
 */
typedef enum cmp_blur_style {
  CMP_BLUR_STYLE_ULTRA_THIN = 0,
  CMP_BLUR_STYLE_THIN,
  CMP_BLUR_STYLE_REGULAR,
  CMP_BLUR_STYLE_THICK,
  CMP_BLUR_STYLE_PROMINENT
} cmp_blur_style_t;

/**
 * @brief macOS Window Material Equivalencies
 */
typedef enum cmp_macos_material {
  CMP_MACOS_MATERIAL_WINDOW_BACKGROUND = 0,
  CMP_MACOS_MATERIAL_BEHIND_WINDOW,          /* Standard translucent sidebars */
  CMP_MACOS_MATERIAL_UNDER_WINDOW_BACKGROUND /* Standard scrolling content */
} cmp_macos_material_t;

/**
 * @brief UIVibrancyEffect mapping
 */
typedef enum cmp_vibrancy_style {
  CMP_VIBRANCY_STYLE_NONE = 0,
  CMP_VIBRANCY_STYLE_LABEL,
  CMP_VIBRANCY_STYLE_SECONDARY_LABEL,
  CMP_VIBRANCY_STYLE_TERTIARY_LABEL,
  CMP_VIBRANCY_STYLE_FILL,
  CMP_VIBRANCY_STYLE_SECONDARY_FILL
} cmp_vibrancy_style_t;

/**
 * @brief Opaque Material and Blur Rendering Context
 */
typedef struct cmp_materials cmp_materials_t;

/**
 * @brief Initialize the Materials Context
 */
int cmp_materials_create(cmp_materials_t **out_materials);

/**
 * @brief Destroy the Materials Context
 */
int cmp_materials_destroy(cmp_materials_t *materials);

/**
 * @brief Evaluate the parameters for a standard UIBlurEffect style
 * @param style The desired blur thickness
 * @param out_radius_px Pointer to receive the underlying gaussian blur radius
 * in pixels
 * @param out_saturation_multiplier Pointer to receive the saturation boost
 * factor
 */
int cmp_materials_resolve_blur_effect(cmp_materials_t *materials,
                                      cmp_blur_style_t style,
                                      float *out_radius_px,
                                      float *out_saturation_multiplier);

typedef enum cmp_windows_material {
  CMP_WINDOWS_MATERIAL_NONE = 0,
  CMP_WINDOWS_MATERIAL_MICA,         /* Standard Mica (Windows 11) */
  CMP_WINDOWS_MATERIAL_MICA_ALT,     /* Tabbed app background */
  CMP_WINDOWS_MATERIAL_ACRYLIC_BASE, /* Semi-transparent layer */
  CMP_WINDOWS_MATERIAL_ACRYLIC_THIN  /* Tooltips and flyouts */
} cmp_windows_material_t;

/**
 * @brief Map a native macOS material to cross-platform shader hints
 */
int cmp_materials_resolve_macos_material(cmp_materials_t *materials,
                                         cmp_macos_material_t material,
                                         cmp_blur_style_t *out_mapped_style);

/**
 * @brief Request an OS-level window material composition (Mica/Acrylic) on
 * Windows.
 */
int cmp_materials_request_windows_material(cmp_materials_t *materials,
                                           cmp_window_t *window,
                                           cmp_windows_material_t material);

/**
 * @brief Retrieve blend modes required to implement UIVibrancyEffect (text
 * layering over blur)
 */
int cmp_materials_resolve_vibrancy(cmp_materials_t *materials,
                                   cmp_vibrancy_style_t style,
                                   float *out_opacity,
                                   int *out_requires_color_dodge);

/**
 * @brief Calculate the intermediate state of a blur material crossfade
 * transition
 * @param from_style The starting blur thickness
 * @param to_style The ending blur thickness
 * @param progress Normalised float [0.0 - 1.0] representing transition progress
 * @param out_current_radius_px The interpolated blur radius to feed into the
 * shader
 */
int cmp_materials_interpolate_blur_transition(cmp_materials_t *materials,
                                              cmp_blur_style_t from_style,
                                              cmp_blur_style_t to_style,
                                              float progress,
                                              float *out_current_radius_px);

/* Phase 3.4: Iconography & SF Symbols */

/**
 * @brief Symbol Scale Constraints (Relative to current font size)
 */
typedef enum cmp_symbol_scale {
  CMP_SYMBOL_SCALE_SMALL = 0,
  CMP_SYMBOL_SCALE_MEDIUM, /* Default */
  CMP_SYMBOL_SCALE_LARGE
} cmp_symbol_scale_t;

/**
 * @brief Rendering mode for complex multi-layer symbols
 */
typedef enum cmp_symbol_rendering_mode {
  CMP_SYMBOL_RENDERING_MONOCHROME = 0, /* Standard single color */
  CMP_SYMBOL_RENDERING_HIERARCHICAL,   /* Opacity layers of a single color */
  CMP_SYMBOL_RENDERING_PALETTE,        /* Explicit color per layer */
  CMP_SYMBOL_RENDERING_MULTICOLOR      /* Intrinsic original full-color (e.g.
                                          weather) */
} cmp_symbol_rendering_mode_t;

/**
 * @brief Available symbol animation hooks (iOS 17+)
 */
typedef enum cmp_symbol_animation {
  CMP_SYMBOL_ANIM_NONE = 0,
  CMP_SYMBOL_ANIM_BOUNCE,
  CMP_SYMBOL_ANIM_SCALE,
  CMP_SYMBOL_ANIM_PULSE,
  CMP_SYMBOL_ANIM_REPLACE,
  CMP_SYMBOL_ANIM_VARIABLE_COLOR
} cmp_symbol_animation_t;

/**
 * @brief Opaque Symbol Resolution Context
 */
typedef struct cmp_symbols cmp_symbols_t;

/**
 * @brief Create a Symbols Context
 */
int cmp_symbols_create(cmp_symbols_t **out_ctx);

/**
 * @brief Destroy a Symbols Context
 */
int cmp_symbols_destroy(cmp_symbols_t *ctx);

/**
 * @brief Retrieve an SF Symbol via its native OS identifier (e.g. "star.fill")
 * Automatically matches the current text weight.
 */
int cmp_symbols_request(cmp_symbols_t *ctx, const char *symbol_name, int weight,
                        cmp_symbol_scale_t scale, void **out_symbol_handle);

/**
 * @brief Apply rendering styles to a requested symbol
 * @param colors Array of colors. Length determines palette slots.
 *               Length=1 + Hierarchical = varying opacity.
 *               Length=1 + Monochrome = single fill.
 */
int cmp_symbols_apply_style(cmp_symbols_t *ctx, void *symbol_handle,
                            cmp_symbol_rendering_mode_t mode,
                            const uint32_t *colors, size_t color_count);

/**
 * @brief Set the variable fill percentage for symbols that support Variable
 * Color (e.g. Wi-Fi bars)
 */
int cmp_symbols_set_variable_value(cmp_symbols_t *ctx, void *symbol_handle,
                                   float percentage);

/**
 * @brief Trigger an iOS 17+ style bounce/pulse animation on the symbol
 */
int cmp_symbols_trigger_animation(cmp_symbols_t *ctx, void *symbol_handle,
                                  cmp_symbol_animation_t animation,
                                  int looping);

/**
 * @brief Load a custom SVG explicitly exported via Apple's SF Symbols template
 */
int cmp_symbols_load_custom_template(cmp_symbols_t *ctx, const char *svg_path,
                                     void **out_symbol_handle);

/* Phase 3.5: Layout, Margins & Geometry */

/**
 * @brief Continuous Corners (Squircles) - Apple's kCACornerCurveContinuous
 * formula
 */
int cmp_layout_set_corner_curve_continuous(cmp_layout_node_t *node,
                                           int is_continuous);

/**
 * @brief Enforce precise pixel boundaries to prevent sub-pixel anti-aliasing
 * blurring on borders and text
 */
int cmp_layout_enforce_pixel_alignment(cmp_layout_node_t *node, int is_aligned);

/**
 * @brief Opaque System Margins and Safe Area Constants Structure
 */
typedef struct cmp_system_geometry cmp_system_geometry_t;

/**
 * @brief Create a System Geometry Context
 */
int cmp_system_geometry_create(cmp_system_geometry_t **out_geom);

/**
 * @brief Destroy a System Geometry Context
 */
int cmp_system_geometry_destroy(cmp_system_geometry_t *geom);

/**
 * @brief Retrieve standard OS-level safe area insets (avoiding notches, home
 * indicators)
 */
int cmp_system_geometry_get_safe_area(cmp_system_geometry_t *geom, int is_tvos,
                                      cmp_rect_t *out_safe_insets);

/**
 * @brief Retrieve standard OS-level leading/trailing layout margins (16pt
 * compact, 20pt regular)
 */
int cmp_system_geometry_get_layout_margins(cmp_system_geometry_t *geom,
                                           int is_compact_width,
                                           float *out_leading,
                                           float *out_trailing);

/**
 * @brief Retrieve the readable content guide (max text width constraint for
 * iPads/Macs)
 */
int cmp_system_geometry_get_readable_content_guide(
    cmp_system_geometry_t *geom, float available_width,
    float *out_max_readable_width);

/* Phase 4.1: Spring Physics & Timing */

/**
 * @brief Opaque UIViewPropertyAnimator equivalency context
 */
typedef struct cmp_spring_animator cmp_spring_animator_t;

/**
 * @brief Create a scrubbable spring animator (CASpringAnimation equivalency)
 * @param mass Mass of the simulated object (default 1.0)
 * @param stiffness Spring stiffness (default 100.0)
 * @param damping Spring damping (default 10.0)
 * @param initial_velocity The velocity in points-per-second passed from a
 * gesture (swipe/scroll)
 */
int cmp_spring_animator_create(float mass, float stiffness, float damping,
                               float initial_velocity,
                               cmp_spring_animator_t **out_animator);

/**
 * @brief Destroy a spring animator
 */
int cmp_spring_animator_destroy(cmp_spring_animator_t *animator);

/**
 * @brief Interpolate an animation mid-flight. The new animation uses the
 * current presentation value and velocity as its starting point.
 */
int cmp_spring_animator_interrupt(cmp_spring_animator_t *animator,
                                  float new_target_value);

/**
 * @brief Pause, scrub, or reverse the animation interactively
 * @param fraction_complete Normalized progress [0.0 - 1.0] representing the
 * scrub offset
 */
int cmp_spring_animator_scrub(cmp_spring_animator_t *animator,
                              float fraction_complete);

/**
 * @brief Evaluate the current value of the spring, automatically accounting for
 * overshoot & bounciness
 * @param dt_seconds Time elapsed since the animation started
 * @param out_current_value The output coordinate or property value
 * @param out_is_settled 1 if the spring has completely settled (velocity is 0),
 * otherwise 0
 */
int cmp_spring_animator_evaluate(cmp_spring_animator_t *animator,
                                 float dt_seconds, float *out_current_value,
                                 int *out_is_settled);

/**
 * @brief Convert raw swipe/pan gesture points into a points-per-second velocity
 * metric to feed into a spring
 */
int cmp_spring_calculate_gesture_velocity(float delta_x, float delta_y,
                                          float dt_seconds,
                                          float *out_velocity_x,
                                          float *out_velocity_y);

/* Phase 4.2: ProMotion (120Hz) & Render Loop */

/**
 * @brief Opaque ProMotion/VRR Context
 */
typedef struct cmp_promotion_link cmp_promotion_link_t;

/**
 * @brief Available Target Frame Rates
 */
typedef enum cmp_frame_rate {
  CMP_FRAME_RATE_DEFAULT = 0, /* Standard 60Hz */
  CMP_FRAME_RATE_LOW,         /* 10Hz/24Hz VRR saving */
  CMP_FRAME_RATE_HIGH         /* 120Hz ProMotion */
} cmp_frame_rate_t;

/**
 * @brief Create a ProMotion DisplayLink Context
 */
int cmp_promotion_link_create(cmp_promotion_link_t **out_link);

/**
 * @brief Destroy a ProMotion DisplayLink Context
 */
int cmp_promotion_link_destroy(cmp_promotion_link_t *link);

/**
 * @brief Synchronize the render loop to CADisplayLink / Choreographer
 * equivalent callbacks
 */
int cmp_promotion_link_sync(cmp_promotion_link_t *link, int is_sync_enabled);

/**
 * @brief Request an explicit preferred frame rate range (unlocks 120Hz if
 * available)
 */
int cmp_promotion_link_request_rate(cmp_promotion_link_t *link,
                                    cmp_frame_rate_t requested_rate);

/**
 * @brief Evaluates current UI activity (scrolling/animating vs static) to
 * dynamically shift VRR targets
 */
int cmp_promotion_link_evaluate_vrr(cmp_promotion_link_t *link,
                                    int is_animating, int is_scrolling,
                                    cmp_frame_rate_t *out_target_rate);

/**
 * @brief Simulated telemetry hook: Validates that 0 frames were dropped in the
 * last continuous scrolling/animating interaction window
 */
int cmp_promotion_link_validate_frame_drops(cmp_promotion_link_t *link,
                                            int *out_dropped_frames);

/* Phase 5.1: Routing Models (Apple HIG specific) */

/**
 * @brief Routing presentation styles
 */
typedef enum cmp_presentation_style {
  CMP_PRESENTATION_STYLE_PUSH =
      0, /* Standard Navigation Bar push/pop (Hierarchical) */
  CMP_PRESENTATION_STYLE_TAB, /* Instant switch, pops stack to root (Flat) */
  CMP_PRESENTATION_STYLE_SPLIT_VIEW, /* Master-Detail / Sidebar (Content-Driven)
                                      */
  CMP_PRESENTATION_STYLE_FULLSCREEN, /* Hides status bars (Immersive) */
  CMP_PRESENTATION_STYLE_SHEET /* Slides from bottom, parent recedes (Modal) */
} cmp_presentation_style_t;

/**
 * @brief Push a route onto the stack with a specific Apple HIG presentation
 * style
 */
int cmp_router_push_with_style(cmp_router_t *router, const char *uri,
                               cmp_presentation_style_t style);

/**
 * @brief Pop the current route (with specific animation based on how it was
 * presented)
 */
int cmp_router_pop_with_style(cmp_router_t *router);

/**
 * @brief For Hierarchical navigation, retrieve the exact title of the
 * *previous* view to populate the back button
 */
int cmp_router_get_previous_title(cmp_router_t *router, char *out_title,
                                  size_t title_cap);

/**
 * @brief Force a specific tab to become active, automatically popping any deep
 * navigation inside it back to root
 */
int cmp_router_switch_tab(cmp_router_t *router, const char *tab_uri);

/**
 * @brief Opaque Split View (Master-Detail) Controller
 */
typedef struct cmp_split_view cmp_split_view_t;

/**
 * @brief Create a Split View Controller context
 */
int cmp_split_view_create(cmp_split_view_t **out_split_view);

/**
 * @brief Destroy a Split View Controller
 */
int cmp_split_view_destroy(cmp_split_view_t *split_view);

/**
 * @brief Assign routes to the Master (Sidebar) and Detail (Content) panes
 */
int cmp_split_view_set_routes(cmp_split_view_t *split_view,
                              const char *master_uri, const char *detail_uri);

/* Phase 5.2: Navigation Bars (Apple HIG specific) */

/**
 * @brief Navigation Bar Appearance States
 */
typedef enum cmp_nav_bar_appearance {
  CMP_NAV_BAR_APPEARANCE_STANDARD =
      0,                          /* Solid/Blurred based on scroll position */
  CMP_NAV_BAR_APPEARANCE_COMPACT, /* Condensed for landscape iPhones */
  CMP_NAV_BAR_APPEARANCE_SCROLL_EDGE /* Transparent when scrolled to the very
                                        top */
} cmp_nav_bar_appearance_t;

/**
 * @brief Opaque Navigation Bar Context
 */
typedef struct cmp_nav_bar cmp_nav_bar_t;

/**
 * @brief Create a Navigation Bar Controller
 */
int cmp_nav_bar_create(cmp_nav_bar_t **out_nav_bar);

/**
 * @brief Destroy a Navigation Bar Controller
 */
int cmp_nav_bar_destroy(cmp_nav_bar_t *nav_bar);

/**
 * @brief Set Large Titles (Default True for primary views)
 */
int cmp_nav_bar_set_prefers_large_titles(cmp_nav_bar_t *nav_bar,
                                         int prefers_large_titles);

/**
 * @brief Calculate the scroll collapse of a large title into a standard inline
 * title
 * @param scroll_y Current vertical scroll offset
 * @param out_title_scale Output scale multiplier [1.0 to ~0.6]
 * @param out_title_y_offset Output Y translation for the title text
 */
int cmp_nav_bar_calculate_scroll_collapse(cmp_nav_bar_t *nav_bar,
                                          float scroll_y,
                                          float *out_title_scale,
                                          float *out_title_y_offset);

/**
 * @brief Configure the strict appearance state
 */
int cmp_nav_bar_set_appearance(cmp_nav_bar_t *nav_bar,
                               cmp_nav_bar_appearance_t appearance);

/**
 * @brief Determine back button truncation based on available width and previous
 * title length
 * @param previous_title The raw string of the previous view's title
 * @param available_width The layout width available before hitting center title
 * @param out_label Populated with the exact string to render ("Back" if
 * truncated)
 */
int cmp_nav_bar_resolve_back_button_label(cmp_nav_bar_t *nav_bar,
                                          const char *previous_title,
                                          float available_width,
                                          char *out_label, size_t label_cap);

/**
 * @brief Configure exact OS-native Back Button Chevron layout metrics
 */
int cmp_nav_bar_get_chevron_metrics(cmp_nav_bar_t *nav_bar, float *out_weight,
                                    float *out_leading_padding);

/* Phase 5.3: Tab Bars & Toolbars (Apple HIG specific) */

/**
 * @brief Represents the placement rules for Tab Bars dynamically
 */
typedef enum cmp_tab_bar_placement {
  CMP_TAB_BAR_PLACEMENT_BOTTOM = 0, /* Default iPhone */
  CMP_TAB_BAR_PLACEMENT_LEADING     /* Default iPadOS/macOS Sidebar */
} cmp_tab_bar_placement_t;

/**
 * @brief Opaque Tab Bar Controller
 */
typedef struct cmp_tab_bar cmp_tab_bar_t;

/**
 * @brief Create a Tab Bar Controller
 */
int cmp_tab_bar_create(cmp_tab_bar_t **out_tab_bar);

/**
 * @brief Destroy a Tab Bar Controller
 */
int cmp_tab_bar_destroy(cmp_tab_bar_t *tab_bar);

/**
 * @brief Add a tab to the controller. The framework enforces HIG limits
 * internally (max 5 visible, rest overflow to 'More').
 */
int cmp_tab_bar_add_tab(cmp_tab_bar_t *tab_bar, const char *title,
                        const char *symbol_icon, const char *route_uri);

/**
 * @brief Set a notification badge on a specific tab index
 * @param badge_value Positive integer to display (e.g. 3), 0 to hide.
 */
int cmp_tab_bar_set_badge(cmp_tab_bar_t *tab_bar, size_t tab_index,
                          int badge_value);

/**
 * @brief Resolve the dynamic layout constraints for the Tab Bar/Sidebar
 * depending on the active Viewport width
 * @param available_width The current container width (used to toggle iPhone vs
 * iPad behaviors)
 * @param out_placement Will return Bottom vs Leading
 * @param out_material Will return standard vs BehindWindow material bindings
 */
int cmp_tab_bar_resolve_layout(cmp_tab_bar_t *tab_bar, float available_width,
                               cmp_tab_bar_placement_t *out_placement,
                               cmp_macos_material_t *out_material);

/**
 * @brief Contextual Toolbar Position Logic
 * Retrieves HIG placement mapping for toolbars (bottom on iPhone,
 * top-nav-integrated on iPad/Mac).
 */
int cmp_toolbar_resolve_placement(float available_width,
                                  int *out_is_bottom_placed);

/* Phase 5.4: Search Architecture (Apple HIG specific) */

/**
 * @brief Search Bar Display States
 */
typedef enum cmp_search_bar_state {
  CMP_SEARCH_BAR_INACTIVE = 0, /* Centered placeholder, no cancel button */
  CMP_SEARCH_BAR_ACTIVE /* Leading aligned, cancel button visible, clear button
                           visible if text exists */
} cmp_search_bar_state_t;

/**
 * @brief Opaque Search Controller
 */
typedef struct cmp_search_controller cmp_search_controller_t;

/**
 * @brief Create a Search Controller
 */
int cmp_search_controller_create(cmp_search_controller_t **out_controller);

/**
 * @brief Destroy a Search Controller
 */
int cmp_search_controller_destroy(cmp_search_controller_t *controller);

/**
 * @brief Set current text input in the search bar
 */
int cmp_search_controller_set_text(cmp_search_controller_t *controller,
                                   const char *text);

/**
 * @brief Add a scope filter option (e.g. "All", "Unread")
 */
int cmp_search_controller_add_scope(cmp_search_controller_t *controller,
                                    const char *scope_title);

/**
 * @brief Triggers focus and animates the placeholder from Center to Leading
 */
int cmp_search_controller_set_active(cmp_search_controller_t *controller,
                                     int active);

/**
 * @brief Evaluates visual metrics for the search bar (clear button, cancel
 * button offsets)
 * @param out_show_clear Returns 1 if 'x' clear button should be rendered
 * @param out_show_cancel Returns 1 if "Cancel" button should slide in from the
 * right
 * @param out_placeholder_offset Returns the X translation offset for the
 * placeholder text (0.0 for leading, >0 for center based on width)
 */
int cmp_search_controller_resolve_metrics(cmp_search_controller_t *controller,
                                          float available_width,
                                          int *out_show_clear,
                                          int *out_show_cancel,
                                          float *out_placeholder_offset);

/**
 * @brief Submits suggestion results overlaid on the UI
 */
int cmp_search_controller_set_suggestions(cmp_search_controller_t *controller,
                                          const char **suggestions,
                                          size_t count);

/* Phase 5.5: Deep Linking & State Restoration (Apple HIG specific) */

/**
 * @brief Opaque State Restoration Context (NSUserActivity/UIWindowScene
 * equivalent)
 */
typedef struct cmp_state_restoration_ctx cmp_state_restoration_ctx_t;

/**
 * @brief Create a State Restoration Context
 */
int cmp_state_restoration_ctx_create(cmp_state_restoration_ctx_t **out_ctx);

/**
 * @brief Destroy a State Restoration Context
 */
int cmp_state_restoration_ctx_destroy(cmp_state_restoration_ctx_t *ctx);

/**
 * @brief Handles Universal Links (Deep Links) routing deeply into the framework
 * Maps directly to NSUserActivity continuations on iOS/macOS.
 */
int cmp_deep_link_handle_universal_link(cmp_state_restoration_ctx_t *ctx,
                                        const char *url_string,
                                        cmp_router_t *router);

/**
 * @brief Encodes the current view controller and scroll state into a
 * persistable buffer
 * @param out_buffer The serialized state blob
 * @param out_size The size of the serialized blob
 */
int cmp_state_restoration_encode(cmp_state_restoration_ctx_t *ctx,
                                 cmp_router_t *router, void **out_buffer,
                                 size_t *out_size);

/**
 * @brief Decodes a previously saved state buffer, restoring the exact
 * navigation stack and scroll offsets
 */
int cmp_state_restoration_decode(cmp_state_restoration_ctx_t *ctx,
                                 cmp_router_t *router, const void *buffer,
                                 size_t size);

/**
 * @brief Registers a multi-window UIWindowScene state boundary.
 * Essential for iPadOS/macOS to ensure separate windows don't overwrite each
 * other's state restorations.
 */
int cmp_state_restoration_set_scene_id(cmp_state_restoration_ctx_t *ctx,
                                       const char *scene_identifier);

/* Phase 6.1: Buttons & Toggles (Apple HIG specific) */

/**
 * @brief Button styling configurations
 */
typedef enum cmp_button_style {
  CMP_BUTTON_STYLE_PLAIN = 0, /* Text/Icon only, dim to 30% opacity on press */
  CMP_BUTTON_STYLE_FILLED,    /* Solid prominent background (capsule or rounded
                                 rect) */
  CMP_BUTTON_STYLE_TINTED, /* 15-20% opacity background of the accent color */
  CMP_BUTTON_STYLE_GRAY,   /* Subtle, translucent gray background */
  CMP_BUTTON_STYLE_CLOSE   /* Small, circular, blurred gray background with dark
                              'x' */
} cmp_button_style_t;

/**
 * @brief Opaque Button Control
 */
typedef struct cmp_button cmp_button_t;

/**
 * @brief Opaque Toggle (Switch) Control
 */
typedef struct cmp_toggle cmp_toggle_t;

/**
 * @brief Create a Button Control
 */
int cmp_button_create(cmp_button_t **out_button);

/**
 * @brief Destroy a Button Control
 */
int cmp_button_destroy(cmp_button_t *button);

/**
 * @brief Set the button style
 */
int cmp_button_set_style(cmp_button_t *button, cmp_button_style_t style);

/**
 * @brief Handle an event targeted at the button (updates internal state like
 * is_pressed)
 */
int cmp_button_handle_event(cmp_button_t *button, const cmp_event_t *event);

/**
 * @brief Tick the button's internal state machine
 */
int cmp_button_update(cmp_button_t *button, float dt_ms);

/**
 * @brief Retrieves the current state of the button
 */
int cmp_button_get_state(const cmp_button_t *button,
                         cmp_button_style_t *out_style, int *out_is_pressed);

/**
 * @brief Create a Toggle Control (Switch)
 */
int cmp_toggle_create(cmp_toggle_t **out_toggle);

/**
 * @brief Destroy a Toggle Control
 */
int cmp_toggle_destroy(cmp_toggle_t *toggle);

/**
 * @brief Set toggle state (ON / OFF)
 */
int cmp_toggle_set_state(cmp_toggle_t *toggle, int is_on);

/**
 * @brief Handle an event targeted at the toggle switch (e.g. click to flip
 * state)
 */
int cmp_toggle_handle_event(cmp_toggle_t *toggle, const cmp_event_t *event);

/**
 * @brief Tick the toggle switch's internal state machine
 */
int cmp_toggle_update(cmp_toggle_t *toggle, float dt_ms);

/**
 * @brief Retrieve current toggle state
 */
int cmp_toggle_get_state(const cmp_toggle_t *toggle, int *out_is_on);

/* Phase 6.2: Menus & Actions (Apple HIG specific) */

/**
 * @brief Menu Item Roles
 */
typedef enum cmp_menu_item_role {
  CMP_MENU_ITEM_ROLE_STANDARD = 0,
  CMP_MENU_ITEM_ROLE_DESTRUCTIVE, /* Rendered strictly in system red */
  CMP_MENU_ITEM_ROLE_CANCEL       /* Distinct separation in action sheets */
} cmp_menu_item_role_t;

/**
 * @brief Menu Item Checkmark States
 */
typedef enum cmp_menu_item_state {
  CMP_MENU_ITEM_STATE_OFF = 0,
  CMP_MENU_ITEM_STATE_ON,
  CMP_MENU_ITEM_STATE_MIXED
} cmp_menu_item_state_t;

/**
 * @brief Menu Presentation Style
 */
typedef enum cmp_menu_presentation {
  CMP_MENU_PRESENTATION_CONTEXT =
      0, /* Long-press, heavily blurred bg, lifts target */
  CMP_MENU_PRESENTATION_ACTION_SHEET, /* Slide up from bottom */
  CMP_MENU_PRESENTATION_POPOVER /* Floating panel with arrow pointing to origin
                                 */
} cmp_menu_presentation_t;

/**
 * @brief Opaque Menu Controller
 */
typedef struct cmp_menu cmp_menu_t;

/**
 * @brief Opaque Menu Item
 */
typedef struct cmp_menu_item cmp_menu_item_t;

/**
 * @brief Create a Menu Controller
 */
int cmp_menu_create(cmp_menu_t **out_menu,
                    cmp_menu_presentation_t presentation);

/**
 * @brief Destroy a Menu Controller
 */
int cmp_menu_destroy(cmp_menu_t *menu);

/**
 * @brief Create a generic menu item
 */
int cmp_menu_item_create(cmp_menu_item_t **out_item, const char *title,
                         const char *symbol, cmp_menu_item_role_t role);

/**
 * @brief Add an item to a menu
 */
int cmp_menu_add_item(cmp_menu_t *menu, cmp_menu_item_t *item);

/**
 * @brief Create a submenu item (infinitely nested)
 */
int cmp_menu_item_create_submenu(cmp_menu_item_t **out_item, const char *title,
                                 const char *symbol, cmp_menu_t *child_menu);

/**
 * @brief Create a deferred menu item (shows loading spinner while async
 * fetches)
 */
int cmp_menu_item_create_deferred(cmp_menu_item_t **out_item,
                                  const char *title);

/**
 * @brief Set the checkmark state for a menu item
 */
int cmp_menu_item_set_state(cmp_menu_item_t *item, cmp_menu_item_state_t state);

/**
 * @brief Get presentation specifics for a menu
 * @param out_bg_blur Returns 1 if background should be heavily blurred
 * (Context)
 * @param out_lift_target Returns 1 if the originating element should be
 * visually lifted (Context)
 * @param out_has_arrow Returns 1 if a popover arrow should be drawn
 */
int cmp_menu_resolve_presentation(cmp_menu_t *menu, int *out_bg_blur,
                                  int *out_lift_target, int *out_has_arrow);

/**
 * @brief Resolves color and state icon for a menu item
 * @param out_is_red Returns 1 if text should be system red (Destructive)
 * @param out_state_icon Returns specific icon for ON/MIXED states (e.g.
 * "checkmark", "minus")
 */
int cmp_menu_item_resolve_visuals(cmp_menu_item_t *item, int *out_is_red,
                                  const char **out_state_icon);

/* Phase 6.3: Inputs & Selectors (Apple HIG specific) */

/**
 * @brief Opaque Wheel Picker (Slot machine style)
 */
typedef struct cmp_wheel_picker cmp_wheel_picker_t;

/**
 * @brief Opaque Segmented Control
 */
typedef struct cmp_segmented_control cmp_segmented_control_t;

/**
 * @brief Opaque Stepper (+/- buttons)
 */
typedef struct cmp_stepper cmp_stepper_t;

/**
 * @brief Opaque Slider (Continuous track)
 */
typedef struct cmp_slider cmp_slider_t;

/**
 * @brief Create a Wheel Picker
 */
int cmp_wheel_picker_create(cmp_wheel_picker_t **out_picker);
int cmp_wheel_picker_destroy(cmp_wheel_picker_t *picker);
int cmp_wheel_picker_set_items(cmp_wheel_picker_t *picker, const char **items,
                               size_t count);
int cmp_wheel_picker_scroll(cmp_wheel_picker_t *picker, float delta_y);
int cmp_wheel_picker_get_selected(cmp_wheel_picker_t *picker,
                                  size_t *out_index);

/**
 * @brief Create a Segmented Control
 */
int cmp_segmented_control_create(cmp_segmented_control_t **out_control);
int cmp_segmented_control_destroy(cmp_segmented_control_t *control);
int cmp_segmented_control_set_segments(cmp_segmented_control_t *control,
                                       const char **segments, size_t count);
int cmp_segmented_control_select(cmp_segmented_control_t *control,
                                 size_t index);
int cmp_segmented_control_get_visuals(cmp_segmented_control_t *control,
                                      size_t *out_selected_idx,
                                      float *out_slider_offset_x);

/**
 * @brief Create a Stepper (+/-)
 */
int cmp_stepper_create(cmp_stepper_t **out_stepper);
int cmp_stepper_destroy(cmp_stepper_t *stepper);
int cmp_stepper_set_limits(cmp_stepper_t *stepper, int min_val, int max_val,
                           int step);
int cmp_stepper_get_value(cmp_stepper_t *stepper, int *out_val);
int cmp_stepper_increment(cmp_stepper_t *stepper);
int cmp_stepper_decrement(cmp_stepper_t *stepper);

/**
 * @brief Create a Slider
 */
int cmp_slider_create(cmp_slider_t **out_slider);
int cmp_slider_destroy(cmp_slider_t *slider);
int cmp_slider_set_limits(cmp_slider_t *slider, float min_val, float max_val);
int cmp_slider_set_value(cmp_slider_t *slider, float val);
int cmp_slider_get_visuals(cmp_slider_t *slider, float *out_thumb_x_percent);

/**
 * @brief Launch OS-native Color Picker
 */
int cmp_system_color_picker_show(cmp_window_t *window);

/**
 * @brief Launch OS-native Date Picker (Compact style)
 */
int cmp_system_date_picker_show(cmp_window_t *window);

/* Phase 6.4: Text Fields & Editors (Apple HIG specific) */

/**
 * @brief Software Keyboard Layout Types
 */
typedef enum cmp_keyboard_type {
  CMP_KEYBOARD_TYPE_DEFAULT = 0,
  CMP_KEYBOARD_TYPE_EMAIL_ADDRESS,
  CMP_KEYBOARD_TYPE_URL,
  CMP_KEYBOARD_TYPE_NUMBER_PAD,
  CMP_KEYBOARD_TYPE_DECIMAL_PAD,
  CMP_KEYBOARD_TYPE_PHONE_PAD
} cmp_keyboard_type_t;

/**
 * @brief Software Keyboard Return Key Types
 */
typedef enum cmp_return_key_type {
  CMP_RETURN_KEY_DEFAULT = 0,
  CMP_RETURN_KEY_GO,
  CMP_RETURN_KEY_SEARCH,
  CMP_RETURN_KEY_NEXT,
  CMP_RETURN_KEY_DONE,
  CMP_RETURN_KEY_SEND
} cmp_return_key_type_t;

/**
 * @brief Auto-Capitalization Behaviors
 */
typedef enum cmp_auto_capitalization {
  CMP_AUTO_CAPITALIZATION_NONE = 0,
  CMP_AUTO_CAPITALIZATION_WORDS,
  CMP_AUTO_CAPITALIZATION_SENTENCES,
  CMP_AUTO_CAPITALIZATION_ALL_CHARACTERS
} cmp_auto_capitalization_t;

/**
 * @brief Opaque Text Field Control
 */
typedef struct cmp_text_field cmp_text_field_t;

/**
 * @brief Create a Text Field
 */
int cmp_text_field_create(cmp_text_field_t **out_field);
int cmp_text_field_destroy(cmp_text_field_t *field);

/**
 * @brief Configure OS keyboard hints
 */
int cmp_text_field_set_keyboard_type(cmp_text_field_t *field,
                                     cmp_keyboard_type_t type);
int cmp_text_field_set_return_key_type(cmp_text_field_t *field,
                                       cmp_return_key_type_t type);
int cmp_text_field_set_secure_text_entry(cmp_text_field_t *field, int secure);
int cmp_text_field_set_auto_capitalization(cmp_text_field_t *field,
                                           cmp_auto_capitalization_t cap);
int cmp_text_field_set_spellcheck_enabled(cmp_text_field_t *field, int enabled);

/**
 * @brief Mount an input accessory view (custom toolbar above the soft keyboard)
 */
int cmp_text_field_set_input_accessory_view(cmp_text_field_t *field,
                                            cmp_ui_node_t *accessory_node);

/**
 * @brief Insert text at the current caret position.
 * @param text UTF-8 string to insert.
 */
int cmp_text_field_insert_text(cmp_text_field_t *field, const char *text);

/**
 * @brief Delete one UTF-8 character before the caret.
 */
int cmp_text_field_delete_backward(cmp_text_field_t *field);

/**
 * @brief Set the caret position (byte offset).
 */
int cmp_text_field_set_caret_position(cmp_text_field_t *field, size_t pos);

/**
 * @brief Get the caret position.
 */
int cmp_text_field_get_caret_position(const cmp_text_field_t *field,
                                      size_t *out_pos);

/**
 * @brief Get the current text buffer.
 */
int cmp_text_field_get_text(const cmp_text_field_t *field,
                            const char **out_text);

/**
 * @brief Handle a keyboard or pointer event for text editing.
 */
int cmp_text_field_handle_event(cmp_text_field_t *field,
                                const cmp_event_t *event);

/**
 * @brief Data Detector Types (for Rich Text Views)
 */
typedef enum cmp_data_detector_types {
  CMP_DATA_DETECTOR_NONE = 0,
  CMP_DATA_DETECTOR_PHONE_NUMBER = 1 << 0,
  CMP_DATA_DETECTOR_LINK = 1 << 1,
  CMP_DATA_DETECTOR_ADDRESS = 1 << 2,
  CMP_DATA_DETECTOR_CALENDAR_EVENT = 1 << 3,
  CMP_DATA_DETECTOR_ALL = 0xFFFFFFFF
} cmp_data_detector_types_t;

/**
 * @brief Opaque Rich Text View
 */
typedef struct cmp_rich_text_view cmp_rich_text_view_t;

int cmp_rich_text_view_create(cmp_rich_text_view_t **out_view);
int cmp_rich_text_view_destroy(cmp_rich_text_view_t *view);
int cmp_rich_text_view_set_data_detectors(cmp_rich_text_view_t *view,
                                          cmp_data_detector_types_t flags);

/* Phase 7.1: Lists & Tables (Apple HIG specific) */

/**
 * @brief List Collection Styles
 */
typedef enum cmp_list_style {
  CMP_LIST_STYLE_PLAIN = 0,    /* Edge-to-edge separators */
  CMP_LIST_STYLE_GROUPED,      /* Blocks of rows with section headers */
  CMP_LIST_STYLE_INSET_GROUPED /* iOS 15+ rounded blocks with margins */
} cmp_list_style_t;

/**
 * @brief Swipe Action Execution Styles
 */
typedef enum cmp_swipe_action_style {
  CMP_SWIPE_ACTION_STYLE_NORMAL = 0,
  CMP_SWIPE_ACTION_STYLE_DESTRUCTIVE
} cmp_swipe_action_style_t;

/**
 * @brief Opaque List Controller
 */
typedef struct cmp_list cmp_list_t;

/**
 * @brief Opaque List Row
 */
typedef struct cmp_list_row cmp_list_row_t;

/**
 * @brief Create a List Controller
 */
int cmp_list_create(cmp_list_t **out_list, cmp_list_style_t style);

/**
 * @brief Destroy a List Controller
 */
int cmp_list_destroy(cmp_list_t *list);

/**
 * @brief Add a basic row to the list
 */
int cmp_list_add_row(cmp_list_t *list, cmp_list_row_t *row);

/**
 * @brief Create a list row
 */
int cmp_list_row_create(cmp_list_row_t **out_row, const char *title);

/**
 * @brief Destroy a list row
 */
int cmp_list_row_destroy(cmp_list_row_t *row);

/**
 * @brief Configure separator insets
 * @param inset_left Sets how far the separator indents to align with text,
 * bypassing icons.
 */
int cmp_list_row_set_separator_inset(cmp_list_row_t *row, float inset_left);

/**
 * @brief Register a contextual swipe action
 * @param is_leading 1 for swiping right-to-left (leading), 0 for left-to-right
 * (trailing)
 * @param allows_continuous 1 to allow execution via deep swipe
 */
int cmp_list_row_add_swipe_action(cmp_list_row_t *row, int is_leading,
                                  const char *title,
                                  cmp_swipe_action_style_t style,
                                  int allows_continuous);

/**
 * @brief Enable edit mode on the list (reveals delete minus signs and reorder
 * grabbers)
 */
int cmp_list_set_edit_mode(cmp_list_t *list, int is_editing);

/**
 * @brief Evaluates visual metrics based on list style and edit mode
 * @param out_margin_horizontal Left/Right margins (0.0 for plain, ~16.0 for
 * inset grouped)
 * @param out_corner_radius Row clipping radius (0.0 for plain, ~10.0 for inset
 * grouped ends)
 * @param out_content_offset_x X offset for row content to make room for
 * edit/delete buttons
 */
int cmp_list_resolve_metrics(cmp_list_t *list, float *out_margin_horizontal,
                             float *out_corner_radius,
                             float *out_content_offset_x);

/* Phase 7.2: Scroll Views (Apple HIG specific) */

/**
 * @brief Scroll Deceleration Rates
 */
typedef enum cmp_scroll_deceleration_rate {
  CMP_SCROLL_DECELERATION_NORMAL = 0, /* Standard list scrolling */
  CMP_SCROLL_DECELERATION_FAST        /* Paging or gallery snapping */
} cmp_scroll_deceleration_rate_t;

/**
 * @brief Keyboard Dismissal Modes
 */
typedef enum cmp_scroll_keyboard_dismiss {
  CMP_SCROLL_KEYBOARD_DISMISS_NONE = 0,
  CMP_SCROLL_KEYBOARD_DISMISS_ON_DRAG, /* Dismiss the moment a touch drag begins
                                        */
  CMP_SCROLL_KEYBOARD_DISMISS_INTERACTIVE /* Push the keyboard down
                                             interactively as the user scrolls
                                             down */
} cmp_scroll_keyboard_dismiss_t;

/**
 * @brief Opaque Scroll View Extension Context
 */
typedef struct cmp_scroll_view cmp_scroll_view_t;

/**
 * @brief Create a Scroll View Extension
 */
int cmp_scroll_view_create(cmp_scroll_view_t **out_scroll_view);

/**
 * @brief Destroy a Scroll View Extension
 */
int cmp_scroll_view_destroy(cmp_scroll_view_t *scroll_view);

/**
 * @brief Sets the deceleration rate multiplier
 */
int cmp_scroll_view_set_deceleration_rate(cmp_scroll_view_t *scroll_view,
                                          cmp_scroll_deceleration_rate_t rate);

/**
 * @brief Enables pagination mode (snapping to view bounds)
 */
int cmp_scroll_view_set_paging_enabled(cmp_scroll_view_t *scroll_view,
                                       int is_paging_enabled);

/**
 * @brief Evaluates whether the scrollbar should be actively rendering based on
 * state
 * @param is_scrolling Current active scrolling state from touch/wheel
 * @param time_since_last_scroll Milliseconds elapsed since the scroll stopped
 * @param out_opacity Computed opacity of the scrollbar (1.0 during scroll,
 * fading to 0.0)
 */
int cmp_scroll_view_evaluate_scrollbar(cmp_scroll_view_t *scroll_view,
                                       int is_scrolling,
                                       float time_since_last_scroll,
                                       float *out_opacity);

/**
 * @brief Checks if a fast drag over the scroll indicator area should transition
 * into interactive fast-scrolling
 * @param pointer_x Current touch X
 * @param pointer_y Current touch Y
 * @param view_width Total width of the scroll view
 * @param out_is_interactive Populated with 1 if the scrollbar is now captured
 * by the user
 */
int cmp_scroll_view_hit_test_scrollbar(cmp_scroll_view_t *scroll_view,
                                       float pointer_x, float pointer_y,
                                       float view_width,
                                       int *out_is_interactive);

/**
 * @brief Evaluates Apple's specific friction formula when over-scrolling past
 * content bounds (Rubber-Banding)
 * @param overscroll_delta The raw physical translation attempted past the bound
 * @param view_dimension The total size of the viewport (used as the scaling
 * denominator)
 * @param out_visual_translation The actual translated pixel amount to render (a
 * logarithmic curve)
 */
int cmp_scroll_view_calculate_rubber_band(cmp_scroll_view_t *scroll_view,
                                          float overscroll_delta,
                                          float view_dimension,
                                          float *out_visual_translation);

/**
 * @brief Configure keyboard dismissal behaviors on scroll
 */
int cmp_scroll_view_set_keyboard_dismiss_mode(
    cmp_scroll_view_t *scroll_view, cmp_scroll_keyboard_dismiss_t mode);

/* Phase 7.3: Collections, Grids & Complex Layouts (Apple HIG specific) */

/**
 * @brief Opaque Collection View Controller
 */
typedef struct cmp_collection cmp_collection_t;

/**
 * @brief Opaque Compositional Layout Section
 */
typedef struct cmp_collection_section cmp_collection_section_t;

/**
 * @brief Opaque Diffable Data Source
 */
typedef struct cmp_diffable_datasource cmp_diffable_datasource_t;

/**
 * @brief Orthogonal Scrolling Behaviors
 */
typedef enum cmp_orthogonal_behavior {
  CMP_ORTHOGONAL_NONE = 0,
  CMP_ORTHOGONAL_CONTINUOUS,               /* Standard horizontal scrolling */
  CMP_ORTHOGONAL_CONTINUOUS_GROUP_LEADING, /* Snaps to leading edge of groups */
  CMP_ORTHOGONAL_PAGING                    /* Page-by-page snapping */
} cmp_orthogonal_behavior_t;

/**
 * @brief Create a Collection View Controller
 */
int cmp_collection_create(cmp_collection_t **out_collection);

/**
 * @brief Destroy a Collection View Controller
 */
int cmp_collection_destroy(cmp_collection_t *collection);

/**
 * @brief Create a Compositional Layout Section
 */
int cmp_collection_section_create(cmp_collection_section_t **out_section);

/**
 * @brief Destroy a Compositional Layout Section
 */
int cmp_collection_section_destroy(cmp_collection_section_t *section);

/**
 * @brief Configure an adaptive Flow Layout that reflows columns dynamically
 * @param min_column_width The absolute minimum width an item can be before
 * reflowing into a new row
 */
int cmp_collection_section_set_flow_layout(cmp_collection_section_t *section,
                                           float min_column_width);

/**
 * @brief Configure Orthogonal (perpendicular) scrolling for this specific
 * section (e.g. an App Store carousel)
 */
int cmp_collection_section_set_orthogonal_behavior(
    cmp_collection_section_t *section, cmp_orthogonal_behavior_t behavior);

/**
 * @brief Add a section to the main Collection View layout
 */
int cmp_collection_add_section(cmp_collection_t *collection,
                               cmp_collection_section_t *section);

/**
 * @brief Create a Diffable Data Source
 */
int cmp_diffable_datasource_create(cmp_diffable_datasource_t **out_ds);

/**
 * @brief Destroy a Diffable Data Source
 */
int cmp_diffable_datasource_destroy(cmp_diffable_datasource_t *ds);

/**
 * @brief Apply a new state snapshot to the Diffable Data Source
 * In reality, this calculates insertions, deletions, and moves based on hash
 * hashes.
 * @param items Array of arbitrary 64-bit identifier hashes representing the new
 * state
 */
int cmp_diffable_datasource_apply_snapshot(cmp_diffable_datasource_t *ds,
                                           const uint64_t *items, size_t count);

/**
 * @brief System UI Views
 * Mounts OS-native abstractions seamlessly into the layout tree
 */
int cmp_system_map_view_mount(cmp_ui_node_t *node, float latitude,
                              float longitude);
int cmp_system_web_view_mount(cmp_ui_node_t *node, const char *url);

/* Phase 8.1: Touch & Multi-Touch Gestures (Apple HIG specific) */

/**
 * @brief Gesture Disambiguation Conflict Resolution
 * When two gesture recognizers recognize simultaneously, decide which one
 * fails. E.g., fail a pan-horizontal if a pan-vertical is already tracking.
 */
int cmp_gesture_require_failure(cmp_gesture_t *primary_gesture,
                                cmp_gesture_t *other_gesture_to_fail);

/**
 * @brief Configure pinch-to-zoom bounds
 */
int cmp_complex_gesture_set_zoom_limits(cmp_complex_gesture_t *gesture,
                                        float min_scale, float max_scale);

/**
 * @brief Retrieve pinch-to-zoom centroid and scaled value
 * @param out_centroid_x Output center X between fingers
 * @param out_centroid_y Output center Y between fingers
 * @param out_scale The calculated scale multiplier, incorporating
 * rubber-banding if past limits
 */
int cmp_complex_gesture_get_zoom(const cmp_complex_gesture_t *gesture,
                                 float *out_centroid_x, float *out_centroid_y,
                                 float *out_scale);

/**
 * @brief Configure rotation snapping (e.g. snapping to 0, 90, 180 degrees)
 */
int cmp_complex_gesture_set_rotation_snapping(cmp_complex_gesture_t *gesture,
                                              int enable_snapping);

/**
 * @brief Retrieve rotation angle
 */
int cmp_complex_gesture_get_rotation(const cmp_complex_gesture_t *gesture,
                                     float *out_radians);

/**
 * @brief Screen Edge Swipe Back (Navigation Pop)
 * Hooks into the routing engine to tie pop progress to finger position
 */
typedef struct cmp_edge_swipe cmp_edge_swipe_t;
int cmp_edge_swipe_create(cmp_edge_swipe_t **out_swipe, cmp_router_t *router);
int cmp_edge_swipe_destroy(cmp_edge_swipe_t *swipe);
int cmp_edge_swipe_process(cmp_edge_swipe_t *swipe, float touch_x,
                           float screen_width, cmp_gesture_state_t state);

/**
 * @brief Multi-Touch Override Safe-Guard
 * Cancels active UI gestures if system gestures (3, 4, 5 fingers) are detected.
 */
int cmp_gesture_cancel_on_system_override(cmp_gesture_t *gesture,
                                          int active_touches_count);

/* Phase 8.2: Keyboard & Trackpad (Apple HIG specific) */

/**
 * @brief iPadOS Pointer Interaction Styles
 */
typedef enum cmp_pointer_interaction_style {
  CMP_POINTER_INTERACTION_AUTOMATIC = 0,
  CMP_POINTER_INTERACTION_LIFT, /* Pointer morphs into object, lifts it visually
                                 */
  CMP_POINTER_INTERACTION_HIGHLIGHT, /* Pointer shrinks, object background
                                        highlights */
  CMP_POINTER_INTERACTION_HOVER /* Pointer retains shape but scales up/down */
} cmp_pointer_interaction_style_t;

/**
 * @brief Represents a system keyboard shortcut (e.g. Cmd-C, Shift-Tab)
 */
typedef struct cmp_keyboard_shortcut cmp_keyboard_shortcut_t;

/**
 * @brief Opaque UI Command (maps to UIMenu/UICommand/macOS menu bar)
 */
typedef struct cmp_ui_command cmp_ui_command_t;

/**
 * @brief Pointer Region Definition
 */
typedef struct cmp_pointer_region cmp_pointer_region_t;

int cmp_pointer_region_create(cmp_pointer_region_t **out_region);
int cmp_pointer_region_destroy(cmp_pointer_region_t *region);
int cmp_pointer_region_set_style(cmp_pointer_region_t *region,
                                 cmp_pointer_interaction_style_t style);

/**
 * @brief Retrieve computed morphing scales based on interaction style
 * @param out_scale_x Populated with the scale factor for the pointer/element
 */
int cmp_pointer_region_get_morph_scale(cmp_pointer_region_t *region,
                                       float *out_scale_x, float *out_scale_y);

/**
 * @brief Create a Keyboard Shortcut definition
 * @param key The base character (e.g., 'c', 'v')
 * @param modifier_flags Bitmask of modifiers (e.g., Command | Shift)
 */
int cmp_keyboard_shortcut_create(cmp_keyboard_shortcut_t **out_shortcut,
                                 char key, uint32_t modifier_flags);
int cmp_keyboard_shortcut_destroy(cmp_keyboard_shortcut_t *shortcut);

/**
 * @brief Create a UI Command node
 * @param title The human-readable string displayed in the menu
 * @param action_id A unique string identifier routed through the event bus
 */
int cmp_ui_command_create(cmp_ui_command_t **out_command, const char *title,
                          const char *action_id);
int cmp_ui_command_destroy(cmp_ui_command_t *command);

/**
 * @brief Bind a shortcut to a UI Command
 */
int cmp_ui_command_set_shortcut(cmp_ui_command_t *command,
                                cmp_keyboard_shortcut_t *shortcut);

/**
 * @brief Accelerating Key Repeat Logic
 * @param time_held_ms Milliseconds the key has been held down
 * @param out_repeat_interval_ms The calculated delay before firing the next
 * synthetic keypress event
 */
int cmp_keyboard_calculate_key_repeat(float time_held_ms,
                                      float *out_repeat_interval_ms);

/**
 * @brief Trackpad Specific Gestures
 * Natively supports two-finger semantic intent translations
 */
int cmp_trackpad_evaluate_gesture(float delta_x, float delta_y,
                                  float *out_pan_x, float *out_pan_y);

/* Phase 8.3: Apple Pencil & Stylus (Apple HIG specific) */

/**
 * @brief Stylus Hardware Interaction Events
 */
typedef enum cmp_stylus_event_type {
  CMP_STYLUS_EVENT_DOUBLE_TAP = 0, /* Pencil Gen 2 */
  CMP_STYLUS_EVENT_SQUEEZE         /* Pencil Pro */
} cmp_stylus_event_type_t;

/**
 * @brief Opaque Stylus Interaction Context
 */
typedef struct cmp_stylus_context cmp_stylus_context_t;

int cmp_stylus_context_create(cmp_stylus_context_t **out_context);
int cmp_stylus_context_destroy(cmp_stylus_context_t *context);

/**
 * @brief Register a callback for hardware button/barrel events
 */
typedef void (*cmp_stylus_event_cb)(cmp_stylus_event_type_t event_type,
                                    void *userdata);
int cmp_stylus_context_set_event_callback(cmp_stylus_context_t *context,
                                          cmp_stylus_event_cb callback,
                                          void *userdata);

/**
 * @brief Evaluates stylus metrics for rendering digital ink (pressure,
 * altitude, azimuth)
 * @param event The generic pointer event containing raw OS metrics
 * @param out_brush_opacity Populated based on pressure [0.0 - 1.0]
 * @param out_brush_width Populated based on altitude/tilt (e.g. flat pencil
 * draws wider strokes)
 */
int cmp_stylus_resolve_ink_metrics(const cmp_event_t *event, float base_width,
                                   float *out_brush_opacity,
                                   float *out_brush_width);

/**
 * @brief Evaluates Hover State (iPad Pro M2+)
 * @param event The pointer event
 * @param out_is_hovering 1 if the pencil is detected above the screen but not
 * touching
 * @param out_distance Distance from screen (normalized 0.0 to 1.0 if supported
 * by OS, else just binary)
 */
int cmp_stylus_evaluate_hover(const cmp_event_t *event, int *out_is_hovering,
                              float *out_distance);

/**
 * @brief Enable global Scribble (Handwriting to text) routing for standard text
 * inputs
 */
int cmp_stylus_set_scribble_enabled(int is_enabled);

/* Phase 8.4: Drag and Drop (Apple HIG specific extensions) */

/**
 * @brief Evaluates visual metrics during a drag initiation
 * @param progress 0.0 (at rest) to 1.0 (fully lifted state)
 * @param out_scale Returns the visual multiplier (e.g. 1.05 for lifted)
 * @param out_shadow_opacity Returns the intensity of the drop shadow
 */
int cmp_dnd_evaluate_lift_animation(float progress, float *out_scale,
                                    float *out_shadow_opacity);

/**
 * @brief Adds an additional item to the current drag session, grouping them
 * into a visual stack
 */
int cmp_dnd_add_item_to_stack(cmp_dnd_t *dnd,
                              const char *additional_payload_text);

/**
 * @brief Returns the total number of items currently grouped in this drag
 * session
 */
int cmp_dnd_get_stack_count(const cmp_dnd_t *dnd, size_t *out_count);

/**
 * @brief Evaluates the visual highlight for a valid drop target
 * @param is_hovered 1 if the drag pointer is currently inside this target
 * bounds
 * @param out_expansion_scale Returns > 1.0f if the target should slightly
 * expand
 * @param out_bg_overlay_opacity Returns the opacity for a selection highlight
 * overlay
 */
int cmp_dnd_evaluate_drop_target_highlight(int is_hovered,
                                           float *out_expansion_scale,
                                           float *out_bg_overlay_opacity);

/**
 * @brief Manages "Spring Loading" (Hovering to open)
 * @param hover_duration_ms How long the drag has been hovering over the target
 * @param out_should_trigger Returns 1 if the threshold has been passed and the
 * folder/link should open
 */
int cmp_dnd_evaluate_spring_loading(float hover_duration_ms,
                                    int *out_should_trigger);

/* Phase 8.5: Game Controllers (Apple HIG specific extensions) */

/**
 * @brief Game Controller D-Pad/Thumbstick to Focus Mapping
 * Reads current gamepad state and emits simulated arrow key events to the event
 * loop if thresholds are crossed, allowing full UI navigation via controller.
 * @param gamepad The polled gamepad state
 * @param dt_ms Time elapsed to handle synthetic repeat
 */
int cmp_gamepad_evaluate_focus_navigation(const cmp_gamepad_t *gamepad,
                                          float dt_ms);

/**
 * @brief Haptic Rumble Waveforms
 */
typedef enum cmp_gamepad_rumble_type {
  CMP_GAMEPAD_RUMBLE_LIGHT = 0,
  CMP_GAMEPAD_RUMBLE_MEDIUM,
  CMP_GAMEPAD_RUMBLE_HEAVY,
  CMP_GAMEPAD_RUMBLE_SUCCESS,
  CMP_GAMEPAD_RUMBLE_ERROR
} cmp_gamepad_rumble_type_t;

/**
 * @brief Triggers a predefined rumble waveform on the controller
 * @param index The controller index
 * @param type The waveform to trigger
 */
int cmp_gamepad_trigger_rumble(int index, cmp_gamepad_rumble_type_t type);

/**
 * @brief Adaptive Trigger Resistance (DualSense mapping)
 */
typedef struct cmp_adaptive_trigger_config {
  int is_active;
  float start_position; /* 0.0 to 1.0 */
  float resistance;     /* 0.0 to 1.0 */
} cmp_adaptive_trigger_config_t;

/**
 * @brief Configure adaptive trigger resistance for L2/R2
 * @param index The controller index
 * @param is_left 1 for L2, 0 for R2
 * @param config The resistance configuration
 */
int cmp_gamepad_set_adaptive_trigger(
    int index, int is_left, const cmp_adaptive_trigger_config_t *config);

/* Phase 9.1: App Extensions & Capabilities (Apple HIG specific) */

/**
 * @brief Widget Family Sizes (maps to WidgetFamily on Apple platforms)
 */
typedef enum cmp_widget_family {
  CMP_WIDGET_FAMILY_SYSTEM_SMALL = 0,
  CMP_WIDGET_FAMILY_SYSTEM_MEDIUM,
  CMP_WIDGET_FAMILY_SYSTEM_LARGE,
  CMP_WIDGET_FAMILY_SYSTEM_EXTRA_LARGE,
  CMP_WIDGET_FAMILY_ACCESSORY_CIRCULAR, /* Lock Screen / Watch */
  CMP_WIDGET_FAMILY_ACCESSORY_RECTANGULAR,
  CMP_WIDGET_FAMILY_ACCESSORY_INLINE
} cmp_widget_family_t;

/**
 * @brief Live Activity Presentation Contexts (Dynamic Island)
 */
typedef enum cmp_live_activity_presentation {
  CMP_LIVE_ACTIVITY_PRESENTATION_LOCK_SCREEN = 0,
  CMP_LIVE_ACTIVITY_PRESENTATION_COMPACT_LEADING, /* Dynamic Island */
  CMP_LIVE_ACTIVITY_PRESENTATION_COMPACT_TRAILING,
  CMP_LIVE_ACTIVITY_PRESENTATION_MINIMAL,
  CMP_LIVE_ACTIVITY_PRESENTATION_EXPANDED
} cmp_live_activity_presentation_t;

/**
 * @brief Opaque Widget Context
 */
typedef struct cmp_widget_ctx cmp_widget_ctx_t;

/**
 * @brief Opaque Live Activity Context
 */
typedef struct cmp_live_activity_ctx cmp_live_activity_ctx_t;

/**
 * @brief Initialize a constrained Widget Context (enforces memory/CPU limits)
 */
int cmp_widget_ctx_create(cmp_widget_ctx_t **out_ctx,
                          cmp_widget_family_t family);
int cmp_widget_ctx_destroy(cmp_widget_ctx_t *ctx);

/**
 * @brief Mount a static snapshot layout into the Widget Context
 */
int cmp_widget_mount_snapshot(cmp_widget_ctx_t *ctx, cmp_ui_node_t *root_node);

/**
 * @brief Configure Interactive Widget intent mapping (iOS 17+)
 * App Intents map to specific button clicks on a widget, executing in the
 * background.
 */
int cmp_widget_bind_intent(cmp_widget_ctx_t *ctx, cmp_ui_node_t *button_node,
                           const char *intent_identifier);

/**
 * @brief Create a Live Activity (Dynamic Island)
 */
int cmp_live_activity_ctx_create(cmp_live_activity_ctx_t **out_ctx);
int cmp_live_activity_ctx_destroy(cmp_live_activity_ctx_t *ctx);

/**
 * @brief Mount a specific layout for a Dynamic Island presentation state
 */
int cmp_live_activity_mount_presentation(
    cmp_live_activity_ctx_t *ctx, cmp_live_activity_presentation_t presentation,
    cmp_ui_node_t *node);

/**
 * @brief Utility to verify the framework executable size fits App Clip/App
 * Extension limits
 * @param out_is_compliant Returns 1 if total footprint is under the 10MB limit
 */
int cmp_extension_verify_footprint(int *out_is_compliant);

/* Phase 9.2: OS Communications (Apple HIG specific) */

/**
 * @brief Opaque System Communication Context
 */
typedef struct cmp_os_communications cmp_os_communications_t;

int cmp_os_communications_create(cmp_os_communications_t **out_ctx);
int cmp_os_communications_destroy(cmp_os_communications_t *ctx);

/**
 * @brief Siri & App Intents Registration
 * @param intent_id Unique identifier matching the INIntent definition
 * @param title Human readable title for the Shortcuts App
 */
int cmp_os_communications_register_intent(cmp_os_communications_t *ctx,
                                          const char *intent_id,
                                          const char *title);

/**
 * @brief Show System Share Sheet (UIActivityViewController equivalent)
 */
int cmp_os_communications_show_share_sheet(cmp_os_communications_t *ctx,
                                           cmp_window_t *window,
                                           const char *url_to_share,
                                           const char *text_to_share);

/**
 * @brief Broadcast current activity for Handoff (Continuity)
 * @param activity_type Uniquely identifies the task type
 * @param payload Arbitrary string (usually JSON/URI) representing state
 */
int cmp_os_communications_broadcast_handoff(cmp_os_communications_t *ctx,
                                            const char *activity_type,
                                            const char *payload);

/**
 * @brief Index content directly into the OS Search (CoreSpotlight)
 */
int cmp_os_communications_index_for_spotlight(cmp_os_communications_t *ctx,
                                              const char *item_id,
                                              const char *title,
                                              const char *description);

/**
 * @brief Check if the current OS Focus Mode should suppress non-urgent UI
 * updates (e.g. Work or Sleep modes active)
 * @param out_is_suppressed Populated with 1 if focus mode is active and
 * restrictive
 */
int cmp_os_communications_evaluate_focus_mode(cmp_os_communications_t *ctx,
                                              int *out_is_suppressed);

/**
 * @brief Initialize a SharePlay (GroupActivities) session
 */
int cmp_os_communications_start_shareplay(cmp_os_communications_t *ctx,
                                          const char *activity_id);

/* Phase 9.3: Media, Audio & AVKit (Apple HIG specific) */

/**
 * @brief Audio Session Categories (maps to AVAudioSessionCategory)
 */
typedef enum cmp_audio_session_category {
  CMP_AUDIO_SESSION_AMBIENT =
      0, /* Silenced by ringer switch, mixes with other audio */
  CMP_AUDIO_SESSION_SOLO_AMBIENT, /* Silenced by ringer switch, pauses other
                                     audio */
  CMP_AUDIO_SESSION_PLAYBACK,     /* Plays even if ringer switch is silent (e.g.
                                     music apps) */
  CMP_AUDIO_SESSION_RECORD,
  CMP_AUDIO_SESSION_PLAY_AND_RECORD
} cmp_audio_session_category_t;

/**
 * @brief Opaque Media Player Context (AVPlayerViewController equivalent)
 */
typedef struct cmp_media_player cmp_media_player_t;

/**
 * @brief Opaque Audio Session Context
 */
typedef struct cmp_audio_session cmp_audio_session_t;

int cmp_media_player_create(cmp_media_player_t **out_player);
int cmp_media_player_destroy(cmp_media_player_t *player);

/**
 * @brief Load a media URL
 */
int cmp_media_player_load_url(cmp_media_player_t *player, const char *url);

/**
 * @brief Mount the media player into the layout tree (uses OS native controls
 * if requested)
 */
int cmp_media_player_mount(cmp_media_player_t *player, cmp_ui_node_t *node,
                           int use_system_controls);

/**
 * @brief Set Picture-in-Picture capability
 */
int cmp_media_player_set_pip_enabled(cmp_media_player_t *player,
                                     int is_enabled);

/**
 * @brief Configure the OS Now Playing Info Center (Lock Screen / Watch
 * metadata)
 */
int cmp_media_player_update_now_playing(cmp_media_player_t *player,
                                        const char *title, const char *artist,
                                        float duration, float current_time);

/**
 * @brief Enable Spatial Audio processing capabilities
 */
int cmp_media_player_set_spatial_audio_enabled(cmp_media_player_t *player,
                                               int is_enabled);

/**
 * @brief Handle remote commands (headphones, lock screen buttons)
 */
typedef void (*cmp_remote_command_cb)(int command_id, void *userdata);
int cmp_media_player_set_remote_command_handler(cmp_media_player_t *player,
                                                cmp_remote_command_cb callback,
                                                void *userdata);

int cmp_audio_session_create(cmp_audio_session_t **out_session);
int cmp_audio_session_destroy(cmp_audio_session_t *session);

/**
 * @brief Set the global application audio session category
 */
int cmp_audio_session_set_category(cmp_audio_session_t *session,
                                   cmp_audio_session_category_t category);

/**
 * @brief Activate the audio session (may pause background music depending on
 * category)
 */
int cmp_audio_session_activate(cmp_audio_session_t *session);

/* Phase 9.4: Transactions, Files & Cloud (Apple HIG specific) */

/**
 * @brief Opaque Payment Context
 */
typedef struct cmp_payment_ctx cmp_payment_ctx_t;

/**
 * @brief Opaque StoreKit Context
 */
typedef struct cmp_storekit_ctx cmp_storekit_ctx_t;

/**
 * @brief Opaque Cloud Sync Context
 */
typedef struct cmp_cloud_sync_ctx cmp_cloud_sync_ctx_t;

/**
 * @brief Apple Pay Button Styles
 */
typedef enum cmp_apple_pay_button_style {
  CMP_APPLE_PAY_BUTTON_BLACK = 0,
  CMP_APPLE_PAY_BUTTON_WHITE,
  CMP_APPLE_PAY_BUTTON_WHITE_OUTLINE
} cmp_apple_pay_button_style_t;

/**
 * @brief Apple Pay Button Types (e.g. "Buy with", "Subscribe with")
 */
typedef enum cmp_apple_pay_button_type {
  CMP_APPLE_PAY_BUTTON_TYPE_PLAIN = 0,
  CMP_APPLE_PAY_BUTTON_TYPE_BUY,
  CMP_APPLE_PAY_BUTTON_TYPE_SUBSCRIBE
} cmp_apple_pay_button_type_t;

int cmp_payment_ctx_create(cmp_payment_ctx_t **out_ctx);
int cmp_payment_ctx_destroy(cmp_payment_ctx_t *ctx);

/**
 * @brief Mounts an unmodified PKPaymentButton to the UI tree (HIG requirement)
 */
int cmp_payment_mount_apple_pay_button(cmp_payment_ctx_t *ctx,
                                       cmp_ui_node_t *node,
                                       cmp_apple_pay_button_style_t style,
                                       cmp_apple_pay_button_type_t type);

/**
 * @brief Initiate an Apple Pay transaction
 */
int cmp_payment_request_apple_pay(cmp_payment_ctx_t *ctx,
                                  const char *merchant_identifier,
                                  const char *currency_code, float amount);

int cmp_storekit_ctx_create(cmp_storekit_ctx_t **out_ctx);
int cmp_storekit_ctx_destroy(cmp_storekit_ctx_t *ctx);

/**
 * @brief Triggers the native StoreKit purchase modal
 */
int cmp_storekit_purchase_product(cmp_storekit_ctx_t *ctx,
                                  const char *product_identifier);

/**
 * @brief Triggers the native StoreKit restore purchases flow
 */
int cmp_storekit_restore_purchases(cmp_storekit_ctx_t *ctx);

int cmp_cloud_sync_ctx_create(cmp_cloud_sync_ctx_t **out_ctx);
int cmp_cloud_sync_ctx_destroy(cmp_cloud_sync_ctx_t *ctx);

/**
 * @brief Syncs a small string to NSUbiquitousKeyValueStore
 */
int cmp_cloud_sync_set_key_value(cmp_cloud_sync_ctx_t *ctx, const char *key,
                                 const char *value);

/**
 * @brief Asynchronously syncs a large payload to CloudKit
 */
int cmp_cloud_sync_upload_record(cmp_cloud_sync_ctx_t *ctx,
                                 const char *record_type, const char *record_id,
                                 const char *json_payload);

/**
 * @brief Declare a file/folder to be exposed in the iOS Files App / macOS
 * Finder
 */
int cmp_cloud_sync_expose_to_files_app(cmp_cloud_sync_ctx_t *ctx,
                                       const char *relative_path);

/* Phase 10.1: Platform-Specific Master Checklist -> iOS (iPhone) */

/**
 * @brief Modal Sheet Detents (Snap Points)
 */
typedef enum cmp_sheet_detent {
  CMP_SHEET_DETENT_MEDIUM = 0, /* ~Half screen */
  CMP_SHEET_DETENT_LARGE,      /* Full screen */
  CMP_SHEET_DETENT_CUSTOM
} cmp_sheet_detent_t;

/**
 * @brief Opaque iOS-specific features controller
 */
typedef struct cmp_ios_features cmp_ios_features_t;

int cmp_ios_features_create(cmp_ios_features_t **out_features);
int cmp_ios_features_destroy(cmp_ios_features_t *features);

/**
 * @brief Calculate auto-scroll offset to avoid soft keyboard obscuring an
 * active input
 * @param keyboard_height The OS-reported soft keyboard height
 * @param input_y The absolute Y coordinate of the bottom of the focused text
 * input
 * @param view_height Total height of the screen
 * @param out_scroll_adjustment Populated with the required vertical scroll
 * offset delta
 */
int cmp_ios_calculate_keyboard_avoidance(float keyboard_height, float input_y,
                                         float view_height,
                                         float *out_scroll_adjustment);

/**
 * @brief Pull-to-Refresh mechanics
 * Evaluates continuous negative scrolling against a threshold to trigger a
 * native spinner
 * @param current_overscroll_y The raw negative rubber-band overscroll
 * @param out_spinner_opacity 0.0 to 1.0 based on pull depth
 * @param out_should_trigger Returns 1 if pull crossed the threshold and
 * released
 */
int cmp_ios_evaluate_pull_to_refresh(float current_overscroll_y,
                                     float *out_spinner_opacity,
                                     int *out_should_trigger);

/**
 * @brief Modal Sheet Detent Snapping
 * @param current_y Current swipe/drag position of the modal sheet
 * @param screen_height Total screen height
 * @param out_snapped_detent Populates with the closest detent based on Apple's
 * velocity/release heuristics
 */
int cmp_ios_evaluate_sheet_detent_snap(float current_y, float screen_height,
                                       cmp_sheet_detent_t *out_snapped_detent);

/**
 * @brief Reachability Layout Mitigation
 * Validates that active touches or geometry calculations don't break when the
 * OS pushes the view bounds down halfway
 */
int cmp_ios_mitigate_reachability_offset(float *io_touch_y,
                                         float reachability_offset);

/**
 * @brief Context Menu Peeking
 * Evaluates pressure for iOS 13/14 style deep-press "peek" previews before
 * popping into navigation.
 * @param pressure 0.0 to 1.0 (from 3D Touch or simulated long press duration)
 * @param out_preview_scale How large the preview card should render (starts
 * small, scales up to ~0.8 before full "pop")
 */
int cmp_ios_evaluate_context_menu_peek(float pressure,
                                       float *out_preview_scale);

/* Phase 10.2: Platform-Specific Master Checklist -> iPadOS (iPad) */
/**
 * @brief iOS Size Classes
 */
typedef enum cmp_size_class {
  CMP_SIZE_CLASS_COMPACT = 0, /* iPhone portrait, iPad Slide Over */
  CMP_SIZE_CLASS_REGULAR      /* iPad full screen, iPhone Max landscape */
} cmp_size_class_t;

/**
 * @brief Split View Styles
 */
typedef enum cmp_split_view_style {
  CMP_SPLIT_VIEW_DOUBLE_COLUMN = 0,
  CMP_SPLIT_VIEW_TRIPLE_COLUMN
} cmp_split_view_style_t;

/**
 * @brief Opaque iPadOS features controller
 */
typedef struct cmp_ipados_features cmp_ipados_features_t;

int cmp_ipados_features_create(cmp_ipados_features_t **out_features);
int cmp_ipados_features_destroy(cmp_ipados_features_t *features);

/**
 * @brief Evaluates Size Classes based on dynamic window dimensions (handles
 * multitasking/Slide Over)
 */
int cmp_ipados_resolve_size_classes(float window_width, float window_height,
                                    cmp_size_class_t *out_horizontal,
                                    cmp_size_class_t *out_vertical);

/**
 * @brief Resolves Sidebar layout behavior
 * @param horizontal_class Evaluated size class
 * @param out_is_collapsed Returns 1 if the sidebar should collapse into a
 * toggle button (iPhone mode)
 */
int cmp_ipados_resolve_sidebar_state(cmp_size_class_t horizontal_class,
                                     int *out_is_collapsed);

/**
 * @brief Request the OS to spawn a new, independent Window Scene (Stage Manager
 * / Split View support)
 */
int cmp_ipados_request_scene_activation(cmp_ipados_features_t *features,
                                        const char *activity_identifier);

/**
 * @brief Enable Center Stage (auto-panning/zooming) on supported iPad Pro
 * cameras
 */
int cmp_ipados_set_center_stage_enabled(cmp_ipados_features_t *features,
                                        int is_enabled);

/* Phase 10.3: Platform-Specific Master Checklist -> macOS (Mac) */

/**
 * @brief Opaque macOS features controller
 */
typedef struct cmp_macos_features cmp_macos_features_t;

int cmp_macos_features_create(cmp_macos_features_t **out_features);
int cmp_macos_features_destroy(cmp_macos_features_t *features);

/**
 * @brief Toggles visibility of standard macOS window controls (Traffic Lights)
 */
int cmp_macos_set_window_controls_visible(cmp_macos_features_t *features,
                                          int visible);

/**
 * @brief Sets a document proxy icon next to the window title
 * @param file_url URL to the represented file, or NULL to remove
 */
int cmp_macos_set_document_proxy(cmp_macos_features_t *features,
                                 const char *file_url);

/**
 * @brief Enables a system tray icon in the macOS menu bar
 */
int cmp_macos_set_menu_bar_extra(cmp_macos_features_t *features, int enabled);

/**
 * @brief Integrates directly with NSPrintOperation to show the macOS print
 * dialog
 */
int cmp_macos_invoke_print_panel(cmp_macos_features_t *features);

/**
 * @brief Sets up the global application menu bar (File, Edit, View, etc.)
 */
int cmp_macos_setup_global_menu_bar(cmp_macos_features_t *features);

/**
 * @brief Creates a standard macOS desktop context menu (no blurred background)
 */
int cmp_macos_setup_context_menu(cmp_macos_features_t *features, void *node);

/**
 * @brief Sets precise mouse hover states for macOS UI elements
 */
int cmp_macos_set_hover_state(cmp_macos_features_t *features, void *node,
                              int is_hovered);

/**
 * @brief Configures deep diffuse shadows for active windows, and flat for
 * inactive
 */
int cmp_macos_set_window_shadow(cmp_macos_features_t *features, int is_active);

/**
 * @brief Enables native right-click "Customize Toolbar..." logic
 */
int cmp_macos_setup_toolbar_customization(cmp_macos_features_t *features);

/* Phase 10.4: Platform-Specific Master Checklist -> watchOS (Apple Watch) */

/**
 * @brief Opaque watchOS features controller
 */
typedef struct cmp_watchos_features cmp_watchos_features_t;

int cmp_watchos_features_create(cmp_watchos_features_t **out_features);
int cmp_watchos_features_destroy(cmp_watchos_features_t *features);

/**
 * @brief Translates digital crown rotation to vertical scroll offsets,
 * triggering detent haptics
 */
int cmp_watchos_handle_digital_crown(cmp_watchos_features_t *features,
                                     float delta);

/**
 * @brief Responds to the physical Double Tap finger gesture on Series 9/Ultra 2
 */
int cmp_watchos_handle_double_tap(cmp_watchos_features_t *features);

/**
 * @brief Sets the Always-On Display (AOD) active state, dimming components and
 * hiding sensitive info
 */
int cmp_watchos_set_aod_state(cmp_watchos_features_t *features,
                              int is_wrist_down);

/**
 * @brief Exports rich JSON data mapped to CLKComplicationTemplate variants
 */
int cmp_watchos_export_complication_data(cmp_watchos_features_t *features,
                                         const char *json_data);

/**
 * @brief Provides relevant UI snippets to the watchOS 10+ Smart Stack
 */
int cmp_watchos_export_smart_stack(cmp_watchos_features_t *features,
                                   const char *json_data);

/**
 * @brief Removes side insets, cards, and borders to create a pure black #000000
 * edge-to-edge layout
 */
int cmp_watchos_apply_edge_to_edge_styling(cmp_watchos_features_t *features,
                                           void *node);

/**
 * @brief Adjusts primary action buttons to be full-width and pill-shaped at the
 * bottom of the view
 */
int cmp_watchos_apply_pill_button_styling(cmp_watchos_features_t *features,
                                          void *button_node);

/**
 * @brief Forces navigation routing into vertical, paginated scrolling or simple
 * 1-deep push stacks
 */
int cmp_watchos_apply_hierarchical_pagination(cmp_watchos_features_t *features,
                                              void *view_controller);

/* Phase 10.5: Platform-Specific Master Checklist -> tvOS (Apple TV) */

/**
 * @brief Opaque tvOS features controller
 */
typedef struct cmp_tvos_features cmp_tvos_features_t;

int cmp_tvos_features_create(cmp_tvos_features_t **out_features);
int cmp_tvos_features_destroy(cmp_tvos_features_t *features);

/**
 * @brief Drives the 2D spatial navigation and applies parallax/spring scaling
 * to the active node
 */
int cmp_tvos_handle_focus_engine_update(cmp_tvos_features_t *features,
                                        void *focused_node, float *out_scale,
                                        float *out_tilt_x, float *out_tilt_y);

/**
 * @brief Calculates generous outer margins for older physical television bezels
 */
int cmp_tvos_set_overscan_margins(cmp_tvos_features_t *features,
                                  float *out_margin_top,
                                  float *out_margin_bottom,
                                  float *out_margin_left,
                                  float *out_margin_right);

/**
 * @brief Provides dynamic, scrollable full-screen imagery or inset grids to the
 * tvOS home screen Top Shelf
 * @param is_inset 0 for Carousel, 1 for Inset
 */
int cmp_tvos_export_top_shelf(cmp_tvos_features_t *features, int is_inset,
                              const char *json_data);

/**
 * @brief Responds to the physical Play/Pause button on the Siri Remote
 */
int cmp_tvos_handle_hardware_play_pause(cmp_tvos_features_t *features);

/**
 * @brief Responds to the physical Menu (Back) button on the Siri Remote
 */
int cmp_tvos_handle_hardware_menu_button(cmp_tvos_features_t *features);

/* Phase 10.6: Platform-Specific Master Checklist -> visionOS (Spatial
 * Computing) */

/**
 * @brief Opaque visionOS features controller
 */
typedef struct cmp_visionos_features cmp_visionos_features_t;

int cmp_visionos_features_create(cmp_visionos_features_t **out_features);
int cmp_visionos_features_destroy(cmp_visionos_features_t *features);

/**
 * @brief Applies standard OS glass background materials reacting to virtual
 * room lighting
 */
int cmp_visionos_apply_glass_material(cmp_visionos_features_t *features,
                                      void *node);

/**
 * @brief Renders a subtle glow/hover effect when the user looks at the element
 * (eye tracking)
 */
int cmp_visionos_handle_eye_tracking_hover(cmp_visionos_features_t *features,
                                           void *node, int is_looking);

/**
 * @brief Positions a modal element closer to the user on the Z-axis
 */
int cmp_visionos_set_modal_z_depth(cmp_visionos_features_t *features,
                                   void *modal_node, float depth_offset);

/**
 * @brief Configures toolbars to float slightly outside and in front of the main
 * window bounds
 */
int cmp_visionos_setup_ornament(cmp_visionos_features_t *features,
                                void *toolbar_node);

/**
 * @brief Requests the OS to switch between mixed (0), progressive (1), and full
 * (2) immersion styles
 */
int cmp_visionos_request_immersion_level(cmp_visionos_features_t *features,
                                         int level);

/**
 * @brief Differentiates direct physical poke vs indirect look-and-pinch
 */
int cmp_visionos_handle_touch_event(cmp_visionos_features_t *features,
                                    int is_direct, float x, float y, float z);

/**
 * @brief Configures whether the interface acts as a flat Window or a 3D Volume
 */
int cmp_visionos_set_window_geometry(cmp_visionos_features_t *features,
                                     int is_volume);

/**
 * @brief Marks the app as a good citizen, sharing space without forcing
 * Immersive takeover
 */
int cmp_visionos_set_shared_space_behavior(cmp_visionos_features_t *features,
                                           int allow_shared);

/* Phase 10.7: Platform-Specific Master Checklist -> CarPlay */

/**
 * @brief Opaque CarPlay features controller
 */
typedef struct cmp_carplay_features cmp_carplay_features_t;

int cmp_carplay_features_create(cmp_carplay_features_t **out_features);
int cmp_carplay_features_destroy(cmp_carplay_features_t *features);

/**
 * @brief Translates UI sub-tree to CarPlay standard high-contrast, minimal text
 * styles
 */
int cmp_carplay_apply_driving_focus(cmp_carplay_features_t *features,
                                    void *node);

/**
 * @brief Translates the layout tree strictly into CarPlay system templates
 * (List, Grid, Map, Audio Player)
 */
int cmp_carplay_export_template_data(cmp_carplay_features_t *features,
                                     const char *template_type,
                                     const char *json_data);

/**
 * @brief Bridges UI actions to Siri Intents for pure voice navigation
 */
int cmp_carplay_handle_siri_intent(cmp_carplay_features_t *features,
                                   const char *intent_id);

/**
 * @brief Maps rotary knob input to focus engine spatial navigation
 */
int cmp_carplay_handle_knob_navigation(cmp_carplay_features_t *features,
                                       float delta);

/**
 * @brief Manages graceful offline degradation (tunnels, dead zones) without
 * blocking alerts
 */
int cmp_carplay_set_network_status(cmp_carplay_features_t *features,
                                   int is_online);

/* Phase 11.2: Internationalization & Localization (i18n) -> Text, Data
 * Formatting & Collation */

/**
 * @brief Opaque i18n formatting controller
 */
typedef struct cmp_i18n_formatting cmp_i18n_formatting_t;

int cmp_i18n_formatting_create(cmp_i18n_formatting_t **out_format);
int cmp_i18n_formatting_destroy(cmp_i18n_formatting_t *format);

/**
 * @brief Allows text labels to expand up to 50% without clipping for verbose
 * languages
 */
int cmp_i18n_handle_dynamic_expansion(cmp_i18n_formatting_t *format,
                                      void *text_node);

/**
 * @brief Parses localized .stringsdict files to handle plural forms and
 * inflection (Grammar Agreement)
 */
int cmp_i18n_load_stringsdict(cmp_i18n_formatting_t *format,
                              const char *file_data);

/**
 * @brief Formats unix timestamps according to OS locale rules
 */
int cmp_i18n_format_date(cmp_i18n_formatting_t *format,
                         long long unix_timestamp, char *out_str,
                         size_t max_len);

/**
 * @brief Formats currency dynamically pulling symbols and decimal separators
 * from OS locale
 */
int cmp_i18n_format_currency(cmp_i18n_formatting_t *format, double amount,
                             const char *currency_code, char *out_str,
                             size_t max_len);

/**
 * @brief Uses system PersonNameComponentsFormatter equivalents for correct
 * regional name ordering
 */
int cmp_i18n_format_person_name(cmp_i18n_formatting_t *format,
                                const char *given_name, const char *family_name,
                                char *out_str, size_t max_len);

/**
 * @brief Automatically converts and displays measurements based on locale (e.g.
 * miles vs km)
 */
int cmp_i18n_format_measurement(cmp_i18n_formatting_t *format, double value,
                                const char *unit, char *out_str,
                                size_t max_len);

/**
 * @brief Formats arrays of strings naturally based on locale (e.g. "A, B y C"
 * in Spanish)
 */
int cmp_i18n_format_list(cmp_i18n_formatting_t *format, const char **items,
                         size_t item_count, char *out_str, size_t max_len);

/**
 * @brief Applies vertical layout rendering and ruby annotations for CJK scripts
 */
int cmp_i18n_apply_cjk_vertical_text(cmp_i18n_formatting_t *format,
                                     void *text_node);

/**
 * @brief Sorts strings using localized collation rules (ignoring
 * diacritics/case) instead of raw ASCII
 */
int cmp_i18n_localized_sort(cmp_i18n_formatting_t *format, const char *a,
                            const char *b, int *out_result);

/**
 * @brief Placeholder for ABI stability initialization
 */
int cmp_interop_mock_init(void);

/**
 * @brief Safely bridges a CoreFoundation string (CFStringRef) to a Swift String
 * via ARC
 */
int cmp_interop_cfstring_bridge(void *cf_string_ref);

/**
 * @brief Allocates an object utilizing
s_returns_retained to seamlessly transfer ownership to ARC (+1 retain count)
 */
int cmp_interop_allocate_retained_object(void **out_object)
    CMP_RETURNS_RETAINED;

/**
 * @brief Safely releases a bridged object at the C boundary (-1 retain count)
 */
int cmp_interop_release_object(void *object);

/* Phase 13.1 & 13.2: Testing, Validation & Quality Assurance */

/**
 * @brief Opaque UI testing automation controller
 */
typedef struct cmp_testing_automation cmp_testing_automation_t;

int cmp_testing_automation_create(cmp_testing_automation_t **out_automation);
int cmp_testing_automation_destroy(cmp_testing_automation_t *automation);

/**
 * @brief Globally disables all transitions and springs to speed up tests
 * (UIView.setAnimationsEnabled)
 */
int cmp_testing_set_animations_enabled(cmp_testing_automation_t *automation,
                                       int enabled);

/**
 * @brief Automates UI testing by dispatching simulated touches based on A11y
 * labels (XCTest equivalent)
 */
int cmp_testing_tap_by_accessibility_label(cmp_testing_automation_t *automation,
                                           const char *label);

/**
 * @brief Triggers a pixel-perfect render comparing Light/Dark, Dynamic Type,
 * and RTL states against baselines
 */
int cmp_testing_snapshot_verify(cmp_testing_automation_t *automation,
                                void *node, const char *snapshot_name);

/**
 * @brief Performs strict structural checks akin to the Xcode Accessibility
 * Inspector to catch contrast/size errors
 */
int cmp_testing_audit_accessibility(cmp_testing_automation_t *automation,
                                    void *node);

/* Phase 13.3: Memory, Threading & Performance Profiling */

/**
 * @brief Opaque profiling telemetry controller
 */
typedef struct cmp_profiling_telemetry cmp_profiling_telemetry_t;

int cmp_profiling_telemetry_create(cmp_profiling_telemetry_t **out_telemetry);
int cmp_profiling_telemetry_destroy(cmp_profiling_telemetry_t *telemetry);

/**
 * @brief Emits low-latency OSSignpost metrics for Xcode Instruments tracking
 */
int cmp_profiling_emit_os_signpost(cmp_profiling_telemetry_t *telemetry,
                                   const char *event_name, int is_begin);

/**
 * @brief Enforces that layer mutations or render ticks execute solely on the
 * main thread
 */
int cmp_profiling_enforce_main_thread(cmp_profiling_telemetry_t *telemetry);

/**
 * @brief Defers a heavy workload (text shaping, image decoding) to the
 * background worker pool
 */
int cmp_profiling_offload_heavy_task(cmp_profiling_telemetry_t *telemetry,
                                     void (*task)(void *), void *user_data);

/**
 * @brief Scans a UI tree and event graph for strong memory cycles, returning
 * CMP_ERROR_CYCLE_DETECTED if found
 */
int cmp_profiling_detect_retain_cycles(cmp_profiling_telemetry_t *telemetry,
                                       void *root_node);

/**
 * @brief Replaces an object pointer with NULL and cleans up references
 * synchronously to prevent Zombie accesses
 */
int cmp_profiling_safe_destroy_node(cmp_profiling_telemetry_t *telemetry,
                                    void **node_ptr);

/* Phase 14.1: Documentation, DX & Framework APIs -> Developer Experience (DX)
 */

/**
 * @brief Opaque Developer Experience (DX) toolset
 */
typedef struct cmp_developer_experience cmp_developer_experience_t;

int cmp_developer_experience_create(cmp_developer_experience_t **out_dx);
int cmp_developer_experience_destroy(cmp_developer_experience_t *dx);

/**
 * @brief Instantiates a node using a declarative API surface mimicking SwiftUI
 */
int cmp_dx_build_declarative_node(cmp_developer_experience_t *dx,
                                  const char *type, void **out_node);

/**
 * @brief Applies strongly typed struct parameters for attributes rather than
 * loose strings
 */
int cmp_dx_apply_typed_color(cmp_developer_experience_t *dx, void *node,
                             int color_enum_val);

/**
 * @brief Connects the layout tree to a hot-reloading socket for Xcode
 * Canvas-style Live Previews
 */
int cmp_dx_enable_live_preview(cmp_developer_experience_t *dx, void *root_node);

/**
 * @brief Exports the entire scene graph as a 3D visual hierarchy matching
 * Xcode's Debug View Hierarchy
 */
int cmp_dx_export_debug_hierarchy(cmp_developer_experience_t *dx,
                                  void *root_node, char *out_json,
                                  size_t max_len);

/* Phase 14.2: Documentation, DX & Framework APIs -> Documentation & Examples */

/**
 * @brief Placeholder for DocC and snippet validation checks
 *
 * > HIG Reference: This ensures all API surfaces are comprehensively documented
 * > matching Apple's standard DocC syntax expectations.
 *
 * @code
 * // Example:
 * cmp_documentation_mock_init();
 * @endcode
 */
int cmp_documentation_mock_init(void);

/* Phase 15.1 & 15.2: Performance, Battery & System Resources */

/**
 * @brief Opaque system resource manager
 */
typedef struct cmp_resource_manager cmp_resource_manager_t;

int cmp_resource_manager_create(cmp_resource_manager_t **out_rm);
int cmp_resource_manager_destroy(cmp_resource_manager_t *rm);

/**
 * @brief Responds to ProcessInfo.thermalState (0=nominal, 1=fair, 2=serious,
 * 3=critical) to scale frame rates
 */
int cmp_resources_set_thermal_state(cmp_resource_manager_t *rm, int state);

/**
 * @brief Responds to OS backgrounding events, instantly pausing loops,
 * animations, and timers to save battery
 */
int cmp_resources_set_background_state(cmp_resource_manager_t *rm,
                                       int is_backgrounded);

/**
 * @brief Explicitly marks nodes as opaque = true to skip costly alpha blending
 * on the GPU
 */
int cmp_resources_mark_node_opaque(cmp_resource_manager_t *rm, void *node,
                                   int is_opaque);

/**
 * @brief Respects the OS "Low Data Mode" flag, preventing large downloads or
 * auto-playing videos
 */
int cmp_resources_set_low_data_mode(cmp_resource_manager_t *rm,
                                    int is_low_data);

/**
 * @brief Asynchronously decodes and aggressively caches remote images resized
 * to exact display geometry
 */
int cmp_resources_cache_remote_image(cmp_resource_manager_t *rm,
                                     const char *url, float target_width,
                                     float target_height);

/**
 * @brief Monitored drawRect equivalent that avoids allocating massive offscreen
 * bitmap contexts unless strictly necessary
 */
int cmp_resources_allocate_offscreen_bitmap(cmp_resource_manager_t *rm,
                                            float width, float height,
                                            void **out_bitmap);

/* Phase 16.1 & 16.2: Error Handling, Edge Cases & Resilience */

/**
 * @brief Opaque resilience and error state controller
 */
typedef struct cmp_resilience cmp_resilience_t;

int cmp_resilience_create(cmp_resilience_t **out_res);
int cmp_resilience_destroy(cmp_resilience_t *res);

/**
 * @brief Injects an informative "Empty State" (Icon, Title, CTA) when data
 * sources are empty
 */
int cmp_resilience_show_empty_state(cmp_resilience_t *res,
                                    void *container_node);

/**
 * @brief Renders a shimmering skeleton mimicking the final layout rather than a
 * blocking spinner
 */
int cmp_resilience_show_loading_skeleton(cmp_resilience_t *res,
                                         void *container_node);

/**
 * @brief Presents an inline banner or subtle toast notification for
 * non-critical network failures
 */
int cmp_resilience_show_non_blocking_error(cmp_resilience_t *res,
                                           void *container_node,
                                           const char *msg);

/**
 * @brief Prompts the user to "Save Draft" or "Discard" when closing a dirty
 * modal sheet
 */
int cmp_resilience_handle_discard_changes_prompt(cmp_resilience_t *res,
                                                 void *sheet_node);

/**
 * @brief Provides a standard, safe fallback for hardware specific features
 * (LiDAR, ARKit)
 */
int cmp_resilience_graceful_degradation(cmp_resilience_t *res,
                                        const char *feature_name);

/* Phase 17.1: App Store Review Guidelines Compliance */

/**
 * @brief Placeholder indicating that the framework restricts hidden features,
 * Easter eggs, and non-native platform paradigms
 */
int cmp_app_store_mock_init(void);

/**
 * @brief Resolves the appropriate VTable for a given UI node.
 * @param node The UI node to evaluate.
 * @return A pointer to the correct theme VTable.
 */
const cmp_theme_vtable_t *cmp_resolve_vtable(const cmp_ui_node_t *node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_H */