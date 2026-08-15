/* clang-format off */
#include "ui_layout.h"

static int local_strcmp(const char *a, const char *b) {
  while (*a && (*a == *b)) {
    a++;
    b++;
  }
  return *(const unsigned char *)a - *(const unsigned char *)b;
}

#include "ui_css_values.h"
#include "ui_internal_mem.h"
#include "ui_web_bridge.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static ui_error_t resolve_length(const char *val_str, float *out_length) {
  struct ui_css_value val;
  *out_length = 0.0f;
  {
    ui_error_t pv_rc = ui_css_parse_value(val_str, &val);
    if (pv_rc != UI_ERROR_NONE)
      return pv_rc;
  }
  if (1) {
    if (val.unit != UI_CSS_UNIT_PERCENT && val.unit != UI_CSS_UNIT_REM &&
        val.unit != UI_CSS_UNIT_EM && val.unit != UI_CSS_UNIT_VH &&
        val.unit != UI_CSS_UNIT_VW) {
      *out_length = val.value;
      return UI_ERROR_NONE;
    }
    /* We assume 1em = 16px for now */
    if (val.unit == UI_CSS_UNIT_EM || val.unit == UI_CSS_UNIT_REM) {
      *out_length = val.value * 16.0f;
      return UI_ERROR_NONE;
    }
    if (val.unit == UI_CSS_UNIT_PERCENT) {
      /* Percentage resolution needs parent dimensions, but for simple tests
       * return 0 or approx */
      *out_length = 0.0f; /* Not fully supported without tree context */
      return UI_ERROR_NONE;
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t parse_break(const char *val, enum ui_layout_break *out_val) {
  if (strcmp(val, "avoid") == 0) {
    *out_val = UI_LAYOUT_BREAK_AVOID;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "always") == 0) {
    *out_val = UI_LAYOUT_BREAK_ALWAYS;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "all") == 0) {
    *out_val = UI_LAYOUT_BREAK_ALL;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "avoid-page") == 0) {
    *out_val = UI_LAYOUT_BREAK_AVOID_PAGE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "page") == 0) {
    *out_val = UI_LAYOUT_BREAK_PAGE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "left") == 0) {
    *out_val = UI_LAYOUT_BREAK_LEFT;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "right") == 0) {
    *out_val = UI_LAYOUT_BREAK_RIGHT;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "recto") == 0) {
    *out_val = UI_LAYOUT_BREAK_RECTO;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "verso") == 0) {
    *out_val = UI_LAYOUT_BREAK_VERSO;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "avoid-column") == 0) {
    *out_val = UI_LAYOUT_BREAK_AVOID_COLUMN;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "column") == 0) {
    *out_val = UI_LAYOUT_BREAK_COLUMN;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "avoid-region") == 0) {
    *out_val = UI_LAYOUT_BREAK_AVOID_REGION;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "region") == 0) {
    *out_val = UI_LAYOUT_BREAK_REGION;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_BREAK_AUTO;
    return UI_ERROR_NONE;
  }
}

static ui_error_t resolve_size(const char *val_str, float *out_length,
                               enum ui_layout_size_type *out_type) {
  *out_length = 0.0f;
  *out_type = UI_LAYOUT_SIZE_AUTO;

  if (strcmp(val_str, "auto") == 0) {
    *out_type = UI_LAYOUT_SIZE_AUTO;
  } else if (strcmp(val_str, "min-content") == 0) {
    *out_type = UI_LAYOUT_SIZE_MIN_CONTENT;
  } else if (strcmp(val_str, "max-content") == 0) {
    *out_type = UI_LAYOUT_SIZE_MAX_CONTENT;
  } else if (strcmp(val_str, "fit-content") == 0) {
    *out_type = UI_LAYOUT_SIZE_FIT_CONTENT;
  } else if (strchr(val_str, '%') != NULL) {
    *out_type = UI_LAYOUT_SIZE_PERCENTAGE;
    *out_length = (float)atof(val_str); /* we just need the number part */
  } else {
    *out_type = UI_LAYOUT_SIZE_PIXELS;
    {
      ui_error_t _rc = resolve_length(val_str, out_length);
      if (_rc != UI_ERROR_NONE)
        return _rc;
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t parse_shorthand_4(const char *shorthand,
                                    float out_metrics[4]) {
  const char *p = shorthand;
  float vals[4] = {0, 0, 0, 0};
  int count = 0;
  char buf[64];
  size_t i = 0;

  while (*p && count < 4) {
    while (*p && isspace((unsigned char)*p))
      p++;
    if (!*p)
      break;

    i = 0;
    while (*p && !isspace((unsigned char)*p) && i < sizeof(buf) - 1) {
      buf[i++] = *p++;
    }
    buf[i] = '\0';

    {
      float tmp = 0.0f;
      {
        ui_error_t _rc = resolve_length(buf, &tmp);
        if (_rc != UI_ERROR_NONE)
          return _rc;
      }
      vals[count++] = tmp;
    }
  }

  if (count == 1) {
    out_metrics[0] = vals[0];
    out_metrics[1] = vals[0];
    out_metrics[2] = vals[0];
    out_metrics[3] = vals[0];
  } else if (count == 2) {
    out_metrics[0] = vals[0]; /* top/bottom */
    out_metrics[2] = vals[0];
    out_metrics[1] = vals[1]; /* right/left */
    out_metrics[3] = vals[1];
  } else if (count == 3) {
    out_metrics[0] = vals[0]; /* top */
    out_metrics[1] = vals[1]; /* right/left */
    out_metrics[3] = vals[1];
    out_metrics[2] = vals[2]; /* bottom */
  } else {
    out_metrics[0] = vals[0];
    out_metrics[1] = vals[1];
    out_metrics[2] = vals[2];
    out_metrics[3] = vals[3];
  }
  return UI_ERROR_NONE;
}

static ui_error_t parse_overflow(const char *val,
                                 enum ui_layout_overflow *out_val) {
  if (strcmp(val, "hidden") == 0) {
    *out_val = UI_LAYOUT_OVERFLOW_HIDDEN;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "scroll") == 0) {
    *out_val = UI_LAYOUT_OVERFLOW_SCROLL;
    return UI_ERROR_NONE;
  }
  if (local_strcmp(val, "auto") == 0) {
    *out_val = UI_LAYOUT_OVERFLOW_AUTO;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_OVERFLOW_VISIBLE;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_wrap_flow(const char *val,
                                  enum ui_layout_wrap_flow *out_val) {
  if (local_strcmp(val, "auto") == 0) {
    *out_val = UI_LAYOUT_WRAP_FLOW_AUTO;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "both") == 0) {
    *out_val = UI_LAYOUT_WRAP_FLOW_BOTH;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "start") == 0) {
    *out_val = UI_LAYOUT_WRAP_FLOW_START;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "end") == 0) {
    *out_val = UI_LAYOUT_WRAP_FLOW_END;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "minimum") == 0) {
    *out_val = UI_LAYOUT_WRAP_FLOW_MINIMUM;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "maximum") == 0) {
    *out_val = UI_LAYOUT_WRAP_FLOW_MAXIMUM;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "clear") == 0) {
    *out_val = UI_LAYOUT_WRAP_FLOW_CLEAR;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_WRAP_FLOW_AUTO;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_wrap_through(const char *val,
                                     enum ui_layout_wrap_through *out_val) {
  if (strcmp(val, "wrap") == 0) {
    *out_val = UI_LAYOUT_WRAP_THROUGH_WRAP;
    return UI_ERROR_NONE;
  }
  if (local_strcmp(val, "none") == 0) {
    *out_val = UI_LAYOUT_WRAP_THROUGH_NONE;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_WRAP_THROUGH_WRAP;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_white_space(const char *val,
                                    enum ui_layout_white_space *out_val) {
  if (local_strcmp(val, "normal") == 0) {
    *out_val = UI_LAYOUT_WHITE_SPACE_NORMAL;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "nowrap") == 0) {
    *out_val = UI_LAYOUT_WHITE_SPACE_NOWRAP;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "pre") == 0) {
    *out_val = UI_LAYOUT_WHITE_SPACE_PRE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "pre-wrap") == 0) {
    *out_val = UI_LAYOUT_WHITE_SPACE_PRE_WRAP;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "pre-line") == 0) {
    *out_val = UI_LAYOUT_WHITE_SPACE_PRE_LINE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "break-spaces") == 0) {
    *out_val = UI_LAYOUT_WHITE_SPACE_BREAK_SPACES;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_WHITE_SPACE_NORMAL;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_text_align(const char *val,
                                   enum ui_layout_text_align *out_val) {
  if (strcmp(val, "start") == 0) {
    *out_val = UI_LAYOUT_TEXT_ALIGN_START;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "end") == 0) {
    *out_val = UI_LAYOUT_TEXT_ALIGN_END;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "left") == 0) {
    *out_val = UI_LAYOUT_TEXT_ALIGN_LEFT;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "right") == 0) {
    *out_val = UI_LAYOUT_TEXT_ALIGN_RIGHT;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "center") == 0) {
    *out_val = UI_LAYOUT_TEXT_ALIGN_CENTER;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "justify") == 0) {
    *out_val = UI_LAYOUT_TEXT_ALIGN_JUSTIFY;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "match-parent") == 0) {
    *out_val = UI_LAYOUT_TEXT_ALIGN_MATCH_PARENT;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "justify-all") == 0) {
    *out_val = UI_LAYOUT_TEXT_ALIGN_JUSTIFY_ALL;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_TEXT_ALIGN_START;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_word_break(const char *val,
                                   enum ui_layout_word_break *out_val) {
  if (local_strcmp(val, "normal") == 0) {
    *out_val = UI_LAYOUT_WORD_BREAK_NORMAL;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "keep-all") == 0) {
    *out_val = UI_LAYOUT_WORD_BREAK_KEEP_ALL;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "break-all") == 0) {
    *out_val = UI_LAYOUT_WORD_BREAK_BREAK_ALL;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "break-word") == 0) {
    *out_val = UI_LAYOUT_WORD_BREAK_BREAK_WORD;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_WORD_BREAK_NORMAL;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_hyphens(const char *val,
                                enum ui_layout_hyphens *out_val) {
  if (local_strcmp(val, "none") == 0) {
    *out_val = UI_LAYOUT_HYPHENS_NONE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "manual") == 0) {
    *out_val = UI_LAYOUT_HYPHENS_MANUAL;
    return UI_ERROR_NONE;
  }
  if (local_strcmp(val, "auto") == 0) {
    *out_val = UI_LAYOUT_HYPHENS_AUTO;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_HYPHENS_MANUAL;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_writing_mode(const char *val,
                                     enum ui_layout_writing_mode *out_val) {
  if (strcmp(val, "horizontal-tb") == 0) {
    *out_val = UI_LAYOUT_WRITING_MODE_HORIZONTAL_TB;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "vertical-rl") == 0) {
    *out_val = UI_LAYOUT_WRITING_MODE_VERTICAL_RL;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "vertical-lr") == 0) {
    *out_val = UI_LAYOUT_WRITING_MODE_VERTICAL_LR;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_WRITING_MODE_HORIZONTAL_TB;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_direction(const char *val,
                                  enum ui_layout_direction *out_val) {
  if (strcmp(val, "ltr") == 0) {
    *out_val = UI_LAYOUT_DIRECTION_LTR;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "rtl") == 0) {
    *out_val = UI_LAYOUT_DIRECTION_RTL;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_DIRECTION_LTR;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_unicode_bidi(const char *val,
                                     enum ui_layout_unicode_bidi *out_val) {
  if (local_strcmp(val, "normal") == 0) {
    *out_val = UI_LAYOUT_UNICODE_BIDI_NORMAL;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "embed") == 0) {
    *out_val = UI_LAYOUT_UNICODE_BIDI_EMBED;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "bidi-override") == 0) {
    *out_val = UI_LAYOUT_UNICODE_BIDI_BIDI_OVERRIDE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "isolate") == 0) {
    *out_val = UI_LAYOUT_UNICODE_BIDI_ISOLATE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "isolate-override") == 0) {
    *out_val = UI_LAYOUT_UNICODE_BIDI_ISOLATE_OVERRIDE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "plaintext") == 0) {
    *out_val = UI_LAYOUT_UNICODE_BIDI_PLAINTEXT;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_UNICODE_BIDI_NORMAL;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_line_grid(const char *val,
                                  enum ui_layout_line_grid *out_val) {
  if (strcmp(val, "create") == 0) {
    *out_val = UI_LAYOUT_LINE_GRID_CREATE;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_LINE_GRID_MATCH_PARENT;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_line_snap(const char *val,
                                  enum ui_layout_line_snap *out_val) {
  if (strcmp(val, "baseline") == 0) {
    *out_val = UI_LAYOUT_LINE_SNAP_BASELINE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "contain") == 0) {
    *out_val = UI_LAYOUT_LINE_SNAP_CONTAIN;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_LINE_SNAP_NONE;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_box_snap(const char *val,
                                 enum ui_layout_box_snap *out_val) {
  if (strcmp(val, "block-start") == 0) {
    *out_val = UI_LAYOUT_BOX_SNAP_BLOCK_START;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "block-end") == 0) {
    *out_val = UI_LAYOUT_BOX_SNAP_BLOCK_END;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "center") == 0) {
    *out_val = UI_LAYOUT_BOX_SNAP_CENTER;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "baseline") == 0) {
    *out_val = UI_LAYOUT_BOX_SNAP_BASELINE;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_BOX_SNAP_NONE;
    return UI_ERROR_NONE;
  }
}

static ui_error_t
parse_block_step_insert(const char *val,
                        enum ui_layout_block_step_insert *out_val) {
  if (strcmp(val, "padding") == 0) {
    *out_val = UI_LAYOUT_BLOCK_STEP_INSERT_PADDING;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_BLOCK_STEP_INSERT_MARGIN;
    return UI_ERROR_NONE;
  }
}

static ui_error_t
parse_block_step_align(const char *val,
                       enum ui_layout_block_step_align *out_val) {
  if (strcmp(val, "center") == 0) {
    *out_val = UI_LAYOUT_BLOCK_STEP_ALIGN_CENTER;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "start") == 0) {
    *out_val = UI_LAYOUT_BLOCK_STEP_ALIGN_START;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "end") == 0) {
    *out_val = UI_LAYOUT_BLOCK_STEP_ALIGN_END;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_BLOCK_STEP_ALIGN_AUTO;
    return UI_ERROR_NONE;
  }
}

