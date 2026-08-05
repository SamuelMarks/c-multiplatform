#ifndef UI_ACTION_SHEET_BASE_H
#define UI_ACTION_SHEET_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_bottom_sheet_base.h"
#include "ui_focus_manager.h"
#include "ui_keyboard_responder.h"
#include "ui_event.h"
#include "ui_signal.h"
#include "ui_computed.h"
/* clang-format on */

struct ui_action_sheet_base;

typedef ui_error_t (*ui_action_sheet_on_close_t)(
    struct ui_action_sheet_base *sheet, void *user_data);

/** \brief ui_error */
ui_error_t ui_action_sheet_base_create(struct ui_action_sheet_base **out_sheet);
ui_error_t ui_action_sheet_base_destroy(struct ui_action_sheet_base *sheet);

/** \brief ui_error */
ui_error_t ui_action_sheet_base_add_action(struct ui_action_sheet_base *sheet,
                                           struct ui_component *action_comp);

/** \brief ui_error */
ui_error_t
ui_action_sheet_base_set_cancel_action(struct ui_action_sheet_base *sheet,
                                       struct ui_component *cancel_comp);

ui_error_t ui_action_sheet_base_set_open(struct ui_action_sheet_base *sheet,
                                         int is_open);
/** \brief ui_error */
ui_error_t
ui_action_sheet_base_is_open(const struct ui_action_sheet_base *sheet,
                             int *out_is_open);

/** \brief ui_error */
ui_error_t
ui_action_sheet_base_set_overlay_director(struct ui_action_sheet_base *sheet,
                                          struct ui_overlay_director *director);

/** \brief ui_error */
ui_error_t
ui_action_sheet_base_set_on_close(struct ui_action_sheet_base *sheet,
                                  ui_action_sheet_on_close_t on_close,
                                  void *user_data);

/** \brief ui_action_sheet_base_attach_focus_and_keyboard */
ui_error_t ui_action_sheet_base_attach_focus_and_keyboard(
    struct ui_action_sheet_base *sheet, struct ui_focus_manager *focus_manager,
    struct ui_keyboard_responder *keyboard_responder);

/** \brief ui_error */
ui_error_t
ui_action_sheet_base_process_event(struct ui_action_sheet_base *sheet,
                                   const struct ui_event *event,
                                   double timestamp_ms);

ui_error_t ui_action_sheet_base_update(struct ui_action_sheet_base *sheet,
                                       double timestamp_ms);

/** \brief ui_error */
ui_error_t
ui_action_sheet_base_get_component(struct ui_action_sheet_base *sheet,
                                   struct ui_component **out_component);

/**
 * @brief Binds the action sheet's open state to a signal.
 *
 * @param sheet The action sheet.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_action_sheet_base_bind_open(struct ui_action_sheet_base *sheet,
                                          struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the action sheet is
 * animating.
 *
 * @param sheet The action sheet.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_action_sheet_base_get_animating_signal(struct ui_action_sheet_base *sheet,
                                          struct ui_computed **out_animating);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_ACTION_SHEET_BASE_H */
