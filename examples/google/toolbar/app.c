/* clang-format off */
#include "app.h"
#include "cmp.h"
#include <stdio.h>
/* clang-format on */

static cmp_window_t *g_window = NULL;
static cmp_ui_node_t *g_ui_tree = NULL;

static int g_is_dark = 0;
static int g_is_rtl = 0;
static int g_lang = 0;
static int g_palette_idx = 0;

static const uint32_t surface_light[] = {0xFFFEF7FF, 0xFFF8FDFF, 0xFFFFFBF7,
                                         0xFFFFF8FB};
static const uint32_t surface_dark[] = {0xFF141218, 0xFF0E1419, 0xFF121411,
                                        0xFF1A110F};
static const uint32_t on_surface_light[] = {0xFF1D1B20, 0xFF191C20, 0xFF1A1C19,
                                            0xFF201A19};
static const uint32_t on_surface_dark[] = {0xFFE6E0E9, 0xFFE1E2E8, 0xFFE2E3DF,
                                           0xFFEAE0DF};

static int build_ui(void);

static void on_lang_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  (void)evt;
  (void)node;
  (void)ctx;
  g_lang = (g_lang + 1) % 3;
  g_is_rtl = (g_lang != 0);
  cmp_i18n_set_bidi_direction(g_is_rtl ? CMP_TEXT_DIR_RTL : CMP_TEXT_DIR_LTR);
  build_ui();
  if (g_ui_tree) {
    /* Use current window bounds */
    cmp_layout_calculate(g_ui_tree->layout, g_ui_tree->layout->width,
                         g_ui_tree->layout->height);
  }
  if (g_window)
    cmp_window_set_ui_tree(g_window, g_ui_tree);
}

static void on_theme_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  cmp_framebuffer_capture_t *capture = NULL;
  (void)evt;
  (void)node;
  (void)ctx;
  if (g_window) {
    (void)cmp_compositor_capture_framebuffer(g_window, &capture);
  }
  g_is_dark = !g_is_dark;
  build_ui();
  if (g_ui_tree) {
    cmp_layout_calculate(g_ui_tree->layout, g_ui_tree->layout->width,
                         g_ui_tree->layout->height);
  }
  if (g_window) {
    cmp_window_set_ui_tree(g_window, g_ui_tree);
    if (capture) {
      (void)cmp_compositor_start_crossfade(g_window, capture, 200.0, NULL);
      (void)cmp_compositor_release_framebuffer(capture);
    }
  }
}

static void on_palette_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  cmp_framebuffer_capture_t *capture = NULL;
  (void)evt;
  (void)node;
  (void)ctx;
  if (g_window) {
    (void)cmp_compositor_capture_framebuffer(g_window, &capture);
  }
  g_palette_idx = (g_palette_idx + 1) % 4;
  build_ui();
  if (g_ui_tree) {
    cmp_layout_calculate(g_ui_tree->layout, g_ui_tree->layout->width,
                         g_ui_tree->layout->height);
  }
  if (g_window) {
    cmp_window_set_ui_tree(g_window, g_ui_tree);
    if (capture) {
      (void)cmp_compositor_start_crossfade(g_window, capture, 200.0, NULL);
      (void)cmp_compositor_release_framebuffer(capture);
    }
  }
}

static int create_m3_icon_button(cmp_ui_node_t **out_btn, const char *icon_str,
                                 const char *aria_label,
                                 void (*cb)(cmp_event_t *, cmp_ui_node_t *,
                                            void *)) {
  cmp_ui_icon_button_t *icon_btn = NULL;
  cmp_ui_node_t *btn_node = NULL;

  if (!out_btn) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (cmp_ui_icon_button_create(&icon_btn, icon_str, 0 /* STANDARD */) !=
      CMP_SUCCESS) {
    return CMP_ERROR_NOT_FOUND;
  }

  if (cmp_ui_icon_button_get_node(icon_btn, &btn_node) != CMP_SUCCESS) {
    cmp_ui_icon_button_destroy(icon_btn);
    return CMP_ERROR_NOT_FOUND;
  }

  /* 4. Iconography & Relative Touch Targets */
  btn_node->layout->width = 48.0f;  /* 3rem (48dp) touch target */
  btn_node->layout->height = 48.0f; /* 3rem (48dp) touch target */
  btn_node->border_radius = 24.0f;  /* 50% border radius for circle */
  btn_node->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
  btn_node->layout->align_items = CMP_FLEX_ALIGN_CENTER;

  /* The child is the text node containing the icon */
  if (btn_node->child_count > 0) {
    btn_node->children[0]->font_size =
        24.0f; /* 1.5rem (24dp) icon glyph size */
    /* font-variation-settings logic goes here conceptually */
    btn_node->children[0]->text_color =
        g_is_dark ? on_surface_dark[g_palette_idx]
                  : on_surface_light[g_palette_idx]; /* M3 On Surface Variant */
  }

  /* Interactive state layers: hover/pressed opacity setup */
  btn_node->hover_opacity = 0.08f; /* 8% hover overlay */
  btn_node->press_opacity = 0.12f; /* 12% pressed overlay */

  /* Apply default transparent bg color */
  btn_node->bg_color = 0x00000000;

  if (aria_label) {
    /* Setup accessibility role and label */
    cmp_a11y_tree_t *tree = NULL;
    if (cmp_a11y_tree_create(&tree) == CMP_SUCCESS) {
      cmp_a11y_tree_add_node(tree, btn_node->layout->id, "button", aria_label);
      /* In a real app we'd bind this tree to the window/document */
    }
  }

  if (cb) {
    (void)cmp_ui_node_add_event_listener(btn_node, 1 /*MOUSE*/, 2 /*UP*/, cb,
                                         NULL);
  }

  *out_btn = btn_node;
  return CMP_SUCCESS;
}

