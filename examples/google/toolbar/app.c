/* clang-format off */
#include "app.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
/* clang-format on */

typedef enum app_language {
  APP_LANG_EN = 0,
  APP_LANG_HE = 1,
  APP_LANG_AR = 2
} app_language_t;

typedef enum app_theme_palette {
  APP_PALETTE_DEFAULT = 0,
  APP_PALETTE_ALT1 = 1,
  APP_PALETTE_ALT2 = 2,
  APP_PALETTE_ALT3 = 3
} app_theme_palette_t;

typedef enum app_design_language {
  APP_DESIGN_INHERIT = 0,
  APP_DESIGN_MATERIAL3 = 1,
  APP_DESIGN_FLUENT2 = 2,
  APP_DESIGN_CUPERTINO = 3,
  APP_DESIGN_UNSTYLED = 4
} app_design_language_t;

static cmp_window_t *g_window = NULL;
static cmp_ui_node_t *g_ui_tree = NULL;

static int g_is_dark = 0;
static int g_is_rtl = 0;
static app_language_t g_lang = APP_LANG_EN;
static app_theme_palette_t g_palette_idx = APP_PALETTE_DEFAULT;
static float g_scale_factor = 1.0f; /* NATIVE DPI SCALE FIX */
static float g_window_width = 800.0f;
static float g_window_height = 600.0f;

static const uint32_t surface_light[] = {0xFFFEF7FF, 0xFFF8FDFF, 0xFFFFFBF7,
                                         0xFFFFF8FB};
static const uint32_t surface_dark[] = {0xFF141218, 0xFF0E1419, 0xFF121411,
                                        0xFF1A110F};

static int build_ui(void);

static int g_needs_rebuild = 0;

static void on_lang_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  int rc;
  if (evt && evt->action != CMP_ACTION_UP) {
    LOG_DEBUG("Ignoring non-UP event for lang click\n");
    return;
  }
  if (node || ctx) {
    LOG_DEBUG("Clicked lang node\n");
  }

  g_lang = (app_language_t)((g_lang + 1) % 3);
  g_is_rtl = (g_lang != APP_LANG_EN);
  rc = cmp_i18n_set_bidi_direction(g_is_rtl ? CMP_TEXT_DIR_RTL
                                            : CMP_TEXT_DIR_LTR);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_i18n_set_bidi_direction failed: %d\n", rc);
  }
  g_needs_rebuild = 1;
}

static void on_theme_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  int rc;
  rc = 0;
  if (evt && evt->action != CMP_ACTION_UP) {
    LOG_DEBUG("Ignoring non-UP event for theme click\n");
    return;
  }
  if (node || ctx) {
    LOG_DEBUG("Clicked theme node\n");
  }

  g_is_dark = !g_is_dark;
  g_needs_rebuild = 1;
  if (rc != 0) {
    LOG_DEBUG("rc %d\n", rc);
  }
}

static void on_palette_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  int rc;
  rc = 0;
  if (evt && evt->action != CMP_ACTION_UP) {
    LOG_DEBUG("Ignoring non-UP event for palette click\n");
    return;
  }
  if (node || ctx) {
    LOG_DEBUG("Clicked palette node\n");
  }

  g_palette_idx = (app_theme_palette_t)((g_palette_idx + 1) % 4);
  g_needs_rebuild = 1;
  if (rc != 0) {
    LOG_DEBUG("rc %d\n", rc);
  }
}

static int create_simple_button(cmp_ui_node_t **out_btn, const char *text,
                                const char *aria_label,
                                void (*cb)(cmp_event_t *, cmp_ui_node_t *,
                                           void *)) {
  int rc;
  cmp_ui_node_t *btn_node = NULL;
  cmp_a11y_tree_t *tree = NULL;

  if (!out_btn) {
    LOG_DEBUG("out_btn is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_ui_button_create(&btn_node, text, -1);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_button_create failed: %d\n", rc);
    return rc;
  }

  btn_node->layout->width = 64.0f;
  btn_node->layout->height = 40.0f;
  btn_node->layout->margin[3] = 0.0f; /* Relies on actions_row column_gap now */
  btn_node->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
  btn_node->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  btn_node->bg_color = g_is_dark ? 0xFF4A4458 : 0xFFE8DEF8;
  btn_node->text_color = g_is_dark ? 0xFFE8DEF8 : 0xFF1D192B;
  btn_node->font_size = 14.0f;
  btn_node->hover_opacity = 0.08f;
  btn_node->press_opacity = 0.12f;

  if (aria_label) {
    rc = cmp_a11y_tree_create(&tree);
    if (rc == CMP_SUCCESS) {
      rc = cmp_a11y_tree_add_node(tree, btn_node->layout->id, "button",
                                  aria_label);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_a11y_tree_add_node failed: %d\n", rc);
      }
    } else {
      LOG_DEBUG("cmp_a11y_tree_create failed: %d\n", rc);
    }
  }

  if (cb) {
    rc = cmp_ui_node_add_event_listener(btn_node, CMP_EVENT_TYPE_MOUSE, 0, cb,
                                        NULL);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_node_add_event_listener failed: %d\n", rc);
    }
  }

  *out_btn = btn_node;
  return CMP_SUCCESS;
}

