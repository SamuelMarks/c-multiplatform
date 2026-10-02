/**
 * @file cupertino_photo_picker.h
 * @brief Privacy-preserving Apple Photo Picker (PHPickerViewController) sheet.
 */

#ifndef CUPERTINO_CUPERTINO_PHOTO_PICKER_H
#define CUPERTINO_CUPERTINO_PHOTO_PICKER_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_PHOTO_PICKER_MAX_ASSETS 64

/**
 * @enum cupertino_photo_filter
 * @brief Filter type for media assets.
 */
enum cupertino_photo_filter {
  CUPERTINO_PHOTO_FILTER_ALL = 0,    /**< All photos and videos. */
  CUPERTINO_PHOTO_FILTER_IMAGES,     /**< Static images only. */
  CUPERTINO_PHOTO_FILTER_VIDEOS,     /**< Video media only. */
  CUPERTINO_PHOTO_FILTER_LIVE_PHOTOS /**< Live Photos. */
};

/**
 * @struct cupertino_photo_item
 * @brief Single media asset in the picker grid.
 */
struct cupertino_photo_item {
  char id[64];
  char title[64];
  int is_video;
  int is_selected;
  int selection_order; /**< 1-based order badge number. */
  int is_visible;      /**< Visibility after search filter. */
};

/**
 * @struct cupertino_photo_picker_descriptor
 * @brief Configuration descriptor for photo picker sheet.
 */
struct cupertino_photo_picker_descriptor {
  int selection_limit; /**< 0 for unlimited, or maximum selectable count. */
  enum cupertino_photo_filter filter; /**< Initial media filter. */
};

/**
 * @struct cupertino_photo_picker
 * @brief Photo picker instance.
 */
struct cupertino_photo_picker {
  int selection_limit;
  enum cupertino_photo_filter filter;
  int asset_count;
  struct cupertino_photo_item assets[CUPERTINO_PHOTO_PICKER_MAX_ASSETS];
  int selected_count;
  float width;
  float height;
};

/**
 * @brief Creates a new Cupertino Photo Picker instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_picker Pointer to receive newly created picker instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_photo_picker_create(
    struct ui_engine *engine,
    const struct cupertino_photo_picker_descriptor *desc,
    struct cupertino_photo_picker **out_picker);

/**
 * @brief Destroys a Cupertino Photo Picker instance.
 *
 * @param picker Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_photo_picker_destroy(struct cupertino_photo_picker *picker);

/**
 * @brief Adds a media asset to the photo picker library.
 *
 * @param picker Target photo picker.
 * @param id Unique asset identifier.
 * @param title Display title / filename.
 * @param is_video 1 if video asset, 0 if photo.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_photo_picker_add_asset(
    struct cupertino_photo_picker *picker, const char *id, const char *title,
    int is_video);

/**
 * @brief Toggles selection state of an asset by index.
 *
 * @param picker Target photo picker.
 * @param asset_index Index of asset in library.
 * @param out_is_selected Pointer to receive updated selection state (1 or 0).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_photo_picker_toggle_selection(struct cupertino_photo_picker *picker,
                                        int asset_index, int *out_is_selected);

/**
 * @brief Gets the total count of currently selected assets.
 *
 * @param picker Target photo picker.
 * @param out_count Pointer to receive count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_photo_picker_get_selected_count(
    const struct cupertino_photo_picker *picker, int *out_count);

/**
 * @brief Gets configured selection limit.
 *
 * @param picker Target photo picker.
 * @param out_limit Pointer to receive selection limit.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_photo_picker_get_selection_limit(
    const struct cupertino_photo_picker *picker, int *out_limit);

/**
 * @brief Clears all current asset selections.
 *
 * @param picker Target photo picker.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_photo_picker_clear_selection(struct cupertino_photo_picker *picker);

/**
 * @brief Sets the media type filter.
 *
 * @param picker Target photo picker.
 * @param filter New filter type.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_photo_picker_set_filter(
    struct cupertino_photo_picker *picker, enum cupertino_photo_filter filter);

/**
 * @brief Gets the current media type filter.
 *
 * @param picker Target photo picker.
 * @param out_filter Pointer to receive filter type.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_photo_picker_get_filter(const struct cupertino_photo_picker *picker,
                                  enum cupertino_photo_filter *out_filter);

/**
 * @brief Filters assets by text query string.
 *
 * @param picker Target photo picker.
 * @param query Query search text (or NULL to reset).
 * @param out_matched_count Pointer to receive count of visible assets.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_photo_picker_filter_search(struct cupertino_photo_picker *picker,
                                     const char *query, int *out_matched_count);

/**
 * @brief Gets dimensions of the photo picker sheet.
 *
 * @param picker Target photo picker.
 * @param out_width Pointer to receive width.
 * @param out_height Pointer to receive height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_photo_picker_get_dimensions(
    const struct cupertino_photo_picker *picker, float *out_width,
    float *out_height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_PHOTO_PICKER_H */