static void on_window_resize(int width, int height, void *user_data) {
  (void)user_data;
  if (g_ui_tree) {
    g_ui_tree->layout->width = (float)width;
    g_ui_tree->layout->height = (float)height;
    if (g_ui_tree->layout->child_count > 0) {
      g_ui_tree->layout->children[0]->width = (float)width;
    }
    cmp_layout_calculate(g_ui_tree->layout, (float)width, (float)height);
  }
}

static int build_ui(void) {
  cmp_ui_node_t *app_bar = NULL;
  cmp_ui_node_t *title = NULL;
  cmp_ui_node_t *actions_row = NULL;
  cmp_ui_node_t *btn_lang = NULL;
  cmp_ui_node_t *btn_theme = NULL;
  cmp_ui_node_t *btn_palette = NULL;
  int rc;
  const char *title_text = "Hello navbar";

  if (g_ui_tree != NULL) {
    cmp_ui_node_destroy(g_ui_tree);
    g_ui_tree = NULL;
  }

  rc = cmp_ui_box_create(&g_ui_tree);
  if (rc != CMP_SUCCESS)
    return rc;

  g_ui_tree->layout->direction = CMP_FLEX_COLUMN;
  g_ui_tree->layout->width = 1024.0f;
  g_ui_tree->layout->height = -1.0f;
  g_ui_tree->layout->justify_content = CMP_FLEX_ALIGN_START;
  g_ui_tree->bg_color = g_is_dark ? surface_dark[g_palette_idx]
                                  : surface_light[g_palette_idx]; /* Surface */
  g_ui_tree->design_language_override = 1;                        /* M3 */

  /* M3 Top App Bar */
  rc = cmp_ui_box_create(&app_bar);
  if (rc != CMP_SUCCESS)
    return rc;

  app_bar->layout->direction = CMP_FLEX_ROW;
  app_bar->layout->width = 1024.0f;
  app_bar->layout->height = 64.0f;
  app_bar->layout->flex_shrink = 0.0f;
  app_bar->layout->justify_content = CMP_FLEX_ALIGN_SPACE_BETWEEN;
  app_bar->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  app_bar->layout->padding[0] = 8.0f;                    /* Top */
  app_bar->layout->padding[1] = g_is_rtl ? 16.0f : 4.0f; /* Right */
  app_bar->layout->padding[2] = 8.0f;                    /* Bottom */
  app_bar->layout->padding[3] = g_is_rtl ? 4.0f : 16.0f; /* Left */
  app_bar->bg_color = g_is_dark ? surface_dark[g_palette_idx]
                                : surface_light[g_palette_idx]; /* Surface */

  if (g_lang == 1) {
    title_text = "\xD7\x94\xD7\x99\xD7\x99 \xD7\xA1\xD7\xA8\xD7\x92\xD7\x9C "
                 "\xD7\xA0\xD7\x99\xD7\x95\xD7\x95\xD7\x98"; /* Hebrew */
  } else if (g_lang == 2) {
    title_text = "\xD9\x85\xD8\xB1\xD8\xAD\xD8\xA8\xD8\xA7 navbar"; /* Arabic */
  }

  rc = cmp_ui_text_create(&title, title_text, -1);
  if (rc == CMP_SUCCESS) {
    title->text_color = g_is_dark
                            ? on_surface_dark[g_palette_idx]
                            : on_surface_light[g_palette_idx]; /* On Surface */
    title->font_size = 22.0f;                                  /* Title Large */
    title->layout->flex_shrink = 1.0f;
    title->layout->width = 200.0f;
  }

  /* Actions Row */
  rc = cmp_ui_box_create(&actions_row);
  if (rc == CMP_SUCCESS) {
    actions_row->layout->direction = CMP_FLEX_ROW;
    actions_row->layout->height = 48.0f;
    actions_row->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    actions_row->layout->column_gap = 8.0f;
    actions_row->layout->flex_shrink = 0.0f;

    (void)create_m3_icon_button(&btn_lang, "language", "Switch Language",
                                on_lang_click);
    (void)create_m3_icon_button(&btn_theme,
                                g_is_dark ? "light_mode" : "dark_mode",
                                "Toggle dark mode", on_theme_click);
    (void)create_m3_icon_button(&btn_palette, "palette", "Change theme color",
                                on_palette_click);

    if (g_is_rtl) {
      cmp_ui_node_add_child(actions_row, btn_palette);
      cmp_ui_node_add_child(actions_row, btn_theme);
      cmp_ui_node_add_child(actions_row, btn_lang);
    } else {
      cmp_ui_node_add_child(actions_row, btn_lang);
      cmp_ui_node_add_child(actions_row, btn_theme);
      cmp_ui_node_add_child(actions_row, btn_palette);
    }
  }

  if (g_is_rtl) {
    cmp_ui_node_add_child(app_bar, actions_row);
    cmp_ui_node_add_child(app_bar, title);
  } else {
    cmp_ui_node_add_child(app_bar, title);
    cmp_ui_node_add_child(app_bar, actions_row);
  }

  cmp_ui_node_add_child(g_ui_tree, app_bar);

  return CMP_SUCCESS;
}