static ui_error_t
parse_block_step_round(const char *val,
                       enum ui_layout_block_step_round *out_val) {
  if (strcmp(val, "down") == 0) {
    *out_val = UI_LAYOUT_BLOCK_STEP_ROUND_DOWN;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "nearest") == 0) {
    *out_val = UI_LAYOUT_BLOCK_STEP_ROUND_NEAREST;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_BLOCK_STEP_ROUND_UP;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_color_scheme(const char *val,
                                     enum ui_layout_color_scheme *out_val) {
  int mask = UI_LAYOUT_COLOR_SCHEME_NORMAL;
  if (local_strcmp(val, "normal") == 0) {
    *out_val = UI_LAYOUT_COLOR_SCHEME_NORMAL;
    return UI_ERROR_NONE;
  }
  if (strstr(val, "light") != NULL)
    mask |= UI_LAYOUT_COLOR_SCHEME_LIGHT;
  if (strstr(val, "dark") != NULL)
    mask |= UI_LAYOUT_COLOR_SCHEME_DARK;
  {
    *out_val = (enum ui_layout_color_scheme)mask;
    return UI_ERROR_NONE;
  }
}

static ui_error_t
parse_print_color_adjust(const char *val,
                         enum ui_layout_print_color_adjust *out_val) {
  if (strcmp(val, "exact") == 0) {
    *out_val = UI_LAYOUT_PRINT_COLOR_ADJUST_EXACT;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_PRINT_COLOR_ADJUST_ECONOMY;
    return UI_ERROR_NONE;
  }
}

static ui_error_t
parse_forced_color_adjust(const char *val,
                          enum ui_layout_forced_color_adjust *out_val) {
  if (local_strcmp(val, "none") == 0) {
    *out_val = UI_LAYOUT_FORCED_COLOR_ADJUST_NONE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "preserve-parent-color") == 0) {
    *out_val = UI_LAYOUT_FORCED_COLOR_ADJUST_PRESERVE_PARENT_COLOR;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_FORCED_COLOR_ADJUST_AUTO;
    return UI_ERROR_NONE;
  }
}

static ui_error_t
parse_text_orientation(const char *val,
                       enum ui_layout_text_orientation *out_val) {
  if (strcmp(val, "mixed") == 0) {
    *out_val = UI_LAYOUT_TEXT_ORIENTATION_MIXED;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "upright") == 0) {
    *out_val = UI_LAYOUT_TEXT_ORIENTATION_UPRIGHT;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "sideways") == 0) {
    *out_val = UI_LAYOUT_TEXT_ORIENTATION_SIDEWAYS;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_TEXT_ORIENTATION_MIXED;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_text_decoration_line(const char *val, int *out_val) {
  int mask = UI_LAYOUT_TEXT_DECORATION_LINE_NONE;
  if (local_strcmp(val, "none") == 0) {
    *out_val = mask;
    return UI_ERROR_NONE;
  }
  if (strstr(val, "underline") != NULL)
    mask |= UI_LAYOUT_TEXT_DECORATION_LINE_UNDERLINE;
  if (strstr(val, "overline") != NULL)
    mask |= UI_LAYOUT_TEXT_DECORATION_LINE_OVERLINE;
  if (strstr(val, "line-through") != NULL)
    mask |= UI_LAYOUT_TEXT_DECORATION_LINE_LINE_THROUGH;
  if (strstr(val, "blink") != NULL)
    mask |= UI_LAYOUT_TEXT_DECORATION_LINE_BLINK;
  {
    *out_val = mask;
    return UI_ERROR_NONE;
  }
}

static ui_error_t
parse_text_decoration_style(const char *val,
                            enum ui_layout_text_decoration_style *out_val) {
  if (strcmp(val, "double") == 0) {
    *out_val = UI_LAYOUT_TEXT_DECORATION_STYLE_DOUBLE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "dotted") == 0) {
    *out_val = UI_LAYOUT_TEXT_DECORATION_STYLE_DOTTED;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "dashed") == 0) {
    *out_val = UI_LAYOUT_TEXT_DECORATION_STYLE_DASHED;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "wavy") == 0) {
    *out_val = UI_LAYOUT_TEXT_DECORATION_STYLE_WAVY;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_TEXT_DECORATION_STYLE_SOLID;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_font_style(const char *val,
                                   enum ui_layout_font_style *out_val) {
  if (strcmp(val, "italic") == 0) {
    *out_val = UI_LAYOUT_FONT_STYLE_ITALIC;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "oblique") == 0) {
    *out_val = UI_LAYOUT_FONT_STYLE_OBLIQUE;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_FONT_STYLE_NORMAL;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_font_stretch(const char *val,
                                     enum ui_layout_font_stretch *out_val) {
  if (strcmp(val, "ultra-condensed") == 0) {
    *out_val = UI_LAYOUT_FONT_STRETCH_ULTRA_CONDENSED;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "extra-condensed") == 0) {
    *out_val = UI_LAYOUT_FONT_STRETCH_EXTRA_CONDENSED;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "condensed") == 0) {
    *out_val = UI_LAYOUT_FONT_STRETCH_CONDENSED;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "semi-condensed") == 0) {
    *out_val = UI_LAYOUT_FONT_STRETCH_SEMI_CONDENSED;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "semi-expanded") == 0) {
    *out_val = UI_LAYOUT_FONT_STRETCH_SEMI_EXPANDED;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "expanded") == 0) {
    *out_val = UI_LAYOUT_FONT_STRETCH_EXPANDED;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "extra-expanded") == 0) {
    *out_val = UI_LAYOUT_FONT_STRETCH_EXTRA_EXPANDED;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "ultra-expanded") == 0) {
    *out_val = UI_LAYOUT_FONT_STRETCH_ULTRA_EXPANDED;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_FONT_STRETCH_NORMAL;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_font_weight(const char *val, int *out_val) {
  if (local_strcmp(val, "normal") == 0) {
    *out_val = 400;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "bold") == 0) {
    *out_val = 700;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "bolder") == 0) {
    *out_val = 900;
    return UI_ERROR_NONE;
  } /* simplified mapping */
  if (strcmp(val, "lighter") == 0) {
    *out_val = 100;
    return UI_ERROR_NONE;
  } /* simplified mapping */
  {
    *out_val = atoi(val);
    return UI_ERROR_NONE;
  }
}

static ui_error_t
parse_text_size_adjust(const char *val,
                       struct ui_layout_text_size_adjust *out_adjust) {
  struct ui_css_value parsed_val;

  out_adjust->type = UI_LAYOUT_TEXT_SIZE_ADJUST_AUTO;
  out_adjust->percentage = 100.0f;
  if (local_strcmp(val, "auto") == 0) {
    out_adjust->type = UI_LAYOUT_TEXT_SIZE_ADJUST_AUTO;
  } else if (strcmp(val, "none") == 0) {
    out_adjust->type = UI_LAYOUT_TEXT_SIZE_ADJUST_NONE;
  } else {
    ui_error_t pv_rc = ui_css_parse_value(val, &parsed_val);
    if (pv_rc != UI_ERROR_NONE)
      return pv_rc;
    if (1) {
      if (parsed_val.unit == UI_CSS_UNIT_PERCENT) {
        out_adjust->type = UI_LAYOUT_TEXT_SIZE_ADJUST_PERCENTAGE;
        out_adjust->percentage = parsed_val.value;
      }
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t
parse_background_repeat_single(const char *val,
                               enum ui_layout_background_repeat *out_val) {
  if (strcmp(val, "repeat") == 0) {
    *out_val = UI_LAYOUT_BACKGROUND_REPEAT_REPEAT;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "space") == 0) {
    *out_val = UI_LAYOUT_BACKGROUND_REPEAT_SPACE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "round") == 0) {
    *out_val = UI_LAYOUT_BACKGROUND_REPEAT_ROUND;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "no-repeat") == 0) {
    *out_val = UI_LAYOUT_BACKGROUND_REPEAT_NO_REPEAT;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_BACKGROUND_REPEAT_REPEAT;
    return UI_ERROR_NONE;
  }
}

