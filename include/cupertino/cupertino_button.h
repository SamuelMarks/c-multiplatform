/**
 * @file cupertino_button.h
 * @brief Cupertino Button component wrapping ui_button_base.
 */

#ifndef CUPERTINO_CUPERTINO_BUTTON_H
#define CUPERTINO_CUPERTINO_BUTTON_H

/* clang-format off */
#include "ui_button_base.h"
#include "ui_color_space.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum cupertino_button_style
 * @brief Apple HIG visual button styles.
 */
enum cupertino_button_style {
  CUPERTINO_BUTTON_FILLED = 0,
  CUPERTINO_BUTTON_TINTED,
  CUPERTINO_BUTTON_GRAY,
  CUPERTINO_BUTTON_PLAIN,
  CUPERTINO_BUTTON_CAPSULE,
  CUPERTINO_BUTTON_STYLE_COUNT
};

/**
 * @enum cupertino_button_size
 * @brief Apple HIG button sizing hierarchy.
 */
enum cupertino_button_size {
  CUPERTINO_BUTTON_SIZE_SMALL = 0,
  CUPERTINO_BUTTON_SIZE_MEDIUM,
  CUPERTINO_BUTTON_SIZE_LARGE
};

/**
 * @struct cupertino_button_descriptor
 * @brief Configuration descriptor for creating a Cupertino button.
 */
struct cupertino_button_descriptor {
  enum cupertino_button_style style; /**< Visual button variant. */
  enum cupertino_button_size size;   /**< Sizing variant. */
  const char *text;                  /**< Button label text. */
  int is_disabled;                   /**< Disabled interaction state. */
};

/**
 * @struct cupertino_button
 * @brief Cupertino button instance holding state and wrapping ui_button_base.
 */
struct cupertino_button {
  struct ui_button_base *base;       /**< Wrapped CDK primitive. */
  enum cupertino_button_style style; /**< Visual button variant. */
  enum cupertino_button_size size;   /**< Sizing variant. */
  char label[64];                    /**< Cached label string. */
  int is_pressed;                    /**< Touch/pointer down state. */
  float press_scale;                 /**< 0.96f when pressed, 1.0f normal. */
  float press_opacity;               /**< 0.60f when pressed, 1.0f normal. */
};

/**
 * @brief Creates a new Cupertino button instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_button Pointer to receive newly created button instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_button_create(
    struct ui_engine *engine, const struct cupertino_button_descriptor *desc,
    struct cupertino_button **out_button);

/**
 * @brief Destroys a Cupertino button and its underlying base primitive.
 *
 * @param button Button instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_button_destroy(struct cupertino_button *button);

/**
 * @brief Updates the visual style of a Cupertino button.
 *
 * @param button Target button instance.
 * @param style New visual style.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_button_set_style(
    struct cupertino_button *button, enum cupertino_button_style style);

/**
 * @brief Updates the label text of a Cupertino button.
 *
 * @param button Target button instance.
 * @param text New label string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_button_set_text(struct cupertino_button *button, const char *text);

/**
 * @brief Sets the interactive pressed state, applying Apple spring scale and
 * opacity fade.
 *
 * @param button Target button instance.
 * @param is_pressed 1 if pressed, 0 if released.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_button_set_pressed(struct cupertino_button *button, int is_pressed);

/**
 * @brief Retrieves the wrapped CDK ui_button_base primitive.
 *
 * @param button Target button instance.
 * @param out_base Pointer to receive underlying ui_button_base.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_button_get_base(
    struct cupertino_button *button, struct ui_button_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_BUTTON_H */
