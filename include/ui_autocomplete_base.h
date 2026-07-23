#ifndef UI_AUTOCOMPLETE_BASE_H
#define UI_AUTOCOMPLETE_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_event.h"
#include "ui_listbox_base.h"
#include "ui_input_base.h"
#include "ui_popover_base.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

struct ui_autocomplete_base;
struct ui_overlay_director;
struct ui_focus_manager;

/**
 * @brief Callback invoked when the user selects an item from the autocomplete
 * list.
 *
 * @param autocomplete The autocomplete component.
 * @param index The selected item index.
 * @param user_data Opaque user data.
 */
typedef enum ui_error (*ui_autocomplete_on_selection_t)(
    struct ui_autocomplete_base *autocomplete, int index, void *user_data);

/**
 * @brief Callback invoked when the text in the input changes, allowing the
 * application to update item count/data.
 *
 * @param autocomplete The autocomplete component.
 * @param text The new text string.
 * @param user_data Opaque user data.
 */
typedef enum ui_error (*ui_autocomplete_on_text_change_t)(
    struct ui_autocomplete_base *autocomplete, const char *text,
    void *user_data);

/**
 * @brief Creates a new unstyled autocomplete base component.
 *
 * @param out_autocomplete Pointer to receive the allocated autocomplete base.
 * @param out_cva Optional pointer to receive the control value accessor.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_autocomplete_base_create(struct ui_autocomplete_base **out_autocomplete,
                            struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys an autocomplete component.
 *
 * @param autocomplete The autocomplete to destroy.
 */
void ui_autocomplete_base_destroy(struct ui_autocomplete_base *autocomplete);

/**
 * @brief Retrieves the root UI component (which wraps the input).
 *
 * @param autocomplete The autocomplete.
 * @return The component, or NULL.
 */
enum ui_error
ui_autocomplete_base_get_component(struct ui_autocomplete_base *autocomplete,
                                   struct ui_component **out_component);

/**
 * @brief Retrieves the internal input base component.
 *
 * @param autocomplete The autocomplete.
 * @return The input base, or NULL.
 */
enum ui_error
ui_autocomplete_base_get_input(struct ui_autocomplete_base *autocomplete,
                               struct ui_input_base **out_input);

/**
 * @brief Retrieves the internal listbox base component.
 *
 * @param autocomplete The autocomplete.
 * @return The listbox base, or NULL.
 */
enum ui_error
ui_autocomplete_base_get_listbox(struct ui_autocomplete_base *autocomplete,
                                 struct ui_listbox_base **out_listbox);

/**
 * @brief Configures overlay dependencies required to render the popover.
 *
 * @param autocomplete The autocomplete.
 * @param director The global overlay director.
 * @param focus_mgr The global focus manager.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_autocomplete_base_set_overlay_dependencies(
    struct ui_autocomplete_base *autocomplete,
    struct ui_overlay_director *director, struct ui_focus_manager *focus_mgr);

/**
 * @brief Sets the text change callback to handle filtering.
 *
 * @param autocomplete The autocomplete.
 * @param on_text_change The callback.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_autocomplete_base_set_on_text_change(
    struct ui_autocomplete_base *autocomplete,
    ui_autocomplete_on_text_change_t on_text_change, void *user_data);

/**
 * @brief Sets the selection callback.
 *
 * @param autocomplete The autocomplete.
 * @param on_selection The callback.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_autocomplete_base_set_on_selection(
    struct ui_autocomplete_base *autocomplete,
    ui_autocomplete_on_selection_t on_selection, void *user_data);

/**
 * @brief Opens the autocomplete dropdown popover.
 *
 * @param autocomplete The autocomplete.
 * @param trigger_layout The layout node of the trigger element for positioning.
 * @param viewport_width Total viewport width.
 * @param viewport_height Total viewport height.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_autocomplete_base_open(struct ui_autocomplete_base *autocomplete,
                          const struct ui_layout_node *trigger_layout,
                          float viewport_width, float viewport_height);

/**
 * @brief Closes the autocomplete dropdown popover.
 *
 * @param autocomplete The autocomplete.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_autocomplete_base_close(struct ui_autocomplete_base *autocomplete);

/**
 * @brief Processes an input event, managing focus routing between input and
 * listbox.
 *
 * @param autocomplete The autocomplete.
 * @param event The input event.
 * @param timestamp_ms Current time in ms.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_autocomplete_base_process_event(struct ui_autocomplete_base *autocomplete,
                                   const struct ui_event *event,
                                   double timestamp_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_AUTOCOMPLETE_BASE_H */
