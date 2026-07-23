#ifndef UI_FORM_NODE_INTERNAL_H
#define UI_FORM_NODE_INTERNAL_H

/* clang-format off */
#include "ui_form_node.h"
#include "ui_form_control.h"
/* clang-format on */

#define UI_FORM_MAX_DEPTH 64

enum ui_error _ui_form_node_get_status_internal(ui_form_node_t node,
                                                enum ui_form_status *out_status,
                                                size_t depth);

/** \brief _ui_form_group_get_status_internal */
enum ui_error _ui_form_group_get_status_internal(
    struct ui_form_group *group, enum ui_form_status *out_status, size_t depth);

/** \brief _ui_form_array_get_status_internal */
enum ui_error _ui_form_array_get_status_internal(
    struct ui_form_array *array, enum ui_form_status *out_status, size_t depth);

#endif /* UI_FORM_NODE_INTERNAL_H */
