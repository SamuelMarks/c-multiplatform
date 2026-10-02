/**
 * @file cupertino_spell_check_toolbar.h
 * @brief iOS Spell Check Suggestions Popover Toolbar conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_SPELL_CHECK_TOOLBAR_H
#define CUPERTINO_CUPERTINO_SPELL_CHECK_TOOLBAR_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_SPELL_CHECK_MAX_SUGGESTIONS 5
#define CUPERTINO_SPELL_CHECK_TOOLBAR_HEIGHT 44.0f

/**
 * @struct cupertino_spell_check_toolbar_descriptor
 * @brief Initialization descriptor for the spell check suggestions popover.
 */
struct cupertino_spell_check_toolbar_descriptor {
  int allow_add_to_dictionary; /**< 1 if "Add to Dictionary" option shown. */
};

/**
 * @struct cupertino_spell_check_toolbar
 * @brief Instance managing iOS spell check replacement suggestions bubble.
 */
struct cupertino_spell_check_toolbar {
  int is_visible;              /**< Popover visibility flag. */
  int allow_add_to_dictionary; /**< Add to dictionary enabled flag. */
  char misspelled_word[64];    /**< The target misspelled text. */
  char suggestions[CUPERTINO_SPELL_CHECK_MAX_SUGGESTIONS]
                  [64];    /**< Candidates. */
  size_t suggestion_count; /**< Number of active suggestions. */
  float target_x;          /**< Misspelled word bounding box X. */
  float target_y;          /**< Misspelled word bounding box Y. */
  float target_w;          /**< Misspelled word bounding box width. */
  float target_h;          /**< Misspelled word bounding box height. */
  float bubble_x;          /**< Popover bubble top-left X in points. */
  float bubble_y;          /**< Popover bubble top-left Y in points. */
  float bubble_w;          /**< Popover bubble width in points. */
  float bubble_h;          /**< Popover bubble height in points. */
  int arrow_on_bottom;     /**< 1 if arrow points down towards text. */
};

/**
 * @brief Creates a new Cupertino Spell Check Suggestions toolbar.
 *
 * @param engine Pointer to ui_engine instance.
 * @param desc Configuration descriptor.
 * @param out_toolbar Pointer to receive allocated toolbar.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spell_check_toolbar_create(
    struct ui_engine *engine,
    const struct cupertino_spell_check_toolbar_descriptor *desc,
    struct cupertino_spell_check_toolbar **out_toolbar);

/**
 * @brief Destroys a spell check suggestions toolbar.
 *
 * @param toolbar Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spell_check_toolbar_destroy(
    struct cupertino_spell_check_toolbar *toolbar);

/**
 * @brief Recomputes layout and bubble bounds for the spell check toolbar.
 *
 * @param toolbar Target toolbar.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spell_check_recompute_bounds(
    struct cupertino_spell_check_toolbar *toolbar);

/**
 * @brief Displays the suggestion popover anchored over misspelled text.
 *
 * @param toolbar Target toolbar.
 * @param target_x Top-left X of misspelled text.
 * @param target_y Top-left Y of misspelled text.
 * @param target_w Width of misspelled text.
 * @param target_h Height of misspelled text.
 * @param misspelled_word Original misspelled word.
 * @param suggestions Array of string suggestions.
 * @param suggestion_count Number of candidate suggestions.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spell_check_toolbar_show(
    struct cupertino_spell_check_toolbar *toolbar, float target_x,
    float target_y, float target_w, float target_h, const char *misspelled_word,
    const char **suggestions, size_t suggestion_count);

/**
 * @brief Hides the suggestions popover.
 *
 * @param toolbar Target toolbar.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_spell_check_toolbar_hide(
    struct cupertino_spell_check_toolbar *toolbar);

/**
 * @brief Checks if the toolbar is currently visible.
 *
 * @param toolbar Target toolbar.
 * @param out_visible Pointer to receive visibility flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spell_check_toolbar_is_visible(
    const struct cupertino_spell_check_toolbar *toolbar, int *out_visible);

/**
 * @brief Gets the number of displayed suggestions.
 *
 * @param toolbar Target toolbar.
 * @param out_count Pointer to receive suggestion count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spell_check_toolbar_get_suggestion_count(
    const struct cupertino_spell_check_toolbar *toolbar, size_t *out_count);

/**
 * @brief Selects a suggestion candidate to replace the misspelled text.
 *
 * @param toolbar Target toolbar.
 * @param index Suggestion index.
 * @param out_replacement Pointer to receive chosen replacement string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spell_check_toolbar_select_suggestion(
    struct cupertino_spell_check_toolbar *toolbar, size_t index,
    const char **out_replacement);

/**
 * @brief Invokes the "Add to Dictionary" action.
 *
 * @param toolbar Target toolbar.
 * @param out_added Pointer to receive 1 if added and dismissed, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spell_check_toolbar_add_to_dictionary(
    struct cupertino_spell_check_toolbar *toolbar, int *out_added);

/**
 * @brief Gets the target misspelled word.
 *
 * @param toolbar Target toolbar.
 * @param out_word Pointer to receive pointer to misspelled word string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spell_check_toolbar_get_misspelled_word(
    const struct cupertino_spell_check_toolbar *toolbar, const char **out_word);

/**
 * @brief Gets the layout bounds of the suggestion popover bubble.
 *
 * @param toolbar Target toolbar.
 * @param out_x Pointer to receive top-left X in points.
 * @param out_y Pointer to receive top-left Y in points.
 * @param out_w Pointer to receive bubble width in points.
 * @param out_h Pointer to receive bubble height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_spell_check_toolbar_get_bounds(
    const struct cupertino_spell_check_toolbar *toolbar, float *out_x,
    float *out_y, float *out_w, float *out_h);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SPELL_CHECK_TOOLBAR_H */
