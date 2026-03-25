#ifndef CMP_H
#define CMP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
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
  CMP_SUCCESS = 0,           /**< Success */
  CMP_ERROR_OOM = 1,         /**< Out of memory */
  CMP_ERROR_INVALID_ARG = 2, /**< Invalid argument provided */
  CMP_ERROR_NOT_FOUND = 3,   /**< Resource not found */
  CMP_ERROR_BOUNDS = 4       /**< Out of bounds access */
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
typedef struct cmp_rect {
  float x;
  float y;
  float width;
  float height;
} cmp_rect_t;

/**
 * @brief Flexbox Alignment Options
 */
typedef enum cmp_flex_align {
  CMP_FLEX_ALIGN_START = 0,
  CMP_FLEX_ALIGN_CENTER = 1,
  CMP_FLEX_ALIGN_END = 2,
  CMP_FLEX_ALIGN_STRETCH = 3,
  CMP_FLEX_ALIGN_SPACE_BETWEEN = 4,
  CMP_FLEX_ALIGN_SPACE_AROUND = 5
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
  CMP_POSITION_ABSOLUTE = 1
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
 * @return 0 on success, or an error code.
 */
int cmp_ui_text_create(cmp_ui_node_t **out_node, const char *text);

/**
 * @brief Initialize a UI button component
 * @param out_node Pointer to receive the allocated node
 * @param label Text to display inside the button
 * @return 0 on success, or an error code.
 */
int cmp_ui_button_create(cmp_ui_node_t **out_node, const char *label);

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
typedef struct cmp_texture {
  void *internal_handle;
  int width;
  int height;
  int format;
} cmp_texture_t;

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

/**
 * @brief Color structure (RGBA)
 */
typedef struct cmp_color {
  float r;
  float g;
  float b;
  float a;
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
  int is_dark_mode;
  int high_contrast_mode;
  int reduce_motion;
  float global_scale;
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
 * @brief Generate a dynamic palette from a seed color (e.g. Material You
 * extraction)
 * @param seed The base color
 * @param out_palette Pointer to receive the generated palette
 * @return 0 on success, or an error code.
 */
int cmp_theme_generate_palette(cmp_color_t seed, cmp_palette_t *out_palette);

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
 * @return The active direction
 */
cmp_text_direction_t cmp_i18n_get_bidi_direction(void);

/**
 * @brief Abstract Representation of a custom Shader Pipeline
 */
typedef struct cmp_shader {
  void *internal_handle;
} cmp_shader_t;

/**
 * @brief Opaque Renderer Context
 */
typedef struct cmp_renderer cmp_renderer_t;

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
 * @brief Render a basic test payload (e.g. "Hello World" or clear color) to the
 * window
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_H */
