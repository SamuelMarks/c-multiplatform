/**
 * @file ui_form_node.h
 * @brief Core abstractions for form nodes.
 *
 * This header defines the generic tagged union for form nodes, which can be
 * a control, a group, or an array.
 */

#ifndef UI_FORM_NODE_H
#define UI_FORM_NODE_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Discriminant for a generic form node.
 */
enum ui_form_node_type {
  UI_FORM_NODE_CONTROL = 0, /**< A single form control. */
  UI_FORM_NODE_GROUP =
      1, /**< A form group containing multiple nodes mapped by keys. */
  UI_FORM_NODE_ARRAY =
      2 /**< A form array containing multiple nodes linearly. */
};

/**
 * @brief Forward declaration for a form control.
 */
struct ui_form_control;

/**
 * @brief Forward declaration for a form group.
 */
struct ui_form_group;

/**
 * @brief Forward declaration for a form array.
 */
struct ui_form_array;

/**
 * @brief Tagged union representing a generic form node.
 */
typedef struct ui_form_node {
  enum ui_form_node_type type; /**< The type of the form node. */
  /** \brief Union containing the specific node handle. */
  union {
    struct ui_form_control *control; /**< Control handle. */
    struct ui_form_group *group;     /**< Group handle. */
    struct ui_form_array *array;     /**< Array handle. */
  } node;
} ui_form_node_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FORM_NODE_H */
