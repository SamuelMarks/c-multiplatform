/* clang-format off */
#include "material_catalog.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
/* clang-format on */

static uint32_t catalog_color_to_uint32(const cmp_color_t *c) {
  uint32_t r = (uint32_t)(c->r * 255.0f);
  uint32_t g = (uint32_t)(c->g * 255.0f);
  uint32_t b = (uint32_t)(c->b * 255.0f);
  uint32_t a = (uint32_t)(c->a * 255.0f);
  return (r << 24) | (g << 16) | (b << 8) | a;
}

/**
 * @brief Global registry of Material components.
 */
static const material_component_t g_components[] = {
    {1, "App bars: bottom",
     "Bottom app bars provide access to a bottom navigation drawer and up to "
     "four actions, including the floating action button.",
     0, 1, "", "", ""},
    {2, "App bars: top",
     "The top app bar provides content and actions related to the current "
     "screen. It's used for branding, screen titles, navigation, and actions.",
     0, 1, "", "", ""},
    {3, "Backdrop",
     "A backdrop appears behind all other surfaces in an app, displaying "
     "contextual and actionable content.",
     0, 1, "", "", ""},
    {4, "Badges",
     "Badges can contain numbers or icons. They are used to indicate "
     "notifications or status.",
     0, 1, "", "", ""},
    {5, "Bottom navigation",
     "Bottom navigation bars allow movement between primary destinations in an "
     "app.",
     0, 1, "", "", ""},
    {6, "Buttons",
     "Buttons allow users to take actions, and make choices, with a single "
     "tap.",
     0, 1, "", "", ""},
    {7, "Buttons: floating action button",
     "A floating action button (FAB) represents the primary action of a "
     "screen.",
     0, 1, "", "", ""},
    {8, "Cards", "Cards contain content and actions about a single subject.", 0,
     1, "", "", ""},
    {9, "Checkboxes",
     "Checkboxes allow the user to select one or more items from a set.", 0, 1,
     "", "", ""},
    {10, "Chips",
     "Chips are compact elements that represent an input, attribute, or "
     "action.",
     0, 1, "", "", ""},
    {11, "Dialogs",
     "Dialogs inform users about a task and can contain critical information, "
     "require decisions, or involve multiple tasks.",
     0, 1, "", "", ""},
    {12, "Dividers",
     "A divider is a thin line that groups content in lists and layouts.", 0, 1,
     "", "", ""},
    {13, "Lists", "Lists are continuous, vertical indexes of text or images.",
     0, 1, "", "", ""},
    {14, "Menus", "Menus display a list of choices on temporary surfaces.", 0,
     1, "", "", ""},
    {15, "Navigation drawer",
     "Navigation drawers provide access to destinations in your app.", 0, 1, "",
     "", ""},
    {16, "Navigation rail",
     "Navigation rails provide access to primary destinations in your app when "
     "using tablet and desktop screens.",
     0, 1, "", "", ""},
    {17, "Progress indicators",
     "Progress indicators express an unspecified wait time or display the "
     "length of a process.",
     0, 1, "", "", ""},
    {18, "Radio buttons",
     "Radio buttons allow the user to select one option from a set.", 0, 1, "",
     "", ""},
    {19, "Sheets: bottom",
     "Bottom sheets are surfaces containing supplementary content that are "
     "anchored to the bottom of the screen.",
     0, 1, "", "", ""},
    {20, "Sliders",
     "Sliders allow users to make selections from a range of values.", 0, 1, "",
     "", ""},
    {21, "Snackbars",
     "Snackbars provide brief messages about app processes at the bottom of "
     "the screen.",
     0, 1, "", "", ""},
    {22, "Switches", "Switches toggle the state of a single setting on or off.",
     0, 1, "", "", ""},
    {23, "Tabs",
     "Tabs organize content across different screens, data sets, and other "
     "interactions.",
     0, 1, "", "", ""},
    {24, "Text fields", "Text fields let users enter and edit text.", 0, 1, "",
     "", ""}};

static const int g_component_count =
    sizeof(g_components) / sizeof(g_components[0]);

/* Forward declarations to resolve function pointer casting warnings */
static void wrap_render_bottom_app_bar(cmp_ui_node_t *node);
static void wrap_render_top_app_bar(cmp_ui_node_t *node);
static void wrap_render_backdrop(cmp_ui_node_t *node);
static void wrap_render_badges(cmp_ui_node_t *node);
static void wrap_render_bottom_nav(cmp_ui_node_t *node);
static void wrap_render_nav_rail(cmp_ui_node_t *node);
static void wrap_render_buttons(cmp_ui_node_t *node);
static void wrap_render_fabs(cmp_ui_node_t *node);
static void wrap_render_cards(cmp_ui_node_t *node);
static void wrap_render_checkboxes(cmp_ui_node_t *node);
static void wrap_render_radio_buttons(cmp_ui_node_t *node);
static void wrap_render_switches(cmp_ui_node_t *node);
static void wrap_render_chips(cmp_ui_node_t *node);
static void wrap_render_dialogs(cmp_ui_node_t *node);
static void wrap_render_lists(cmp_ui_node_t *node);
static void wrap_render_menus(cmp_ui_node_t *node);
static void wrap_render_drawers_sheets(cmp_ui_node_t *node);
static void wrap_render_progress_indicators(cmp_ui_node_t *node);
static void wrap_render_sliders(cmp_ui_node_t *node);
static void wrap_render_snackbars(cmp_ui_node_t *node);
static void wrap_render_tabs(cmp_ui_node_t *node);
static void wrap_render_text_fields(cmp_ui_node_t *node);

static const material_example_t g_examples[] = {
    {1, "Simple bottom app bar", "A basic bottom app bar.", "",
     wrap_render_bottom_app_bar},
    {1, "Bottom app bar with FAB",
     "Bottom app bar with floating action button.", "",
     wrap_render_bottom_app_bar},
    {2, "Center aligned top app bar", "A top app bar with the title centered.",
     "", wrap_render_top_app_bar},
    {2, "Small top app bar", "A compact top app bar.", "",
     wrap_render_top_app_bar},
    {2, "Medium top app bar", "A medium top app bar.", "",
     wrap_render_top_app_bar},
    {2, "Large top app bar", "A large top app bar.", "",
     wrap_render_top_app_bar},
    {3, "Backdrop scaffold", "A standard backdrop scaffold.", "",
     wrap_render_backdrop},
    {4, "Badges with numbers", "Badges displaying numbers.", "",
     wrap_render_badges},
    {4, "Dot badges", "Simple dot badges.", "", wrap_render_badges},
    {5, "Primary bottom navigation", "Standard bottom navigation.", "",
     wrap_render_bottom_nav},
    {6, "Filled button", "A filled button.", "", wrap_render_buttons},
    {6, "Outlined button", "An outlined button.", "", wrap_render_buttons},
    {6, "Text button", "A text button.", "", wrap_render_buttons},
    {6, "Elevated button", "An elevated button.", "", wrap_render_buttons},
    {6, "Tonal button", "A filled tonal button.", "", wrap_render_buttons},
    {7, "Small FAB", "A small floating action button.", "", wrap_render_fabs},
    {7, "Standard FAB", "A standard floating action button.", "",
     wrap_render_fabs},
    {7, "Large FAB", "A large floating action button.", "", wrap_render_fabs},
    {7, "Extended FAB", "An extended floating action button.", "",
     wrap_render_fabs},
    {8, "Elevated card", "An elevated card.", "", wrap_render_cards},
    {8, "Filled card", "A filled card.", "", wrap_render_cards},
    {8, "Outlined card", "An outlined card.", "", wrap_render_cards},
    {9, "Checkbox", "A standard checkbox.", "", wrap_render_checkboxes},
    {10, "Assist chip", "A standard assist chip.", "", wrap_render_chips},
    {10, "Filter chip", "A standard filter chip.", "", wrap_render_chips},
    {10, "Input chip", "A standard input chip.", "", wrap_render_chips},
    {10, "Suggestion chip", "A standard suggestion chip.", "",
     wrap_render_chips},
    {11, "Basic dialog", "A basic dialog.", "", wrap_render_dialogs},
    {11, "Full screen dialog", "A full screen dialog.", "",
     wrap_render_dialogs},
    {13, "One-line lists", "A one-line list.", "", wrap_render_lists},
    {13, "Two-line lists", "A two-line list.", "", wrap_render_lists},
    {13, "Three-line lists", "A three-line list.", "", wrap_render_lists},
    {14, "Dropdown menu", "A standard dropdown menu.", "", wrap_render_menus},
    {15, "Modal drawer", "A modal navigation drawer.", "",
     wrap_render_drawers_sheets},
    {17, "Linear progress indicator", "A standard linear progress indicator.",
     "", wrap_render_progress_indicators},
    {17, "Circular progress indicator",
     "A standard circular progress indicator.", "",
     wrap_render_progress_indicators},
    {19, "Bottom sheet", "A standard bottom sheet.", "",
     wrap_render_drawers_sheets},
    {18, "Radio button", "A standard radio button.", "",
     wrap_render_radio_buttons},
    {20, "Slider", "A standard slider.", "", wrap_render_sliders},
    {21, "Snackbar", "A standard snackbar.", "", wrap_render_snackbars},
    {22, "Switch", "A standard switch.", "", wrap_render_switches},
    {23, "Primary tabs", "Standard primary tabs.", "", wrap_render_tabs},
    {23, "Secondary tabs", "Standard secondary tabs.", "", wrap_render_tabs},
    {24, "Filled text field", "A standard filled text field.", "",
     wrap_render_text_fields},
    {24, "Outlined text field", "A standard outlined text field.", "",
     wrap_render_text_fields},
    {16, "Navigation rail", "A standard navigation rail.", "",
     wrap_render_nav_rail}};

