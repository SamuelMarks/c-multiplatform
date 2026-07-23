#ifndef UI_SCAFFOLD_BASE_H
#define UI_SCAFFOLD_BASE_H

struct ui_computed;

struct ui_signal;

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents an unstyled application scaffold/shell.
 */
struct ui_scaffold_base {
  struct ui_component base;
  struct ui_dom_node *slot_top_bar;
  struct ui_dom_node *slot_bottom_bar;
  struct ui_dom_node *slot_side_nav;
  struct ui_dom_node *slot_main_content;
  struct ui_signal *data_signal;
};

/**
 * @brief Creates a new base scaffold component.
 *
 * @param out_scaffold Pointer to output the initialized scaffold.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_scaffold_base_create(struct ui_scaffold_base **out_scaffold);

/**
 * @brief Sets the top app bar component of the scaffold.
 *
 * @param scaffold The scaffold component.
 * @param top_bar The top bar component.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_scaffold_base_set_top_bar(struct ui_scaffold_base *scaffold,
                                           struct ui_component *top_bar);

/**
 * @brief Sets the main content component of the scaffold.
 *
 * @param scaffold The scaffold component.
 * @param content The main content component.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_scaffold_base_set_main_content(struct ui_scaffold_base *scaffold,
                                  struct ui_component *content);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_scaffold_base_bind_data(struct ui_scaffold_base *widget,
                                         struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SCAFFOLD_BASE_H */
