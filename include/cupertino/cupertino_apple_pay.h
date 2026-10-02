/**
 * @file cupertino_apple_pay.h
 * @brief Apple Pay and Sign in with Apple official HIG buttons.
 */

#ifndef CUPERTINO_CUPERTINO_APPLE_PAY_H
#define CUPERTINO_CUPERTINO_APPLE_PAY_H

/* clang-format off */
#include "ui_button_base.h"
#include "ui_color_space.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_APPLE_PAY_MIN_WIDTH 140.0f
#define CUPERTINO_APPLE_PAY_MIN_HEIGHT 44.0f
#define CUPERTINO_APPLE_PAY_DEFAULT_CORNER_RADIUS 4.0f

/**
 * @enum cupertino_apple_pay_type
 * @brief PKPaymentButtonType variants per Apple PassKit HIG.
 */
enum cupertino_apple_pay_type {
  CUPERTINO_APPLE_PAY_PLAIN = 0, /**< Standard Apple Pay logo only. */
  CUPERTINO_APPLE_PAY_BUY,       /**< 'Buy with Apple Pay'. */
  CUPERTINO_APPLE_PAY_SET_UP,    /**< 'Set up Apple Pay'. */
  CUPERTINO_APPLE_PAY_DONATE,    /**< 'Donate with Apple Pay'. */
  CUPERTINO_APPLE_PAY_CHECKOUT,  /**< 'Check out with Apple Pay'. */
  CUPERTINO_APPLE_PAY_SUBSCRIBE, /**< 'Subscribe with Apple Pay'. */
  CUPERTINO_APPLE_PAY_BOOK,      /**< 'Book with Apple Pay'. */
  CUPERTINO_APPLE_PAY_RELOAD,    /**< 'Reload with Apple Pay'. */
  CUPERTINO_APPLE_PAY_TOP_UP,    /**< 'Top Up with Apple Pay'. */
  CUPERTINO_APPLE_PAY_RENT,      /**< 'Rent with Apple Pay'. */
  CUPERTINO_APPLE_PAY_SUPPORT,   /**< 'Support with Apple Pay'. */
  CUPERTINO_APPLE_PAY_TYPE_COUNT
};

/**
 * @enum cupertino_apple_pay_style
 * @brief PKPaymentButtonStyle variants per Apple PassKit HIG.
 */
enum cupertino_apple_pay_style {
  CUPERTINO_APPLE_PAY_STYLE_WHITE = 0, /**< White background, black glyphs. */
  CUPERTINO_APPLE_PAY_STYLE_WHITE_OUTLINE, /**< White background with hairline
                                              border. */
  CUPERTINO_APPLE_PAY_STYLE_BLACK /**< Black background, white glyphs. */
};

/**
 * @struct cupertino_apple_pay_descriptor
 * @brief Configuration descriptor for creating an Apple Pay button.
 */
struct cupertino_apple_pay_descriptor {
  enum cupertino_apple_pay_type type;   /**< Action type variant. */
  enum cupertino_apple_pay_style style; /**< Visual color style. */
  float width;         /**< Target width (clamped to >= 140pt). */
  float height;        /**< Target height (clamped to >= 44pt). */
  float corner_radius; /**< Corner radius (default 4.0pt). */
};

/**
 * @struct cupertino_apple_pay_button
 * @brief Cupertino Apple Pay button instance wrapping ui_button_base.
 */
struct cupertino_apple_pay_button {
  struct ui_button_base *base;          /**< Underlying button base. */
  enum cupertino_apple_pay_type type;   /**< Action type variant. */
  enum cupertino_apple_pay_style style; /**< Visual color style. */
  float width;                          /**< Current button width. */
  float height;                         /**< Current button height. */
  float corner_radius;                  /**< Button corner radius. */
  float scale;                          /**< Touch animation scale. */
  float opacity;                        /**< Touch animation opacity. */
  int is_pressed;                       /**< Pressed state indicator. */
  ui_color_t background_color;          /**< Background color. */
  ui_color_t foreground_color;          /**< Foreground label color. */
  ui_color_t border_color;              /**< Outline border color. */
  char label[64];                       /**< Localized button label text. */
};