static const int g_example_count = sizeof(g_examples) / sizeof(g_examples[0]);

static material_catalog_state_t *g_current_state =
    NULL; /* Used strictly by the wrappers */

static void wrap_render_bottom_app_bar(cmp_ui_node_t *node) {
  material_catalog_render_bottom_app_bar_example(g_current_state, node);
}

static void wrap_render_top_app_bar(cmp_ui_node_t *node) {
  material_catalog_render_top_app_bar_example(g_current_state, node);
}

static void wrap_render_backdrop(cmp_ui_node_t *node) {
  material_catalog_render_backdrop_example(g_current_state, node);
}

static void wrap_render_badges(cmp_ui_node_t *node) {
  material_catalog_render_badges_example(g_current_state, node);
}

static void wrap_render_bottom_nav(cmp_ui_node_t *node) {
  material_catalog_render_bottom_navigation_example(g_current_state, node);
}

static void wrap_render_nav_rail(cmp_ui_node_t *node) {
  material_catalog_render_navigation_rail_example(g_current_state, node);
}

static void wrap_render_buttons(cmp_ui_node_t *node) {
  material_catalog_render_buttons_example(g_current_state, node);
}

static void wrap_render_fabs(cmp_ui_node_t *node) {
  material_catalog_render_fabs_example(g_current_state, node);
}

static void wrap_render_cards(cmp_ui_node_t *node) {
  material_catalog_render_cards_example(g_current_state, node);
}

static void wrap_render_checkboxes(cmp_ui_node_t *node) {
  material_catalog_render_checkboxes_example(g_current_state, node);
}

static void wrap_render_radio_buttons(cmp_ui_node_t *node) {
  material_catalog_render_radio_buttons_example(g_current_state, node);
}

static void wrap_render_switches(cmp_ui_node_t *node) {
  material_catalog_render_switches_example(g_current_state, node);
}

static void wrap_render_chips(cmp_ui_node_t *node) {
  material_catalog_render_chips_example(g_current_state, node);
}

static void wrap_render_dialogs(cmp_ui_node_t *node) {
  material_catalog_render_dialogs_example(g_current_state, node);
}

static void wrap_render_lists(cmp_ui_node_t *node) {
  material_catalog_render_lists_example(g_current_state, node);
}

static void wrap_render_menus(cmp_ui_node_t *node) {
  material_catalog_render_menus_example(g_current_state, node);
}

static void wrap_render_drawers_sheets(cmp_ui_node_t *node) {
  material_catalog_render_drawers_sheets_example(g_current_state, node);
}

static void wrap_render_progress_indicators(cmp_ui_node_t *node) {
  material_catalog_render_progress_indicators_example(g_current_state, node);
}

static void wrap_render_sliders(cmp_ui_node_t *node) {
  material_catalog_render_sliders_example(g_current_state, node);
}

static void wrap_render_snackbars(cmp_ui_node_t *node) {
  material_catalog_render_snackbars_example(g_current_state, node);
}

static void wrap_render_tabs(cmp_ui_node_t *node) {
  material_catalog_render_tabs_example(g_current_state, node);
}

static void wrap_render_text_fields(cmp_ui_node_t *node) {
  material_catalog_render_text_fields_example(g_current_state, node);
}

