#ifndef F2_SELECTION_H
#define F2_SELECTION_H

/**
 * @file f2_selection.h
 * @brief Microsoft Fluent 2 selection widgets (checkbox, switch, radio,
 * slider).
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_text.h"
#include "f2/f2_color.h"
/* clang-format on */

/** @brief Default Checkbox size. */
#define F2_CHECKBOX_DEFAULT_SIZE 20.0f
/** @brief Default Checkbox corner radius. */
#define F2_CHECKBOX_DEFAULT_CORNER_RADIUS 4.0f
/** @brief Default Checkbox border width. */
#define F2_CHECKBOX_DEFAULT_BORDER_WIDTH 1.0f

/** @brief Default Radio size. */
#define F2_RADIO_DEFAULT_SIZE 20.0f
/** @brief Default Radio border width. */
#define F2_RADIO_DEFAULT_BORDER_WIDTH 1.0f
/** @brief Default Radio dot radius. */
#define F2_RADIO_DEFAULT_DOT_RADIUS 5.0f

/** @brief Default Switch track width. */
#define F2_SWITCH_DEFAULT_TRACK_WIDTH 40.0f
/** @brief Default Switch track height. */
#define F2_SWITCH_DEFAULT_TRACK_HEIGHT 20.0f

/** @brief Fill/border/mark colors for selection widgets. */
typedef struct F2SelectionColors {
  CMPColor fill;   /**< Fill color. */
  CMPColor border; /**< Border color. */
  CMPColor mark;   /**< Check mark/dot color. */
} F2SelectionColors;

/** @brief Fluent 2 Checkbox style. */
typedef struct F2CheckboxStyle {
  CMPScalar size;              /**< Checkbox size in pixels (> 0). */
  CMPScalar corner_radius;     /**< Corner radius in pixels (>= 0). */
  CMPScalar border_width;      /**< Border width in pixels (>= 0). */
  F2SelectionColors unchecked; /**< Colors when unchecked. */
  F2SelectionColors checked;   /**< Colors when checked. */
  F2SelectionColors mixed;     /**< Colors when mixed (indeterminate). */
  F2SelectionColors disabled_unchecked; /**< Colors when unchecked/disabled. */
  F2SelectionColors disabled_checked;   /**< Colors when checked/disabled. */
} F2CheckboxStyle;

struct F2Checkbox;

/** @brief Checkbox change callback. */
typedef int(CMP_CALL *F2CheckboxOnChange)(void *ctx,
                                          struct F2Checkbox *checkbox,
                                          CMPBool checked);

/** @brief Fluent 2 Checkbox widget. */
typedef struct F2Checkbox {
  CMPWidget widget;             /**< Widget interface. */
  F2CheckboxStyle style;        /**< Style. */
  CMPRect bounds;               /**< Layout bounds. */
  const char *utf8_label;       /**< Optional label text. */
  cmp_usize utf8_len;           /**< Label text length. */
  CMPBool checked;              /**< CMP_TRUE when checked. */
  CMPBool mixed_state;          /**< CMP_TRUE when mixed. */
  CMPBool hovered;              /**< CMP_TRUE when hovered. */
  CMPBool pressed;              /**< CMP_TRUE when pressed. */
  F2CheckboxOnChange on_change; /**< Change callback. */
  void *on_change_ctx;          /**< Callback context. */
} F2Checkbox;

/**
 * @brief Initialize a default Fluent 2 Checkbox style.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_checkbox_style_init(F2CheckboxStyle *style);

/**
 * @brief Initialize a Fluent 2 Checkbox.
 * @param checkbox Checkbox instance.
 * @param style Style descriptor.
 * @param checked Initial checked state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_checkbox_init(F2Checkbox *checkbox,
                                      const F2CheckboxStyle *style,
                                      CMPBool checked);

/**
 * @brief Set checkbox checked state.
 * @param checkbox Checkbox instance.
 * @param checked Checked state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_checkbox_set_checked(F2Checkbox *checkbox,
                                             CMPBool checked);

/**
 * @brief Set checkbox mixed (indeterminate) state.
 * @param checkbox Checkbox instance.
 * @param mixed_state Mixed state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_checkbox_set_mixed_state(F2Checkbox *checkbox,
                                                 CMPBool mixed_state);

/**
 * @brief Set checkbox change callback.
 * @param checkbox Checkbox instance.
 * @param on_change Callback function.
 * @param ctx Context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_checkbox_set_on_change(F2Checkbox *checkbox,
                                               F2CheckboxOnChange on_change,
                                               void *ctx);

/** @brief Fluent 2 Switch style. */
typedef struct F2SwitchStyle {
  CMPScalar track_width;          /**< Track width. */
  CMPScalar track_height;         /**< Track height. */
  F2SelectionColors off;          /**< Colors when off. */
  F2SelectionColors on;           /**< Colors when on. */
  F2SelectionColors disabled_off; /**< Colors when off/disabled. */
  F2SelectionColors disabled_on;  /**< Colors when on/disabled. */
} F2SwitchStyle;