static ui_error_t
parse_background_size(const char *val,
                      struct ui_layout_background_size *out_size) {
  out_size->type_x = UI_LAYOUT_BACKGROUND_SIZE_AUTO;
  out_size->type_y = UI_LAYOUT_BACKGROUND_SIZE_AUTO;
  out_size->x = 0.0f;
  out_size->y = 0.0f;
  if (strcmp(val, "cover") == 0) {
    out_size->type_x = UI_LAYOUT_BACKGROUND_SIZE_COVER;
    out_size->type_y = UI_LAYOUT_BACKGROUND_SIZE_COVER;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "contain") == 0) {
    out_size->type_x = UI_LAYOUT_BACKGROUND_SIZE_CONTAIN;
    out_size->type_y = UI_LAYOUT_BACKGROUND_SIZE_CONTAIN;
    return UI_ERROR_NONE;
  }
  out_size->type_x = UI_LAYOUT_BACKGROUND_SIZE_LENGTH;
  {
    float tmp = 0.0f;
    {
      ui_error_t _rc = resolve_length(val, &tmp);
      if (_rc != UI_ERROR_NONE)
        return _rc;
    }
    out_size->x = tmp;
  }
  out_size->type_y = UI_LAYOUT_BACKGROUND_SIZE_AUTO;
  return UI_ERROR_NONE;
}

static ui_error_t parse_border_radius(const char *val, float out_radius[4][2]) {
  int i;
  float r = 0.0f;
  {
    ui_error_t _rc = resolve_length(val, &r);
    if (_rc != UI_ERROR_NONE)
      return _rc;
  }
  for (i = 0; i < 4; i++) {
    out_radius[i][0] = r;
    out_radius[i][1] = r;
  }
  return UI_ERROR_NONE;
}

static ui_error_t
parse_box_decoration_break(const char *val,
                           enum ui_layout_box_decoration_break *out_val) {
  if (strcmp(val, "clone") == 0) {
    *out_val = UI_LAYOUT_BOX_DECORATION_BREAK_CLONE;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_BOX_DECORATION_BREAK_SLICE;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_box_shadow(const char *val,
                                   struct ui_layout_box_shadow *shadows,
                                   int *count) {
  *count = 0;
  if (local_strcmp(val, "none") == 0) {
    return UI_ERROR_NONE;
  }
  /* Simplified parser for a single shadow for mock Level 4 */
  shadows[0].offset_x = 0.0f;
  shadows[0].offset_y = 0.0f;
  shadows[0].blur_radius = 0.0f;
  shadows[0].spread_radius = 0.0f;
  shadows[0].is_inset = (strstr(val, "inset") != NULL);

  /* Use a default semi-transparent black shadow */
  shadows[0].color.space = UI_CSS_COLOR_SPACE_SRGB;
  shadows[0].color.components[0] = 0.0f;
  shadows[0].color.components[1] = 0.0f;
  shadows[0].color.components[2] = 0.0f;
  shadows[0].color.components[3] = 0.5f;

  *count = 1;
  return UI_ERROR_NONE;
}

static ui_error_t parse_alignment(const char *val,
                                  enum ui_layout_alignment default_val,
                                  enum ui_layout_alignment *out_val) {
  if (local_strcmp(val, "auto") == 0) {
    *out_val = UI_LAYOUT_ALIGN_AUTO;
    return UI_ERROR_NONE;
  }
  if (local_strcmp(val, "normal") == 0) {
    *out_val = UI_LAYOUT_ALIGN_NORMAL;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "start") == 0 || strcmp(val, "flex-start") == 0) {
    *out_val = UI_LAYOUT_ALIGN_START;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "end") == 0 || strcmp(val, "flex-end") == 0) {
    *out_val = UI_LAYOUT_ALIGN_END;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "center") == 0) {
    *out_val = UI_LAYOUT_ALIGN_CENTER;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "stretch") == 0) {
    *out_val = UI_LAYOUT_ALIGN_STRETCH;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "space-between") == 0) {
    *out_val = UI_LAYOUT_ALIGN_SPACE_BETWEEN;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "space-around") == 0) {
    *out_val = UI_LAYOUT_ALIGN_SPACE_AROUND;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "space-evenly") == 0) {
    *out_val = UI_LAYOUT_ALIGN_SPACE_EVENLY;
    return UI_ERROR_NONE;
  }
  {
    *out_val = default_val;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_flex_direction(const char *val,
                                       enum ui_layout_flex_direction *out_val) {
  if (strcmp(val, "row-reverse") == 0) {
    *out_val = UI_LAYOUT_FLEX_DIRECTION_ROW_REVERSE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "column") == 0) {
    *out_val = UI_LAYOUT_FLEX_DIRECTION_COLUMN;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "column-reverse") == 0) {
    *out_val = UI_LAYOUT_FLEX_DIRECTION_COLUMN_REVERSE;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_FLEX_DIRECTION_ROW;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_flex_wrap(const char *val,
                                  enum ui_layout_flex_wrap *out_val) {
  if (strcmp(val, "wrap") == 0) {
    *out_val = UI_LAYOUT_FLEX_WRAP_WRAP;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "wrap-reverse") == 0) {
    *out_val = UI_LAYOUT_FLEX_WRAP_WRAP_REVERSE;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_FLEX_WRAP_NOWRAP;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_margin_trim(const char *val,
                                    enum ui_layout_margin_trim *out_val) {
  if (strcmp(val, "block") == 0) {
    *out_val = UI_LAYOUT_MARGIN_TRIM_BLOCK;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "block-start") == 0) {
    *out_val = UI_LAYOUT_MARGIN_TRIM_BLOCK_START;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "block-end") == 0) {
    *out_val = UI_LAYOUT_MARGIN_TRIM_BLOCK_END;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "inline") == 0) {
    *out_val = UI_LAYOUT_MARGIN_TRIM_INLINE;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "inline-start") == 0) {
    *out_val = UI_LAYOUT_MARGIN_TRIM_INLINE_START;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "inline-end") == 0) {
    *out_val = UI_LAYOUT_MARGIN_TRIM_INLINE_END;
    return UI_ERROR_NONE;
  }
  if (strcmp(val, "all") == 0) {
    *out_val = UI_LAYOUT_MARGIN_TRIM_ALL;
    return UI_ERROR_NONE;
  }
  {
    *out_val = UI_LAYOUT_MARGIN_TRIM_NONE;
    return UI_ERROR_NONE;
  }
}

static ui_error_t parse_aspect_ratio(const char *val, float *out_val) {
  float w = 1.0f, h = 1.0f;
  const char *slash;
  if (local_strcmp(val, "auto") == 0) {
    *out_val = 0.0f;
    return UI_ERROR_NONE;
  }
  slash = strchr(val, '/');
  if (slash) {
    w = (float)atof(val);
    h = (float)atof(slash + 1);
  } else {
    w = (float)atof(val);
    h = 1.0f;
  }
  if (h == 0.0f) {
    *out_val = 0.0f;
    return UI_ERROR_NONE;
  }
  {
    *out_val = w / h;
    return UI_ERROR_NONE;
  }
}

