#ifndef M3_SELECTION_H
#define M3_SELECTION_H

/**
 * @file m3_selection.h
 * @brief Selection widgets (checkbox, switch, radio) for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_ui.h"

/** @brief Default checkbox size in pixels. */
#define M3_CHECKBOX_DEFAULT_SIZE 20.0f
/** @brief Default checkbox corner radius in pixels. */
#define M3_CHECKBOX_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Default checkbox border width in pixels. */
#define M3_CHECKBOX_DEFAULT_BORDER_WIDTH 2.0f
/** @brief Default checkbox check mark thickness in pixels. */
#define M3_CHECKBOX_DEFAULT_CHECK_THICKNESS 2.0f

/** @brief Default radio size in pixels. */
#define M3_RADIO_DEFAULT_SIZE 20.0f
/** @brief Default radio border width in pixels. */
#define M3_RADIO_DEFAULT_BORDER_WIDTH 2.0f
/** @brief Default radio dot radius in pixels. */
#define M3_RADIO_DEFAULT_DOT_RADIUS 4.0f

/** @brief Default switch track width in pixels. */
#define M3_SWITCH_DEFAULT_TRACK_WIDTH 52.0f
/** @brief Default switch track height in pixels. */
#define M3_SWITCH_DEFAULT_TRACK_HEIGHT 32.0f
/** @brief Default switch thumb inset in pixels. */
#define M3_SWITCH_DEFAULT_THUMB_INSET 2.0f

/**
 * @brief Fill/border/mark colors for selection widgets.
 */
typedef struct M3SelectionColors {
  M3Color fill;   /**< Fill color. */
  M3Color border; /**< Border color. */
  M3Color mark;   /**< Check mark/dot color. */
} M3SelectionColors;

/**
 * @brief Checkbox style descriptor.
 */
typedef struct M3CheckboxStyle {
  M3Scalar size;               /**< Checkbox size in pixels (> 0). */
  M3Scalar corner_radius;      /**< Corner radius in pixels (>= 0). */
  M3Scalar border_width;       /**< Border width in pixels (>= 0). */
  M3Scalar check_thickness;    /**< Check mark thickness in pixels (> 0). */
  M3SelectionColors unchecked; /**< Colors when unchecked. */
  M3SelectionColors checked;   /**< Colors when checked. */
  M3SelectionColors
      disabled_unchecked; /**< Colors when unchecked and disabled. */
  M3SelectionColors disabled_checked; /**< Colors when checked and disabled. */
} M3CheckboxStyle;

struct M3Checkbox;

/**
 * @brief Checkbox change callback signature.
 * @param ctx User callback context pointer.
 * @param checkbox Checkbox instance that changed.
 * @param checked M3_TRUE when checked.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3CheckboxOnChange)(void *ctx, struct M3Checkbox *checkbox,
                                         M3Bool checked);

/**
 * @brief Checkbox widget instance.
 */
typedef struct M3Checkbox {
  M3Widget widget;        /**< Widget interface (points to this instance). */
  M3CheckboxStyle style;  /**< Current checkbox style. */
  M3Rect bounds;          /**< Layout bounds. */
  const char *utf8_label; /**< UTF-8 label for semantics (optional). */
  m3_usize utf8_len;      /**< UTF-8 label length in bytes. */
  M3Bool checked;         /**< M3_TRUE when checked. */
  M3Bool pressed;         /**< M3_TRUE when pressed. */
  M3CheckboxOnChange on_change; /**< Change callback (may be NULL). */
  void *on_change_ctx;          /**< Change callback context pointer. */
} M3Checkbox;