struct F2Switch;

/** @brief Switch change callback. */
typedef int(CMP_CALL *F2SwitchOnChange)(void *ctx, struct F2Switch *widget,
                                        CMPBool on);

/** @brief Fluent 2 Switch widget. */
typedef struct F2Switch {
  CMPWidget widget;           /**< Widget interface. */
  F2SwitchStyle style;        /**< Style. */
  CMPRect bounds;             /**< Layout bounds. */
  const char *utf8_label;     /**< Optional label text. */
  cmp_usize utf8_len;         /**< Label text length. */
  CMPBool on;                 /**< CMP_TRUE when on. */
  CMPBool hovered;            /**< CMP_TRUE when hovered. */
  CMPBool pressed;            /**< CMP_TRUE when pressed. */
  F2SwitchOnChange on_change; /**< Change callback. */
  void *on_change_ctx;        /**< Callback context. */
} F2Switch;

/**
 * @brief Initialize a default Fluent 2 Switch style.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_switch_style_init(F2SwitchStyle *style);

/**
 * @brief Initialize a Fluent 2 Switch.
 * @param widget Switch instance.
 * @param style Style descriptor.
 * @param on Initial on state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_switch_init(F2Switch *widget,
                                    const F2SwitchStyle *style, CMPBool on);

/**
 * @brief Set switch on state.
 * @param widget Switch instance.
 * @param on On state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_switch_set_on(F2Switch *widget, CMPBool on);

/**
 * @brief Set switch change callback.
 * @param widget Switch instance.
 * @param on_change Callback function.
 * @param ctx Context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_switch_set_on_change(F2Switch *widget,
                                             F2SwitchOnChange on_change,
                                             void *ctx);

/** @brief Fluent 2 Radio style. */
typedef struct F2RadioStyle {
  CMPScalar size;                       /**< Radio size. */
  CMPScalar border_width;               /**< Border width. */
  CMPScalar dot_radius;                 /**< Dot radius. */
  F2SelectionColors unchecked;          /**< Colors when unchecked. */
  F2SelectionColors checked;            /**< Colors when checked. */
  F2SelectionColors disabled_unchecked; /**< Colors when unchecked/disabled. */
  F2SelectionColors disabled_checked;   /**< Colors when checked/disabled. */
} F2RadioStyle;

struct F2Radio;

/** @brief Radio change callback. */
typedef int(CMP_CALL *F2RadioOnChange)(void *ctx, struct F2Radio *radio,
                                       CMPBool selected);

/** @brief Fluent 2 Radio widget. */
typedef struct F2Radio {
  CMPWidget widget;          /**< Widget interface. */
  F2RadioStyle style;        /**< Style. */
  CMPRect bounds;            /**< Layout bounds. */
  const char *utf8_label;    /**< Optional label text. */
  cmp_usize utf8_len;        /**< Label text length. */
  CMPBool selected;          /**< CMP_TRUE when selected. */
  CMPBool hovered;           /**< CMP_TRUE when hovered. */
  CMPBool pressed;           /**< CMP_TRUE when pressed. */
  F2RadioOnChange on_change; /**< Change callback. */
  void *on_change_ctx;       /**< Callback context. */
} F2Radio;

/**
 * @brief Initialize a default Fluent 2 Radio style.
 * @param style Style descriptor.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_radio_style_init(F2RadioStyle *style);

/**
 * @brief Initialize a Fluent 2 Radio.
 * @param radio Radio instance.
 * @param style Style descriptor.
 * @param selected Initial selected state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_radio_init(F2Radio *radio, const F2RadioStyle *style,
                                   CMPBool selected);

/**
 * @brief Set radio selected state.
 * @param radio Radio instance.
 * @param selected Selected state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_radio_set_selected(F2Radio *radio, CMPBool selected);

/**
 * @brief Set radio change callback.
 * @param radio Radio instance.
 * @param on_change Callback function.
 * @param ctx Context.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_radio_set_on_change(F2Radio *radio,
                                            F2RadioOnChange on_change,
                                            void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* F2_SELECTION_H */
