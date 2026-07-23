#ifndef UI_FORM_ARRAY_H
#define UI_FORM_ARRAY_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_arena.h"
#include "ui_signal.h"
#include "ui_form_control.h"
#include "ui_form_node.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle to a form array.
 */
typedef struct ui_form_array ui_form_array_t;

/**
 * @brief Creates a new form array.
 *
 * @param arena The arena to allocate from.
 * @param mode The signal mode.
 * @param out_array The pointer to store the created form array.
 * @return enum ui_error
 */
enum ui_error ui_form_array_create(struct ui_arena *arena,
                                   enum ui_signal_mode mode,
                                   ui_form_array_t **out_array);

/**
 * @brief Pushes a node to the end of the form array.
 *
 * @param array The form array.
 * @param node The form node to push.
 * @return enum ui_error
 */
enum ui_error ui_form_array_push(ui_form_array_t *array, ui_form_node_t node);

/**
 * @brief Inserts a node at the specified index.
 *
 * @param array The form array.
 * @param index The index to insert at.
 * @param node The form node to insert.
 * @return enum ui_error
 */
enum ui_error ui_form_array_insert_at(ui_form_array_t *array, size_t index,
                                      ui_form_node_t node);

/**
 * @brief Removes a node at the specified index.
 *
 * @param array The form array.
 * @param index The index to remove.
 * @return enum ui_error
 */
enum ui_error ui_form_array_remove_at(ui_form_array_t *array, size_t index);

/**
 * @brief Clears all nodes from the form array.
 *
 * @param array The form array.
 * @return enum ui_error
 */
enum ui_error ui_form_array_clear(ui_form_array_t *array);

/**
 * @brief Gets a node from the form array.
 *
 * @param array The form array.
 * @param index The index of the node.
 * @param out_node The pointer to store the retrieved node.
 * @return enum ui_error
 */
enum ui_error ui_form_array_get_node(ui_form_array_t *array, size_t index,
                                     ui_form_node_t *out_node);

/**
 * @brief Traverses the form array and computes its aggregated status.
 *
 * @param array The form array.
 * @param out_status The computed status.
 * @return enum ui_error
 */
enum ui_error ui_form_array_get_status(ui_form_array_t *array,
                                       enum ui_form_status *out_status);

/**
 * @brief Destroys the form array.
 *
 * @param array The form array.
 * @return enum ui_error
 */
enum ui_error ui_form_array_destroy(ui_form_array_t *array);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FORM_ARRAY_H */
