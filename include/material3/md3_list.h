/**
 * @file md3_list.h
 * @brief Material 3 List and List Item components wrapping ui_list_base.
 */

#ifndef MATERIAL3_MD3_LIST_H
#define MATERIAL3_MD3_LIST_H

/* clang-format off */
#include "ui_error.h"
#include "ui_list_base.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md3_list_item_lines
 * @brief Material 3 list item line count density metrics.
 */
enum md3_list_item_lines {
  MD3_LIST_ITEM_ONE_LINE = 1,  /**< Single-line: 56dp height */
  MD3_LIST_ITEM_TWO_LINE = 2,  /**< Two-line: 72dp height */
  MD3_LIST_ITEM_THREE_LINE = 3 /**< Three-line: 88dp height */
};

/**
 * @struct md3_list
 * @brief Material 3 list container wrapping ui_list_base.
 */
struct md3_list {
  struct ui_list_base *base;
  int is_segmented;
};

/**
 * @struct md3_list_item
 * @brief Material 3 list item wrapping ui_list_item_base.
 */
struct md3_list_item {
  struct ui_list_item_base *base;
  enum md3_list_item_lines lines;
  char headline[128];
  char supporting_text[256];
  char trailing_supporting_text[64];
  int selected;
  int disabled;
};

/**
 * @brief Creates a Material 3 List container.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_list Pointer to receive newly created list.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_create(struct ui_engine *engine, struct md3_list **out_list);

/**
 * @brief Destroys a Material 3 List container.
 *
 * @param list Pointer to list to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_destroy(struct md3_list *list);

/**
 * @brief Sets whether the list uses Expressive segmented styling.
 *
 * @param list The list.
 * @param is_segmented Non-zero if segmented pill styling is enabled.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_set_segmented(struct md3_list *list, int is_segmented);

/**
 * @brief Appends a Material 3 List Item to the list.
 *
 * @param list The list container.
 * @param item The list item to append.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_append_item(struct md3_list *list, struct md3_list_item *item);

/**
 * @brief Creates a Material 3 List Item.
 *
 * @param engine Pointer to ui_engine instance.
 * @param lines Line count density metric (1, 2, or 3).
 * @param out_item Pointer to receive newly created list item.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_item_create(struct ui_engine *engine, enum md3_list_item_lines lines,
                     struct md3_list_item **out_item);

/**
 * @brief Destroys a Material 3 List Item.
 *
 * @param item The list item to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_item_destroy(struct md3_list_item *item);

/**
 * @brief Sets the headline text of the list item.
 *
 * @param item The list item.
 * @param headline Headline string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_item_set_headline(struct md3_list_item *item, const char *headline);

/**
 * @brief Sets the supporting text of the list item.
 *
 * @param item The list item.
 * @param supporting_text Supporting text string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_list_item_set_supporting_text(
    struct md3_list_item *item, const char *supporting_text);

/**
 * @brief Sets the trailing supporting text (e.g. metadata or timestamp).
 *
 * @param item The list item.
 * @param trailing_text Trailing text string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_item_set_trailing_supporting_text(struct md3_list_item *item,
                                           const char *trailing_text);

/**
 * @brief Sets the selected state of the list item.
 *
 * @param item The list item.
 * @param selected Non-zero if selected.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_item_set_selected(struct md3_list_item *item, int selected);

/**
 * @brief Gets the selected state of the list item.
 *
 * @param item The list item.
 * @param out_selected Pointer to receive selected state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_list_item_get_selected(const struct md3_list_item *item, int *out_selected);

/**
 * @brief Gets the underlying component of a list item.
 *
 * @param item The list item.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_list_item_get_component(
    struct md3_list_item *item, struct ui_component **out_component);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_LIST_H */
