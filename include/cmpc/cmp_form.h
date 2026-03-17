#ifndef CMP_FORM_H
#define CMP_FORM_H

/**
 * @file cmp_form.h
 * @brief Form and focus management container for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_event.h"
/* clang-format on */

struct CMPForm;

/**
 * @brief Form submission callback.
 * @param ctx User context pointer.
 * @param form Form instance.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPFormSubmitFn)(void *ctx, struct CMPForm *form);

/**
 * @brief Form container widget.
 *
 * Manages input focus iteration (Tab/Shift-Tab) and submission via Enter key.
 */
typedef struct CMPForm {
  CMPWidget widget;               /**< Widget interface. */
  CMPRect bounds;                 /**< Layout bounds. */
  CMPEventDispatcher *dispatcher; /**< UI event dispatcher. */
  CMPWidget *root_widget;         /**< The visual root of the form. */
  CMPWidget **inputs;             /**< Array of focusable input widgets. */
  cmp_usize input_count;          /**< Number of input widgets. */
  CMPWidget *submit_btn;          /**< Form submit button (optional). */
  CMPFormSubmitFn on_submit;      /**< Submit callback. */
  void *user_data;                /**< Context for callbacks. */
  CMPBool is_submitting;          /**< CMP_TRUE while form is loading. */
} CMPForm;

/**
 * @brief Initialize a form widget.
 * @param form Form instance.
 * @param dispatcher Event dispatcher for focus management.
 * @param root_widget Root visual widget to wrap.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_form_init(CMPForm *form,
                                   CMPEventDispatcher *dispatcher,
                                   CMPWidget *root_widget);

/**
 * @brief Configure the focusable inputs of the form.
 * @param form Form instance.
 * @param inputs Array of widget pointers.
 * @param input_count Number of widgets.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_form_set_inputs(CMPForm *form, CMPWidget **inputs,
                                         cmp_usize input_count);

/**
 * @brief Configure the submit callback and button.
 * @param form Form instance.
 * @param submit_btn Widget acting as the submit button (optional).
 * @param on_submit Callback triggered on submit.
 * @param user_data User data for the callback.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_form_set_submit(CMPForm *form, CMPWidget *submit_btn,
                                         CMPFormSubmitFn on_submit,
                                         void *user_data);

/**
 * @brief Set the submitting state of the form.
 *
 * When submitting is true, input events are generally suppressed to
 * prevent duplicate submissions, and visual components can render disabled
 * states.
 *
 * @param form Form instance.
 * @param is_submitting CMP_TRUE to block interaction.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_form_set_submitting(CMPForm *form,
                                             CMPBool is_submitting);

/**
 * @brief Trigger form submission manually (e.g., from a button callback).
 * @param form Form instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_form_submit(CMPForm *form);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_FORM_H */