static void on_window_resize(int width, int height, void *user_data) {
  int rc;
  if (user_data) {
    LOG_DEBUG("Resize user_data: %p\n", user_data);
  }

  if (g_ui_tree) {
    g_window_width = (float)width / g_scale_factor;
    g_window_height = (float)height / g_scale_factor;
    rc = cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                              g_window_height);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_layout_calculate failed: %d\n", rc);
    }
    if (g_window) {
      rc = cmp_window_set_ui_tree(g_window, g_ui_tree);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_window_set_ui_tree failed: %d\n", rc);
      }
    }
  }
}

static int build_ui(void) {
  int rc;
  cmp_ui_node_t *app_bar = NULL;
  cmp_ui_node_t *title = NULL;
  cmp_ui_node_t *actions_row = NULL;
  cmp_ui_node_t *btn_lang = NULL;
  cmp_ui_node_t *btn_theme = NULL;
  cmp_ui_node_t *btn_palette = NULL;
  cmp_ui_node_t *divider = NULL;
  const char *title_text = "Hello navbar";

  if (g_ui_tree != NULL) {
    rc = cmp_ui_node_destroy(g_ui_tree);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_node_destroy failed: %d\n", rc);
    }
    g_ui_tree = NULL;
  }

  rc = cmp_ui_box_create(&g_ui_tree);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_box_create failed: %d\n", rc);
    return rc;
  }

  g_ui_tree->layout->direction = CMP_FLEX_COLUMN;
  g_ui_tree->layout->width = -1.0f;
  g_ui_tree->layout->height = -1.0f;
  g_ui_tree->layout->justify_content = CMP_FLEX_ALIGN_START;
  g_ui_tree->layout->align_items = CMP_FLEX_ALIGN_STRETCH;
  g_ui_tree->bg_color = g_is_dark ? 0xFF000000 : 0xFFF0F0F0;

  /* Crucial: Keep APP_DESIGN_INHERIT so GDI fallback rendering draws our native
   * nodes */
  g_ui_tree->design_language_override = APP_DESIGN_INHERIT;

  rc = cmp_ui_box_create(&app_bar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_box_create app_bar failed: %d\n", rc);
    return rc;
  }

  app_bar->layout->direction = CMP_FLEX_ROW;
  app_bar->layout->width =
      -1.0f; /* Let it stretch via parent's align_items = STRETCH */
  app_bar->layout->height = 64.0f;
  app_bar->layout->flex_shrink = 0.0f;
  app_bar->layout->justify_content = CMP_FLEX_ALIGN_SPACE_BETWEEN;
  app_bar->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  app_bar->layout->padding[0] = 0.0f;
  app_bar->layout->padding[1] = 4.0f; /* Natively mirrors on RTL */
  app_bar->layout->padding[2] = 0.0f;
  app_bar->layout->padding[3] = 16.0f; /* Natively mirrors on RTL */
  app_bar->bg_color =
      g_is_dark ? surface_dark[g_palette_idx] : surface_light[g_palette_idx];
  app_bar->elevation = 2.0f;

  if (g_lang == APP_LANG_HE) {
    title_text = "\xD7\x94\xD7\x99\xD7\x99 \xD7\xA1\xD7\xA8\xD7\x92\xD7\x9C "
                 "\xD7\xA0\xD7\x99\xD7\x95\xD7\x95\xD7\x98";
  } else if (g_lang == APP_LANG_AR) {
    title_text = "\xD9\x85\xD8\xB1\xD8\xAD\xD8\xA8\xD8\xA7 navbar";
  }

  rc = cmp_ui_text_create(&title, title_text, -1);
  if (rc == CMP_SUCCESS) {
    title->text_color = g_is_dark ? 0xFFE6E0E9 : 0xFF1D1B20;
    title->font_size = 22.0f;
    title->layout->width = 200.0f;
    title->layout->height =
        -1.0f; /* Auto-height for perfect vertical centering */
    title->layout->flex_shrink = 1.0f;
  } else {
    LOG_DEBUG("cmp_ui_text_create title failed: %d\n", rc);
  }

  rc = create_simple_button(&btn_lang, "EN", "Switch Language", on_lang_click);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("create_simple_button lang failed: %d\n", rc);
  }

  rc = create_simple_button(&btn_theme, g_is_dark ? "D/L" : "L/D",
                            "Toggle dark mode", on_theme_click);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("create_simple_button theme failed: %d\n", rc);
  }

  rc = create_simple_button(&btn_palette, "CLR", "Change theme color",
                            on_palette_click);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("create_simple_button palette failed: %d\n", rc);
  }

  /* Add margins to buttons for spacing */
  if (btn_theme)
    btn_theme->layout->margin[3] = 16.0f;
  if (btn_palette)
    btn_palette->layout->margin[3] = 16.0f;

  rc = cmp_ui_box_create(&actions_row);
  if (rc == CMP_SUCCESS) {
    actions_row->layout->direction = CMP_FLEX_ROW;
    actions_row->layout->width =
        240.0f; /* Fixed width prevents off-screen layout math bugs */
    actions_row->layout->height = 40.0f;
    actions_row->layout->align_items = CMP_FLEX_ALIGN_CENTER;
    actions_row->layout->justify_content = CMP_FLEX_ALIGN_END;

    if (btn_lang) {
      rc = cmp_ui_node_add_child(actions_row, btn_lang);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_node_add_child failed: %d\n", rc);
    }
    if (btn_theme) {
      rc = cmp_ui_node_add_child(actions_row, btn_theme);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_node_add_child failed: %d\n", rc);
    }
    if (btn_palette) {
      rc = cmp_ui_node_add_child(actions_row, btn_palette);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_ui_node_add_child failed: %d\n", rc);
    }
  } else {
    LOG_DEBUG("cmp_ui_box_create actions_row failed: %d\n", rc);
  }

  /* Add children to app_bar (Natively mirrored by layout engine in RTL) */
  if (title) {
    rc = cmp_ui_node_add_child(app_bar, title);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_node_add_child failed: %d\n", rc);
  }
  if (actions_row) {
    rc = cmp_ui_node_add_child(app_bar, actions_row);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_node_add_child failed: %d\n", rc);
  }

  rc = cmp_ui_node_add_child(g_ui_tree, app_bar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_node_add_child app_bar failed: %d\n", rc);
  }

  rc = cmp_ui_box_create(&divider);
  if (rc == CMP_SUCCESS) {
    divider->layout->width = -1.0f;
    divider->layout->height = 1.0f;
    divider->layout->flex_shrink = 0.0f;
    divider->bg_color = g_is_dark ? 0xFF49454F : 0xFFCAC4D0;
    rc = cmp_ui_node_add_child(g_ui_tree, divider);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_node_add_child divider failed: %d\n", rc);
    }
  } else {
    LOG_DEBUG("cmp_ui_box_create divider failed: %d\n", rc);
  }

  return CMP_SUCCESS;
}

