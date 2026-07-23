#ifndef UI_LABEL_BASE_H
#define UI_LABEL_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "ui_event.h"
/* clang-format on */

struct ui_label_base;

/**
 * @brief Creates a new unstyled label base component.
 *
 * @param out_label Pointer to receive the allocated label base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_label_base_create(struct ui_label_base **out_label);

/**
 * @brief Destroys a label base component.
 *
 * @param label The label to destroy.
 */
void ui_label_base_destroy(struct ui_label_base *label);

/**
 * @brief Sets the "for" attribute equivalent, linking this label to a target
 * DOM node ID.
 *
 * @param label The label.
 * @param target_id The ID of the target DOM node.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_label_base_set_for(struct ui_label_base *label,
                                    const char *target_id);

/**
 * @brief Binds a specific target DOM node to the label, bypassing ID lookup.
 *
 * @param label The label.
 * @param target_node The DOM node to associate with this label.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_label_base_set_target_node(struct ui_label_base *label,
                                            struct ui_dom_node *target_node);

/**
 * @brief Processes an incoming input event to trigger forwarding to the target.
 *
 * @param label The label.
 * @param event The input event.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_label_base_process_event(struct ui_label_base *label,
                                          const struct ui_event *event,
                                          double timestamp_ms);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param label The label.
 * @return The underlying component.
 */
enum ui_error ui_label_base_get_component(struct ui_label_base *label,
                                          struct ui_component **out_component);

/**
 * @brief Binds the text property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_label_base_bind_text(struct ui_label_base *widget,
                                      struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_LABEL_BASE_H */
