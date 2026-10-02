/**
 * @file cupertino_activity_view.h
 * @brief Cupertino Activity View (UIActivityViewController / Share Sheet)
 * modal sharing surface conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_ACTIVITY_VIEW_H
#define CUPERTINO_CUPERTINO_ACTIVITY_VIEW_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_ACTIVITY_VIEW_MAX_ITEMS 24

/**
 * @enum cupertino_activity_category
 * @brief Categorization tier for activity items in the Apple Share Sheet.
 */
enum cupertino_activity_category {
  CUPERTINO_ACTIVITY_AIRDROP =
      0, /**< Top AirDrop contact/device discovery row. */
  CUPERTINO_ACTIVITY_APP_EXTENSION =
      1, /**< Horizontal sharing app icons row. */
  CUPERTINO_ACTIVITY_SYSTEM_ACTION =
      2 /**< Vertical system action list items (Copy, Save, Print). */
};

/**
 * @struct cupertino_activity_item
 * @brief Action item or sharing destination definition.
 */
struct cupertino_activity_item {
  char title[64];       /**< Localized action label. */
  char symbol_name[64]; /**< SF Symbol or icon identifier. */
  enum cupertino_activity_category category; /**< Visual grouping category. */
  int is_disabled; /**< Non-zero if item is dimmed/disabled. */
};

/**
 * @struct cupertino_activity_view_descriptor
 * @brief Configuration descriptor for creating a Cupertino Activity View.
 */
struct cupertino_activity_view_descriptor {
  const char *share_text; /**< Payload text snippet to share. */
  const char *share_url;  /**< Payload URL to share. */
  int is_dark;            /**< Non-zero for dark mode styling. */
};

/**
 * @struct cupertino_activity_view
 * @brief Cupertino Activity View instance.
 */
struct cupertino_activity_view {
  char share_text[256]; /**< Shared text payload. */
  char share_url[256];  /**< Shared URL payload. */
  int is_dark;          /**< Dark mode appearance state. */
  int is_open;          /**< Presentation open status. */
  size_t item_count;    /**< Configured activity count. */
  struct cupertino_activity_item
      items[CUPERTINO_ACTIVITY_VIEW_MAX_ITEMS]; /**< Activity items. */
  int last_activated_index; /**< Last triggered activity index (-1 if none). */
};

/**
 * @brief Creates a new Cupertino Activity View (Share Sheet).
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_view Pointer to receive newly created activity view.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_activity_view_create(
    struct ui_engine *engine,
    const struct cupertino_activity_view_descriptor *desc,
    struct cupertino_activity_view **out_view);

/**
 * @brief Destroys a Cupertino Activity View.
 *
 * @param view Activity view instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_view_destroy(struct cupertino_activity_view *view);

/**
 * @brief Appends an activity item to the share sheet.
 *
 * @param view Target activity view.
 * @param item Pointer to activity item definition.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if maximum items
 * reached, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_view_add_item(struct cupertino_activity_view *view,
                                 const struct cupertino_activity_item *item);

/**
 * @brief Gets number of configured activity items.
 *
 * @param view Target activity view.
 * @param out_count Pointer to receive item count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_activity_view_get_item_count(
    const struct cupertino_activity_view *view, size_t *out_count);

/**
 * @brief Gets activity item at specified index.
 *
 * @param view Target activity view.
 * @param index 0-based index of item.
 * @param out_item Pointer to receive item copy.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if index invalid,
 * or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_activity_view_get_item(
    const struct cupertino_activity_view *view, size_t index,
    struct cupertino_activity_item *out_item);

/**
 * @brief Presents the share sheet modal presentation.
 *
 * @param view Target activity view.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_view_present(struct cupertino_activity_view *view);

/**
 * @brief Dismisses the share sheet modal presentation.
 *
 * @param view Target activity view.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_view_dismiss(struct cupertino_activity_view *view);

/**
 * @brief Checks if the share sheet is currently presented.
 *
 * @param view Target activity view.
 * @param out_is_open Pointer to receive presented flag (1 or 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_activity_view_is_open(
    const struct cupertino_activity_view *view, int *out_is_open);

/**
 * @brief Triggers execution of an activity item by index, recording selection
 * and dismissing sheet.
 *
 * @param view Target activity view.
 * @param index 0-based index of the activity.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if index invalid,
 * or UI_ERROR_INVALID_ARGUMENT if item is disabled.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_view_perform_activity(struct cupertino_activity_view *view,
                                         size_t index);

/**
 * @brief Gets index of last activated activity item.
 *
 * @param view Target activity view.
 * @param out_index Pointer to receive index (-1 if none).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_view_get_last_activated_index(
    const struct cupertino_activity_view *view, int *out_index);

/**
 * @brief Gets current shared text and url payloads.
 *
 * @param view Target activity view.
 * @param out_text Pointer to receive shared text pointer.
 * @param out_url Pointer to receive shared URL pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_activity_view_get_share_content(
    const struct cupertino_activity_view *view, const char **out_text,
    const char **out_url);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_ACTIVITY_VIEW_H */