int app_init(void) {
  int rc;
  cmp_window_config_t config;
  cmp_dpi_t *dpi = NULL;

  rc = cmp_event_system_init();
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_event_system_init failed: %d\n", rc);
  }

  rc = cmp_vfs_init();
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_init failed: %d\n", rc);
  }

  rc = cmp_window_system_init();
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_window_system_init failed: %d\n", rc);
  }

  rc = build_ui();
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("build_ui failed: %d\n", rc);
  }

  if (g_ui_tree) {
    rc = cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                              g_window_height);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_layout_calculate failed: %d\n", rc);
    }
  }

  config.title = "Google Toolbar Example";
  config.width = (int)g_window_width;
  config.height = (int)g_window_height;
  config.x = -1;
  config.y = -1;
  config.hidden = 0;
  config.frameless = 0;
  config.use_legacy_backend = 0;

  rc = cmp_window_create(&config, &g_window);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_window_create failed: %d\n", rc);
    return CMP_ERROR_NOT_FOUND;
  }

  rc = cmp_dpi_create(&dpi);
  if (rc == CMP_SUCCESS) {
    rc = cmp_dpi_update_window_scale(dpi, g_window, 0);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_dpi_update_window_scale failed: %d\n", rc);
    }
    rc = cmp_dpi_destroy(dpi);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_dpi_destroy failed: %d\n", rc);
    }
  } else {
    LOG_DEBUG("cmp_dpi_create failed: %d\n", rc);
  }

  rc = cmp_window_set_resize_callback(g_window, on_window_resize, NULL);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_window_set_resize_callback failed: %d\n", rc);
  }

  rc = cmp_window_set_ui_tree(g_window, g_ui_tree);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_window_set_ui_tree failed: %d\n", rc);
  }

  rc = cmp_window_show(g_window);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_window_show failed: %d\n", rc);
  }

  return CMP_SUCCESS;
}

