/**
 * @file cupertino_sheet.h
 * @brief Cupertino Bottom Sheet & Action Sheet component conforming to Apple
 * HIG.
 */

#ifndef CUPERTINO_CUPERTINO_SHEET_H
#define CUPERTINO_CUPERTINO_SHEET_H

/* clang-format off */
#include "ui_bottom_sheet_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_SHEET_MAX_ACTIONS 16
#define CUPERTINO_SHEET_GRABBER_WIDTH 36.0f
#define CUPERTINO_SHEET_GRABBER_HEIGHT 5.0f

/**
 * @enum cupertino_sheet_mode
 * @brief Mode of presentation for a Cupertino sheet.
 */
enum cupertino_sheet_mode {
  CUPERTINO_SHEET_MODE_BOTTOM_SHEET =
      0, /**< Standard detented bottom modal sheet. */
  CUPERTINO_SHEET_MODE_ACTION_SHEET /**< Action sheet with grouped items and
                                       cancel pill. */
};

/**
 * @enum cupertino_sheet_detent
 * @brief Presentation detents according to Apple HIG.
 */
enum cupertino_sheet_detent {
  CUPERTINO_SHEET_DETENT_MEDIUM = 0, /**< Covers ~50% of screen height. */
  CUPERTINO_SHEET_DETENT_LARGE,      /**< Expands to full height below safe area
                                        (~90-95%). */
  CUPERTINO_SHEET_DETENT_CUSTOM      /**< Custom height specified in points. */
};

/**
 * @struct cupertino_sheet_action
 * @brief Action item within an action sheet.
 */
struct cupertino_sheet_action {
  char title[64];     /**< Action title label. */
  int is_destructive; /**< 1 for SystemRed destructive text. */
  int is_disabled;    /**< 1 if disabled. */
};

/**
 * @struct cupertino_sheet_descriptor
 * @brief Configuration descriptor for creating a Cupertino sheet.
 */
struct cupertino_sheet_descriptor {
  enum cupertino_sheet_mode mode;     /**< Presentation mode. */
  enum cupertino_sheet_detent detent; /**< Initial detent. */
  float custom_detent_height;         /**< Height if detent is CUSTOM. */
  int show_drag_grabber;              /**< 1 to render top capsule grabber. */
  const char *title;                  /**< Optional title (for action sheet). */
  const char *message; /**< Optional message (for action sheet). */
};

/**
 * @struct cupertino_sheet
 * @brief Cupertino Sheet instance wrapping ui_bottom_sheet_base.
 */
struct cupertino_sheet {
  struct ui_bottom_sheet_base *base;  /**< CDK bottom sheet primitive. */
  enum cupertino_sheet_mode mode;     /**< Bottom sheet vs action sheet. */
  enum cupertino_sheet_detent detent; /**< Active presentation detent. */
  float custom_detent_height;         /**< Height for custom detent. */
  int show_drag_grabber;              /**< Drag grabber visibility. */
  int is_open;                        /**< Presentation open status. */
  char title[128];                    /**< Action sheet header title. */
  char message[256];                  /**< Action sheet body message. */
  char cancel_title[64];              /**< Isolated cancel button title. */
  int has_cancel_action;              /**< 1 if cancel action configured. */
  enum cupertino_sheet_detent
      largest_undimmed_detent; /**< Largest undimmed detent. */
  int passthrough_enabled;     /**< Non-modal pass-through flag. */
  size_t action_count;         /**< Action items count. */
  struct cupertino_sheet_action
      actions[CUPERTINO_SHEET_MAX_ACTIONS]; /**< Actions. */
};