/**
 * @brief Creates a new Apple Pay button instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_button Pointer to receive newly created button instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_apple_pay_create(
    struct ui_engine *engine, const struct cupertino_apple_pay_descriptor *desc,
    struct cupertino_apple_pay_button **out_button);

/**
 * @brief Destroys an Apple Pay button instance.
 *
 * @param button Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_apple_pay_destroy(struct cupertino_apple_pay_button *button);

/**
 * @brief Simulates touch-down press state on Apple Pay button.
 *
 * @param button Target button.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_apple_pay_press_start(struct cupertino_apple_pay_button *button);

/**
 * @brief Simulates touch-up release state on Apple Pay button.
 *
 * @param button Target button.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_apple_pay_press_end(struct cupertino_apple_pay_button *button);

/**
 * @brief Gets current dimensions of the Apple Pay button.
 *
 * @param button Target button.
 * @param out_width Pointer to receive width.
 * @param out_height Pointer to receive height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_apple_pay_get_dimensions(
    const struct cupertino_apple_pay_button *button, float *out_width,
    float *out_height);

/**
 * @brief Gets the localized label text for accessibility and drawing.
 *
 * @param button Target button.
 * @param out_label Pointer to receive label string pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_apple_pay_get_label(
    const struct cupertino_apple_pay_button *button, const char **out_label);

/**
 * @brief Retrieves the underlying CDK button primitive.
 *
 * @param button Target button.
 * @param out_base Pointer to receive ui_button_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_apple_pay_get_base(struct cupertino_apple_pay_button *button,
                             struct ui_button_base **out_base);

/* --- Sign in with Apple (ASAuthorizationAppleIDButton) --- */

/**
 * @enum cupertino_apple_id_button_type
 * @brief Sign in with Apple button types.
 */
enum cupertino_apple_id_button_type {
  CUPERTINO_APPLE_ID_SIGN_IN = 0, /**< 'Sign in with Apple'. */
  CUPERTINO_APPLE_ID_CONTINUE,    /**< 'Continue with Apple'. */
  CUPERTINO_APPLE_ID_SIGN_UP,     /**< 'Sign up with Apple'. */
  CUPERTINO_APPLE_ID_TYPE_COUNT
};

/**
 * @struct cupertino_apple_id_descriptor
 * @brief Configuration descriptor for Sign in with Apple button.
 */
struct cupertino_apple_id_descriptor {
  enum cupertino_apple_id_button_type
      type; /**< Type of authorization button. */
  enum cupertino_apple_pay_style
      style;           /**< Visual style (White, Outline, Black). */
  float width;         /**< Width (>= 140pt). */
  float height;        /**< Height (>= 44pt). */
  float corner_radius; /**< Corner radius. */
};

/**
 * @struct cupertino_apple_id_button
 * @brief Sign in with Apple button instance wrapping ui_button_base.
 */
struct cupertino_apple_id_button {
  struct ui_button_base *base;              /**< Underlying button base. */
  enum cupertino_apple_id_button_type type; /**< Button type variant. */
  enum cupertino_apple_pay_style style;     /**< Visual color style. */
  float width;                              /**< Current button width. */
  float height;                             /**< Current button height. */
  float corner_radius;                      /**< Button corner radius. */
  float scale;                              /**< Touch animation scale. */
  float opacity;                            /**< Touch animation opacity. */
  int is_pressed;                           /**< Pressed state indicator. */
  ui_color_t background_color;              /**< Background color. */
  ui_color_t foreground_color;              /**< Foreground label color. */
  ui_color_t border_color;                  /**< Outline border color. */
  char label[64];                           /**< Localized button label. */
};

/**
 * @brief Creates a new Sign in with Apple button.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_button Pointer to receive created button instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_apple_id_button_create(
    struct ui_engine *engine, const struct cupertino_apple_id_descriptor *desc,
    struct cupertino_apple_id_button **out_button);

/**
 * @brief Destroys a Sign in with Apple button instance.
 *
 * @param button Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_apple_id_button_destroy(struct cupertino_apple_id_button *button);

/**
 * @brief Simulates touch-down press on Sign in with Apple button.
 *
 * @param button Target button.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_apple_id_button_press_start(struct cupertino_apple_id_button *button);

/**
 * @brief Simulates touch-up release on Sign in with Apple button.
 *
 * @param button Target button.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_apple_id_button_press_end(struct cupertino_apple_id_button *button);

/**
 * @brief Gets dimensions of Sign in with Apple button.
 *
 * @param button Target button.
 * @param out_width Pointer to receive width.
 * @param out_height Pointer to receive height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_apple_id_button_get_dimensions(
    const struct cupertino_apple_id_button *button, float *out_width,
    float *out_height);

/**
 * @brief Gets label text of Sign in with Apple button.
 *
 * @param button Target button.
 * @param out_label Pointer to receive label string pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_apple_id_button_get_label(
    const struct cupertino_apple_id_button *button, const char **out_label);

/**
 * @brief Gets wrapped CDK button base.
 *
 * @param button Target button.
 * @param out_base Pointer to receive ui_button_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_apple_id_button_get_base(
    struct cupertino_apple_id_button *button, struct ui_button_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_APPLE_PAY_H */