int app_run(void) {
  int rc;
  cmp_event_t evt;
  int running = 1;

  while (running) {
    rc = cmp_window_poll_events(g_window);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_window_poll_events failed: %d\n", rc);
    }

    while (cmp_event_pop(&evt) == CMP_SUCCESS) {
      cmp_hit_test_t *ht = NULL;
      cmp_ui_node_t *target = NULL;

      if (evt.type == CMP_EVENT_TYPE_RESIZE) {
        g_window_width = (float)evt.x / g_scale_factor;
        g_window_height = (float)evt.y / g_scale_factor;
        if (g_ui_tree) {
          rc = cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                                    g_window_height);
          if (rc != CMP_SUCCESS) {
            LOG_DEBUG("cmp_layout_calculate resize failed: %d\n", rc);
          }
          if (g_window) {
            rc = cmp_window_set_ui_tree(g_window, g_ui_tree);
            if (rc != CMP_SUCCESS) {
              LOG_DEBUG("cmp_window_set_ui_tree resize failed: %d\n", rc);
            }
          }
        }
        continue;
      }

      rc = cmp_hit_test_create(g_ui_tree, &ht);
      if (rc == CMP_SUCCESS) {
        rc = cmp_hit_test_query(ht, (float)evt.x, (float)evt.y, &target);
        if (rc == CMP_SUCCESS && target != NULL) {
          rc = cmp_event_dispatch_run(g_ui_tree, target, &evt);
          if (rc != CMP_SUCCESS) {
            LOG_DEBUG("cmp_event_dispatch_run failed: %d\n", rc);
          }
        } else if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_hit_test_query failed: %d\n", rc);
        }
        rc = cmp_hit_test_destroy(ht);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_hit_test_destroy failed: %d\n", rc);
        }
      } else {
        LOG_DEBUG("cmp_hit_test_create failed: %d\n", rc);
      }
    }

    if (g_needs_rebuild) {
      g_needs_rebuild = 0;
      rc = build_ui();
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("build_ui failed: %d\n", rc);
      }
      if (g_ui_tree && g_window) {
        rc = cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                                  g_window_height);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_layout_calculate rebuild failed: %d\n", rc);
        }
        rc = cmp_window_set_ui_tree(g_window, g_ui_tree);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_window_set_ui_tree rebuild failed: %d\n", rc);
        }
      }
    }

    rc = cmp_window_should_close(g_window);
    if (rc) {
      running = 0;
    }
  }

  return CMP_SUCCESS;
}

int app_shutdown(void) {
  int rc;

  if (g_ui_tree) {
    rc = cmp_ui_node_destroy(g_ui_tree);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_node_destroy failed: %d\n", rc);
    }
    g_ui_tree = NULL;
  }

  if (g_window) {
    rc = cmp_window_destroy(g_window);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_window_destroy failed: %d\n", rc);
    }
    g_window = NULL;
  }

  rc = cmp_window_system_shutdown();
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_window_system_shutdown failed: %d\n", rc);
  }

  rc = cmp_vfs_shutdown();
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_shutdown failed: %d\n", rc);
  }

  rc = cmp_event_system_shutdown();
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_event_system_shutdown failed: %d\n", rc);
  }

  return CMP_SUCCESS;
}
