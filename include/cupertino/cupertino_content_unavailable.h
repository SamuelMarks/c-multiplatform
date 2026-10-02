/**
 * @file cupertino_content_unavailable.h
 * @brief Cupertino Content Unavailable View (UIContentUnavailableView) empty
 * state container.
 */

#ifndef CUPERTINO_CUPERTINO_CONTENT_UNAVAILABLE_H
#define CUPERTINO_CUPERTINO_CONTENT_UNAVAILABLE_H

/* clang-format off */
#include "ui_empty_state_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_CONTENT_UNAVAILABLE_HERO_ICON_SIZE 48.0f
#define CUPERTINO_CONTENT_UNAVAILABLE_DEFAULT_WIDTH 320.0f
#define CUPERTINO_CONTENT_UNAVAILABLE_DEFAULT_HEIGHT 240.0f

/**
 * @enum cupertino_content_unavailable_style
 * @brief Visual presentation style of the empty state.
 */
enum cupertino_content_unavailable_style {
  CUPERTINO_CONTENT_UNAVAILABLE_STANDARD =
      0, /**< Standard empty state with hero symbol. */
  CUPERTINO_CONTENT_UNAVAILABLE_SEARCH /**< Search empty state highlighting
                                          query string. */
};

/**
 * @struct cupertino_content_unavailable_descriptor
 * @brief Configuration descriptor for creating a content unavailable view.
 */
struct cupertino_content_unavailable_descriptor {
  enum cupertino_content_unavailable_style style; /**< Style variant. */
  const char *symbol_name;      /**< SF Symbol glyph name (e.g. "tray"). */
  const char *title;            /**< Primary bold headline. */
  const char *description_text; /**< Secondary explanatory text. */
  const char *action_title;     /**< Optional button label (or NULL). */
  const char *search_query;     /**< Search term for search variant. */
};

/**
 * @struct cupertino_content_unavailable_view
 * @brief Cupertino empty state view instance wrapping ui_empty_state_base.
 */
struct cupertino_content_unavailable_view {
  struct ui_empty_state_base *base;
  enum cupertino_content_unavailable_style style;
  char symbol_name[64];
  char title[128];
  char description_text[256];
  char action_title[64];
  char search_query[64];
  int action_triggered_count;
  float width;
  float height;
};

/**
 * @brief Creates a new Cupertino Content Unavailable empty state view.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_view Pointer to receive newly created view instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_content_unavailable_create(
    struct ui_engine *engine,
    const struct cupertino_content_unavailable_descriptor *desc,
    struct cupertino_content_unavailable_view **out_view);

/**
 * @brief Destroys a Cupertino Content Unavailable view.
 *
 * @param view Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_content_unavailable_destroy(
    struct cupertino_content_unavailable_view *view);

/**
 * @brief Updates primary headline title.
 *
 * @param view Target view.
 * @param title New title string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_content_unavailable_set_title(
    struct cupertino_content_unavailable_view *view, const char *title);

/**
 * @brief Gets current headline title.
 *
 * @param view Target view.
 * @param out_title Pointer to receive const string pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_content_unavailable_get_title(
    const struct cupertino_content_unavailable_view *view,
    const char **out_title);

/**
 * @brief Updates secondary description text.
 *
 * @param view Target view.
 * @param description_text New description string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_content_unavailable_set_description(
    struct cupertino_content_unavailable_view *view,
    const char *description_text);

/**
 * @brief Gets current secondary description text.
 *
 * @param view Target view.
 * @param out_description Pointer to receive const string pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_content_unavailable_get_description(
    const struct cupertino_content_unavailable_view *view,
    const char **out_description);

/**
 * @brief Updates search query string for search style empty state.
 *
 * @param view Target view.
 * @param search_query Search term (or NULL).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_content_unavailable_set_search_query(
    struct cupertino_content_unavailable_view *view, const char *search_query);

/**
 * @brief Simulates clicking the primary action CTA button.
 *
 * @param view Target view.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_content_unavailable_trigger_action(
    struct cupertino_content_unavailable_view *view);

/**
 * @brief Gets dimensions of the empty state container.
 *
 * @param view Target view.
 * @param out_width Pointer to receive width in points.
 * @param out_height Pointer to receive height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_content_unavailable_get_dimensions(
    const struct cupertino_content_unavailable_view *view, float *out_width,
    float *out_height);

/**
 * @brief Retrieves underlying CDK empty state base.
 *
 * @param view Target view.
 * @param out_base Pointer to receive ui_empty_state_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_content_unavailable_get_base(
    struct cupertino_content_unavailable_view *view,
    struct ui_empty_state_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_CONTENT_UNAVAILABLE_H */
