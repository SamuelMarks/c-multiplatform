/**
 * @file cupertino_list_tile.h
 * @brief Cupertino List Tile component conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_LIST_TILE_H
#define CUPERTINO_CUPERTINO_LIST_TILE_H

/* clang-format off */
#include "ui_list_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_LIST_TILE_HEIGHT_DEFAULT 44.0f
#define CUPERTINO_LIST_TILE_HEIGHT_SUBTITLE 54.0f

/**
 * @enum cupertino_list_tile_style
 * @brief Standard Apple HIG table view cell layout styles.
 */
enum cupertino_list_tile_style {
  CUPERTINO_LIST_TILE_DEFAULT = 0, /**< Title + optional leading image. */
  CUPERTINO_LIST_TILE_SUBTITLE, /**< Title on top, secondary subtitle below. */
  CUPERTINO_LIST_TILE_VALUE1,   /**< Title on leading side, right-aligned info
                                   trailing. */
  CUPERTINO_LIST_TILE_VALUE2 /**< Small blue category title, value on right. */
};

/**
 * @enum cupertino_list_tile_accessory
 * @brief Trailing accessory view type.
 */
enum cupertino_list_tile_accessory {
  CUPERTINO_LIST_TILE_ACCESSORY_NONE = 0,           /**< No accessory. */
  CUPERTINO_LIST_TILE_ACCESSORY_DISCLOSURE_CHEVRON, /**< Chevron pointing
                                                       trailing. */
  CUPERTINO_LIST_TILE_ACCESSORY_DETAIL_BUTTON,      /**< Info circle button. */
  CUPERTINO_LIST_TILE_ACCESSORY_CHECKMARK           /**< Selection checkmark. */
};

/**
 * @struct cupertino_list_tile_descriptor
 * @brief Configuration descriptor for creating a Cupertino List Tile.
 */
struct cupertino_list_tile_descriptor {
  enum cupertino_list_tile_style style;         /**< Layout style variant. */
  enum cupertino_list_tile_accessory accessory; /**< Trailing accessory type. */
  const char *title;                            /**< Primary label text. */
  const char *subtitle;                         /**< Secondary subtitle text. */
  const char *additional_info; /**< Right-aligned detail string (Value 1). */
  const char *leading_icon;    /**< Leading icon/glyph identifier. */
  int is_rtl;                  /**< 1 if RTL layout direction. */
};

/**
 * @struct cupertino_list_tile
 * @brief Cupertino List Tile instance wrapping ui_list_item_base.
 */
struct cupertino_list_tile {
  struct ui_list_item_base *base;               /**< CDK list item primitive. */
  enum cupertino_list_tile_style style;         /**< Cell style. */
  enum cupertino_list_tile_accessory accessory; /**< Accessory type. */
  char title[128];                              /**< Primary title string. */
  char subtitle[128];                           /**< Subtitle string. */
  char additional_info[64];                     /**< Additional info string. */
  char leading_icon[64];                        /**< Leading icon identifier. */
  int is_rtl;                                   /**< BiDi direction. */
  int is_pressed;                               /**< Press / touch active. */
  float highlight_opacity; /**< 0.0 (rest) to 0.15 (highlighted). */
  int is_stacked_layout;   /**< 1 if vertical stacked layout (AX1+ mode). */
  float
      dynamic_type_scale; /**< Scale factor (1.0 = standard, >= 1.5 = AX1+). */
};

