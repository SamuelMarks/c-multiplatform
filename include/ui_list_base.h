/**
 * @file ui_list_base.h
 * @brief Base list and list item component definitions.
 */

#ifndef UI_LIST_BASE_H
#define UI_LIST_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

struct ui_list_base;
struct ui_list_item_base;

/**
 * @brief List orientation types for rendering flow.
 */
enum ui_list_orientation {
  UI_LIST_ORIENTATION_VERTICAL,
  UI_LIST_ORIENTATION_HORIZONTAL
};

/**
 * @brief Creates a new unstyled list component (`role="list"`).
 *
 * @param out_list Pointer to receive the allocated list base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_list_base_create(struct ui_list_base **out_list);

/**
 * @brief Destroys a list component.
 *
 * @param list The list to destroy.
 */
ui_error_t ui_list_base_destroy(struct ui_list_base *list);

/**
 * @brief Gets the underlying component for the list.
 *
 * @param list The list.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_list_base_get_component(struct ui_list_base *list,
                                      struct ui_component **out_component);

/**
 * @brief Sets the orientation of the list (vertical or horizontal).
 *
 * @param list The list.
 * @param orientation The orientation.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_list_base_set_orientation(struct ui_list_base *list,
                                        enum ui_list_orientation orientation);

/**
 * @brief Gets the current orientation of the list.
 *
 * @param list The list.
 * @param out_orientation Pointer to receive the orientation.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_list_base_get_orientation(struct ui_list_base *list,
                             enum ui_list_orientation *out_orientation);

/**
 * @brief Appends a list item to the list.
 *
 * @param list The list.
 * @param item The list item to append.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_list_base_append_item(struct ui_list_base *list,
                                    struct ui_list_item_base *item);

/**
 * @brief Creates a new unstyled list item component (`role="listitem"`).
 *
 * @param out_item Pointer to receive the allocated list item base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_list_item_base_create(struct ui_list_item_base **out_item);

/**
 * @brief Destroys a list item component.
 *
 * @param item The list item to destroy.
 */
ui_error_t ui_list_item_base_destroy(struct ui_list_item_base *item);

/**
 * @brief Gets the underlying component for the list item.
 *
 * @param item The list item.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_list_item_base_get_component(struct ui_list_item_base *item,
                                           struct ui_component **out_component);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_list_base_bind_data(struct ui_list_base *widget,
                                  struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_LIST_BASE_H */