int app_init(void) {
  cmp_window_config_t config;
  float initial_width = 1024.0f;
  float initial_height = 768.0f;
  cmp_dpi_t *dpi = NULL;

  cmp_event_system_init();
  cmp_vfs_init();
  cmp_window_system_init();

  build_ui();
  if (g_ui_tree) {
    g_ui_tree->layout->width = initial_width;
    g_ui_tree->layout->height = initial_height;
    if (g_ui_tree->layout->child_count > 0) {
      g_ui_tree->layout->children[0]->width = initial_width;
    }
    cmp_layout_calculate(g_ui_tree->layout, initial_width, initial_height);
  }

  config.title = "Google Toolbar Example";
  config.width = (int)initial_width;
  config.height = (int)initial_height;
  config.x = -1;
  config.y = -1;
  config.hidden = 0;
  config.frameless = 0;
  config.use_legacy_backend = 0;

  if (cmp_window_create(&config, &g_window) != CMP_SUCCESS) {
    return CMP_ERROR_NOT_FOUND;
  }

  if (cmp_dpi_create(&dpi) == CMP_SUCCESS) {
    cmp_dpi_update_window_scale(dpi, g_window, 0);
    cmp_dpi_destroy(dpi);
  }

  cmp_window_set_resize_callback(g_window, on_window_resize, NULL);

  cmp_window_set_ui_tree(g_window, g_ui_tree);
  cmp_window_show(g_window);

  return CMP_SUCCESS;
}

int app_run(void) {
  cmp_event_t evt;
  int running = 1;

  while (running) {
    cmp_window_poll_events(g_window);

    while (cmp_event_pop(&evt) == CMP_SUCCESS) {
      cmp_hit_test_t *ht;
      cmp_ui_node_t *target = NULL;

      if (cmp_hit_test_create(g_ui_tree, &ht) == CMP_SUCCESS) {
        if (cmp_hit_test_query(ht, (float)evt.x, (float)evt.y, &target) ==
                CMP_SUCCESS &&
            target != NULL) {
          cmp_event_dispatch_run(g_ui_tree, target, &evt);
        }
        cmp_hit_test_destroy(ht);
      }
    }

    cmp_window_render_test_frame(g_window);
    if (cmp_window_should_close(g_window)) {
      running = 0;
    }
  }

  return CMP_SUCCESS;
}

int app_shutdown(void) {
  if (g_ui_tree) {
    cmp_ui_node_destroy(g_ui_tree);
    g_ui_tree = NULL;
  }

  if (g_window) {
    cmp_window_destroy(g_window);
    g_window = NULL;
  }

  cmp_window_system_shutdown();
  cmp_vfs_shutdown();
  cmp_event_system_shutdown();

  return CMP_SUCCESS;
}
