/* clang-format off */
#include "material_catalog.h"
#include "m3_color.h"
#include "m3_buttons.h"
#include "m3_inputs.h"
#include "m3_containment.h"
#include "m3_i18n.h"
#include <string.h>
#include <stdio.h>
/* clang-format on */

static void on_theme_segment_changed(cmp_event_t *e, cmp_ui_node_t *n,
                                     void *user_data) {
  int index = (int)(uintptr_t)user_data;
  cmp_framebuffer_capture_t *capture = NULL;
  (void)n;
  if (e->action != 2 /* CMP_ACTION_UP */)
    return;

  if (g_m3_state) {
    if (g_m3_state->window) {
      cmp_compositor_capture_framebuffer(g_m3_state->window, &capture);
    }

    if (index == 0)
      g_m3_state->current_theme = CATALOG_THEME_LIGHT;
    else if (index == 1)
      g_m3_state->current_theme = CATALOG_THEME_DARK;
    /* index 2 is SYSTEM, could just use light for now or check OS */

    cmp_m3_sys_colors_generate(g_m3_state->sys_colors.primary,
                               g_m3_state->current_theme == CATALOG_THEME_DARK,
                               0, &g_m3_state->sys_colors);
    material_catalog_update_sys_colors_hex(g_m3_state);
    material_catalog_invalidate_paint(g_m3_state);

    if (g_m3_state->window && capture) {
      cmp_compositor_start_crossfade(g_m3_state->window, capture, 300.0,
                                     NULL); /* Emphasized Decelerate */
    }
  }
}

static void on_locale_selected(cmp_event_t *e, cmp_ui_node_t *n,
                               void *user_data) {
  const char *locale = (const char *)user_data;
  (void)n;
  if (e->action != 2 /* CMP_ACTION_UP */)
    return;

  if (g_m3_state) {
    m3_i18n_set_locale(g_m3_state, locale);
    material_catalog_invalidate_ui(g_m3_state);
  }
}

static void on_color_click(cmp_event_t *e, cmp_ui_node_t *n, void *user_data) {
  uint32_t color = (uint32_t)(uintptr_t)user_data;
  cmp_color_t seed;
  (void)n;
  if (e->action != 2 /* CMP_ACTION_UP */)
    return;

  seed.r = ((color >> 16) & 0xFF) / 255.0f;
  seed.g = ((color >> 8) & 0xFF) / 255.0f;
  seed.b = (color & 0xFF) / 255.0f;
  seed.a = ((color >> 24) & 0xFF) / 255.0f;
  seed.space = 0; /* CMP_COLOR_SPACE_SRGB */

  if (g_m3_state) {
    cmp_m3_sys_colors_generate(
        seed, g_m3_state->current_theme == CATALOG_THEME_DARK,
        0 /* CMP_M3_CONTRAST_STANDARD */, &g_m3_state->sys_colors);
    material_catalog_update_sys_colors_hex(g_m3_state);
    material_catalog_invalidate_paint(g_m3_state);
  }
}

static void on_hex_input(cmp_event_t *e, cmp_ui_node_t *n, void *user_data) {
  /* Handle hex input */
  (void)e;
  (void)n;
  (void)user_data;
}