/**
 * @brief Creates a new Cupertino sheet instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_sheet Pointer to receive newly created sheet.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_sheet_create(
    struct ui_engine *engine, const struct cupertino_sheet_descriptor *desc,
    struct cupertino_sheet **out_sheet);

/**
 * @brief Destroys a Cupertino sheet instance.
 *
 * @param sheet Sheet instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_sheet_destroy(struct cupertino_sheet *sheet);

/**
 * @brief Sets the open/visible presentation state.
 *
 * @param sheet Target sheet instance.
 * @param is_open 1 to open, 0 to close.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_sheet_set_open(struct cupertino_sheet *sheet, int is_open);

/**
 * @brief Queries whether the sheet is open.
 *
 * @param sheet Target sheet instance.
 * @param out_is_open Pointer to receive 1 if open, 0 if closed.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_sheet_is_open(const struct cupertino_sheet *sheet, int *out_is_open);

/**
 * @brief Changes the active presentation detent.
 *
 * @param sheet Target sheet instance.
 * @param detent New detent (Medium, Large, Custom).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_sheet_set_detent(
    struct cupertino_sheet *sheet, enum cupertino_sheet_detent detent);

/**
 * @brief Queries the active presentation detent.
 *
 * @param sheet Target sheet instance.
 * @param out_detent Pointer to receive current detent.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_sheet_get_detent(const struct cupertino_sheet *sheet,
                           enum cupertino_sheet_detent *out_detent);

/**
 * @brief Configures custom detent height in points.
 *
 * @param sheet Target sheet instance.
 * @param height Height in points (> 0.0f).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_sheet_set_custom_detent_height(struct cupertino_sheet *sheet,
                                         float height);

/**
 * @brief Computes effective sheet height in points given the screen/viewport
 * height.
 *
 * @param sheet Target sheet instance.
 * @param viewport_height Full screen/viewport height in points.
 * @param out_height Pointer to receive computed sheet height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_sheet_get_computed_height(const struct cupertino_sheet *sheet,
                                    float viewport_height, float *out_height);

/**
 * @brief Computes parent view scaling and corner radius during card
 * presentation.
 *
 * As the sheet opens (progress 0.0 to 1.0), parent view scales from 1.0 down to
 * ~0.92, top corners round from 0 to 12pt, and background dims.
 *
 * @param sheet Target sheet instance.
 * @param progress Open animation progress in [0.0, 1.0].
 * @param out_scale Pointer to receive scale factor (~0.92 to 1.0).
 * @param out_corner_radius Pointer to receive corner radius in points (0
 * to 12.0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_sheet_get_parent_card_scale(
    const struct cupertino_sheet *sheet, float progress, float *out_scale,
    float *out_corner_radius);

/**
 * @brief Appends an action item to an action sheet.
 *
 * @param sheet Target sheet instance.
 * @param title Action label text.
 * @param is_destructive 1 for red destructive styling, 0 for blue.
 * @param out_index Pointer to receive action index.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_OUT_OF_MEMORY.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_sheet_add_action(struct cupertino_sheet *sheet, const char *title,
                           int is_destructive, size_t *out_index);

/**
 * @brief Configures the isolated Cancel action button separated by an 8pt gap
 * at bottom.
 *
 * @param sheet Target sheet instance.
 * @param title Cancel action title (e.g. "Cancel").
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_sheet_set_cancel_action(
    struct cupertino_sheet *sheet, const char *title);

/**
 * @brief Retrieves total number of action items.
 *
 * @param sheet Target sheet instance.
 * @param out_count Pointer to receive action count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_sheet_get_action_count(
    const struct cupertino_sheet *sheet, size_t *out_count);

/**
 * @brief Retrieves the underlying CDK bottom sheet base primitive.
 *
 * @param sheet Target sheet instance.
 * @param out_base Pointer to receive ui_bottom_sheet_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_sheet_get_base(
    struct cupertino_sheet *sheet, struct ui_bottom_sheet_base **out_base);

/**
 * @brief Configures non-modal background pass-through interaction and largest
 * undimmed detent.
 *
 * @param sheet Target sheet instance.
 * @param detent The largest detent that permits pass-through.
 * @param enabled 1 to enable non-modal pass-through, 0 for modal dimming.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_sheet_set_largest_undimmed_detent(struct cupertino_sheet *sheet,
                                            enum cupertino_sheet_detent detent,
                                            int enabled);

/**
 * @brief Queries current largest undimmed detent and pass-through
 * configuration.
 *
 * @param sheet Target sheet instance.
 * @param out_detent Pointer to receive detent.
 * @param out_enabled Pointer to receive enabled flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_sheet_get_largest_undimmed_detent(
    const struct cupertino_sheet *sheet,
    enum cupertino_sheet_detent *out_detent, int *out_enabled);

/**
 * @brief Checks if pass-through interaction is currently active given sheet
 * detent and open state.
 *
 * @param sheet Target sheet instance.
 * @param out_active Pointer to receive 1 if background interaction is
 * permitted, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_sheet_is_passthrough_active(
    const struct cupertino_sheet *sheet, int *out_active);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SHEET_H */
