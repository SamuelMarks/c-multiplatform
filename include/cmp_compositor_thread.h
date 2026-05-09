/* clang-format off */
#ifndef CMP_COMPOSITOR_THREAD_H
#define CMP_COMPOSITOR_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Opaque Compositor Thread Context
 */
typedef struct cmp_compositor_thread cmp_compositor_thread_t;

/**
 * @brief Create a dedicated compositor thread for 120hz VSync updates
 * @param out_thread Pointer to receive the context
 * @return 0 on success
 */
int cmp_compositor_thread_create(cmp_compositor_thread_t **out_thread);

/**
 * @brief Destroy the compositor thread context
 * @param thread The context to destroy
 * @return 0 on success
 */
int cmp_compositor_thread_destroy(cmp_compositor_thread_t *thread);

/**
 * @brief Push an immutable snapshot of the UI layer tree to the compositor
 * @param thread The compositor thread context
 * @param layer_tree_opaque Pointer to the layer tree
 * @return 0 on success
 */
int cmp_compositor_thread_push_tree(cmp_compositor_thread_t *thread, void *layer_tree_opaque);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CMP_COMPOSITOR_THREAD_H */
/* clang-format on */
