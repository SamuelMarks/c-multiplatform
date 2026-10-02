/**
 * @file cupertino_quick_look.h
 * @brief Quick Look Document Preview Sheet (QLPreviewController) conforming to
 * Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_QUICK_LOOK_H
#define CUPERTINO_CUPERTINO_QUICK_LOOK_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum cupertino_ql_preview_item_type
 * @brief Preview item document type supported by Quick Look.
 */
enum cupertino_ql_preview_item_type {
  CUPERTINO_QL_TYPE_DOCUMENT = 0, /**< PDF, Pages, Office documents. */
  CUPERTINO_QL_TYPE_IMAGE, /**< Raster/vector images (PNG, JPEG, HEIC, SVG). */
  CUPERTINO_QL_TYPE_AUDIO, /**< Audio files and waveforms. */
  CUPERTINO_QL_TYPE_VIDEO  /**< Video media players. */
};

/**
 * @enum cupertino_ql_markup_tool
 * @brief Active markup annotation tool in Quick Look bottom toolbar.
 */
enum cupertino_ql_markup_tool {
  CUPERTINO_QL_MARKUP_NONE = 0,    /**< No tool active / markup closed. */
  CUPERTINO_QL_MARKUP_PEN,         /**< Standard drawing pen. */
  CUPERTINO_QL_MARKUP_HIGHLIGHTER, /**< Semi-translucent highlighter. */
  CUPERTINO_QL_MARKUP_ERASER,      /**< Object/stroke eraser. */
  CUPERTINO_QL_MARKUP_SIGNATURE    /**< Vector signature tool. */
};

/**
 * @def CUPERTINO_QL_MAX_ITEMS
 * @brief Maximum preview items managed simultaneously in a Quick Look session.
 */
#define CUPERTINO_QL_MAX_ITEMS 32

/**
 * @struct cupertino_ql_preview_item
 * @brief Single preview document item entry.
 */
struct cupertino_ql_preview_item {
  char url[256];   /**< Document file URL or path. */
  char title[128]; /**< Title displayed in top navigation bar. */
  enum cupertino_ql_preview_item_type
      item_type;      /**< Document format classification. */
  size_t page_count;  /**< Page count for documents/PDF, or frame count. */
  float duration_sec; /**< Duration in seconds for audio/video. */
};

/**
 * @struct cupertino_quick_look_descriptor
 * @brief Configuration descriptor for Quick Look preview sheet.
 */
struct cupertino_quick_look_descriptor {
  int is_markup_enabled; /**< Non-zero to show markup pencil icon & bottom bar.
                          */
  int is_share_enabled;  /**< Non-zero to enable Share / AirDrop button in
                            header. */
  float initial_zoom;    /**< Initial zoom scale factor (default 1.0f). */
};

/**
 * @struct cupertino_quick_look
 * @brief Quick Look preview controller state structure.
 */
struct cupertino_quick_look {
  int is_open;           /**< Non-zero if sheet is presented. */
  int is_markup_open;    /**< Non-zero if markup annotation tools are open. */
  int is_share_enabled;  /**< AirDrop / share enabled. */
  int is_markup_enabled; /**< Markup allowed. */
  enum cupertino_ql_markup_tool
      active_tool;      /**< Currently active annotation tool. */
  float current_zoom;   /**< Current pinch zoom level (1.0f = 100%). */
  float min_zoom;       /**< Min clamped zoom (e.g. 0.5f). */
  float max_zoom;       /**< Max clamped zoom (e.g. 5.0f). */
  size_t item_count;    /**< Number of preview documents loaded. */
  size_t current_index; /**< Active preview document index. */
  struct cupertino_ql_preview_item
      items[CUPERTINO_QL_MAX_ITEMS]; /**< Items table. */
  int share_invoked_count; /**< Number of times AirDrop / Share was triggered.
                            */
  int done_invoked_count;  /**< Number of times Done was triggered. */
};

/**
 * @brief Creates a new Cupertino Quick Look preview sheet instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_ql Pointer to receive newly created quick look instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_quick_look_create(struct ui_engine *engine,
                            const struct cupertino_quick_look_descriptor *desc,
                            struct cupertino_quick_look **out_ql);

/**
 * @brief Destroys a Cupertino Quick Look preview sheet instance.
 *
 * @param ql Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_quick_look_destroy(struct cupertino_quick_look *ql);

/**
 * @brief Adds a document item to the Quick Look preview stack.
 *
 * @param ql Target Quick Look sheet.
 * @param url Document file URL or path.
 * @param title Navigation bar title string.
 * @param item_type Type of document.
 * @param page_count Page count (for documents) or 0.
 * @param duration_sec Duration in seconds (for audio/video) or 0.0f.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_quick_look_add_item(
    struct cupertino_quick_look *ql, const char *url, const char *title,
    enum cupertino_ql_preview_item_type item_type, size_t page_count,
    float duration_sec);

/**
 * @brief Sets the active preview document index.
 *
 * @param ql Target Quick Look sheet.
 * @param index 0-based document item index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_quick_look_set_current_index(
    struct cupertino_quick_look *ql, size_t index);

/**
 * @brief Gets the current document item in view.
 *
 * @param ql Target Quick Look sheet.
 * @param out_item Pointer to receive pointer to active preview item.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_quick_look_get_current_item(
    const struct cupertino_quick_look *ql,
    const struct cupertino_ql_preview_item **out_item);

/**
 * @brief Opens or dismisses the Quick Look preview sheet.
 *
 * @param ql Target Quick Look sheet.
 * @param is_open Non-zero to present, 0 to dismiss.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_quick_look_set_open(struct cupertino_quick_look *ql, int is_open);

/**
 * @brief Queries whether the Quick Look preview sheet is currently presented.
 *
 * @param ql Target Quick Look sheet.
 * @param out_is_open Pointer to receive presentation state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_quick_look_is_open(
    const struct cupertino_quick_look *ql, int *out_is_open);

/**
 * @brief Toggles the markup annotation toolbar at the bottom of the sheet.
 *
 * @param ql Target Quick Look sheet.
 * @param is_open Non-zero to open markup bar, 0 to close.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_quick_look_set_markup_open(
    struct cupertino_quick_look *ql, int is_open);

/**
 * @brief Sets the active markup annotation tool.
 *
 * @param ql Target Quick Look sheet.
 * @param tool Tool to activate.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_quick_look_set_markup_tool(
    struct cupertino_quick_look *ql, enum cupertino_ql_markup_tool tool);

/**
 * @brief Gets the active markup annotation tool.
 *
 * @param ql Target Quick Look sheet.
 * @param out_tool Pointer to receive active tool.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_quick_look_get_markup_tool(const struct cupertino_quick_look *ql,
                                     enum cupertino_ql_markup_tool *out_tool);

/**
 * @brief Applies pinch-to-zoom magnification delta with spring bounds clamping.
 *
 * @param ql Target Quick Look sheet.
 * @param zoom_delta Multiplicative scale delta (e.g. 1.1f for 10% zoom in).
 * @param out_clamped_zoom Pointer to receive final clamped zoom scale.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_quick_look_apply_pinch_zoom(
    struct cupertino_quick_look *ql, float zoom_delta, float *out_clamped_zoom);

/**
 * @brief Triggers the Share / AirDrop system action.
 *
 * @param ql Target Quick Look sheet.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_quick_look_trigger_share(struct cupertino_quick_look *ql);

/**
 * @brief Triggers the Done button action to dismiss the sheet.
 *
 * @param ql Target Quick Look sheet.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_quick_look_trigger_done(struct cupertino_quick_look *ql);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_QUICK_LOOK_H */
