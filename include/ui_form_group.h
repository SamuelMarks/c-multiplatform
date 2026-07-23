#ifndef UI_FORM_GROUP_H
#define UI_FORM_GROUP_H

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
 * @brief Opaque handle to a form group.
 */
typedef struct ui_form_group ui_form_group_t;

/**
 * @brief Creates a new form group.
 *
 * @param arena The arena.
 * @param mode The signal mode.
 * @param out_group The pointer to store the created form group.
 * @return enum ui_error
 */
enum ui_error ui_form_group_create(struct ui_arena *arena,
                                   enum ui_signal_mode mode,
                                   ui_form_group_t **out_group);

/**
 * @brief Adds a node to the form group.
 *
 * @param group The form group.
 * @param name The name of the node.
 * @param node The form node to add.
 * @return enum ui_error
 */
enum ui_error ui_form_group_add_node(ui_form_group_t *group, const char *name,
                                     ui_form_node_t node);

/**
 * @brief Gets a node from the form group.
 *
 * @param group The form group.
 * @param name The name of the node.
 * @param out_node The pointer to store the retrieved node.
 * @return enum ui_error
 */
enum ui_error ui_form_group_get_node(ui_form_group_t *group, const char *name,
                                     ui_form_node_t *out_node);

/**
 * @brief Adds a control to the form group (helper).
 *
 * @param group The form group.
 * @param name The name of the control.
 * @param control The control to add.
 * @return enum ui_error
 */
enum ui_error ui_form_group_add_control(ui_form_group_t *group,
                                        const char *name,
                                        ui_form_control_t *control);

/**
 * @brief Gets a control from the form group (helper).
 *
 * @param group The form group.
 * @param name The name of the control.
 * @param out_control The pointer to store the retrieved control.
 * @return enum ui_error
 */
enum ui_error ui_form_group_get_control(ui_form_group_t *group,
                                        const char *name,
                                        ui_form_control_t **out_control);

/**
 * @brief Traverses the form group and computes its aggregated status.
 *
 * @param group The form group.
 * @param out_status The computed status.
 * @return enum ui_error
 */
enum ui_error ui_form_group_get_status(ui_form_group_t *group,
                                       enum ui_form_status *out_status);

/**
 * @brief Computes whether the form group is valid.
 *
 * @param group The form group.
 * @param out_valid True if valid.
 * @return enum ui_error
 */
enum ui_error ui_form_group_is_valid(ui_form_group_t *group,
                                     ui_bool_t *out_valid);

/**
 * @brief Destroys the form group.
 *
 * @param group The form group.
 * @return enum ui_error
 */
enum ui_error ui_form_group_destroy(ui_form_group_t *group);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FORM_GROUP_H */