/**
 * @brief Initialize a checkbox style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_checkbox_style_init(M3CheckboxStyle *style);

/**
 * @brief Initialize a checkbox widget.
 * @param checkbox Checkbox instance.
 * @param style Checkbox style descriptor.
 * @param checked Initial checked state.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_checkbox_init(M3Checkbox *checkbox,
                                    const M3CheckboxStyle *style,
                                    M3Bool checked);

/**
 * @brief Update the checkbox checked state.
 * @param checkbox Checkbox instance.
 * @param checked New checked state.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_checkbox_set_checked(M3Checkbox *checkbox,
                                           M3Bool checked);

/**
 * @brief Retrieve the checkbox checked state.
 * @param checkbox Checkbox instance.
 * @param out_checked Receives the checked state.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_checkbox_get_checked(const M3Checkbox *checkbox,
                                           M3Bool *out_checked);

/**
 * @brief Update the checkbox style.
 * @param checkbox Checkbox instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_checkbox_set_style(M3Checkbox *checkbox,
                                         const M3CheckboxStyle *style);

/**
 * @brief Update the checkbox semantics label.
 * @param checkbox Checkbox instance.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Label length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_checkbox_set_label(M3Checkbox *checkbox,
                                         const char *utf8_label,
                                         m3_usize utf8_len);

/**
 * @brief Assign a checkbox change callback.
 * @param checkbox Checkbox instance.
 * @param on_change Change callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_checkbox_set_on_change(M3Checkbox *checkbox,
                                             M3CheckboxOnChange on_change,
                                             void *ctx);

/**
 * @brief Switch track/thumb colors.
 */
typedef struct M3SwitchColors {
  M3Color track; /**< Track color. */
  M3Color thumb; /**< Thumb color. */
} M3SwitchColors;

/**
 * @brief Switch style descriptor.
 */
typedef struct M3SwitchStyle {
  M3Scalar track_width;        /**< Track width in pixels (> 0). */
  M3Scalar track_height;       /**< Track height in pixels (> 0). */
  M3Scalar thumb_inset;        /**< Thumb inset in pixels (>= 0). */
  M3SwitchColors off;          /**< Colors when off. */
  M3SwitchColors on;           /**< Colors when on. */
  M3SwitchColors disabled_off; /**< Colors when off and disabled. */
  M3SwitchColors disabled_on;  /**< Colors when on and disabled. */
} M3SwitchStyle;

struct M3Switch;

/**
 * @brief Switch change callback signature.
 * @param ctx User callback context pointer.
 * @param widget Switch instance that changed.
 * @param on M3_TRUE when on.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3SwitchOnChange)(void *ctx, struct M3Switch *widget,
                                       M3Bool on);

/**
 * @brief Switch widget instance.
 */
typedef struct M3Switch {
  M3Widget widget;        /**< Widget interface (points to this instance). */
  M3SwitchStyle style;    /**< Current switch style. */
  M3Rect bounds;          /**< Layout bounds. */
  const char *utf8_label; /**< UTF-8 label for semantics (optional). */
  m3_usize utf8_len;      /**< UTF-8 label length in bytes. */
  M3Bool on;              /**< M3_TRUE when on. */
  M3Bool pressed;         /**< M3_TRUE when pressed. */
  M3SwitchOnChange on_change; /**< Change callback (may be NULL). */
  void *on_change_ctx;        /**< Change callback context pointer. */
} M3Switch;

/**
 * @brief Initialize a switch style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_switch_style_init(M3SwitchStyle *style);

/**
 * @brief Initialize a switch widget.
 * @param widget Switch instance.
 * @param style Switch style descriptor.
 * @param on Initial on state.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_switch_init(M3Switch *widget, const M3SwitchStyle *style,
                                  M3Bool on);

/**
 * @brief Update the switch on state.
 * @param widget Switch instance.
 * @param on New on state.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_switch_set_on(M3Switch *widget, M3Bool on);

/**
 * @brief Retrieve the switch on state.
 * @param widget Switch instance.
 * @param out_on Receives the on state.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_switch_get_on(const M3Switch *widget, M3Bool *out_on);

/**
 * @brief Update the switch style.
 * @param widget Switch instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_switch_set_style(M3Switch *widget,
                                       const M3SwitchStyle *style);

/**
 * @brief Update the switch semantics label.
 * @param widget Switch instance.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Label length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_switch_set_label(M3Switch *widget, const char *utf8_label,
                                       m3_usize utf8_len);

/**
 * @brief Assign a switch change callback.
 * @param widget Switch instance.
 * @param on_change Change callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_switch_set_on_change(M3Switch *widget,
                                           M3SwitchOnChange on_change,
                                           void *ctx);

/**
 * @brief Radio button style descriptor.
 */
