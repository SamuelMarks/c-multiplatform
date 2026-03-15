#ifndef CUPERTINO_BUTTON_H
#define CUPERTINO_BUTTON_H

/**
 * @file cupertino_button.h
 * @brief Apple Cupertino Button components.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_text.h"
#include "cupertino/cupertino_color.h"
#include "cupertino/cupertino_shape.h"
#include "cupertino/cupertino_typography.h"
/* clang-format on */

/** @brief iOS 15+ Button Variants */
typedef enum CupertinoButtonVariant {
  CUPERTINO_BUTTON_VARIANT_PLAIN = 0,
  CUPERTINO_BUTTON_VARIANT_TINTED,
  CUPERTINO_BUTTON_VARIANT_FILLED,
  CUPERTINO_BUTTON_VARIANT_GRAY
} CupertinoButtonVariant;

/** @brief Button Sizes */
typedef enum CupertinoButtonSize {
  CUPERTINO_BUTTON_SIZE_SMALL = 0,
  CUPERTINO_BUTTON_SIZE_MEDIUM,
  CUPERTINO_BUTTON_SIZE_LARGE,
  CUPERTINO_BUTTON_SIZE_MAX
} CupertinoButtonSize;

/** @brief Button Shapes */
typedef enum CupertinoButtonShape {
  CUPERTINO_BUTTON_SHAPE_CAPSULE = 0,
  CUPERTINO_BUTTON_SHAPE_ROUNDED_RECTANGLE,
  CUPERTINO_BUTTON_SHAPE_CIRCLE
} CupertinoButtonShape;

/** @brief Button Style descriptor */
typedef struct CupertinoButtonStyle {
  CupertinoButtonVariant variant; /**< Button visual variant. */
  CupertinoButtonSize size;       /**< Button layout size. */
  CupertinoButtonShape shape;     /**< Button background shape. */
  CMPColor tint_color;            /**< Primary tint/accent color. */
  CMPBool is_dark_mode;           /**< Dark mode active state. */
  CMPTextStyle text_style;        /**< Text style for label. */
} CupertinoButtonStyle;

/** @brief Cupertino Button Widget */
typedef struct CupertinoButton {
  CMPWidget widget;            /**< Base widget interface. */
  CMPTextBackend text_backend; /**< Text backend for drawing labels. */
  CupertinoButtonStyle style;  /**< Button style. */
  CMPRect bounds;              /**< Layout bounds. */
  const char *label_utf8;      /**< Label text. */
  cmp_usize label_len;         /**< Label length. */
  CMPBool is_hovered;          /**< Hover state (macOS mostly). */
  CMPBool is_pressed;          /**< Pressed state. */
  CMPBool is_disabled;         /**< Disabled state. */
} CupertinoButton;

/**
 * @brief Initializes a Cupertino button style with defaults (Plain, Medium,
 * Blue Tint).
 * @param style Style to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_button_style_init(CupertinoButtonStyle *style);

/**
 * @brief Initializes a Cupertino button widget.
 * @param button Button to initialize.
 * @param text_backend Text backend for rendering.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_button_init(CupertinoButton *button,
                                           const CMPTextBackend *text_backend);

/**
 * @brief Sets the button label text.
 * @param button Button instance.
 * @param label_utf8 UTF-8 text.
 * @param label_len Length of text.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_button_set_label(CupertinoButton *button,
                                                const char *label_utf8,
                                                cmp_usize label_len);

/**
 * @brief Sets the button style.
 * @param button Button instance.
 * @param style Style to apply.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_button_set_style(
    CupertinoButton *button, const CupertinoButtonStyle *style);

/**
 * @brief Measures the button dimensions.
 * @param button Button instance.
 * @param width_spec Width specification.
 * @param height_spec Height specification.
 * @param out_size Resulting size.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_button_measure(CupertinoButton *button,
                                              CMPMeasureSpec width_spec,
                                              CMPMeasureSpec height_spec,
                                              CMPSize *out_size);

/**
 * @brief Sets the layout bounds for the button.
 * @param button Button instance.
 * @param bounds Layout bounds.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_button_layout(CupertinoButton *button,
                                             CMPRect bounds);

/**
 * @brief Paints the button using the provided graphics interface.
 * @param button Button instance.
 * @param ctx Paint context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cupertino_button_paint(const CupertinoButton *button,
                                            CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_BUTTON_H */
