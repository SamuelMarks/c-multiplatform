/* clang-format off */
#ifndef M3_MEMORY_H
#define M3_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cmp.h>
#include <stddef.h>

/* Forward declarations */
typedef struct m3_memory_state_t m3_memory_state_t;
typedef struct m3_fallback_node_t m3_fallback_node_t;

struct m3_fallback_node_t {
    void *ptr;
    m3_fallback_node_t *next;
};

/* String interning pool structure */
typedef struct m3_string_pool_t {
    cmp_arena_t arena;
    char **strings;
    size_t count;
    size_t capacity;
    m3_fallback_node_t *fallback_allocs;
} m3_string_pool_t;

struct m3_memory_state_t {
    cmp_arena_t m3_state_arena;
    m3_fallback_node_t *state_fallback_allocs;

    cmp_arena_t m3_ui_arena;
    m3_fallback_node_t *ui_fallback_allocs;

    m3_string_pool_t string_pool;
};

/**
 * \brief Initializes the entire memory management system.
 * \param state Pointer to the memory state struct.
 * \return 0 on success, non-zero on failure.
 */
int m3_memory_init(m3_memory_state_t *state);

/**
 * \brief Cleans up the memory management system, including fallbacks.
 * \param state Pointer to the memory state struct.
 * \return 0 on success, non-zero on failure.
 */
int m3_memory_cleanup(m3_memory_state_t *state);

/**
 * \brief Allocates from the state arena. Uses fallback allocator if OOM.
 * \param state Pointer to the memory state struct.
 * \param size Size in bytes to allocate.
 * \param out_ptr Pointer to receive the allocated memory.
 * \return 0 on success, non-zero on failure.
 */
int m3_state_alloc(m3_memory_state_t *state, size_t size, void **out_ptr);

/**
 * \brief Allocates from the UI arena. Uses fallback allocator if OOM.
 * \param state Pointer to the memory state struct.
 * \param size Size in bytes to allocate.
 * \param out_ptr Pointer to receive the allocated memory.
 * \return 0 on success, non-zero on failure.
 */
int m3_ui_alloc(m3_memory_state_t *state, size_t size, void **out_ptr);

/**
 * \brief Clears the transient UI arena and its fallbacks.
 * \param state Pointer to the memory state struct.
 * \return 0 on success, non-zero on failure.
 */
int m3_ui_arena_clear(m3_memory_state_t *state);

/**
 * \brief Interns a string, returning a pointer to the interned string.
 * \param state Pointer to the memory state struct.
 * \param str The string to intern.
 * \param out_str Pointer to receive the interned string pointer.
 * \return 0 on success, non-zero on failure.
 */
int m3_string_intern(m3_memory_state_t *state, const char *str, const char **out_str);

#ifdef __cplusplus
}
#endif

#endif /* M3_MEMORY_H */
/* clang-format on */