typedef struct M3RadioStyle {
  M3Scalar size;               /**< Radio size in pixels (> 0). */
  M3Scalar border_width;       /**< Border width in pixels (>= 0). */
  M3Scalar dot_radius;         /**< Inner dot radius in pixels (>= 0). */
  M3SelectionColors unchecked; /**< Colors when unchecked. */
  M3SelectionColors checked;   /**< Colors when checked. */
  M3SelectionColors
      disabled_unchecked; /**< Colors when unchecked and disabled. */
  M3SelectionColors disabled_checked; /**< Colors when checked and disabled. */
} M3RadioStyle;

struct M3Radio;

/**
 * @brief Radio change callback signature.
 * @param ctx User callback context pointer.
 * @param radio Radio instance that changed.
 * @param selected M3_TRUE when selected.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3RadioOnChange)(void *ctx, struct M3Radio *radio,
                                      M3Bool selected);

/**
 * @brief Radio widget instance.
 */
typedef struct M3Radio {
  M3Widget widget;           /**< Widget interface (points to this instance). */
  M3RadioStyle style;        /**< Current radio style. */
  M3Rect bounds;             /**< Layout bounds. */
  const char *utf8_label;    /**< UTF-8 label for semantics (optional). */
  m3_usize utf8_len;         /**< UTF-8 label length in bytes. */
  M3Bool selected;           /**< M3_TRUE when selected. */
  M3Bool pressed;            /**< M3_TRUE when pressed. */
  M3RadioOnChange on_change; /**< Change callback (may be NULL). */
  void *on_change_ctx;       /**< Change callback context pointer. */
} M3Radio;

/**
 * @brief Initialize a radio style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_radio_style_init(M3RadioStyle *style);

/**
 * @brief Initialize a radio widget.
 * @param radio Radio instance.
 * @param style Radio style descriptor.
 * @param selected Initial selected state.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_radio_init(M3Radio *radio, const M3RadioStyle *style,
                                 M3Bool selected);

/**
 * @brief Update the radio selected state.
 * @param radio Radio instance.
 * @param selected New selected state.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_radio_set_selected(M3Radio *radio, M3Bool selected);

/**
 * @brief Retrieve the radio selected state.
 * @param radio Radio instance.
 * @param out_selected Receives the selected state.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_radio_get_selected(const M3Radio *radio,
                                         M3Bool *out_selected);

/**
 * @brief Update the radio style.
 * @param radio Radio instance.
 * @param style New style descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_radio_set_style(M3Radio *radio,
                                      const M3RadioStyle *style);

/**
 * @brief Update the radio semantics label.
 * @param radio Radio instance.
 * @param utf8_label UTF-8 label text (may be NULL when utf8_len is 0).
 * @param utf8_len Label length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_radio_set_label(M3Radio *radio, const char *utf8_label,
                                      m3_usize utf8_len);

/**
 * @brief Assign a radio change callback.
 * @param radio Radio instance.
 * @param on_change Change callback (may be NULL to clear).
 * @param ctx Callback context pointer.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_radio_set_on_change(M3Radio *radio,
                                          M3RadioOnChange on_change, void *ctx);

#ifdef M3_TESTING
/**
 * @brief Set a selection test fail point.
 * @param point Fail point identifier.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_selection_test_set_fail_point(m3_u32 point);

/**
 * @brief Clear selection test fail points.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_selection_test_clear_fail_points(void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_SELECTION_H */
