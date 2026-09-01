/**
 * @file ui_layout_compute.c
 * @brief ui_layout_compute.c implementation.
 */
/**
 * @brief compute_box_model.
 * @param node Parameter node.
 * @return Return value.
 */
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
        {
          ui_error_t rc_cleanup =
              ui_css_parse_color(val, &node->text_decoration_color);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
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
        {
          ui_error_t rc_cleanup =
              ui_css_parse_color(val, &node->text_emphasis_color);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
  }

  {
    _prop_rc =
        ui_css_computed_style_get_property(node->computed_style, "color", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        {
          ui_error_t rc_cleanup = ui_css_parse_color(val, &node->text_color);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "background-color", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        {
          ui_error_t rc_cleanup =
              ui_css_parse_color(val, &node->background_color);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
  }

  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "background-image", &val);
    if (attr_rc == UI_ERROR_NONE) {
      /* For multiple backgrounds we'd ideally split by comma. For Level 3/4
       * mock, we'll parse the first one */
      {
        {
          ui_error_t rc_cleanup =
              ui_css_parse_image(val, &node->background_image[0]);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
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
        {
          ui_error_t rc_cleanup =
              ui_css_parse_image(val, &node->border_image_source);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
  }
  if (_prop_rc == UI_ERROR_NOT_FOUND) {
    _prop_rc = ui_css_computed_style_get_property(node->computed_style,
                                                  "border-image", &val);
    if (_prop_rc == UI_ERROR_NONE) {
      {
        {
          ui_error_t rc_cleanup =
              ui_css_parse_image(val, &node->border_image_source);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
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
        {
          ui_error_t rc_cleanup =
              ui_css_parse_color(val, &node->border_color[0]);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-right-color", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        {
          ui_error_t rc_cleanup =
              ui_css_parse_color(val, &node->border_color[1]);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-bottom-color", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        {
          ui_error_t rc_cleanup =
              ui_css_parse_color(val, &node->border_color[2]);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
    }
  }
  {
    ui_error_t attr_rc = ui_css_computed_style_get_property(
        node->computed_style, "border-left-color", &val);
    if (attr_rc == UI_ERROR_NONE) {
      {
        {
          ui_error_t rc_cleanup =
              ui_css_parse_color(val, &node->border_color[3]);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
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
