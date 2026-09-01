/**
 * @file ui_color_picker_base.c
 * @brief Implementation of ui_color_picker_base.c.
 */
/* clang-format off */
#include "ui_color_picker_base.h"
#include "ui_internal_mem.h"
#include "ui_dom_node.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

#if defined(_MSC_VER) && _MSC_VER < 1800
/**
 * @brief ui_round.
 * @param number Parameter number.
 * @return Return value.
 */
static double ui_round(double number) {
  return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}
/** @brief internal */
#define ROUND ui_round
#else
/* Provide a fallback for strict C90 compilers where round() isn't available */
/**
 * @brief ui_math_round_fallback.
 * @param number Parameter number.
 * @return Return value.
 */
static double ui_math_round_fallback(double number) {
  return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}
/** @brief internal */
#define ROUND ui_math_round_fallback
#endif

#ifdef UI_TEST_MOCK_ALLOC

int g_color_picker_mock_fail = 0;
int g_color_picker_mock_target = 0;
int g_color_picker_mock_current = 0;

#endif

/**
 * @struct ui_color_picker_base
 * @struct ui_color_picker_base
 * @brief Internal representation of a color picker.
 */
struct ui_color_picker_base {
  struct ui_color_rgb rgb; /**< Current RGB state */
  struct ui_color_hsv hsv; /**< Current HSV state */

  /**
   * @brief CVA: Callback fired when value changes
   */
  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data); /**< user_data) */
  void *cva_on_change_user_data; /**< CVA: User data for on_change callback */

  /**
   * @brief CVA: Callback fired when input is touched
   */
  ui_error_t (*cva_on_touched)(void *user_data); /**< user_data) */
  void *cva_on_touched_user_data; /**< CVA: User data for on_touched callback */

  int is_disabled; /**< 1 if disabled, 0 otherwise */
};

/**
 * @brief trigger_cva_change.
 * @param picker Parameter picker.
 * @return Return value.
 */
