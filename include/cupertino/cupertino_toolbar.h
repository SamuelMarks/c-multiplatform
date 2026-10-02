/**
 * @file cupertino_toolbar.h
 * @brief Cupertino Toolbar component conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_TOOLBAR_H
#define CUPERTINO_CUPERTINO_TOOLBAR_H

/* clang-format off */
#include "ui_toolbar_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_TOOLBAR_MAX_ITEMS 16

/**
 * @enum cupertino_toolbar_item_type
 * @brief Types of items placed within a Cupertino toolbar.
 */
enum cupertino_toolbar_item_type {
  CUPERTINO_TOOLBAR_ITEM_ACTION = 0,
  CUPERTINO_TOOLBAR_ITEM_FLEXIBLE_SPACE,
  CUPERTINO_TOOLBAR_ITEM_FIXED_SPACE
};

/**
 * @struct cupertino_toolbar_item
 * @brief Configuration and state for a single toolbar item.
 */
struct cupertino_toolbar_item {
  enum cupertino_toolbar_item_type type; /**< Item variant. */
  char title[64];                        /**< Action title string. */
  float width;                           /**< Width for fixed space. */
  int is_destructive;                    /**< 1 if destructive red styling. */
  int is_disabled;                       /**< 1 if disabled. */
};

/**
 * @struct cupertino_toolbar_descriptor
 * @brief Configuration descriptor for creating a Cupertino toolbar.
 */
struct cupertino_toolbar_descriptor {
  const char *title;  /**< Optional accessible title for toolbar. */
  int is_translucent; /**< 1 for blur vibrancy material backdrop. */
};

/**
 * @struct cupertino_toolbar
 * @brief Cupertino Toolbar instance wrapping ui_toolbar_base.
 */
struct cupertino_toolbar {
  struct ui_toolbar_base *base; /**< CDK toolbar primitive. */
  int is_translucent;           /**< Translucency flag. */
  size_t item_count;            /**< Total items in toolbar. */
  struct cupertino_toolbar_item
      items[CUPERTINO_TOOLBAR_MAX_ITEMS]; /**< Items. */
};

/**
 * @brief Creates a new Cupertino toolbar instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_toolbar Pointer to receive newly created toolbar.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_toolbar_create(
    struct ui_engine *engine, const struct cupertino_toolbar_descriptor *desc,
    struct cupertino_toolbar **out_toolbar);

/**
 * @brief Destroys a Cupertino toolbar instance.
 *
 * @param toolbar Toolbar instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_toolbar_destroy(struct cupertino_toolbar *toolbar);

/**
 * @brief Appends an action button item to the toolbar.
 *
 * @param toolbar Target toolbar instance.
 * @param title Action button text label.
 * @param is_destructive 1 for destructive red text, 0 for standard tint.
 * @param is_disabled 1 if disabled, 0 if interactive.
 * @param out_index Pointer to receive index of added item.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_OUT_OF_MEMORY.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_toolbar_add_action(
    struct cupertino_toolbar *toolbar, const char *title, int is_destructive,
    int is_disabled, size_t *out_index);

/**
 * @brief Appends a flexible space (UIBarButtonSystemItemFlexibleSpace) to
 * distribute items.
 *
 * @param toolbar Target toolbar instance.
 * @param out_index Pointer to receive index of added space item.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_toolbar_add_flexible_space(
    struct cupertino_toolbar *toolbar, size_t *out_index);

/**
 * @brief Appends a fixed-width space (UIBarButtonSystemItemFixedSpace).
 *
 * @param toolbar Target toolbar instance.
 * @param width Width in points.
 * @param out_index Pointer to receive index of added space item.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_toolbar_add_fixed_space(
    struct cupertino_toolbar *toolbar, float width, size_t *out_index);

/**
 * @brief Retrieves total number of items in the toolbar.
 *
 * @param toolbar Target toolbar instance.
 * @param out_count Pointer to receive item count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_toolbar_get_item_count(
    const struct cupertino_toolbar *toolbar, size_t *out_count);

/**
 * @brief Sets disabled state of a toolbar item.
 *
 * @param toolbar Target toolbar instance.
 * @param index Item index.
 * @param is_disabled 1 for disabled, 0 for enabled.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_toolbar_set_item_disabled(
    struct cupertino_toolbar *toolbar, size_t index, int is_disabled);

/**
 * @brief Queries disabled state of a toolbar item.
 *
 * @param toolbar Target toolbar instance.
 * @param index Item index.
 * @param out_disabled Pointer to receive disabled state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_toolbar_is_item_disabled(
    const struct cupertino_toolbar *toolbar, size_t index, int *out_disabled);

/**
 * @brief Retrieves the standard toolbar height (44.0pt).
 *
 * @param toolbar Target toolbar instance.
 * @param out_height Pointer to receive 44.0f.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_toolbar_get_height(
    const struct cupertino_toolbar *toolbar, float *out_height);

/**
 * @brief Retrieves the underlying CDK toolbar base primitive.
 *
 * @param toolbar Target toolbar instance.
 * @param out_base Pointer to receive ui_toolbar_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_toolbar_get_base(
    struct cupertino_toolbar *toolbar, struct ui_toolbar_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_TOOLBAR_H */
