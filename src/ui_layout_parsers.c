
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