/**
 * @brief Creates a new Cupertino list tile instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_tile Pointer to receive newly created list tile.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_create(
    struct ui_engine *engine, const struct cupertino_list_tile_descriptor *desc,
    struct cupertino_list_tile **out_tile);

/**
 * @brief Destroys a Cupertino list tile instance.
 *
 * @param tile Tile instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_list_tile_destroy(struct cupertino_list_tile *tile);

/**
 * @brief Sets primary title text.
 *
 * @param tile Tile instance.
 * @param title New title string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_set_title(
    struct cupertino_list_tile *tile, const char *title);

/**
 * @brief Retrieves primary title text.
 *
 * @param tile Tile instance.
 * @param out_title Pointer to receive const pointer to title string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_get_title(
    const struct cupertino_list_tile *tile, const char **out_title);

/**
 * @brief Sets secondary subtitle text.
 *
 * @param tile Tile instance.
 * @param subtitle New subtitle string, or NULL to clear.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_set_subtitle(
    struct cupertino_list_tile *tile, const char *subtitle);

/**
 * @brief Retrieves secondary subtitle text.
 *
 * @param tile Tile instance.
 * @param out_subtitle Pointer to receive const pointer to subtitle string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_get_subtitle(
    const struct cupertino_list_tile *tile, const char **out_subtitle);

/**
 * @brief Sets additional detail info text (Value 1 style).
 *
 * @param tile Tile instance.
 * @param info New additional info string, or NULL to clear.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_list_tile_set_additional_info(struct cupertino_list_tile *tile,
                                        const char *info);

/**
 * @brief Retrieves additional detail info text.
 *
 * @param tile Tile instance.
 * @param out_info Pointer to receive const pointer to info string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_list_tile_get_additional_info(const struct cupertino_list_tile *tile,
                                        const char **out_info);

/**
 * @brief Sets trailing accessory view type.
 *
 * @param tile Tile instance.
 * @param accessory Accessory type enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_list_tile_set_accessory(struct cupertino_list_tile *tile,
                                  enum cupertino_list_tile_accessory accessory);

/**
 * @brief Queries trailing accessory view type.
 *
 * @param tile Tile instance.
 * @param out_accessory Pointer to receive accessory type enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_get_accessory(
    const struct cupertino_list_tile *tile,
    enum cupertino_list_tile_accessory *out_accessory);

/**
 * @brief Sets touch press state for tap highlight animation.
 *
 * @param tile Tile instance.
 * @param pressed 1 if pressed/down, 0 if released.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_list_tile_set_pressed(struct cupertino_list_tile *tile, int pressed);

/**
 * @brief Queries touch press state.
 *
 * @param tile Tile instance.
 * @param out_pressed Pointer to receive press status.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_is_pressed(
    const struct cupertino_list_tile *tile, int *out_pressed);

/**
 * @brief Retrieves computed tap highlight overlay opacity.
 *
 * @param tile Tile instance.
 * @param out_opacity Pointer to receive opacity value in [0.0, 0.15].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_list_tile_get_highlight_opacity(
    const struct cupertino_list_tile *tile, float *out_opacity);

/**
 * @brief Retrieves standard height in points according to Apple HIG cell style.
 *
 * Default/Value1: 44.0pt. Subtitle: 54.0pt.
 *
 * @param tile Tile instance.
 * @param out_height Pointer to receive computed cell height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_get_height(
    const struct cupertino_list_tile *tile, float *out_height);

/**
 * @brief Retrieves underlying CDK list item base primitive.
 *
 * @param tile Tile instance.
 * @param out_base Pointer to receive ui_list_item_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_get_base(
    struct cupertino_list_tile *tile, struct ui_list_item_base **out_base);

/**
 * @brief Sets dynamic type scaling factor and auto-switches to stacked layout
 * if scale >= AX1 (>= 1.5).
 *
 * @param tile Tile instance.
 * @param scale_factor Typographic scale factor (1.0 = reference Large, >= 1.5 =
 * AX1+).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_set_scale_factor(
    struct cupertino_list_tile *tile, float scale_factor);

/**
 * @brief Checks if the list tile is currently using stacked vertical layout.
 *
 * @param tile Tile instance.
 * @param out_is_stacked Pointer to receive 1 if stacked, 0 if horizontal row.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_is_stacked(
    const struct cupertino_list_tile *tile, int *out_is_stacked);

/**
 * @brief Computes item height considering dynamic type scale and stacked
 * layout.
 *
 * @param tile Tile instance.
 * @param out_height Pointer to receive computed row height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_tile_compute_height(
    const struct cupertino_list_tile *tile, float *out_height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_LIST_TILE_H */