int material_catalog_get_contrast_ratio(const cmp_color_t *c1,
                                        const cmp_color_t *c2,
                                        float *out_ratio) {
  if (!c1 || !c2 || !out_ratio)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (cmp_color_contrast_ratio(c1, c2, out_ratio) != 0) {
    *out_ratio = 1.0f;
  }
  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_is_light_color(const cmp_color_t *color,
                                    int *out_is_light) {
  float luminance;
  if (!color || !out_is_light)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (cmp_color_luminance(color, &luminance) == 0) {
    *out_is_light = (luminance > 0.5f) ? 1 : 0;
  } else {
    *out_is_light = 0;
  }
  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_get_on_color(const cmp_color_t *bg_color,
                                  cmp_color_t *out_on_color) {
  int is_light = 0;
  if (!bg_color || !out_on_color)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  material_catalog_is_light_color(bg_color, &is_light);
  out_on_color->a = 1.0f;
  out_on_color->space = bg_color->space;
  if (is_light) {
    out_on_color->r = 0.0f;
    out_on_color->g = 0.0f;
    out_on_color->b = 0.0f;
  } else {
    out_on_color->r = 1.0f;
    out_on_color->g = 1.0f;
    out_on_color->b = 1.0f;
  }
  return MATERIAL_CATALOG_SUCCESS;
}

const material_component_t *material_catalog_get_components(int *out_count) {
  if (out_count) {
    *out_count = g_component_count;
  }
  return g_components;
}

static const material_example_t *get_examples_for_component(int component_id,
                                                            int *out_count) {
  static material_example_t filtered[64];
  int count = 0;
  int i;

  for (i = 0; i < g_example_count; ++i) {
    if (g_examples[i].component_id == component_id) {
      if (count < 64) {
        filtered[count++] = g_examples[i];
      }
    }
  }

  if (out_count) {
    *out_count = count;
  }
  return count > 0 ? filtered : NULL;
}

const material_example_t *material_catalog_get_examples(int component_id,
                                                        int *out_count) {
  return get_examples_for_component(component_id, out_count);
}

static const cmp_color_t g_catalog_base_colors[10] = {
    {0x21 / 255.0f, 0x96 / 255.0f, 0xF3 / 255.0f, 1.0f,
     CMP_COLOR_SPACE_SRGB}, /* Blue */
    {0x79 / 255.0f, 0x55 / 255.0f, 0x48 / 255.0f, 1.0f,
     CMP_COLOR_SPACE_SRGB}, /* Brown */
    {0x4C / 255.0f, 0xAF / 255.0f, 0x50 / 255.0f, 1.0f,
     CMP_COLOR_SPACE_SRGB}, /* Green */
    {0x3F / 255.0f, 0x51 / 255.0f, 0xB5 / 255.0f, 1.0f,
     CMP_COLOR_SPACE_SRGB}, /* Indigo */
    {0xFF / 255.0f, 0x98 / 255.0f, 0x00 / 255.0f, 1.0f,
     CMP_COLOR_SPACE_SRGB}, /* Orange */
    {0xE9 / 255.0f, 0x1E / 255.0f, 0x63 / 255.0f, 1.0f,
     CMP_COLOR_SPACE_SRGB}, /* Pink */
    {0x9C / 255.0f, 0x27 / 255.0f, 0xB0 / 255.0f, 1.0f,
     CMP_COLOR_SPACE_SRGB}, /* Purple */
    {0xF4 / 255.0f, 0x43 / 255.0f, 0x36 / 255.0f, 1.0f,
     CMP_COLOR_SPACE_SRGB}, /* Red */
    {0x00 / 255.0f, 0x96 / 255.0f, 0x88 / 255.0f, 1.0f,
     CMP_COLOR_SPACE_SRGB}, /* Teal */
    {0xFF / 255.0f, 0xEB / 255.0f, 0x3B / 255.0f, 1.0f,
     CMP_COLOR_SPACE_SRGB} /* Yellow */
};

int material_catalog_get_palette(catalog_palette_id_t id, catalog_theme_t theme,
                                 struct cmp_palette *out_palette) {
  float hue, chroma, tone;
  int is_dark;
  cmp_color_t base;

  if (!out_palette)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (id < 0 || id > 9)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  is_dark = (theme == CATALOG_THEME_DARK) ? 1 : 0;
  base = g_catalog_base_colors[id];

  if (cmp_m3_srgb_to_hct(&base, &hue, &chroma, &tone) != 0) {
    return MATERIAL_CATALOG_ERROR_NULL_POINTER; /* Handle internally */
  }

  if (cmp_m3_scheme_tonal_spot(hue, chroma, tone, is_dark, out_palette) != 0) {
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  }

  return MATERIAL_CATALOG_SUCCESS;
}

static const theme_typography_style_t g_typography_scales[15] = {
    {57.0f, -0.25f, 400, THEME_FONT_DEFAULT}, /* Display Large */
    {45.0f, 0.0f, 400, THEME_FONT_DEFAULT},   /* Display Medium */
    {36.0f, 0.0f, 400, THEME_FONT_DEFAULT},   /* Display Small */
    {32.0f, 0.0f, 400, THEME_FONT_DEFAULT},   /* Headline Large */
    {28.0f, 0.0f, 400, THEME_FONT_DEFAULT},   /* Headline Medium */
    {24.0f, 0.0f, 400, THEME_FONT_DEFAULT},   /* Headline Small */
    {22.0f, 0.0f, 400, THEME_FONT_DEFAULT},   /* Title Large */
    {16.0f, 0.15f, 500, THEME_FONT_DEFAULT},  /* Title Medium */
    {14.0f, 0.1f, 500, THEME_FONT_DEFAULT},   /* Title Small */
    {14.0f, 0.1f, 500, THEME_FONT_DEFAULT},   /* Label Large */
    {12.0f, 0.5f, 500, THEME_FONT_DEFAULT},   /* Label Medium */
    {11.0f, 0.5f, 500, THEME_FONT_DEFAULT},   /* Label Small */
    {16.0f, 0.5f, 400, THEME_FONT_DEFAULT},   /* Body Large */
    {14.0f, 0.25f, 400, THEME_FONT_DEFAULT},  /* Body Medium */
    {12.0f, 0.4f, 400, THEME_FONT_DEFAULT}    /* Body Small */
};

int material_catalog_get_typography_style(theme_typography_scale_t scale,
                                          theme_typography_style_t *out_style) {
  if (!out_style)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if ((int)scale < 0 || (int)scale > 14)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  *out_style = g_typography_scales[scale];
  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_load_fonts(material_catalog_state_t *state) {
  int i;
  if (!state)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  /* Zero out the array first */
  for (i = 0; i < 15; ++i) {
    state->fonts[i] = NULL;
  }

  /* Load standard typography font families via c-fs (VFS) mapped to scales */
  for (i = 0; i < 15; ++i) {
    const char *font_path = "vfs://fonts/Roboto-Regular.ttf";
    if (g_typography_scales[i].font_weight >= 500) {
      font_path = "vfs://fonts/Roboto-Medium.ttf";
    }
    /* We ignore missing font errors since the catalog repo might not have the
     * files checked in yet */
    cmp_font_load(font_path, g_typography_scales[i].font_size,
                  &state->fonts[i]);
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_get_shape_radius(theme_shape_size_t size,
                                      float *out_radius) {
  if (!out_radius)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (size == THEME_SHAPE_SMALL)
    *out_radius = 16.0f;
  else if (size == THEME_SHAPE_MEDIUM)
    *out_radius = 32.0f;
  else if (size == THEME_SHAPE_LARGE)
    *out_radius = 48.0f;
  else
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_render_theme_picker(material_catalog_state_t *state,
                                         cmp_ui_node_t *container) {
  cmp_ui_node_t *scrim;
  cmp_ui_node_t *sheet;
  cmp_ui_node_t *title;
  cmp_ui_node_t *reset_btn;
  cmp_ui_node_t *drag_handle;
  cmp_m3_sheet_metrics_t sheet_metrics;
  float tonal_opacity, dummy_y, dummy_b, dummy_a, dummy_s;
  cmp_color_t sheet_bg;

  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  cmp_m3_sheet_resolve(CMP_M3_SHEET_BOTTOM_MODAL, &sheet_metrics);

  /* Render Scrim */
  if (sheet_metrics.has_scrim) {
    if (cmp_ui_box_create(&scrim) == 0) {
      scrim->layout->position_type = CMP_POSITION_ABSOLUTE;
      scrim->layout->width = state->window_width;
      scrim->layout->height = state->window_height;
      scrim->layout->position[0] = 0;
      scrim->layout->position[3] = 0;

      /* Scrim is black at 32% opacity */
      scrim->bg_color = (0x51 << 24) | 0x000000;

      /* Intercept clicks */
      cmp_ui_node_add_event_listener(scrim, CMP_EVENT_TYPE_MOUSE, 0, NULL, NULL);
      cmp_ui_node_add_child(container, scrim);
    }
  }

  /* Render Bottom Sheet base */
  if (cmp_ui_box_create(&sheet) != 0) {
    return MATERIAL_CATALOG_ERROR_OUT_OF_MEMORY;
  }

  cmp_m3_elevation_resolve(sheet_metrics.elevation, &tonal_opacity, &dummy_y, &dummy_b, &dummy_a, &dummy_s);
  {
      cmp_linear_blend_t *blend;
      cmp_linear_blend_create(2.2f, &blend);
      cmp_linear_blend_mix(blend, &state->sys_colors.surface, &state->sys_colors.primary, tonal_opacity, &sheet_bg);
      cmp_linear_blend_destroy(blend);
  }

  sheet->bg_color = catalog_color_to_uint32(&sheet_bg);
  sheet->design_language_override = 1; /* M3 */
  /* Corner radius handling typically done natively during GDI render based on shape metric */

  sheet->layout->position_type = CMP_POSITION_ABSOLUTE;
  sheet->layout->width = state->window_width;
  sheet->layout->height = 400.0f; /* Mock bottom sheet height */
  sheet->layout->position[0] = state->window_height - 400.0f; /* top */
  sheet->layout->position[3] = 0;                             /* left */
  sheet->layout->direction = CMP_FLEX_COLUMN;
  sheet->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  sheet->layout->padding[0] = 22.0f; /* top */
  sheet->layout->padding[1] = 16.0f; /* right */
  sheet->layout->padding[3] = 16.0f; /* left */

  /* Drag Handle */
  if (sheet_metrics.has_drag_handle) {
      if (cmp_ui_box_create(&drag_handle) == 0) {
          drag_handle->layout->width = 32.0f;
          drag_handle->layout->height = 4.0f;
          drag_handle->layout->margin[2] = 22.0f; /* margin-bottom to title */
          drag_handle->bg_color = catalog_color_to_uint32(&state->sys_colors.outline);
          cmp_ui_node_add_child(sheet, drag_handle);
      }
  }

  /* Title */
  if (cmp_ui_text_create(&title, "Theme Settings", -1) == 0) {
    title->layout->margin[2] = 16.0f;
    title->text_color = catalog_color_to_uint32(&state->sys_colors.on_surface);
    cmp_ui_node_add_child(sheet, title);
  }

  /* Mock LazyRow for Colors */
  {
    cmp_ui_node_t *color_row;
    if (cmp_ui_box_create(&color_row) == 0) {
      cmp_ui_node_t *color_lbl;
      color_row->layout->direction = CMP_FLEX_ROW;
      color_row->layout->margin[2] = 16.0f;
      if (cmp_ui_text_create(&color_lbl, "[ Colors ]", -1) == 0) {
        color_lbl->text_color = catalog_color_to_uint32(&state->sys_colors.on_surface);
        cmp_ui_node_add_child(color_row, color_lbl);
      }
      cmp_ui_node_add_child(sheet, color_row);
    }
  }

  /* Mock Typography Grid */
  {
    cmp_ui_node_t *typo_row;
    if (cmp_ui_box_create(&typo_row) == 0) {
      cmp_ui_node_t *typo_lbl;
      typo_row->layout->direction = CMP_FLEX_ROW;
      typo_row->layout->margin[2] = 16.0f;
      if (cmp_ui_text_create(&typo_lbl, "[ Typography ]", -1) == 0) {
        typo_lbl->text_color = catalog_color_to_uint32(&state->sys_colors.on_surface);
        cmp_ui_node_add_child(typo_row, typo_lbl);
      }
      cmp_ui_node_add_child(sheet, typo_row);
    }
  }

  /* Reset button */
  if (cmp_ui_button_create(&reset_btn, "Reset to Default", -1) == 0) {
    cmp_ui_node_add_child(sheet, reset_btn);
  }

  cmp_ui_node_add_child(container, sheet);
  return MATERIAL_CATALOG_SUCCESS;
}
int material_catalog_apply_shape(cmp_ui_node_t *node,
                                 theme_shape_corner_family_t family,
                                 float radius) {
  if (!node)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  /* Stub: in real CMPC, we'd cast node->properties to cmp_box_t* */
  /* and apply radius & corner_shape. */
  (void)family;
  (void)radius;
  return MATERIAL_CATALOG_SUCCESS;
}

typedef struct click_ctx {
  material_catalog_state_t *state;
  int id;
  int index;
} click_ctx_t;

static void back_btn_clicked(cmp_event_t *evt, cmp_ui_node_t *node,
                             void *user_data);

int material_catalog_render_scrim(material_catalog_state_t *state,
                                  cmp_ui_node_t *container) {
  cmp_ui_node_t *scrim;
  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  if (cmp_ui_box_create(&scrim) != 0) {
    return MATERIAL_CATALOG_ERROR_OUT_OF_MEMORY;
  }

  scrim->layout->position_type = CMP_POSITION_ABSOLUTE;
  scrim->layout->width = state->window_width;
  scrim->layout->height = state->window_height;
  /* Top and Left using properties if they exist, else margin offsets */
  scrim->layout->margin[0] = 0;
  scrim->layout->margin[3] = 0;

  /* Black at 32% opacity */
  /* TODO: Apply exact rgba(0,0,0,0.32) background */

  /* Block underlying clicks by capturing them and doing nothing */
  cmp_ui_node_add_event_listener(scrim, CMP_EVENT_TYPE_MOUSE, 0, back_btn_clicked, NULL);

  cmp_ui_node_add_child(container, scrim);
  return MATERIAL_CATALOG_SUCCESS;
}
int material_catalog_render_more_menu(material_catalog_state_t *state,
                                      cmp_ui_node_t *container) {
  cmp_menu_t *menu = NULL;
  cmp_menu_item_t *item1 = NULL;
  cmp_menu_item_t *item2 = NULL;

  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  if (cmp_menu_create(&menu, CMP_MENU_PRESENTATION_POPOVER) != 0) {
    return MATERIAL_CATALOG_ERROR_OUT_OF_MEMORY;
  }

  if (cmp_menu_item_create(&item1, "Item 1", NULL,
                           CMP_MENU_ITEM_ROLE_STANDARD) == 0) {
    cmp_menu_add_item(menu, item1);
  }

  if (cmp_menu_item_create(&item2, "Item 2", NULL,
                           CMP_MENU_ITEM_ROLE_STANDARD) == 0) {
    cmp_menu_add_item(menu, item2);
  }

  /* Stub: in full implementation, we'd attach this menu to a button click on
   * the Top App Bar */
  /* and manage its lifecycle relative to the container/anchor */

  /* Destroying immediately for now to avoid leaks since it's just a mock
   * renderer */
  cmp_menu_destroy(menu);

  return MATERIAL_CATALOG_SUCCESS;
}

static void ripple_event_handler(cmp_event_t *evt, cmp_ui_node_t *node,
                                 void *user_data) {
  material_catalog_state_t *state = (material_catalog_state_t *)user_data;
  float base_opacity = 0.0f;
  float anim_opacity = 0.0f;
  float final_opacity = 0.0f;

  if (evt == NULL || node == NULL)
    return;

  /* Logic for updating state layer overlays */
  if (evt->action == CMP_ACTION_DOWN) {
    /* Trigger 12% pressed overlay, trigger expanding ripple animation */
    anim_opacity = 0.12f;
  } else if (evt->action == CMP_ACTION_UP || evt->action == CMP_ACTION_CANCEL) {
    /* Trigger 4% hover overlay if cursor still over, fade out ripple */
    anim_opacity = 0.04f;
  } else if (evt->action == CMP_ACTION_MOVE) {
    /* Trigger 4% hover overlay */
    anim_opacity = 0.04f;
  }

  /* Hook into the ripple animation rendering pipeline */
  if (cmp_anim_compose_numerical(base_opacity, anim_opacity,
                                 CMP_ANIM_COMPOSE_REPLACE,
                                 &final_opacity) == 0) {
    /* Hack for mock catalog: we store the ripple opacity in the upper bits of
       layout flex_shrink or similar safe spot, but `node->properties` is safer
       if the node is a Card (since we used properties for background color).
       Actually, `cmp_ui_node_t` doesn't have an explicit `state_layer` field
       unless we augment it. We will trigger a repaint request. */
    (void)final_opacity;
    if (state) {
      /* Here we'd mark the specific node DIRTY_PAINT, but we'll invalidate the
       * whole UI for this mock */
      material_catalog_invalidate_ui(state);
    }
  }
}

int material_catalog_apply_ripple(material_catalog_state_t *state,
                                  cmp_ui_node_t *node) {
  if (!node)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  cmp_ui_node_add_event_listener(node, CMP_EVENT_TYPE_MOUSE, 0,
                                 ripple_event_handler, state);

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_render_top_app_bar(material_catalog_state_t *state,
                                        cmp_ui_node_t *container,
                                        const char *title,
                                        int show_back_button) {
  cmp_ui_node_t *app_bar;
  cmp_ui_node_t *title_node;
  cmp_m3_top_app_bar_metrics_t metrics;
  cmp_m3_icon_button_metrics_t btn_metrics;

  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  cmp_m3_top_app_bar_resolve(CMP_M3_TOP_APP_BAR_CENTER_ALIGNED, &metrics);
  cmp_m3_icon_button_resolve(CMP_M3_ICON_BUTTON_STANDARD, &btn_metrics);

  if (cmp_ui_box_create(&app_bar) != 0) {
    return MATERIAL_CATALOG_ERROR_OUT_OF_MEMORY;
  }

  app_bar->layout->direction = CMP_FLEX_ROW;
  app_bar->layout->width = state->window_width;
  app_bar->layout->height = metrics.height_collapsed + state->window_insets.y;
  app_bar->layout->padding[0] = state->window_insets.y; /* padding-top */
  app_bar->layout->padding[1] = 4.0f;                   /* padding-right */
  app_bar->layout->padding[3] = 4.0f;                   /* padding-left */
  app_bar->layout->align_items = CMP_FLEX_ALIGN_CENTER;

  app_bar->bg_color = catalog_color_to_uint32(&state->sys_colors.surface);
  app_bar->design_language_override = 1;

  if (show_back_button) {
    cmp_ui_node_t *back_btn;
    if (cmp_ui_image_view_create(&back_btn, "vfs://assets/ic_arrow_back.svg") == 0) {
      click_ctx_t *ctx;
      back_btn->layout->width = btn_metrics.target_size;
      back_btn->layout->height = btn_metrics.target_size;
      /* Mock properties to hold icon tinting */
      back_btn->properties = (void*)(uintptr_t)catalog_color_to_uint32(&state->sys_colors.on_surface);
      if (cmp_arena_alloc(&state->state_arena, sizeof(click_ctx_t),
                          (void **)&ctx) == 0) {
        ctx->state = state;
        ctx->id = 0;
        ctx->index = 0;
        cmp_ui_node_add_event_listener(back_btn, 1, 0, back_btn_clicked, ctx);
      }
      material_catalog_apply_ripple(state, back_btn);
      cmp_ui_node_add_child(app_bar, back_btn);
    }
  } else {
    /* Empty spacer so the title stays centered */
    cmp_ui_node_t *spacer;
    if (cmp_ui_box_create(&spacer) == 0) {
      spacer->layout->width = btn_metrics.target_size;
      spacer->layout->height = btn_metrics.target_size;
      cmp_ui_node_add_child(app_bar, spacer);
    }
  }

  if (cmp_ui_text_create(&title_node, title ? title : "", -1) == 0) {
    title_node->layout->flex_grow = 1.0f;
    title_node->text_color = catalog_color_to_uint32(&state->sys_colors.on_surface);
    /* In a real implementation we would attach the font pointer from state->fonts[THEME_TYPOGRAPHY_TITLE_LARGE] */
    title_node->properties = state->fonts[THEME_TYPOGRAPHY_TITLE_LARGE];
    cmp_ui_node_add_child(app_bar, title_node);
  }

  /* Right trailing actions: Pin, Palette, Ellipsis */
  {
    cmp_ui_node_t *pin_btn;
    cmp_ui_node_t *palette_btn;
    cmp_ui_node_t *more_btn;

    if (cmp_ui_image_view_create(&pin_btn, "vfs://assets/ic_push_pin.svg") == 0) {
      pin_btn->layout->width = btn_metrics.target_size;
      pin_btn->layout->height = btn_metrics.target_size;
      pin_btn->properties = (void*)(uintptr_t)catalog_color_to_uint32(&state->sys_colors.on_surface);
      material_catalog_apply_ripple(state, pin_btn);
      cmp_ui_node_add_child(app_bar, pin_btn);
    }

    if (cmp_ui_image_view_create(&palette_btn, "vfs://assets/ic_palette.svg") == 0) {
      palette_btn->layout->width = btn_metrics.target_size;
      palette_btn->layout->height = btn_metrics.target_size;
      palette_btn->properties = (void*)(uintptr_t)catalog_color_to_uint32(&state->sys_colors.on_surface);
      material_catalog_apply_ripple(state, palette_btn);
      cmp_ui_node_add_child(app_bar, palette_btn);
    }

    if (cmp_ui_image_view_create(&more_btn, "vfs://assets/ic_more_vert.svg") == 0) {
      more_btn->layout->width = btn_metrics.target_size;
      more_btn->layout->height = btn_metrics.target_size;
      more_btn->properties = (void*)(uintptr_t)catalog_color_to_uint32(&state->sys_colors.on_surface);
      material_catalog_apply_ripple(state, more_btn);
      cmp_ui_node_add_child(app_bar, more_btn);
    }
  }

  cmp_ui_node_add_child(container, app_bar);
  return MATERIAL_CATALOG_SUCCESS;
}

void material_catalog_launch_url(const char *url) {
  /* Stub: Would use platform-specific ShellExecute or open */
  (void)url;
}

int material_catalog_update_insets(material_catalog_state_t *state) {
  if (!state)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  /* Fallback mock values */
  state->window_insets.x = 0;
  state->window_insets.y = 24.0f; /* Mock status bar */
  state->window_insets.width = 0;
  state->window_insets.height = 48.0f; /* Mock nav bar */

  /* Try to grab real window insets if the platform supports it natively */
#if defined(__ANDROID__)
  {
    int top, bottom, left, right;
    if (cmp_window_android_get_safe_area(state->window, &top, &bottom, &left,
                                         &right) == 0) {
      state->window_insets.y = (float)top;
      state->window_insets.height = (float)bottom;
      state->window_insets.x = (float)left;
      state->window_insets.width = (float)right;
    }
  }
#endif

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_save_theme(theme_saver_t *saver) {
  FILE *f;
  if (!saver || !saver->config_file_path)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  f = fopen(saver->config_file_path, "wb");
  if (!f)
    return MATERIAL_CATALOG_ERROR_INITIALIZATION_FAILED;
  if (fwrite(&saver->current, sizeof(material_theme_t), 1, f) != 1) {
    fclose(f);
    return MATERIAL_CATALOG_ERROR_INITIALIZATION_FAILED;
  }
  fclose(f);
  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_load_theme(theme_saver_t *saver) {
  FILE *f;
  if (!saver || !saver->config_file_path)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  f = fopen(saver->config_file_path, "rb");
  if (!f)
    return MATERIAL_CATALOG_ERROR_INITIALIZATION_FAILED;
  if (fread(&saver->current, sizeof(material_theme_t), 1, f) != 1) {
    fclose(f);
    return MATERIAL_CATALOG_ERROR_INITIALIZATION_FAILED;
  }
  fclose(f);
  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_map_semantic_node(material_catalog_state_t *state,
                                       cmp_ui_node_t *node, const char *name,
                                       const char *role) {
  if (!state || !state->a11y_tree || !node)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  /* Stub mapping since node->id might not be tracked uniformly right now.
   * Passing dummy ID 1. */
  if (cmp_a11y_tree_add_node(state->a11y_tree, 1, role, name) != 0) {
    return MATERIAL_CATALOG_ERROR_INITIALIZATION_FAILED;
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_apply_viewport_culling(material_catalog_state_t *state,
                                            cmp_ui_node_t *scroll_view) {
  size_t i;
  if (!state || !scroll_view)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  /* Iterate through the scroll view's children */
  /* If a child's absolute layout y + height is < scroll_top, it's above the
   * viewport. */
  /* If a child's absolute layout y > scroll_top + viewport_height, it's below.
   */
  /* In CMPC, hiding them would mean setting an explicit display:none flag or
   * similar on the cmp_layout_node_t. */

  for (i = 0; i < scroll_view->child_count; ++i) {
    cmp_ui_node_t *child = scroll_view->children[i];
    if (child && child->layout) {
      /* Dummy simulation of culling bounds check */
      if (child->layout->position[0] < -1000.0f) {
        /* hide */
      } else {
        /* show */
      }
    }
  }
  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_init(material_catalog_state_t *state) {
  cmp_window_config_t config;
  cmp_dpi_t *dpi = NULL;
  float monitor_scale = 1.0f;
  cmp_color_t seed_color = {0.4f, 0.2f, 0.8f, 1.0f, CMP_COLOR_SPACE_SRGB};

  if (state == NULL) {
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  }

  if (cmp_event_system_init() != 0) {
    return MATERIAL_CATALOG_ERROR_INITIALIZATION_FAILED;
  }

  if (cmp_window_system_init() != 0) {
    return MATERIAL_CATALOG_ERROR_INITIALIZATION_FAILED;
  }

  if (cmp_dpi_awareness_init() == 0) {
    if (cmp_dpi_create(&dpi) == 0) {
      if (cmp_dpi_get_monitor_scale(dpi, 0, &monitor_scale) != 0) {
        monitor_scale = 1.0f;
      }
      cmp_dpi_destroy(dpi);
    }
  }

  memset(state, 0, sizeof(material_catalog_state_t));

  /* Initialize arenas. E.g. 1MB for UI, 1MB for state */
  if (cmp_arena_init(&state->ui_arena, 1024 * 1024) != 0) {
    return MATERIAL_CATALOG_ERROR_OUT_OF_MEMORY;
  }
  if (cmp_arena_init(&state->state_arena, 1024 * 1024) != 0) {
    cmp_arena_free(&state->ui_arena);
    return MATERIAL_CATALOG_ERROR_OUT_OF_MEMORY;
  }

  if (cmp_a11y_tree_create(&state->a11y_tree) != 0) {
    cmp_arena_free(&state->state_arena);
    cmp_arena_free(&state->ui_arena);
    return MATERIAL_CATALOG_ERROR_OUT_OF_MEMORY;
  }

  state->window_width = 1024.0f * monitor_scale;
  state->window_height = 768.0f * monitor_scale;
  state->current_theme = CATALOG_THEME_SYSTEM;
  state->router.stack_size = 1;
  state->router.stack[0].screen_id = CATALOG_SCREEN_HOME;
  state->is_ui_dirty = 1;
  state->is_rtl = 0; /* Default LTR */

  cmp_m3_sys_colors_generate(seed_color, 0 /* default light */,
                             CMP_M3_CONTRAST_STANDARD, &state->sys_colors);

  memset(&config, 0, sizeof(config));
  config.width = (uint32_t)state->window_width;
  config.height = (uint32_t)state->window_height;
  config.x = -1;
  config.y = -1;
  config.hidden = 0;
  config.frameless = 0;
  config.use_legacy_backend = 0;
  config.title = "Material Catalog";

  if (cmp_window_create(&config, &state->window) != 0) {
    cmp_a11y_tree_destroy(state->a11y_tree);
    cmp_arena_free(&state->ui_arena);
    cmp_arena_free(&state->state_arena);
    return MATERIAL_CATALOG_ERROR_INITIALIZATION_FAILED;
  }

  material_catalog_update_insets(state);
  material_catalog_load_fonts(state);

  return MATERIAL_CATALOG_SUCCESS;
}

void material_catalog_invalidate_ui(material_catalog_state_t *state) {
  if (state) {
    state->is_ui_dirty = 1;
  }
}

void material_catalog_navigate_to(material_catalog_state_t *state,
                                  catalog_screen_id_t screen_id,
                                  int component_id, int example_index) {
  if (state && state->router.stack_size < CATALOG_MAX_BACK_STACK) {
    catalog_route_t *route = &state->router.stack[state->router.stack_size++];
    route->screen_id = screen_id;
    route->component_id = component_id;
    route->example_index = example_index;
    material_catalog_invalidate_ui(state);
  }
}

void material_catalog_navigate_back(material_catalog_state_t *state) {
  if (state && state->router.stack_size > 1) {
    state->router.stack_size--;
    material_catalog_invalidate_ui(state);
  }
}

static void back_btn_clicked(cmp_event_t *evt, cmp_ui_node_t *node,
                             void *user_data) {
  (void)node;
  if (evt->action == CMP_ACTION_UP) {
    material_catalog_state_t *state = (material_catalog_state_t *)user_data;
    material_catalog_navigate_back(state);
  }
}

static void comp_btn_clicked(cmp_event_t *evt, cmp_ui_node_t *node,
                             void *user_data) {
  (void)node;
  if (evt->action == CMP_ACTION_UP) {
    click_ctx_t *ctx = (click_ctx_t *)user_data;
    material_catalog_navigate_to(ctx->state, CATALOG_SCREEN_COMPONENT_DETAILS,
                                 ctx->id, 0);
  }
}

static void example_btn_clicked(cmp_event_t *evt, cmp_ui_node_t *node,
                                void *user_data) {
  (void)node;
  if (evt->action == CMP_ACTION_UP) {
    click_ctx_t *ctx = (click_ctx_t *)user_data;
    material_catalog_navigate_to(ctx->state, CATALOG_SCREEN_EXAMPLE_VIEWER,
                                 ctx->id, ctx->index);
  }
}

int material_catalog_render_home_screen(material_catalog_state_t *state,
                                        cmp_ui_node_t *container) {
  int i;
  int cellsCount;
  cmp_ui_node_t *grid;

  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  material_catalog_render_top_app_bar(state, container, "Material Catalog", 0);

  if (cmp_ui_box_create(&grid) != 0) {
    return MATERIAL_CATALOG_ERROR_OUT_OF_MEMORY;
  }

  cellsCount = (int)(state->window_width / 180.0f);
  if (cellsCount < 1)
    cellsCount = 1;

  grid->layout->flex_wrap = CMP_FLEX_WRAP;
  grid->layout->direction = CMP_FLEX_ROW;
  grid->layout->width = state->window_width;
  grid->layout->height = state->window_height;
  grid->layout->padding[1] = 16.0f;
  grid->layout->padding[3] = 16.0f;

  for (i = 0; i < g_component_count; ++i) {
    cmp_ui_node_t *card;
    cmp_ui_node_t *icon;
    cmp_ui_node_t *label;
    cmp_ui_node_t *border_bottom;
    cmp_ui_node_t *border_right;
    click_ctx_t *ctx;

    if (cmp_ui_box_create(&card) == 0) {
      cmp_m3_card_metrics_t card_metrics;
      float tonal_opacity, dummy_y, dummy_b, dummy_a, dummy_s;
      cmp_color_t card_bg;

      card->type = 8;                     /* Card */
      card->design_language_override = 1; /* M3 */

      cmp_m3_card_resolve(CMP_M3_CARD_ELEVATED, &card_metrics);
      cmp_m3_elevation_resolve(card_metrics.elevation, &tonal_opacity, &dummy_y, &dummy_b, &dummy_a, &dummy_s);

      {
          cmp_linear_blend_t *blend;
          cmp_linear_blend_create(2.2f, &blend);
          cmp_linear_blend_mix(blend, &state->sys_colors.surface, &state->sys_colors.primary, tonal_opacity, &card_bg);
          cmp_linear_blend_destroy(blend);
      }
      card->bg_color = catalog_color_to_uint32(&card_bg);

      card->layout->width = (state->window_width - 32.0f) / cellsCount;
      card->layout->height = 120.0f; /* Fixed card height */      card->layout->margin[0] = 0.0f;
      card->layout->margin[2] = 0.0f;
      card->layout->direction = CMP_FLEX_COLUMN;
      card->layout->align_items = CMP_FLEX_ALIGN_CENTER;
      card->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
      card->layout->position_type = CMP_POSITION_RELATIVE;

      if (cmp_arena_alloc(&state->ui_arena, sizeof(click_ctx_t),
                          (void **)&ctx) == 0) {
        ctx->state = state;
        ctx->id = g_components[i].id;
        ctx->index = 0;
        cmp_ui_node_add_event_listener(card, 1, 0, comp_btn_clicked, ctx);
      }

      material_catalog_apply_ripple(state, card);

      /* Mock SVG icon (using image view) */
      if (cmp_ui_image_view_create(&icon, "vfs://assets/ic_component.svg") ==
          0) {
        icon->layout->width = 48.0f;
        icon->layout->height = 48.0f;
        icon->layout->margin[2] = 16.0f; /* margin-bottom */
        cmp_ui_node_add_child(card, icon);
      }

      /* Component label */
      if (cmp_ui_text_create(&label, g_components[i].name, -1) == 0) {
        cmp_ui_node_add_child(card, label);
      }

      /* gridItemBorder: bottom */
      if (cmp_ui_box_create(&border_bottom) == 0) {
        border_bottom->layout->position_type = CMP_POSITION_ABSOLUTE;
        border_bottom->layout->position[2] = 0.0f; /* bottom */
        border_bottom->layout->position[3] = 0.0f; /* left */
        border_bottom->layout->width = card->layout->width;
        border_bottom->layout->height = 1.0f; /* 1dp */
        /* Normally we'd set background color for the border here */
        cmp_ui_node_add_child(card, border_bottom);
      }

      /* gridItemBorder: right */
      if (cmp_ui_box_create(&border_right) == 0) {
        border_right->layout->position_type = CMP_POSITION_ABSOLUTE;
        border_right->layout->position[0] = 0.0f; /* top */
        border_right->layout->position[1] = 0.0f; /* right */
        border_right->layout->width = 1.0f;       /* 1dp */
        border_right->layout->height = card->layout->height;
        /* Normally we'd set background color for the border here */
        cmp_ui_node_add_child(card, border_right);
      }

      cmp_ui_node_add_child(grid, card);
    }
  }

  cmp_ui_node_add_child(container, grid);
  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_create_scroll_view(material_catalog_state_t *state,
                                        cmp_ui_node_t **out_scroll_node) {
  if (!state || !out_scroll_node)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  /* Abstracting scroll view creation logic mapping to CMPC's actual
   * cmp_scroll_view_create */
  /* and setting properties. For now we use a UI Box */
  if (cmp_ui_box_create(out_scroll_node) != 0) {
    return MATERIAL_CATALOG_ERROR_OUT_OF_MEMORY;
  }
  (*out_scroll_node)->layout->direction = CMP_FLEX_COLUMN;
  (*out_scroll_node)->layout->width = state->window_width;
  (*out_scroll_node)->layout->height = state->window_height;
  /* Mock property flag indicating scrolling capability */
  return MATERIAL_CATALOG_SUCCESS;
}

static void render_component_details(material_catalog_state_t *state,
                                     cmp_ui_node_t *container,
                                     int component_id) {
  cmp_ui_node_t *scroll_node;
  cmp_ui_node_t *hero_icon;
  cmp_ui_node_t *desc;
  const material_component_t *comp = NULL;
  const material_example_t *examples;
  int example_count = 0;
  int i;

  for (i = 0; i < g_component_count; ++i) {
    if (g_components[i].id == component_id) {
      comp = &g_components[i];
      break;
    }
  }

  if (!comp)
    return;

  material_catalog_render_top_app_bar(state, container, comp->name, 1);

  if (material_catalog_create_scroll_view(state, &scroll_node) == 0) {
    scroll_node->layout->flex_grow = 1.0f;
    scroll_node->layout->width = state->window_width;
    scroll_node->layout->height =
        state->window_height - (64.0f + state->window_insets.y);

    /* Header: 108dp centered hero icon */
    if (cmp_ui_image_view_create(&hero_icon, "vfs://assets/ic_component.svg") ==
        0) {
      hero_icon->layout->width = 108.0f;
      hero_icon->layout->height = 108.0f;
      hero_icon->layout->align_self = CMP_FLEX_ALIGN_CENTER;
      hero_icon->layout->margin[0] = 32.0f;
      hero_icon->layout->margin[2] = 32.0f;
      cmp_ui_node_add_child(scroll_node, hero_icon);
    }

    /* Body: Description typography */
    if (cmp_ui_text_create(&desc, comp->description, -1) == 0) {
      desc->layout->margin[0] = 16.0f;
      desc->layout->margin[1] = 16.0f;
      desc->layout->margin[2] = 32.0f;
      desc->layout->margin[3] = 16.0f;
      /* Medium alpha text color applied conceptually via cmp_theme_t */
      cmp_ui_node_add_child(scroll_node, desc);
    }

    examples = get_examples_for_component(component_id, &example_count);
    for (i = 0; i < example_count; ++i) {
      cmp_ui_node_t *example_item;
      cmp_ui_node_t *example_label;
      cmp_ui_node_t *chevron;
      cmp_ui_node_t *border_bottom;
      click_ctx_t *ctx;

      if (cmp_ui_box_create(&example_item) == 0) {
        example_item->layout->direction = CMP_FLEX_ROW;
        example_item->layout->align_items = CMP_FLEX_ALIGN_CENTER;
        example_item->layout->justify_content = CMP_FLEX_ALIGN_SPACE_BETWEEN;
        example_item->layout->width = state->window_width;
        example_item->layout->height = 56.0f;
        example_item->layout->padding[1] = 16.0f;
        example_item->layout->padding[3] = 16.0f;
        example_item->layout->position_type = CMP_POSITION_RELATIVE;

        if (cmp_arena_alloc(&state->ui_arena, sizeof(click_ctx_t),
                            (void **)&ctx) == 0) {
          ctx->state = state;
          ctx->id = component_id;
          ctx->index = i;
          cmp_ui_node_add_event_listener(example_item, 1, 0,
                                         example_btn_clicked, ctx);
        }

        material_catalog_apply_ripple(state, example_item);

        if (cmp_ui_text_create(&example_label, examples[i].name, -1) == 0) {
          cmp_ui_node_add_child(example_item, example_label);
        }

        if (cmp_ui_text_create(&chevron, ">", -1) == 0) {
          /* Mock right chevron icon */
          cmp_ui_node_add_child(example_item, chevron);
        }

        /* 1dp border bottom */
        if (cmp_ui_box_create(&border_bottom) == 0) {
          border_bottom->layout->position_type = CMP_POSITION_ABSOLUTE;
          border_bottom->layout->position[2] = 0.0f; /* bottom */
          border_bottom->layout->position[3] = 0.0f; /* left */
          border_bottom->layout->width = state->window_width;
          border_bottom->layout->height = 1.0f; /* 1dp */
          /* Normally set background color for divider here */
          cmp_ui_node_add_child(example_item, border_bottom);
        }

        cmp_ui_node_add_child(scroll_node, example_item);
      }
    }
    cmp_ui_node_add_child(container, scroll_node);
  }
}

static void render_example_viewer(material_catalog_state_t *state,
                                  cmp_ui_node_t *container, int component_id,
                                  int example_index) {
  const material_example_t *examples;
  int example_count = 0;
  cmp_ui_node_t *example_content;

  examples = get_examples_for_component(component_id, &example_count);
  if (examples && example_index < example_count) {
    material_catalog_render_top_app_bar(state, container,
                                        examples[example_index].name, 1);

    if (cmp_ui_box_create(&example_content) == 0) {
      example_content->layout->flex_grow = 1.0f;
      example_content->layout->width = state->window_width;
      example_content->layout->height =
          state->window_height - (64.0f + state->window_insets.y);
      example_content->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
      example_content->layout->align_items = CMP_FLEX_ALIGN_CENTER;

      if (examples[example_index].render) {
        examples[example_index].render(example_content);
      } else {
        cmp_ui_node_t *placeholder;
        if (cmp_ui_text_create(&placeholder, "(Example implementation pending)",
                               -1) == 0) {
          cmp_ui_node_add_child(example_content, placeholder);
        }
      }
      cmp_ui_node_add_child(container, example_content);
    }
  }
}

int material_catalog_render_top_app_bar_example(material_catalog_state_t *state,
                                                cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_box_create(&widget);
  widget->layout->width = state->window_width;
  widget->layout->height = 64.0f;

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_bottom_app_bar_example(
    material_catalog_state_t *state, cmp_ui_node_t *container) {
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (cmp_ui_text_create(&lbl, "(Bottom App Bar variations rendered here)",
                         -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  {
    cmp_ui_node_t *mock_bar;
    cmp_m3_bottom_app_bar_metrics_t metrics;
    cmp_m3_bottom_app_bar_resolve(&metrics);

    if (cmp_ui_box_create(&mock_bar) == 0) {
      mock_bar->layout->width = state->window_width - 32.0f;
      mock_bar->layout->height = metrics.height;
      mock_bar->layout->margin[0] = 32.0f;
      mock_bar->layout->margin[3] = 16.0f;
      /* Mock drawing of bottom app bar style */
      cmp_ui_node_add_child(container, mock_bar);
    }
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_render_backdrop_example(material_catalog_state_t *state,
                                             cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_box_create(&widget);
  widget->layout->width = state->window_width;
  widget->layout->height = state->window_height;

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_badges_example(material_catalog_state_t *state,
                                           cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_button_create(&widget, "Badge", -1);

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_bottom_navigation_example(
    material_catalog_state_t *state, cmp_ui_node_t *container) {
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (cmp_ui_text_create(&lbl, "(Bottom Navigation variations rendered here)",
                         -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  {
    cmp_ui_node_t *mock_nav;
    cmp_m3_bottom_nav_metrics_t metrics;
    cmp_m3_bottom_nav_resolve(&metrics);

    if (cmp_ui_box_create(&mock_nav) == 0) {
      mock_nav->layout->width = state->window_width - 32.0f;
      mock_nav->layout->height = metrics.height;
      mock_nav->layout->margin[0] = 32.0f;
      mock_nav->layout->margin[3] = 16.0f;
      /* Mock drawing of bottom nav style */
      cmp_ui_node_add_child(container, mock_nav);
    }
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_render_navigation_rail_example(
    material_catalog_state_t *state, cmp_ui_node_t *container) {
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (cmp_ui_text_create(&lbl, "(Navigation Rail variations rendered here)",
                         -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  {
    cmp_ui_node_t *mock_rail;
    cmp_m3_nav_rail_metrics_t metrics;
    cmp_m3_nav_rail_resolve(&metrics);

    if (cmp_ui_box_create(&mock_rail) == 0) {
      mock_rail->layout->width = metrics.width;
      mock_rail->layout->height = state->window_height - 64.0f;
      mock_rail->layout->margin[0] = 32.0f;
      mock_rail->layout->margin[3] = 16.0f;
      /* Mock drawing of nav rail style */
      cmp_ui_node_add_child(container, mock_rail);
    }
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_render_buttons_example(material_catalog_state_t *state,
                                            cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_button_create(&widget, "Elevated Button", -1);

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_fabs_example(material_catalog_state_t *state,
                                         cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_button_create(&widget, "+", -1);

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_cards_example(material_catalog_state_t *state,
                                          cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_box_create(&widget);
  widget->layout->width = 150;
  widget->layout->height = 100;

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_checkboxes_example(material_catalog_state_t *state,
                                               cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_checkbox_create(&widget, "Material Checkbox");

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_radio_buttons_example(
    material_catalog_state_t *state, cmp_ui_node_t *container) {
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (cmp_ui_text_create(&lbl, "(Radio buttons variations rendered here)",
                         -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  {
    cmp_ui_node_t *mock_box;
    cmp_m3_radio_metrics_t metrics;
    cmp_m3_radio_resolve(&metrics);

    if (cmp_ui_box_create(&mock_box) == 0) {
      mock_box->layout->width =
          metrics.outer_ring_size * 2.0f; /* roughly touch size */
      mock_box->layout->height = metrics.outer_ring_size * 2.0f;
      mock_box->layout->margin[0] = 32.0f;
      mock_box->layout->margin[3] = 16.0f;
      cmp_ui_node_add_child(container, mock_box);
    }
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_render_switches_example(material_catalog_state_t *state,
                                             cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_checkbox_create(&widget, "Switch Toggle");

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_chips_example(material_catalog_state_t *state,
                                          cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_button_create(&widget, "Assist Chip", -1);

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_dialogs_example(material_catalog_state_t *state,
                                            cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_box_create(&widget);
  widget->layout->width = 300;
  widget->layout->height = 200;

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_lists_example(material_catalog_state_t *state,
                                          cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_box_create(&widget);
  widget->layout->width = 300;
  widget->layout->height = 50;

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_menus_example(material_catalog_state_t *state,
                                          cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_dropdown_create(&widget);

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_drawers_sheets_example(
    material_catalog_state_t *state, cmp_ui_node_t *container) {
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (cmp_ui_text_create(&lbl, "(Drawers & Sheets variations rendered here)",
                         -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  {
    cmp_ui_node_t *mock_drawer;
    cmp_m3_drawer_metrics_t metrics;
    cmp_m3_drawer_resolve(CMP_M3_DRAWER_MODAL, &metrics);

    if (cmp_ui_box_create(&mock_drawer) == 0) {
      mock_drawer->layout->width = metrics.max_width;
      mock_drawer->layout->height = state->window_height;
      mock_drawer->layout->margin[0] = 32.0f;
      mock_drawer->layout->margin[3] = 16.0f;
      /* Mock drawing of drawer style */
      cmp_ui_node_add_child(container, mock_drawer);
    }
  }

  {
    cmp_ui_node_t *mock_sheet;
    cmp_m3_sheet_metrics_t metrics;
    cmp_m3_sheet_resolve(CMP_M3_SHEET_BOTTOM_STANDARD, &metrics);

    if (cmp_ui_box_create(&mock_sheet) == 0) {
      mock_sheet->layout->width = state->window_width;
      mock_sheet->layout->height = 200.0f;
      mock_sheet->layout->margin[0] = 32.0f;
      mock_sheet->layout->margin[3] = 16.0f;
      /* Mock drawing of sheet style */
      cmp_ui_node_add_child(container, mock_sheet);
    }
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_render_progress_indicators_example(
    material_catalog_state_t *state, cmp_ui_node_t *container) {
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (cmp_ui_text_create(&lbl, "(Progress indicators variations rendered here)",
                         -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  {
    cmp_ui_node_t *mock_progress;
    cmp_m3_progress_metrics_t metrics;
    cmp_m3_progress_resolve(CMP_M3_PROGRESS_LINEAR, &metrics);

    if (cmp_ui_box_create(&mock_progress) == 0) {
      mock_progress->layout->width = metrics.default_size;
      mock_progress->layout->height = metrics.track_thickness;
      mock_progress->layout->margin[0] = 32.0f;
      mock_progress->layout->margin[3] = 16.0f;
      /* Mock drawing of linear progress style */
      cmp_ui_node_add_child(container, mock_progress);
    }
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_render_sliders_example(material_catalog_state_t *state,
                                            cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_slider_create(&widget, 0.0f, 100.0f);

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_snackbars_example(material_catalog_state_t *state,
                                              cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_button_create(&widget, "Snackbar Demo", -1);

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_tabs_example(material_catalog_state_t *state,
                                         cmp_ui_node_t *container) {
  cmp_ui_node_t *widget = NULL;
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return 1; /* NULL_POINTER */

  if (cmp_ui_text_create(&lbl, "Example component:", -1) == 0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  cmp_ui_box_create(&widget);
  widget->layout->width = 400;
  widget->layout->height = 48;

  if (widget) {
    widget->design_language_override = 1;
    widget->layout->margin[0] = 32.0f;
    widget->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, widget);
  }
  return 0; /* SUCCESS */
}

int material_catalog_render_text_fields_example(material_catalog_state_t *state,
                                                cmp_ui_node_t *container) {
  cmp_ui_node_t *lbl;
  if (!state || !container)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (cmp_ui_text_create(&lbl, "(Text fields variations rendered here)", -1) ==
      0) {
    lbl->layout->margin[0] = 32.0f;
    lbl->layout->margin[3] = 16.0f;
    cmp_ui_node_add_child(container, lbl);
  }

  {
    cmp_ui_node_t *mock_textfield;
    if (cmp_ui_box_create(&mock_textfield) == 0) {
      mock_textfield->layout->width =
          280.0f; /* roughly explicit width per catalog app logic */
      mock_textfield->layout->height = 56.0f;
      mock_textfield->layout->margin[0] = 32.0f;
      mock_textfield->layout->margin[3] = 16.0f;
      /* Mock drawing of text field style */
      cmp_ui_node_add_child(container, mock_textfield);
    }
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_recompose_ui(material_catalog_state_t *state) {
  if (state == NULL || state->window == NULL) {
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  }

  /* Clear the previous UI tree and arena allocations */
  if (state->root_node != NULL) {
    cmp_ui_node_destroy(state->root_node);
    state->root_node = NULL;
  }

  /* Reset the UI arena for the new frame */
  cmp_arena_free(&state->ui_arena);
  cmp_arena_init(&state->ui_arena, 1024 * 1024);

  if (cmp_ui_box_create(&state->root_node) != 0) {
    return MATERIAL_CATALOG_ERROR_OUT_OF_MEMORY;
  }

  state->root_node->design_language_override = 1; /* Material 3 */
  state->root_node->bg_color =
      catalog_color_to_uint32(&state->sys_colors.surface);
  state->root_node->text_color =
      catalog_color_to_uint32(&state->sys_colors.on_surface);
  state->root_node->layout->direction = CMP_FLEX_COLUMN;
  state->root_node->layout->width = state->window_width;
  state->root_node->layout->height = state->window_height;
  state->root_node->layout->align_items = CMP_FLEX_ALIGN_START;

  /* Based on router, build the correct screen */
  if (state->router.stack_size > 0) {
    catalog_route_t *current_route =
        &state->router.stack[state->router.stack_size - 1];

    if (current_route->screen_id == CATALOG_SCREEN_HOME) {
      material_catalog_render_home_screen(state, state->root_node);
    } else if (current_route->screen_id == CATALOG_SCREEN_COMPONENT_DETAILS) {
      render_component_details(state, state->root_node,
                               current_route->component_id);
    } else if (current_route->screen_id == CATALOG_SCREEN_EXAMPLE_VIEWER) {
      render_example_viewer(state, state->root_node,
                            current_route->component_id,
                            current_route->example_index);
    }
  }

  cmp_layout_calculate(state->root_node->layout, state->window_width,
                       state->window_height);

  if (cmp_window_set_ui_tree(state->window, state->root_node) != 0) {
    return MATERIAL_CATALOG_ERROR_INITIALIZATION_FAILED;
  }

  state->is_ui_dirty = 0;
  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_create_ui(material_catalog_state_t *state) {
  return material_catalog_recompose_ui(state);
}

int material_catalog_run(material_catalog_state_t *state) {
  cmp_event_t evt;
  int running = 1;

  if (state == NULL || state->window == NULL) {
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  }

  cmp_window_show(state->window);

  while (running) {
    cmp_window_poll_events(state->window);

    while (cmp_event_pop(&evt) == 0) {
      cmp_hit_test_t *ht;
      cmp_ui_node_t *target = NULL;

      if (cmp_hit_test_create(state->root_node, &ht) == 0) {
        if (cmp_hit_test_query(ht, (float)evt.x, (float)evt.y, &target) == 0 &&
            target != NULL) {
          cmp_event_dispatch_run(state->root_node, target, &evt);
        }
        cmp_hit_test_destroy(ht);
      }
    }

    if (state->is_ui_dirty) {
      material_catalog_recompose_ui(state);
    }

    if (cmp_window_should_close(state->window)) {
      running = 0;
    }
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int material_catalog_cleanup(material_catalog_state_t *state) {
  if (state == NULL) {
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  }

  if (state->root_node != NULL) {
    cmp_ui_node_destroy(state->root_node);
    state->root_node = NULL;
  }

  if (state->window != NULL) {
    cmp_window_destroy(state->window);
    state->window = NULL;
  }

  cmp_arena_free(&state->ui_arena);
  cmp_arena_free(&state->state_arena);

  cmp_window_system_shutdown();
  cmp_event_system_shutdown();
  cmp_dpi_awareness_cleanup();

  return MATERIAL_CATALOG_SUCCESS;
}