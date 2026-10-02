/**
 * @file cupertino_list_section.h
 * @brief Cupertino List Section component conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_LIST_SECTION_H
#define CUPERTINO_CUPERTINO_LIST_SECTION_H

/* clang-format off */
#include "cupertino/cupertino_list_tile.h"
#include "ui_list_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_LIST_SECTION_MAX_TILES 32
#define CUPERTINO_LIST_SECTION_INSET_MARGIN 16.0f
#define CUPERTINO_LIST_SECTION_CORNER_RADIUS 10.0f
#define CUPERTINO_LIST_SECTION_DEFAULT_INSET 16.0f
#define CUPERTINO_LIST_SECTION_ICON_INSET 56.0f

/**
 * @enum cupertino_list_section_style
 * @brief Plain edge-to-edge vs Inset Grouped presentation.
 */
enum cupertino_list_section_style {
  CUPERTINO_LIST_SECTION_PLAIN = 0,    /**< Edge-to-edge plain style. */
  CUPERTINO_LIST_SECTION_INSET_GROUPED /**< Rounded squircle card with 16pt
                                          margin. */
};

/**
 * @struct cupertino_list_section_descriptor
 * @brief Configuration descriptor for creating a Cupertino List Section.
 */
struct cupertino_list_section_descriptor {
  enum cupertino_list_section_style style; /**< Section visual style. */
  const char *header; /**< Uppercase section header text. */
  const char *footer; /**< Section footer description text. */
};

/**
 * @struct cupertino_list_section
 * @brief Cupertino List Section instance wrapping ui_list_base.
 */
struct cupertino_list_section {
  struct ui_list_base *base;               /**< CDK list primitive. */
  enum cupertino_list_section_style style; /**< Visual style. */
  char header[128];                        /**< Header text string. */
  char footer[256];                        /**< Footer text string. */
  size_t tile_count;                       /**< Number of tiles. */
  struct cupertino_list_tile
      *tiles[CUPERTINO_LIST_SECTION_MAX_TILES]; /**< Tiles array. */
};

/**
 * @brief Creates a new Cupertino list section instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_section Pointer to receive newly created section.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_section_create(
    struct ui_engine *engine,
    const struct cupertino_list_section_descriptor *desc,
    struct cupertino_list_section **out_section);

/**
 * @brief Destroys a Cupertino list section instance.
 *
 * @param section Section instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_list_section_destroy(struct cupertino_list_section *section);

/**
 * @brief Sets section header text.
 *
 * @param section Section instance.
 * @param header Header string, or NULL to clear.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_section_set_header(
    struct cupertino_list_section *section, const char *header);

/**
 * @brief Retrieves section header text.
 *
 * @param section Section instance.
 * @param out_header Pointer to receive const pointer to header string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_section_get_header(
    const struct cupertino_list_section *section, const char **out_header);

/**
 * @brief Sets section footer text.
 *
 * @param section Section instance.
 * @param footer Footer string, or NULL to clear.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_section_set_footer(
    struct cupertino_list_section *section, const char *footer);

/**
 * @brief Retrieves section footer text.
 *
 * @param section Section instance.
 * @param out_footer Pointer to receive const pointer to footer string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_section_get_footer(
    const struct cupertino_list_section *section, const char **out_footer);

/**
 * @brief Appends a list tile to the section.
 *
 * @param section Section instance.
 * @param tile Tile to append.
 * @param out_index Pointer to receive index of added tile.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_OUT_OF_MEMORY.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_section_add_tile(
    struct cupertino_list_section *section, struct cupertino_list_tile *tile,
    size_t *out_index);

/**
 * @brief Retrieves total tile count in the section.
 *
 * @param section Section instance.
 * @param out_count Pointer to receive tile count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_section_get_tile_count(
    const struct cupertino_list_section *section, size_t *out_count);

/**
 * @brief Retrieves tile at specified index.
 *
 * @param section Section instance.
 * @param index Tile index.
 * @param out_tile Pointer to receive cupertino_list_tile pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_section_get_tile(
    const struct cupertino_list_section *section, size_t index,
    struct cupertino_list_tile **out_tile);

/**
 * @brief Determines if separator line below tile index is visible.
 *
 * In Inset Grouped style, the last tile hides the bottom separator.
 *
 * @param section Section instance.
 * @param index Tile index.
 * @param out_visible Pointer to receive 1 if visible, 0 if hidden.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_list_section_is_separator_visible(
    const struct cupertino_list_section *section, size_t index,
    int *out_visible);

/**
 * @brief Computes leading inset for hairline separator in points.
 *
 * Typically 56.0pt when leading icon is present, 16.0pt otherwise.
 *
 * @param section Section instance.
 * @param index Tile index.
 * @param out_inset_leading Pointer to receive leading inset in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_list_section_get_separator_inset(
    const struct cupertino_list_section *section, size_t index,
    float *out_inset_leading);

/**
 * @brief Retrieves underlying CDK list base primitive.
 *
 * @param section Section instance.
 * @param out_base Pointer to receive ui_list_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_list_section_get_base(
    struct cupertino_list_section *section, struct ui_list_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_LIST_SECTION_H */