int material_catalog_render_theme_studio(material_catalog_state_t *state,
                                         cmp_ui_node_t *container) {
  cmp_ui_node_t *box = NULL, *title = NULL;
  cmp_ui_node_t *settings_card = NULL;
  m3_card_config_t card_cfg;
  cmp_ui_node_t *theme_toggle = NULL;
  const char *segments[3];
  cmp_ui_node_t *lang_selector = NULL;
  m3_text_field_config_t tf_cfg;
  cmp_ui_node_t *hc_row = NULL;
  cmp_ui_node_t *hc_label = NULL;
  cmp_ui_node_t *hc_switch = NULL;
  m3_switch_config_t sw_cfg;
  cmp_ui_node_t *colors_title = NULL;
  cmp_ui_node_t *hex_input = NULL;
  m3_text_field_config_t hex_cfg;
  cmp_ui_node_t *swatches_row = NULL;
  uint32_t colors[4];
  int i;
  cmp_ui_node_t *preview_title = NULL;
  cmp_ui_node_t *preview_card = NULL;
  cmp_ui_node_t *preview_fab = NULL;
  m3_fab_config_t fab_cfg;

  if (!state || !container) {
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  }

  segments[0] = m3_i18n_get_string(state, "btn_light");
  segments[1] = m3_i18n_get_string(state, "btn_dark");
  segments[2] = m3_i18n_get_string(state, "btn_system");

  colors[0] = 0xFF6750A4; /* Purple */
  colors[1] = 0xFFB3261E; /* Red/Rose */
  colors[2] = 0xFF006874; /* Indigo/Cyan */
  colors[3] = 0xFF006A60; /* Teal */

  cmp_ui_box_create(&box);
  box->layout->direction = CMP_FLEX_COLUMN;
  box->layout->width = state->content_width; /* 100% */
  box->layout->padding[0] = dp_to_px(state, 16.0f);
  box->layout->padding[1] = dp_to_px(state, 16.0f);
  box->layout->padding[2] = dp_to_px(state, 16.0f);
  box->layout->padding[3] = dp_to_px(state, 16.0f);

  cmp_ui_text_create(&title, m3_i18n_get_string(state, "title_theme_studio"),
                     -1);
  title->layout->margin[2] = dp_to_px(state, 16.0f);
  cmp_ui_node_add_child(box, title);

  /* 1. Settings Section */
  memset(&card_cfg, 0, sizeof(card_cfg));
  card_cfg.type = M3_CARD_TYPE_OUTLINED;
  m3_card_create(state, &card_cfg, &settings_card);
  settings_card->layout->direction = CMP_FLEX_COLUMN;
  settings_card->layout->padding[0] = dp_to_px(state, 16.0f);
  settings_card->layout->padding[1] = dp_to_px(state, 16.0f);
  settings_card->layout->padding[2] = dp_to_px(state, 16.0f);
  settings_card->layout->padding[3] = dp_to_px(state, 16.0f);
  settings_card->layout->margin[2] = dp_to_px(state, 24.0f);

  /* Segmented Toggles: Light, Dark, System */
  m3_segmented_button_create(state, segments, 3, 0, on_theme_segment_changed,
                             NULL, &theme_toggle);
  theme_toggle->layout->margin[2] = dp_to_px(state, 16.0f);
  cmp_ui_node_add_child(settings_card, theme_toggle);

  /* Language Selector (using TextField + Trailing icon mock) */
  memset(&tf_cfg, 0, sizeof(tf_cfg));
  tf_cfg.type = M3_TEXT_FIELD_TYPE_OUTLINED;
  tf_cfg.label = m3_i18n_get_string(state, "label_language");
  /* Actually display current locale name */
  if (strncmp(state->current_locale, "ar", 2) == 0) {
    tf_cfg.text = "العربية";
  } else if (strncmp(state->current_locale, "he", 2) == 0) {
    tf_cfg.text = "עברית";
  } else {
    tf_cfg.text = m3_i18n_get_string(state, "val_english_us");
  }
  tf_cfg.trailing_icon = "arrow_drop_down";
  m3_text_field_create(state, &tf_cfg, &lang_selector);
  lang_selector->layout->margin[2] = dp_to_px(state, 16.0f);

  /* Add click listeners for language switching (mock dropdown for now: click
   * cycles) */
  if (strncmp(state->current_locale, "en", 2) == 0) {
    cmp_ui_node_add_event_listener(lang_selector, 1 /* CMP_EVENT_TYPE_MOUSE */,
                                   1, on_locale_selected, "ar-SA");
  } else if (strncmp(state->current_locale, "ar", 2) == 0) {
    cmp_ui_node_add_event_listener(lang_selector, 1 /* CMP_EVENT_TYPE_MOUSE */,
                                   1, on_locale_selected, "he-IL");
  } else {
    cmp_ui_node_add_event_listener(lang_selector, 1 /* CMP_EVENT_TYPE_MOUSE */,
                                   1, on_locale_selected, "en-US");
  }
  cmp_ui_node_add_child(settings_card, lang_selector);

  /* High Contrast Switch */
  cmp_ui_box_create(&hc_row);
  hc_row->layout->direction = CMP_FLEX_ROW;
  hc_row->layout->justify_content = CMP_FLEX_ALIGN_SPACE_BETWEEN;
  hc_row->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  hc_row->layout->margin[2] = dp_to_px(state, 16.0f);

  cmp_ui_text_create(&hc_label,
                     m3_i18n_get_string(state, "label_high_contrast"), -1);
  cmp_ui_node_add_child(hc_row, hc_label);

  memset(&sw_cfg, 0, sizeof(sw_cfg));
  sw_cfg.is_on = 0;
  m3_switch_create(state, &sw_cfg, &hc_switch);
  cmp_ui_node_add_child(hc_row, hc_switch);

  cmp_ui_node_add_child(settings_card, hc_row);
  cmp_ui_node_add_child(box, settings_card);

  /* 2. Colors Section */
  cmp_ui_text_create(&colors_title,
                     m3_i18n_get_string(state, "title_colors_hct"), -1);
  colors_title->layout->margin[2] = dp_to_px(state, 16.0f);
  cmp_ui_node_add_child(box, colors_title);

  /* Hex Input Field */
  memset(&hex_cfg, 0, sizeof(hex_cfg));
  hex_cfg.type = M3_TEXT_FIELD_TYPE_FILLED;
  hex_cfg.label = m3_i18n_get_string(state, "label_seed_color");
  hex_cfg.text = "#6750A4"; /* Default M3 Purple */
  m3_text_field_create(state, &hex_cfg, &hex_input);
  hex_input->layout->margin[2] = dp_to_px(state, 16.0f);
  cmp_ui_node_add_child(box, hex_input);

  /* Preset Color Swatches */
  cmp_ui_box_create(&swatches_row);
  swatches_row->layout->direction = CMP_FLEX_ROW;
  swatches_row->layout->margin[2] = dp_to_px(state, 24.0f);

  for (i = 0; i < 4; ++i) {
    cmp_ui_node_t *swatch = NULL;
    cmp_ui_box_create(&swatch);
    swatch->layout->width = dp_to_px(state, 48.0f);
    swatch->layout->height = dp_to_px(state, 48.0f);
    swatch->layout->margin[2] = dp_to_px(state, 8.0f);
    swatch->bg_color = colors[i];
    material_catalog_apply_shape(swatch, THEME_SHAPE_ROUNDED,
                                 dp_to_px(state, 24.0f)); /* Circular */

    /* Set callback to make it interactive */
    cmp_ui_node_add_event_listener(swatch, 1 /* CMP_EVENT_TYPE_MOUSE */, 1,
                                   on_color_click,
                                   (void *)(uintptr_t)colors[i]);
    cmp_ui_node_add_child(swatches_row, swatch);
  }
  cmp_ui_node_add_child(box, swatches_row);

  /* 3. Live Theme Preview Dashboard (Mini Dashboard) */
  cmp_ui_text_create(&preview_title,
                     m3_i18n_get_string(state, "title_live_preview"), -1);
  preview_title->layout->margin[2] = dp_to_px(state, 16.0f);
  cmp_ui_node_add_child(box, preview_title);

  memset(&card_cfg, 0, sizeof(card_cfg));
  card_cfg.type = M3_CARD_TYPE_FILLED;
  m3_card_create(state, &card_cfg, &preview_card);
  preview_card->layout->direction = CMP_FLEX_COLUMN;
  preview_card->layout->padding[0] = dp_to_px(state, 16.0f);
  preview_card->layout->padding[1] = dp_to_px(state, 16.0f);
  preview_card->layout->padding[2] = dp_to_px(state, 16.0f);
  preview_card->layout->padding[3] = dp_to_px(state, 16.0f);

  memset(&fab_cfg, 0, sizeof(fab_cfg));
  fab_cfg.size = M3_FAB_SIZE_STANDARD;
  fab_cfg.icon = "add";
  m3_fab_create(state, &fab_cfg, &preview_fab);
  cmp_ui_node_add_child(preview_card, preview_fab);

  cmp_ui_node_add_child(box, preview_card);

  cmp_ui_node_add_child(container, box);

  return MATERIAL_CATALOG_SUCCESS;
}