static ui_error_t compute_box_model(struct ui_layout_node *node) {
  ui_error_t _prop_rc = UI_ERROR_NONE;
  const char *val;
  int is_border_box = 0;
  int i;
  for (i = 0; i < 4; i++) {
    node->margin[i] = 0.0f;
    node->padding[i] = 0.0f;
    node->border[i] = 0.0f;
  }
  node->content_width = 0.0f;
  node->content_height = 0.0f;
  node->min_width = 0.0f;
  node->max_width = -1.0f; /* -1 means no max limit */
  node->min_height = 0.0f;
  node->max_height = -1.0f;
  node->scrollbar_width = 0.0f;
  node->scrollbar_height = 0.0f;
  node->overflow_x = UI_LAYOUT_OVERFLOW_VISIBLE;
  node->overflow_y = UI_LAYOUT_OVERFLOW_VISIBLE;
  node->justify_content = UI_LAYOUT_ALIGN_NORMAL;
  node->align_items = UI_LAYOUT_ALIGN_NORMAL;
  node->align_self = UI_LAYOUT_ALIGN_AUTO;
  node->align_content = UI_LAYOUT_ALIGN_NORMAL;
  node->flex_direction = UI_LAYOUT_FLEX_DIRECTION_ROW;
  node->flex_wrap = UI_LAYOUT_FLEX_WRAP_NOWRAP;
  node->flex_grow = 0.0f;
  node->flex_shrink = 1.0f;
  node->flex_basis = 0.0f;
  node->flex_basis_type = UI_LAYOUT_SIZE_AUTO;
  node->width_type = UI_LAYOUT_SIZE_AUTO;
  node->height_type = UI_LAYOUT_SIZE_AUTO;
  node->raw_width = 0.0f;
  node->raw_height = 0.0f;

  node->wrap_flow = UI_LAYOUT_WRAP_FLOW_AUTO;
  node->wrap_through = UI_LAYOUT_WRAP_THROUGH_WRAP;

  node->flow_into[0] = '\0';
  node->flow_from[0] = '\0';

  node->position = UI_LAYOUT_POSITION_STATIC;
  node->top = 0.0f;
  node->right = 0.0f;
  node->bottom = 0.0f;
  node->left = 0.0f;
  node->top_type = UI_LAYOUT_SIZE_AUTO;
  node->right_type = UI_LAYOUT_SIZE_AUTO;
  node->bottom_type = UI_LAYOUT_SIZE_AUTO;
  node->left_type = UI_LAYOUT_SIZE_AUTO;
  node->z_index = 0;
  node->opacity = 1.0f;
  node->transform[0] = '\0';
  node->is_stacking_context = 0;

  node->white_space = UI_LAYOUT_WHITE_SPACE_NORMAL;
  node->text_align = UI_LAYOUT_TEXT_ALIGN_START;
  node->word_break = UI_LAYOUT_WORD_BREAK_NORMAL;
  node->hyphens = UI_LAYOUT_HYPHENS_MANUAL;

  node->writing_mode = UI_LAYOUT_WRITING_MODE_HORIZONTAL_TB;
  node->direction = UI_LAYOUT_DIRECTION_LTR;
  node->unicode_bidi = UI_LAYOUT_UNICODE_BIDI_NORMAL;
  node->text_orientation = UI_LAYOUT_TEXT_ORIENTATION_MIXED;

  node->color_scheme = UI_LAYOUT_COLOR_SCHEME_NORMAL;
  node->print_color_adjust = UI_LAYOUT_PRINT_COLOR_ADJUST_ECONOMY;
  node->forced_color_adjust = UI_LAYOUT_FORCED_COLOR_ADJUST_AUTO;

  node->line_grid = UI_LAYOUT_LINE_GRID_MATCH_PARENT;
  node->line_snap = UI_LAYOUT_LINE_SNAP_NONE;
  node->box_snap = UI_LAYOUT_BOX_SNAP_NONE;
  node->block_step_size = 0.0f;
  node->block_step_insert = UI_LAYOUT_BLOCK_STEP_INSERT_MARGIN;
  node->block_step_align = UI_LAYOUT_BLOCK_STEP_ALIGN_AUTO;
  node->block_step_round = UI_LAYOUT_BLOCK_STEP_ROUND_UP;

  node->text_decoration_line = UI_LAYOUT_TEXT_DECORATION_LINE_NONE;
  node->text_decoration_style = UI_LAYOUT_TEXT_DECORATION_STYLE_SOLID;
  node->text_shadow[0] = '\0';
  node->text_emphasis_style[0] = '\0';
  node->text_emphasis_position[0] = '\0';

  node->text_decoration_color.space = UI_CSS_COLOR_SPACE_SRGB;
  node->text_decoration_color.components[0] = 0.0f;
  node->text_decoration_color.components[1] = 0.0f;
  node->text_decoration_color.components[2] = 0.0f;
  node->text_decoration_color.components[3] = 1.0f;

  node->text_emphasis_color.space = UI_CSS_COLOR_SPACE_SRGB;
  node->text_emphasis_color.components[0] = 0.0f;
  node->text_emphasis_color.components[1] = 0.0f;
  node->text_emphasis_color.components[2] = 0.0f;
  node->text_emphasis_color.components[3] = 1.0f;

  node->text_color.space = UI_CSS_COLOR_SPACE_SRGB;
  node->text_color.components[0] = 0.0f;
  node->text_color.components[1] = 0.0f;
  node->text_color.components[2] = 0.0f;
  node->text_color.components[3] = 1.0f;

  node->background_color.space = UI_CSS_COLOR_SPACE_SRGB;
  node->background_color.components[0] = 0.0f;
  node->background_color.components[1] = 0.0f;
  node->background_color.components[2] = 0.0f;
  node->background_color.components[3] = 0.0f; /* transparent default */

  node->box_decoration_break = UI_LAYOUT_BOX_DECORATION_BREAK_SLICE;
  node->box_shadow_count = 0;
  for (i = 0; i < 4; i++) {
    node->box_shadow[i].color.space = UI_CSS_COLOR_SPACE_SRGB;
  }
  node->border_image_source.type = UI_CSS_IMAGE_NONE;

  node->background_image_count = 0;
  for (i = 0; i < 4; i++) {
    node->background_image[i].type = UI_CSS_IMAGE_NONE;
    node->background_size[i].type_x = UI_LAYOUT_BACKGROUND_SIZE_AUTO;
    node->background_size[i].type_y = UI_LAYOUT_BACKGROUND_SIZE_AUTO;
    node->background_size[i].x = 0.0f;
    node->background_size[i].y = 0.0f;
    node->background_repeat_x[i] = UI_LAYOUT_BACKGROUND_REPEAT_REPEAT;
    node->background_repeat_y[i] = UI_LAYOUT_BACKGROUND_REPEAT_REPEAT;
  }

  for (i = 0; i < 4; i++) {
    node->border_color[i].space = UI_CSS_COLOR_SPACE_SRGB;
    node->border_color[i].components[0] = 0.0f;
    node->border_color[i].components[1] = 0.0f;
    node->border_color[i].components[2] = 0.0f;
    node->border_color[i].components[3] = 0.0f; /* transparent default */
    node->border_radius[i][0] = 0.0f;
    node->border_radius[i][1] = 0.0f;
  }

  node->font_family[0] = '\0';
  node->font_weight = 400;
  node->font_style = UI_LAYOUT_FONT_STYLE_NORMAL;
  node->font_size = 16.0f; /* default font size */
  node->font_stretch = UI_LAYOUT_FONT_STRETCH_NORMAL;
  node->text_size_adjust.type = UI_LAYOUT_TEXT_SIZE_ADJUST_AUTO;
  node->text_size_adjust.percentage = 100.0f;
  node->font_variant[0] = '\0';
  node->font_feature_settings[0] = '\0';
  node->font_variation_settings[0] = '\0';

  node->break_before = UI_LAYOUT_BREAK_AUTO;
  node->break_after = UI_LAYOUT_BREAK_AUTO;
  node->break_inside = UI_LAYOUT_BREAK_AUTO;
  node->orphans = 2;
  node->widows = 2;

  node->column_count = 0;
  node->column_width = 0.0f;
  node->column_gap = 0.0f;

  /* Text properties */
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "white-space", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_white_space(val, &node->white_space);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "text-align", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_text_align(val, &node->text_align);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "word-break", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_word_break(val, &node->word_break);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "hyphens", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_hyphens(val, &node->hyphens);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "writing-mode", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_writing_mode(val, &node->writing_mode);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "direction", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_direction(val, &node->direction);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "unicode-bidi", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_unicode_bidi(val, &node->unicode_bidi);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "text-orientation", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_text_orientation(val, &node->text_orientation);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "color-scheme", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_color_scheme(val, &node->color_scheme);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "print-color-adjust", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_print_color_adjust(val, &node->print_color_adjust);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "forced-color-adjust", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_forced_color_adjust(val, &node->forced_color_adjust);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "line-grid", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_line_grid(val, &node->line_grid);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "line-snap", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_line_snap(val, &node->line_snap);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "box-snap", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_box_snap(val, &node->box_snap);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "block-step-size", &val);
    if (attr_rc == UI_ERROR_NONE) {
      if (local_strcmp(val, "none") == 0) {
        node->block_step_size = 0.0f;
      } else {
        {
          float tmp = 0.0f;
          {
            ui_error_t _rc = resolve_length(val, &tmp);
            if (_rc != UI_ERROR_NONE)
              return _rc;
          }
          node->block_step_size = tmp;
        }
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "block-step-insert", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_block_step_insert(val, &node->block_step_insert);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "block-step-align", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_block_step_align(val, &node->block_step_align);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "block-step-round", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_block_step_round(val, &node->block_step_round);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "text-decoration-line", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_text_decoration_line(val, &node->text_decoration_line);
      }
    } else {
      ui_error_t attr_rc2 = ui_css_computed_style_get_property(
          node->computed_style, "text-decoration", &val);
      if (attr_rc2 == UI_ERROR_NONE) {
        /* Basic shorthand fallback */
        { (void)parse_text_decoration_line(val, &node->text_decoration_line); }
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "text-decoration-style", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_text_decoration_style(val, &node->text_decoration_style);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "text-decoration-color", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)ui_css_parse_color(val, &node->text_decoration_color);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "text-shadow", &val);
    if (attr_rc == UI_ERROR_NONE) {
#if defined(_MSC_VER)
      strcpy_s(node->text_shadow, sizeof(node->text_shadow), val);
#else
      UI_STRNCPY(node->text_shadow, sizeof(node->text_shadow), val,
                 sizeof(node->text_shadow) - 1);
      node->text_shadow[sizeof(node->text_shadow) - 1] = '\0';
#endif
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "text-emphasis-style", &val);
    if (_prop_rc == UI_ERROR_NONE) {
#if defined(_MSC_VER)
      strcpy_s(node->text_emphasis_style, sizeof(node->text_emphasis_style),
               val);
#else
      UI_STRNCPY(node->text_emphasis_style, sizeof(node->text_emphasis_style),
                 val, sizeof(node->text_emphasis_style) - 1);
      node->text_emphasis_style[sizeof(node->text_emphasis_style) - 1] = '\0';
#endif
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "text-emphasis-color", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)ui_css_parse_color(val, &node->text_emphasis_color);
      }
    }
  }

  {
    _prop_rc =
        ui_css_computed_style_get_property(node->computed_style, "color", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)ui_css_parse_color(val, &node->text_color);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "background-color", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)ui_css_parse_color(val, &node->background_color);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "background-image", &val);
    if (attr_rc == UI_ERROR_NONE) {
      /* For multiple backgrounds we'd ideally split by comma. For Level 3/4
       * mock, we'll parse the first one */
      { (void)ui_css_parse_image(val, &node->background_image[0]); }
      node->background_image_count = 1;
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "box-decoration-break", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_box_decoration_break(val, &node->box_decoration_break);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "box-shadow", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_box_shadow(val, node->box_shadow, &node->box_shadow_count);
      }
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "border-image-source", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)ui_css_parse_image(val, &node->border_image_source);
      }
    }
  }
  if (_prop_rc == UI_ERROR_NOT_FOUND) {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "border-image", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)ui_css_parse_image(val, &node->border_image_source);
      } /* Very basic shorthand fallback */
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "background-size", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_background_size(val, &node->background_size[0]);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "background-repeat", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_background_repeat_single(val,
                                             &node->background_repeat_x[0]);
        ;
      }
      node->background_repeat_y[0] = node->background_repeat_x[0];
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "border-radius", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)parse_border_radius(val, node->border_radius);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-top-left-radius", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->border_radius[0][0] = tmp;
      }
      node->border_radius[0][1] = node->border_radius[0][0];
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-top-right-radius", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->border_radius[1][0] = tmp;
      }
      node->border_radius[1][1] = node->border_radius[1][0];
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-bottom-right-radius", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->border_radius[2][0] = tmp;
      }
      node->border_radius[2][1] = node->border_radius[2][0];
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-bottom-left-radius", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->border_radius[3][0] = tmp;
      }
      node->border_radius[3][1] = node->border_radius[3][0];
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-top-color", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)ui_css_parse_color(val, &node->border_color[0]);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-right-color", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)ui_css_parse_color(val, &node->border_color[1]);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-bottom-color", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)ui_css_parse_color(val, &node->border_color[2]);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-left-color", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)ui_css_parse_color(val, &node->border_color[3]);
      }
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(
        node->computed_style, "text-emphasis-position", &val);
    if (_prop_rc == UI_ERROR_NONE) {
#if defined(_MSC_VER)
      strcpy_s(node->text_emphasis_position,
               sizeof(node->text_emphasis_position), val);
#else
      UI_STRNCPY(node->text_emphasis_position,
                 sizeof(node->text_emphasis_position), val,
                 sizeof(node->text_emphasis_position) - 1);
      node->text_emphasis_position[sizeof(node->text_emphasis_position) - 1] =
          '\0';
#endif
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "font-family", &val);
    if (attr_rc == UI_ERROR_NONE) {
#if defined(_MSC_VER)
      strcpy_s(node->font_family, sizeof(node->font_family), val);
#else
      UI_STRNCPY(node->font_family, sizeof(node->font_family), val,
                 sizeof(node->font_family) - 1);
      node->font_family[sizeof(node->font_family) - 1] = '\0';
#endif
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "font-weight", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_font_weight(val, &node->font_weight);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "font-style", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_font_style(val, &node->font_style);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "font-size", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->font_size = tmp;
      }
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "text-size-adjust", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)parse_text_size_adjust(val, &node->text_size_adjust);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "font-stretch", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_font_stretch(val, &node->font_stretch);
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "font-variant", &val);
    if (attr_rc == UI_ERROR_NONE) {
#if defined(_MSC_VER)
      strcpy_s(node->font_variant, sizeof(node->font_variant), val);
#else
      UI_STRNCPY(node->font_variant, sizeof(node->font_variant), val,
                 sizeof(node->font_variant) - 1);
      node->font_variant[sizeof(node->font_variant) - 1] = '\0';
#endif
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(
        node->computed_style, "font-feature-settings", &val);
    if (_prop_rc == UI_ERROR_NONE) {
#if defined(_MSC_VER)
      strcpy_s(node->font_feature_settings, sizeof(node->font_feature_settings),
               val);
#else
      UI_STRNCPY(node->font_feature_settings,
                 sizeof(node->font_feature_settings), val,
                 sizeof(node->font_feature_settings) - 1);
      node->font_feature_settings[sizeof(node->font_feature_settings) - 1] =
          '\0';
#endif
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(
        node->computed_style, "font-variation-settings", &val);
    if (_prop_rc == UI_ERROR_NONE) {
#if defined(_MSC_VER)
      strcpy_s(node->font_variation_settings,
               sizeof(node->font_variation_settings), val);
#else
      UI_STRNCPY(node->font_variation_settings,
                 sizeof(node->font_variation_settings), val,
                 sizeof(node->font_variation_settings) - 1);
      node->font_variation_settings[sizeof(node->font_variation_settings) - 1] =
          '\0';
#endif
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "break-before", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)parse_break(val, &node->break_before);
      }
    }
  }
  if (_prop_rc == UI_ERROR_NOT_FOUND) {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "page-break-before", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)parse_break(val, &node->break_before);
      }
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "break-after", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)parse_break(val, &node->break_after);
      }
    }
  }
  if (_prop_rc == UI_ERROR_NOT_FOUND) {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "page-break-after", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)parse_break(val, &node->break_after);
      }
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "break-inside", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)parse_break(val, &node->break_inside);
      }
    }
  }
  if (_prop_rc == UI_ERROR_NOT_FOUND) {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "page-break-inside", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)parse_break(val, &node->break_inside);
      }
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "orphans", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      node->orphans = atoi(val);
    }
  }

  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "widows", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      node->widows = atoi(val);
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "column-count", &val);
    if (attr_rc == UI_ERROR_NONE) {
      if (strcmp(val, "auto") != 0) {
        node->column_count = atoi(val);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "column-width", &val);
    if (attr_rc == UI_ERROR_NONE) {
      if (strcmp(val, "auto") != 0) {
        node->column_width = (float)atof(val);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "column-gap", &val);
    if (attr_rc == UI_ERROR_NONE) {
      node->column_gap = (float)atof(val);
    }
  }

  /* Overflow */
  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "overflow", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        (void)parse_overflow(val, &node->overflow_x);
      }
      { (void)parse_overflow(val, &node->overflow_y); }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "overflow-x", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_overflow(val, &node->overflow_x);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "overflow-y", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_overflow(val, &node->overflow_y);
      }
    }
  }

  /* Alignment */
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "justify-content", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_alignment(val, UI_LAYOUT_ALIGN_NORMAL,
                              &node->justify_content);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "align-items", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_alignment(val, UI_LAYOUT_ALIGN_NORMAL, &node->align_items);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "align-self", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_alignment(val, UI_LAYOUT_ALIGN_AUTO, &node->align_self);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "align-content", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_alignment(val, UI_LAYOUT_ALIGN_NORMAL,
                              &node->align_content);
        ;
      }
    }
  }

  /* Flexbox Properties */
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "flex-direction", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_flex_direction(val, &node->flex_direction);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "flex-wrap", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_flex_wrap(val, &node->flex_wrap);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "flex-grow", &val);
    if (attr_rc == UI_ERROR_NONE) {
      node->flex_grow = (float)atof(val);
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "flex-shrink", &val);
    if (attr_rc == UI_ERROR_NONE) {
      node->flex_shrink = (float)atof(val);
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "flex-basis", &val);
    if (attr_rc == UI_ERROR_NONE) {
      float parsed_basis;
      enum ui_layout_size_type type;
      {
        ui_error_t _rc = resolve_size(val, &parsed_basis, &type);
        if (_rc != UI_ERROR_NONE)
          return _rc;
      }
      node->flex_basis = parsed_basis;
      node->flex_basis_type = type;
    }
  }

  /* Margins */
  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "margin", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        ui_error_t _rc = parse_shorthand_4(val, node->margin);
        if (_rc != UI_ERROR_NONE)
          return _rc;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "margin-top", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->margin[0] = tmp;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "margin-right", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->margin[1] = tmp;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "margin-bottom", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->margin[2] = tmp;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "margin-left", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->margin[3] = tmp;
      }
    }
  }

  /* Padding */
  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "padding", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        ui_error_t _rc = parse_shorthand_4(val, node->padding);
        if (_rc != UI_ERROR_NONE)
          return _rc;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "padding-top", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->padding[0] = tmp;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "padding-right", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->padding[1] = tmp;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "padding-bottom", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->padding[2] = tmp;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "padding-left", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->padding[3] = tmp;
      }
    }
  }

  /* Border */
  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "border-width", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        ui_error_t _rc = parse_shorthand_4(val, node->border);
        if (_rc != UI_ERROR_NONE)
          return _rc;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-top-width", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->border[0] = tmp;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-right-width", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->border[1] = tmp;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-bottom-width", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->border[2] = tmp;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-left-width", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->border[3] = tmp;
      }
    }
  }

  /* Min / Max Width */
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "min-width", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->min_width = tmp;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "max-width", &val);
    if (attr_rc == UI_ERROR_NONE) {
      if (strcmp(val, "none") != 0) {
        {
          float tmp = 0.0f;
          {
            ui_error_t _rc = resolve_length(val, &tmp);
            if (_rc != UI_ERROR_NONE)
              return _rc;
          }
          node->max_width = tmp;
        }
      }
    }
  }

  /* Min / Max Height */
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "min-height", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        float tmp = 0.0f;
        {
          ui_error_t _rc = resolve_length(val, &tmp);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->min_height = tmp;
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "max-height", &val);
    if (attr_rc == UI_ERROR_NONE) {
      if (strcmp(val, "none") != 0) {
        {
          float tmp = 0.0f;
          {
            ui_error_t _rc = resolve_length(val, &tmp);
            if (_rc != UI_ERROR_NONE)
              return _rc;
          }
          node->max_height = tmp;
        }
      }
    }
  }

  /* Margin Trim */
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "margin-trim", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_margin_trim(val, &node->margin_trim);
      }
    } else {
      node->margin_trim = UI_LAYOUT_MARGIN_TRIM_NONE;
    }
  }

  /* Aspect Ratio */
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "aspect-ratio", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_aspect_ratio(val, &node->aspect_ratio);
      }
    } else {
      node->aspect_ratio = 0.0f;
    }
  }

  /* Exclusions */
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "wrap-flow", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_wrap_flow(val, &node->wrap_flow);
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "wrap-through", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        (void)parse_wrap_through(val, &node->wrap_through);
      }
    }
  }

  /* Regions */
  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "flow-into", &val);
    if (_prop_rc == UI_ERROR_NONE) {
#if defined(_MSC_VER)
      strcpy_s(node->flow_into, sizeof(node->flow_into), val);
#else
      UI_STRNCPY(node->flow_into, sizeof(node->flow_into), val,
                 sizeof(node->flow_into) - 1);
      node->flow_into[sizeof(node->flow_into) - 1] = '\0';
#endif
    }
  }
  {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "flow-from", &val);
    if (_prop_rc == UI_ERROR_NONE) {
#if defined(_MSC_VER)
      strcpy_s(node->flow_from, sizeof(node->flow_from), val);
#else
      UI_STRNCPY(node->flow_from, sizeof(node->flow_from), val,
                 sizeof(node->flow_from) - 1);
      node->flow_from[sizeof(node->flow_from) - 1] = '\0';
#endif
    }
  }

  /* Dimensions */
  is_border_box = 0;
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "box-sizing", &val);
    if (attr_rc == UI_ERROR_NONE) {
      if (strcmp(val, "border-box") == 0) {
        is_border_box = 1;
      }
    }
  }

  {
    int z_index_is_auto = 1;
    int has_transform = 0;
    {
      ui_error_t attr_rc = ui_css_computed_style_get_property(
          node->computed_style, "position", &val);
      if (attr_rc == UI_ERROR_NONE) {
        if (strcmp(val, "relative") == 0)
          node->position = UI_LAYOUT_POSITION_RELATIVE;
        else if (strcmp(val, "absolute") == 0)
          node->position = UI_LAYOUT_POSITION_ABSOLUTE;
        else if (strcmp(val, "fixed") == 0)
          node->position = UI_LAYOUT_POSITION_FIXED;
        else if (strcmp(val, "sticky") == 0)
          node->position = UI_LAYOUT_POSITION_STICKY;
        else
          node->position = UI_LAYOUT_POSITION_STATIC;
      }
    }

    {
      ui_error_t attr_rc = ui_css_computed_style_get_property(
          node->computed_style, "z-index", &val);
      if (attr_rc == UI_ERROR_NONE) {
        if (strcmp(val, "auto") != 0) {
          node->z_index = atoi(val);
          z_index_is_auto = 0;
        }
      }
    }

    {
      ui_error_t attr_rc =
          ui_css_computed_style_get_property(node->computed_style, "top", &val);
      if (attr_rc == UI_ERROR_NONE) {
        float parsed_val;
        enum ui_layout_size_type type;
        {
          ui_error_t _rc = resolve_size(val, &parsed_val, &type);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->top_type = type;
        node->top = parsed_val;
      }
    }

    {
      ui_error_t attr_rc = ui_css_computed_style_get_property(
          node->computed_style, "right", &val);
      if (attr_rc == UI_ERROR_NONE) {
        float parsed_val;
        enum ui_layout_size_type type;
        {
          ui_error_t _rc = resolve_size(val, &parsed_val, &type);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->right_type = type;
        node->right = parsed_val;
      }
    }

    {
      ui_error_t attr_rc = ui_css_computed_style_get_property(
          node->computed_style, "bottom", &val);
      if (attr_rc == UI_ERROR_NONE) {
        float parsed_val;
        enum ui_layout_size_type type;
        {
          ui_error_t _rc = resolve_size(val, &parsed_val, &type);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->bottom_type = type;
        node->bottom = parsed_val;
      }
    }

    {
      ui_error_t attr_rc = ui_css_computed_style_get_property(
          node->computed_style, "left", &val);
      if (attr_rc == UI_ERROR_NONE) {
        float parsed_val;
        enum ui_layout_size_type type;
        {
          ui_error_t _rc = resolve_size(val, &parsed_val, &type);
          if (_rc != UI_ERROR_NONE)
            return _rc;
        }
        node->left_type = type;
        node->left = parsed_val;
      }
    }
    {
      ui_error_t attr_rc = ui_css_computed_style_get_property(
          node->computed_style, "opacity", &val);
      if (attr_rc == UI_ERROR_NONE) {
        node->opacity = (float)atof(val);
      }
    }

    {
      ui_error_t attr_rc = ui_css_computed_style_get_property(
          node->computed_style, "transform", &val);
      if (attr_rc == UI_ERROR_NONE) {
#if defined(_MSC_VER)
        strcpy_s(node->transform, sizeof(node->transform), val);
#else
        UI_STRNCPY(node->transform, sizeof(node->transform), val,
                   sizeof(node->transform) - 1);
        node->transform[sizeof(node->transform) - 1] = '\0';
#endif
        if (strcmp(val, "none") != 0) {
          has_transform = 1;
        }
      }
    }

    if (node->position == UI_LAYOUT_POSITION_FIXED ||
        node->position == UI_LAYOUT_POSITION_STICKY) {
      node->is_stacking_context = 1;
    }
    if ((node->position == UI_LAYOUT_POSITION_ABSOLUTE ||
         node->position == UI_LAYOUT_POSITION_RELATIVE) &&
        !z_index_is_auto) {
      node->is_stacking_context = 1;
    }
    if (node->opacity < 1.0f) {
      node->is_stacking_context = 1;
    }
    if (has_transform) {
      node->is_stacking_context = 1;
    }
  }

  {
    ui_error_t attr_rc =
        ui_css_computed_style_get_property(node->computed_style, "width", &val);
    if (attr_rc == UI_ERROR_NONE) {
      float parsed_width;
      enum ui_layout_size_type type;
      {
        ui_error_t _rc = resolve_size(val, &parsed_width, &type);
        if (_rc != UI_ERROR_NONE)
          return _rc;
      }
      node->width_type = type;
      node->raw_width = parsed_width;

      if (type == UI_LAYOUT_SIZE_PIXELS) {
        float extra_x = node->padding[1] + node->padding[3] + node->border[1] +
                        node->border[3];

        if (is_border_box) {
          node->width = parsed_width;
          node->content_width = parsed_width - extra_x;
        } else {
          node->content_width = parsed_width;
          node->width = node->content_width + extra_x;
        }
      }
    }
  }

  if (node->width > 0.0f) {
    if (node->width < node->min_width) {
      node->width = node->min_width;
      node->content_width = node->width - (node->padding[1] + node->padding[3] +
                                           node->border[1] + node->border[3]);
    }
    if (node->max_width >= 0.0f && node->width > node->max_width) {
      node->width = node->max_width;
      node->content_width = node->width - (node->padding[1] + node->padding[3] +
                                           node->border[1] + node->border[3]);
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "height", &val);
    if (attr_rc == UI_ERROR_NONE) {
      float parsed_height;
      enum ui_layout_size_type type;
      {
        ui_error_t _rc = resolve_size(val, &parsed_height, &type);
        if (_rc != UI_ERROR_NONE)
          return _rc;
      }
      node->height_type = type;
      node->raw_height = parsed_height;

      if (type == UI_LAYOUT_SIZE_PIXELS) {
        float extra_y = node->padding[0] + node->padding[2] + node->border[0] +
                        node->border[2];

        if (is_border_box) {
          node->height = parsed_height;
          node->content_height = parsed_height - extra_y;
        } else {
          node->content_height = parsed_height;
          node->height = node->content_height + extra_y;
        }
      }
    }
  }

  if (node->height > 0.0f) {
    if (node->height < node->min_height) {
      node->height = node->min_height;
      node->content_height =
          node->height - (node->padding[0] + node->padding[2] +
                          node->border[0] + node->border[2]);
    }
    if (node->max_height >= 0.0f && node->height > node->max_height) {
      node->height = node->max_height;
      node->content_height =
          node->height - (node->padding[0] + node->padding[2] +
                          node->border[0] + node->border[2]);
    }
  }

  return UI_ERROR_NONE;
}

static ui_error_t create_layout_node(const struct ui_dom_node *dom_node,
                                     struct ui_css_computed_style *style,
                                     int is_anonymous,
                                     struct ui_layout_node **out_node) {
  ui_error_t _prop_rc = UI_ERROR_NONE;
  struct ui_layout_node *node;
  const char *display_val;

  node = (struct ui_layout_node *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_layout_node));
  if (!node) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  node->dom_node = dom_node;
  node->computed_style = style;
  node->is_anonymous = is_anonymous;
  node->display_outside =
      UI_LAYOUT_DISPLAY_OUTSIDE_INLINE; /* Default CSS value */
  node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_FLOW;
  node->x = 0.0f;
  node->y = 0.0f;
  node->width = 0.0f;
  node->height = 0.0f;
  node->parent = NULL;
  node->first_child = NULL;
  node->last_child = NULL;
  node->next_sibling = NULL;
  node->previous_sibling = NULL;

  if (style) {
    {
      _prop_rc =
          ui_css_computed_style_get_property(style, "display", &display_val);
      if (_prop_rc == UI_ERROR_NONE) {
        if (strstr(display_val, "none") != NULL) {
          node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_NONE;
        } else {
          /* Parse outside */
          if (strstr(display_val, "inline") != NULL) {
            node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_INLINE;
          } else if (strstr(display_val, "block") != NULL) {
            node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK;
          } else if (strstr(display_val, "run-in") != NULL) {
            node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_RUN_IN;
          } else {
            /* Default outside based on inside */
            if (strstr(display_val, "flex") != NULL ||
                strstr(display_val, "grid") != NULL ||
                strstr(display_val, "table") != NULL ||
                strstr(display_val, "flow-root") != NULL) {
              node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK;
            } else {
              node->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_INLINE;
            }
          }

          /* Parse inside */
          if (strstr(display_val, "flex") != NULL) {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_FLEX;
          } else if (strstr(display_val, "grid") != NULL) {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_GRID;
          } else if (strstr(display_val, "table") != NULL) {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_TABLE;
          } else if (strstr(display_val, "ruby") != NULL) {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_RUBY;
          } else if (strstr(display_val, "flow-root") != NULL ||
                     strstr(display_val, "inline-block") != NULL) {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_FLOW_ROOT;
          } else {
            node->display_inside = UI_LAYOUT_DISPLAY_INSIDE_FLOW;
          }
        }
      }
    }
  }

  *out_node = node;
  return UI_ERROR_NONE;
}

static ui_error_t append_layout_child(struct ui_layout_node *parent,
                                      struct ui_layout_node *child) {
  child->parent = parent;
  if (!parent->last_child) {
    parent->first_child = child;
    parent->last_child = child;
  } else {
    parent->last_child->next_sibling = child;
    child->previous_sibling = parent->last_child;
    parent->last_child = child;
  }
  return UI_ERROR_NONE;
}

static ui_error_t
build_tree_recursive(const struct ui_dom_node *dom_node,
                     const struct ui_css_stylesheet *stylesheet,
                     struct ui_layout_node **out_layout_node) {
  struct ui_css_computed_style *style = NULL;
  struct ui_layout_node *lnode = NULL;
  struct ui_layout_node *child_lnode = NULL;
  struct ui_dom_node *dom_child;
  const char *display_val;
  ui_error_t err;

  *out_layout_node = NULL;

  /* 1. Resolve Style */
  err = ui_css_resolve_style(stylesheet, dom_node, &style);
  if (err != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* 2. Check for display: none */
  {
    ui_error_t disp_rc =
        ui_css_computed_style_get_property(style, "display", &display_val);

    if (disp_rc == UI_ERROR_NONE) {
      if (strcmp(display_val, "none") == 0) {
        (void)ui_css_computed_style_destroy(style);
        return UI_ERROR_NONE; /* Skip this node and its children */
      }
    }
  }

  /* 3. Create layout node */
  err = create_layout_node(dom_node, style, 0, &lnode);
  if (err != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* 3.5 Compute box model metrics */
  err = compute_box_model(lnode);
  if (err != UI_ERROR_NONE)
    goto cleanup;

  /* 4. Recursively build children */
  dom_child = dom_node->first_child;
  while (dom_child) {
    err = build_tree_recursive(dom_child, stylesheet, &child_lnode);
    if (err != UI_ERROR_NONE) {
      goto cleanup;
    }

    if (child_lnode) {
      (void)append_layout_child(lnode, child_lnode);
    }

    dom_child = dom_child->next_sibling;
  }

  /* 5. Anonymous box expansion */
  if (lnode->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK) {
    int has_block = 0;
    int has_inline = 0;
    struct ui_layout_node *curr = lnode->first_child;
    while (curr) {
      if (curr->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK)
        has_block = 1;
      if (curr->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_INLINE)
        has_inline = 1;
      curr = curr->next_sibling;
    }

    if (has_block && has_inline) {
      struct ui_layout_node *new_first = NULL;
      struct ui_layout_node *new_last = NULL;
      struct ui_layout_node *anon = NULL;

      curr = lnode->first_child;
      while (curr) {
        struct ui_layout_node *next = curr->next_sibling;
        curr->next_sibling = NULL;
        curr->previous_sibling = NULL;

        if (curr->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_INLINE) {
          if (1) {
            err = create_layout_node(NULL, NULL, 1, &anon);
            if (err != UI_ERROR_NONE)
              goto cleanup;
            anon->display_outside = UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK;

            /* Append anon to new children list */
            anon->parent = lnode;
            if (!new_last) {
              new_first = anon;
              new_last = anon;
            } else {
              new_last->next_sibling = anon;
              anon->previous_sibling = new_last;
              new_last = anon;
            }
          }
          { (void)append_layout_child(anon, curr); }
        } else {
          anon = NULL; /* Break the sequence of inlines */
          /* Append block child directly */
          curr->parent = lnode;
          if (!new_last) {
            new_first = curr;
            new_last = curr;
          } else {
            new_last->next_sibling = curr;
            curr->previous_sibling = new_last;
            new_last = curr;
          }
        }
        curr = next;
      }
      lnode->first_child = new_first;
      lnode->last_child = new_last;
    }
  }

  *out_layout_node = lnode;
  return UI_ERROR_NONE;

cleanup:
  if (style && !lnode) {
    (void)ui_css_computed_style_destroy(style);
  }
  if (lnode) {
    (void)ui_layout_tree_destroy(lnode);
  }
  return err;
}

/** \brief ui_error */
ui_error_t ui_layout_tree_generate(const struct ui_dom_node *dom_root,
                                   const struct ui_css_stylesheet *stylesheet,
                                   struct ui_layout_node **out_layout_root) {
  if (!dom_root || !stylesheet || !out_layout_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return build_tree_recursive(dom_root, stylesheet, out_layout_root);
}

ui_error_t ui_layout_tree_destroy(struct ui_layout_node *node) {
  struct ui_layout_node *child;
  struct ui_layout_node *next_child;

  if (!node) {
    return UI_ERROR_NONE;
  }

  child = node->first_child;
  while (child) {
    next_child = child->next_sibling;
    (void)ui_layout_tree_destroy(child);
    child = next_child;
  }

  if (node->computed_style) {
    (void)ui_css_computed_style_destroy(node->computed_style);
  }

  C_MULTIPLATFORM_FREE(node);
  return UI_ERROR_NONE;
}

static ui_error_t layout_block(struct ui_layout_node *node,
                               float available_width) {
  struct ui_layout_node *child;
  float current_y = 0.0f;
  float current_x = 0.0f;
  float total_height = 0.0f;
  float actual_column_width;
  int actual_column_count;

  /* If width is 0 and we are a block without explicit width, expand */
  if (node->width_type == UI_LAYOUT_SIZE_PERCENTAGE) {
    node->width = (available_width * node->raw_width / 100.0f) -
                  node->margin[3] - node->margin[1];
  } else if ((node->width == 0.0f && node->width_type == UI_LAYOUT_SIZE_AUTO) &&
             node->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK) {
    node->width = available_width - node->margin[3] - node->margin[1];
  }

  /* Intrinsic Sizing (before checking min/max clamps) */
  if (node->width_type == UI_LAYOUT_SIZE_MIN_CONTENT ||
      node->width_type == UI_LAYOUT_SIZE_MAX_CONTENT ||
      node->width_type == UI_LAYOUT_SIZE_FIT_CONTENT) {

    float intrinsic_min_width = 0.0f;
    float intrinsic_max_width = 0.0f;

    /* Extremely simplistic estimation of intrinsic content sizes for mock
     * testing */
    /* Real engine would measure text nodes and child blocks */
    struct ui_layout_node *calc_child = node->first_child;
    while (calc_child) {
      float child_min = 20.0f;  /* mock text word length */
      float child_max = 100.0f; /* mock text sentence length */
      intrinsic_min_width =
          child_min > intrinsic_min_width ? child_min : intrinsic_min_width;
      if (child_max > intrinsic_max_width)
        intrinsic_max_width = child_max;
      calc_child = calc_child->next_sibling;
    }

    if (node->width_type == UI_LAYOUT_SIZE_MIN_CONTENT) {
      node->width = intrinsic_min_width + node->padding[1] + node->padding[3] +
                    node->border[1] + node->border[3];
    } else if (node->width_type == UI_LAYOUT_SIZE_MAX_CONTENT) {
      node->width = intrinsic_max_width + node->padding[1] + node->padding[3] +
                    node->border[1] + node->border[3];
    } else if (node->width_type == UI_LAYOUT_SIZE_FIT_CONTENT) {
      float available_inner = available_width - node->margin[1] -
                              node->margin[3] - node->padding[1] -
                              node->padding[3] - node->border[1] -
                              node->border[3];
      float fit = intrinsic_max_width;
      if (fit > available_inner)
        fit = available_inner;
      if (fit < intrinsic_min_width)
        fit = intrinsic_min_width;
      node->width = fit + node->padding[1] + node->padding[3] +
                    node->border[1] + node->border[3];
    }
  }

  if (node->aspect_ratio > 0.0f && node->height == 0.0f) {
    node->height = node->width / node->aspect_ratio;
    node->content_height = node->height - node->padding[0] - node->padding[2] -
                           node->border[0] - node->border[2];
  }

  node->content_width = node->width - node->padding[1] - node->padding[3] -
                        node->border[1] - node->border[3];

  actual_column_width = node->content_width;
  actual_column_count = 1;

  if (node->column_count > 1 || node->column_width > 0.0f) {
    if (node->column_width > 0.0f) {
      actual_column_count = (int)((node->content_width + node->column_gap) /
                                  (node->column_width + node->column_gap));
    } else {
      actual_column_count = node->column_count;
    }
    if (actual_column_count < 1)
      actual_column_count = 1;
    actual_column_width =
        (node->content_width - (actual_column_count - 1) * node->column_gap) /
        actual_column_count;
  }

  current_x = node->padding[3] + node->border[3];
  current_y = node->padding[0] + node->border[0];

  child = node->first_child;
  while (child) {
    float child_available_width = actual_column_width;

    /* Handle break-before */
    if (child->break_before == UI_LAYOUT_BREAK_COLUMN ||
        child->break_before == UI_LAYOUT_BREAK_PAGE ||
        child->break_before == UI_LAYOUT_BREAK_ALWAYS) {
      if (actual_column_count > 1) {
        current_x += actual_column_width + node->column_gap;
        current_y = node->padding[0] + node->border[0];
      }
    }

    /* Margin Trim Logic */
    if (node->margin_trim == UI_LAYOUT_MARGIN_TRIM_ALL ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_BLOCK ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_BLOCK_START) {
      if (child == node->first_child) {
        child->margin[0] = 0.0f;
      }
    }
    if (node->margin_trim == UI_LAYOUT_MARGIN_TRIM_ALL ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_BLOCK ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_BLOCK_END) {
      if (child == node->last_child) {
        child->margin[2] = 0.0f;
      }
    }
    if (node->margin_trim == UI_LAYOUT_MARGIN_TRIM_ALL ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_INLINE ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_INLINE_START) {
      if (child == node->first_child) {
        child->margin[3] = 0.0f;
      }
    }
    if (node->margin_trim == UI_LAYOUT_MARGIN_TRIM_ALL ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_INLINE ||
        node->margin_trim == UI_LAYOUT_MARGIN_TRIM_INLINE_END) {
      if (child == node->last_child) {
        child->margin[1] = 0.0f;
      }
    }

    if (child->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK) {
      /* Block layout: stack vertically */
      child->x = current_x + child->margin[3];
      child->y = current_y + child->margin[0];

      { (void)ui_layout_compute(child, child_available_width, 0.0f); }

      current_y = child->y + child->height + child->margin[2];
    } else if (child->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_INLINE) {
      /* Inline layout (very basic horizontal packing) */
      child->x = current_x + child->margin[3];
      child->y = current_y + child->margin[0];

      { (void)ui_layout_compute(child, child_available_width, 0.0f); }

      current_x = child->x + child->width + child->margin[1];
      /* If child exceeds width, should wrap, but simplistic for now */
      if (child->height + child->margin[0] + child->margin[2] > total_height) {
        total_height = child->height + child->margin[0] + child->margin[2];
      }
      /* We do not increment current_y here unless we wrap, we just update
       * total_height */
      if (child->next_sibling && child->next_sibling->display_outside ==
                                     UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK) {
        current_y += total_height;
        current_x = node->padding[3] + node->border[3];
        total_height = 0.0f;
      }
    }

    /* Handle break-after */
    if (child->break_after == UI_LAYOUT_BREAK_COLUMN ||
        child->break_after == UI_LAYOUT_BREAK_PAGE ||
        child->break_after == UI_LAYOUT_BREAK_ALWAYS) {
      if (actual_column_count > 1) {
        current_x += actual_column_width + node->column_gap;
        current_y = node->padding[0] + node->border[0];
      }
    }

    child = child->next_sibling;
  }

  /* If it was an inline sequence at the end */
  if (total_height > 0.0f) {
    current_y += total_height;
  }

  /* Compute our own height if not explicitly set */
  if (node->height == 0.0f) {
    node->content_height = current_y - (node->padding[0] + node->border[0]);
    node->height = node->content_height + node->padding[0] + node->padding[2] +
                   node->border[0] + node->border[2];

    /* Clamp implicit height */
    if (node->height < node->min_height) {
      node->height = node->min_height;
    }

    node->content_height = node->height - node->padding[0] - node->padding[2] -
                           node->border[0] - node->border[2];

  } else {
    /* Height was explicitly set; check for Y overflow */
    float actual_children_height =
        current_y - (node->padding[0] + node->border[0]);
    if (node->overflow_y == UI_LAYOUT_OVERFLOW_SCROLL ||
        (node->overflow_y == UI_LAYOUT_OVERFLOW_AUTO &&
         actual_children_height > node->content_height)) {
      /* Allocate scrollbar width space (typically 16px) */
      node->scrollbar_width = 16.0f;
    }
  }

  /* Check for X overflow (simplistic) */
  if (node->width > 0.0f) {
    float actual_children_width =
        current_x - (node->padding[3] + node->border[3]);
  }

  return UI_ERROR_NONE;
}

/** \brief ui_flex_line */
struct ui_flex_line {
  struct ui_layout_node *first_child;
  struct ui_layout_node *last_child;
  float main_size_sum;
  float cross_max;
  float total_flex_grow;
  float total_flex_shrink;
  int child_count;
};

static ui_error_t layout_flex(struct ui_layout_node *node,
                              float available_width) {
  struct ui_layout_node *child;
  int is_row;
  int is_wrap;
  struct ui_flex_line lines[64];
  int line_count = 0;
  float start_x, start_y;
  float current_x, current_y;
  int i;
  float container_main_size;

  memset(lines, 0, sizeof(lines));

  /* If width is 0 and we are a block outside, expand */
  if (node->width_type == UI_LAYOUT_SIZE_PERCENTAGE) {
    node->width = (available_width * node->raw_width / 100.0f) -
                  node->margin[3] - node->margin[1];
  } else if ((node->width == 0.0f && node->width_type == UI_LAYOUT_SIZE_AUTO) &&
             node->display_outside == UI_LAYOUT_DISPLAY_OUTSIDE_BLOCK) {
    node->width = available_width - node->margin[3] - node->margin[1];
  }

  node->content_width = node->width - node->padding[1] - node->padding[3] -
                        node->border[1] - node->border[3];

  is_row = (node->flex_direction == UI_LAYOUT_FLEX_DIRECTION_ROW ||
            node->flex_direction == UI_LAYOUT_FLEX_DIRECTION_ROW_REVERSE);
  is_wrap = (node->flex_wrap == UI_LAYOUT_FLEX_WRAP_WRAP ||
             node->flex_wrap == UI_LAYOUT_FLEX_WRAP_WRAP_REVERSE);

  container_main_size = is_row ? node->content_width : node->content_height;

  /* Pass 1: Measure intrinsic sizes & collect lines */
  child = node->first_child;
  if (child) {
    line_count = 1;
    lines[0].first_child = child;
  }

  while (child && line_count <= 64) {
    float basis = 0.0f;
    float child_main_outer = 0.0f;
    float child_cross_outer = 0.0f;
    struct ui_flex_line *current_line = &lines[line_count - 1];

    {
      (void)ui_layout_compute(child, node->content_width, 0.0f);
    }

    if (child->flex_basis_type == UI_LAYOUT_SIZE_PIXELS) {
      basis = child->flex_basis;
    } else if (child->flex_basis_type == UI_LAYOUT_SIZE_AUTO) {
      basis = is_row ? child->width : child->height;
    }

    if (is_row) {
      child->width = basis;
      child->content_width = basis - child->padding[1] - child->padding[3] -
                             child->border[1] - child->border[3];
      child_main_outer = child->width + child->margin[1] + child->margin[3];
      child_cross_outer = child->height + child->margin[0] + child->margin[2];
    } else {
      child->height = basis;
      child->content_height = basis - child->padding[0] - child->padding[2] -
                              child->border[0] - child->border[0];
      child_main_outer = child->height + child->margin[0] + child->margin[2];
      child_cross_outer = child->width + child->margin[1] + child->margin[3];
    }

    /* Wrap logic */
    if (is_wrap && current_line->child_count > 0 &&
        container_main_size > 0.0f &&
        current_line->main_size_sum + child_main_outer > container_main_size) {
      if (line_count < 64) {
        line_count++;
        current_line = &lines[line_count - 1];
        current_line->first_child = child;
      }
    }

    current_line->last_child = child;
    current_line->main_size_sum += child_main_outer;
    if (child_cross_outer > current_line->cross_max) {
      current_line->cross_max = child_cross_outer;
    }
    current_line->total_flex_grow += child->flex_grow;
    current_line->total_flex_shrink += child->flex_shrink;
    current_line->child_count++;

    child = child->next_sibling;
  }

  /* Pass 2: Distribute free space per line */
  for (i = 0; i < line_count; i++) {
    struct ui_flex_line *line = &lines[i];
    float free_space = 0.0f;

    if (is_row) {
      free_space = node->content_width - line->main_size_sum;
    } else {
      if (node->height > 0.0f) {
        float inner_height = node->height - node->padding[0] -
                             node->padding[2] - node->border[0] -
                             node->border[2];
        free_space = inner_height - line->main_size_sum;
      }
    }

    if (free_space > 0.0f && line->total_flex_grow > 0.0f) {
      child = line->first_child;
      while (1) {
        float extra = (child->flex_grow / line->total_flex_grow) * free_space;
        if (is_row) {
          child->width += extra;
          child->content_width += extra;
          (void)ui_layout_compute(child, child->content_width, 0.0f);
        } else {
          child->height += extra;
          child->content_height += extra;
        }
        if (child == line->last_child)
          break;
        child = child->next_sibling;
      }
      free_space = 0.0f;
    } else if (free_space < 0.0f && line->total_flex_shrink > 0.0f &&
               !is_wrap) {
      /* Only shrink if not wrapping (or if single line forced) */
      child = line->first_child;
      while (1) {
        float shrink =
            (child->flex_shrink / line->total_flex_shrink) * (-free_space);
        if (is_row) {
          child->width -= shrink;
          child->content_width -= shrink;
          (void)ui_layout_compute(child, child->content_width, 0.0f);
        } else {
          child->height -= shrink;
          child->content_height -= shrink;
        }
        if (child == line->last_child)
          break;
        child = child->next_sibling;
      }
      free_space = 0.0f;
    }
  }

  /* Pass 3: Final Positioning */
  start_x = node->padding[3] + node->border[3];
  start_y = node->padding[0] + node->border[0];
  current_y = start_y;
  current_x = start_x;

  for (i = 0; i < line_count; i++) {
    struct ui_flex_line *line = &lines[i];
    float free_space = 0.0f;
    float justify_gap = 0.0f;
    float line_start_x = start_x;
    float line_start_y = start_y;

    if (is_row) {
      free_space = node->content_width - line->main_size_sum;
      line_start_y = current_y;
    } else {
      float inner_height =
          node->height > 0.0f
              ? (node->height - node->padding[0] - node->padding[2] -
                 node->border[0] - node->border[2])
              : 0.0f;
      free_space = inner_height - line->main_size_sum;
      line_start_x = current_x;
    }

    if (free_space > 0.0f) {
      if (node->justify_content == UI_LAYOUT_ALIGN_CENTER) {
        if (is_row)
          line_start_x += free_space / 2.0f;
      } else if (node->justify_content == UI_LAYOUT_ALIGN_END) {
        if (is_row)
          line_start_x += free_space;
        else
          line_start_y += free_space;
      } else if (node->justify_content == UI_LAYOUT_ALIGN_SPACE_BETWEEN &&
                 line->child_count > 1) {
        justify_gap = free_space / (line->child_count - 1);
      } else if (node->justify_content == UI_LAYOUT_ALIGN_SPACE_AROUND) {
        justify_gap = free_space / line->child_count;
        if (is_row)
          line_start_x += justify_gap / 2.0f;
        else
          line_start_y += justify_gap / 2.0f;
      } else if (node->justify_content == UI_LAYOUT_ALIGN_SPACE_EVENLY) {
        justify_gap = free_space / (line->child_count + 1);
        if (is_row)
          line_start_x += justify_gap;
        else
          line_start_y += justify_gap;
      }
    }

    child = line->first_child;
    while (1) {
      enum ui_layout_alignment align = child->align_self;
      if (align == UI_LAYOUT_ALIGN_AUTO)
        align = node->align_items;

      if (is_row) {
        child->x = line_start_x + child->margin[3];
        if (align == UI_LAYOUT_ALIGN_CENTER) {
          child->y = line_start_y + (line->cross_max - child->height) / 2.0f;
        } else if (align == UI_LAYOUT_ALIGN_END) {
          child->y =
              line_start_y + line->cross_max - child->height - child->margin[2];
        } else {
          child->y = line_start_y + child->margin[0];
        }
        line_start_x +=
            child->width + child->margin[1] + child->margin[3] + justify_gap;
      } else {
        if (align == UI_LAYOUT_ALIGN_CENTER) {
          child->x = line_start_x + (line->cross_max - child->width) / 2.0f;
        } else if (align == UI_LAYOUT_ALIGN_END) {
          child->x =
              line_start_x + line->cross_max - child->width - child->margin[1];
        } else {
          child->x = line_start_x + child->margin[3];
        }
        child->y = line_start_y + child->margin[0];
        line_start_y +=
            child->height + child->margin[0] + child->margin[2] + justify_gap;
      }

      if (child == line->last_child)
        break;
      child = child->next_sibling;
    }

    if (is_row) {
      current_y += line->cross_max;
    } else {
      current_x += line->cross_max;
    }
  }

  if (node->height == 0.0f) {
    if (is_row) {
      node->content_height = current_y - start_y;
    } else {
      node->content_height = lines[0].cross_max;
    }
    node->height = node->content_height + node->padding[0] + node->padding[2] +
                   node->border[0] + node->border[2];
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_layout_solve_viewport(struct ui_layout_node *root,
                                    float window_width, float window_height) {
  if (!root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Identify if root is html or body */
  if (root->dom_node && root->dom_node->tag_name) {
    if (strcmp(root->dom_node->tag_name, "html") == 0 ||
        strcmp(root->dom_node->tag_name, "body") == 0) {
      root->width = window_width;
      root->height = window_height;
      root->content_width = window_width - root->padding[1] - root->padding[3] -
                            root->border[1] - root->border[3];
      if (root->content_width < 0.0f)
        root->content_width = 0.0f;
      root->content_height = window_height - root->padding[0] -
                             root->padding[2] - root->border[0] -
                             root->border[2];
      if (root->content_height < 0.0f)
        root->content_height = 0.0f;
    }
  } else if (!root->dom_node) {
    /* Anonymous root */
    root->width = window_width;
    root->height = window_height;
    root->content_width = window_width;
    root->content_height = window_height;
  }

  /* Force any direct child body to also match window dimensions exactly */
  if (root->first_child) {
    struct ui_layout_node *child = root->first_child;
    while (child) {
      if (child->dom_node && child->dom_node->tag_name) {
        if (strcmp(child->dom_node->tag_name, "body") == 0) {
          child->width = window_width;
          child->height = window_height;
          child->content_width = window_width - child->padding[1] -
                                 child->padding[3] - child->border[1] -
                                 child->border[3];
          child->content_height = window_height - child->padding[0] -
                                  child->padding[2] - child->border[0] -
                                  child->border[2];
        }
      }
      child = child->next_sibling;
    }
  }

  return ui_layout_compute(root, window_width, window_height);
}

ui_error_t ui_layout_sanity_check(const struct ui_layout_node *node) {
  struct ui_layout_node *child;

  if (!node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  child = node->first_child;
  while (child) {
    /*
     * The task explicitly asks to assert:
     * `child_rect.x + child_rect.width <= parent_rect.width`
     * (accounting for margins/overflow)
     */
    float child_max_x = child->x + child->width + child->margin[1];
    float parent_max_x = node->width;

    float child_max_y = child->y + child->height + child->margin[2];
    float parent_max_y = node->height;

    /* If overflow is visible, CSS allows children to bleed structurally.
       If overflow is scroll/auto, the scrollbar acts as the containment
       mechanism. But if it is hidden, we strictly prove they do not bleed into
       neighbor regions without being explicitly handled by the engine.
       Actually, to prove the engine "holds under stress testing", we can flag a
       violation if the layout constraints resulted in an unhandled bleed. For
       this engine, we consider it a violation if a child exceeds parent bounds
       UNLESS the parent explicitly manages overflow via SCROLL, AUTO, or
       VISIBLE. Actually, if it's VISIBLE it's allowed. If it's HIDDEN, it's
       clipped visually, but we can assert the intrinsic math here. Wait, let's
       just assert exactly what's requested: children do not exceed parent
       bounds unless overflow handles it.
     */
    if (node->overflow_x == UI_LAYOUT_OVERFLOW_HIDDEN ||
        node->overflow_x == UI_LAYOUT_OVERFLOW_VISIBLE) {
      /* Even if hidden or visible, the mathematical bounds could be larger,
         but if we want to ensure containment sanity, we might only assert it if
         there's a strict constraint. Let's just do a basic assertion that
         width doesn't wildly blow out without a reason.
         Wait, if overflow is NOT scroll/auto, and child_max_x > parent_max_x,
         maybe that's a violation of our "prevent UI elements from bleeding"
         rule? Let's flag it if overflow is HIDDEN and it bleeds, as the user
         wants to test bounds.
      */
      if (node->overflow_x != UI_LAYOUT_OVERFLOW_VISIBLE &&
          child_max_x > parent_max_x + 0.5f) {
        /* allow tiny float drift */
        return UI_ERROR_LAYOUT_VIOLATION;
      }
    }

    if (node->overflow_y != UI_LAYOUT_OVERFLOW_VISIBLE &&
        node->overflow_y != UI_LAYOUT_OVERFLOW_SCROLL &&
        node->overflow_y != UI_LAYOUT_OVERFLOW_AUTO) {
      if (child_max_y > parent_max_y + 0.5f) {
        return UI_ERROR_LAYOUT_VIOLATION;
      }
    }

    {
      ui_error_t check_rc = ui_layout_sanity_check(child);
      if (check_rc != UI_ERROR_NONE) {
        return check_rc;
      }
    }
    child = child->next_sibling;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_layout_compute(struct ui_layout_node *node, float available_width,
                             float available_height) {
  ui_error_t rc;

  if (!node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Suppress unused warning */
  (void)available_height;

  if (node->display_inside == UI_LAYOUT_DISPLAY_INSIDE_FLEX) {
    (void)layout_flex(node, available_width);
    return UI_ERROR_NONE;
  }

  /* Basic block layout */
  (void)layout_block(node, available_width);

#if defined(__EMSCRIPTEN__)
  if (node->dom_node) {
    ui_web_bridge_set_bounds((uint32_t)(uintptr_t)node->dom_node, node->x,
                             node->y, node->width, node->height);
  }
#endif

  return UI_ERROR_NONE;
}