static ui_error_t trigger_cva_change(struct ui_color_picker_base *picker) {
  union ui_signal_payload payload;
  int color_int;

  if (picker->cva_on_change) {
    color_int = (picker->rgb.r << 24) | (picker->rgb.g << 16) |
                (picker->rgb.b << 8) | 0xFF;
    payload.int_val = color_int;
    return picker->cva_on_change(payload, picker->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief color_picker_cva_write_value.
 * @param component Parameter component.
 * @param value Parameter value.
 * @return Return value.
 */
static ui_error_t color_picker_cva_write_value(void *component,
                                               union ui_signal_payload value) {
  struct ui_color_picker_base *picker =
      (struct ui_color_picker_base *)component;
  struct ui_color_rgb rgb;
  int color_int;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  color_int = value.int_val;
  rgb.r = (unsigned char)((color_int >> 24) & 0xFF);
  rgb.g = (unsigned char)((color_int >> 16) & 0xFF);
  rgb.b = (unsigned char)((color_int >> 8) & 0xFF);

  return ui_color_picker_base_set_rgb(picker, &rgb);
}

/**
 * @brief color_picker_cva_register_on_change.
 * @param component Parameter component.
 * @param new_value Parameter new_value.
 * @param user_data Parameter user_data.
 * @return Return value.
 * @param callback Parameter callback.
 */
static ui_error_t color_picker_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_color_picker_base *picker =
      (struct ui_color_picker_base *)component;
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  picker->cva_on_change = callback;
  picker->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief color_picker_cva_register_on_touched.
 * @param component Parameter component.
 * @param user_data) Parameter user_data).
 * @param user_data Parameter user_data.
 * @return Return value.
 * @param callback Parameter callback.
 */
static ui_error_t color_picker_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_color_picker_base *picker =
      (struct ui_color_picker_base *)component;
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  picker->cva_on_touched = callback;
  picker->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief color_picker_cva_set_disabled_state.
 * @param component Parameter component.
 * @param is_disabled Parameter is_disabled.
 * @return Return value.
 */
static ui_error_t color_picker_cva_set_disabled_state(void *component,
                                                      int is_disabled) {
  struct ui_color_picker_base *picker =
      (struct ui_color_picker_base *)component;
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  picker->is_disabled = is_disabled;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_color_picker_base_create.
 * @param out_picker Parameter out_picker.
 * @param out_cva Parameter out_cva.
 * @return Return value.
 */
ui_error_t
ui_color_picker_base_create(struct ui_color_picker_base **out_picker,
                            struct ui_control_value_accessor *out_cva) {
  struct ui_color_picker_base *picker;

  if (!out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct ui_color_picker_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_color_picker_base));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  /* Initialize to red (#FF0000) */
  picker->rgb.r = 255;
  picker->rgb.g = 0;
  picker->rgb.b = 0;
  picker->hsv.h = 0.0;
  picker->hsv.s = 1.0;
  picker->hsv.v = 1.0;

  picker->cva_on_change = NULL;
  picker->cva_on_change_user_data = NULL;
  picker->cva_on_touched = NULL;
  picker->cva_on_touched_user_data = NULL;
  picker->is_disabled = 0;

  if (out_cva) {
    out_cva->write_value = color_picker_cva_write_value;
    out_cva->register_on_change = color_picker_cva_register_on_change;
    out_cva->register_on_touched = color_picker_cva_register_on_touched;
    out_cva->set_disabled_state = color_picker_cva_set_disabled_state;
  }

  *out_picker = picker;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_color_picker_base_destroy.
 * @param picker Parameter picker.
 * @return Return value.
 */
ui_error_t ui_color_picker_base_destroy(struct ui_color_picker_base *picker) {
  if (picker) {
    C_MULTIPLATFORM_FREE(picker);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_color_picker_hsv_to_rgb.
 * @param hsv Parameter hsv.
 * @param out_rgb Parameter out_rgb.
 * @return Return value.
 */
ui_error_t ui_color_picker_hsv_to_rgb(const struct ui_color_hsv *hsv,
                                      struct ui_color_rgb *out_rgb) {
  double c, x, m;
  double r1, g1, b1;
  double hd;

  if (!hsv || !out_rgb) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  c = hsv->v * hsv->s;
  hd = hsv->h / 60.0;
  x = c * (1.0 - fabs(fmod(hd, 2.0) - 1.0));
  m = hsv->v - c;

  if (hd < 1.0) {
    r1 = c;
    g1 = x;
    b1 = 0.0;
  } else if (hd < 2.0) {
    r1 = x;
    g1 = c;
    b1 = 0.0;
  } else if (hd < 3.0) {
    r1 = 0.0;
    g1 = c;
    b1 = x;
  } else if (hd < 4.0) {
    r1 = 0.0;
    g1 = x;
    b1 = c;
  } else if (hd < 5.0) {
    r1 = x;
    g1 = 0.0;
    b1 = c;
  } else {
    r1 = c;
    g1 = 0.0;
    b1 = x;
  }

  out_rgb->r = (unsigned char)ROUND((r1 + m) * 255.0);
  out_rgb->g = (unsigned char)ROUND((g1 + m) * 255.0);
  out_rgb->b = (unsigned char)ROUND((b1 + m) * 255.0);

  return UI_ERROR_NONE;
}

/**
 * @brief ui_color_picker_rgb_to_hsv.
 * @param rgb Parameter rgb.
 * @param out_hsv Parameter out_hsv.
 * @return Return value.
 */
ui_error_t ui_color_picker_rgb_to_hsv(const struct ui_color_rgb *rgb,
                                      struct ui_color_hsv *out_hsv) {
  double r, g, b;
  double cmax, cmin, delta;
  double max_rg, min_rg;

  if (!rgb || !out_hsv) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  r = rgb->r / 255.0;
  g = rgb->g / 255.0;
  b = rgb->b / 255.0;

  max_rg = r > g ? r : g;
  cmax = max_rg > b ? max_rg : b;
  min_rg = r < g ? r : g;
  cmin = min_rg < b ? min_rg : b;
  delta = cmax - cmin;

  if (delta == 0.0) {
    out_hsv->h = 0.0;
  } else if (cmax == r) {
    out_hsv->h = 60.0 * fmod(((g - b) / delta), 6.0);
    if (out_hsv->h < 0.0) {
      out_hsv->h += 360.0;
    }
  } else if (cmax == g) {
    out_hsv->h = 60.0 * (((b - r) / delta) + 2.0);
  } else {
    out_hsv->h = 60.0 * (((r - g) / delta) + 4.0);
  }

  if (cmax == 0.0) {
    out_hsv->s = 0.0;
  } else {
    out_hsv->s = delta / cmax;
  }

  out_hsv->v = cmax;

  return UI_ERROR_NONE;
}

/**
 * @brief ui_color_picker_rgb_to_hex.
 * @param rgb Parameter rgb.
 * @param out_hex Parameter out_hex.
 * @param hex_size Parameter hex_size.
 * @return Return value.
 */
ui_error_t ui_color_picker_rgb_to_hex(const struct ui_color_rgb *rgb,
                                      char *out_hex, size_t hex_size) {
  if (!rgb || !out_hex || hex_size < 8) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  sprintf_s(out_hex, hex_size, "#%02X%02X%02X", rgb->r, rgb->g, rgb->b);
#else
  sprintf(out_hex, "#%02X%02X%02X", rgb->r, rgb->g, rgb->b);
#endif

  return UI_ERROR_NONE;
}

/**
 * @brief ui_color_picker_hex_to_rgb.
 * @param hex Parameter hex.
 * @param out_rgb Parameter out_rgb.
 * @return Return value.
 */
ui_error_t ui_color_picker_hex_to_rgb(const char *hex,
                                      struct ui_color_rgb *out_rgb) {
  const char *p = hex;
  unsigned int r, g, b;

  if (!hex || !out_rgb) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (p[0] == '#') {
    p++;
  }

  if (strlen(p) != 6) {
    return UI_ERROR_PARSE_FAILED;
  }

#if defined(_MSC_VER)
  if (sscanf_s(p, "%02x%02x%02x", &r, &g, &b) != 3) {
#else
  if (sscanf(p, "%02x%02x%02x", &r, &g, &b) != 3) {
#endif
    return UI_ERROR_PARSE_FAILED;
  }

  out_rgb->r = (unsigned char)r;
  out_rgb->g = (unsigned char)g;
  out_rgb->b = (unsigned char)b;

  return UI_ERROR_NONE;
}

/**
 * @brief ui_color_picker_calc_hsv_from_2d.
 * @param hue Parameter hue.
 * @param x Parameter x.
 * @param y Parameter y.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param out_hsv Parameter out_hsv.
 * @return Return value.
 */
ui_error_t ui_color_picker_calc_hsv_from_2d(double hue, double x, double y,
                                            double width, double height,
                                            struct ui_color_hsv *out_hsv) {
  if (!out_hsv || width <= 0.0 || height <= 0.0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_hsv->h = hue;

  /* Clamp x and y to [0, width] and [0, height] */
  if (x < 0.0) {
    x = 0.0;
  }
  if (x > width) {
    x = width;
  }
  if (y < 0.0) {
    y = 0.0;
  }
  if (y > height) {
    y = height;
  }

  /* x maps directly to saturation */
  out_hsv->s = x / width;

  /* y maps inversely to value (top is 1.0, bottom is 0.0) */
  out_hsv->v = 1.0 - (y / height);

  return UI_ERROR_NONE;
}

/**
 * @brief ui_color_picker_base_get_rgb.
 * @param picker Parameter picker.
 * @param out_rgb Parameter out_rgb.
 * @return Return value.
 */
ui_error_t
ui_color_picker_base_get_rgb(const struct ui_color_picker_base *picker,
                             struct ui_color_rgb *out_rgb) {
  if (!picker || !out_rgb) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_rgb = picker->rgb;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_color_picker_base_set_rgb.
 * @param picker Parameter picker.
 * @param rgb Parameter rgb.
 * @return Return value.
 */
ui_error_t ui_color_picker_base_set_rgb(struct ui_color_picker_base *picker,
                                        const struct ui_color_rgb *rgb) {
  if (!picker || !rgb) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker->rgb = *rgb;
  {
    ui_error_t rc_cleanup =
        ui_color_picker_rgb_to_hsv(&picker->rgb, &picker->hsv);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return trigger_cva_change(picker);
}

/**
 * @brief ui_color_picker_base_set_hsv.
 * @param picker Parameter picker.
 * @param hsv Parameter hsv.
 * @return Return value.
 */
ui_error_t ui_color_picker_base_set_hsv(struct ui_color_picker_base *picker,
                                        const struct ui_color_hsv *hsv) {
  if (!picker || !hsv) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker->hsv = *hsv;
  {
    ui_error_t rc_cleanup =
        ui_color_picker_hsv_to_rgb(&picker->hsv, &picker->rgb);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return trigger_cva_change(picker);
}
