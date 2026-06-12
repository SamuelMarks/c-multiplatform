/* clang-format off */
#include "app.h"
#include "cmp.h"
#include "cmp_ui_action_button.h"
#include "cmp_ui_app_bar.h"
#include "cmp_log.h"
#include <stdio.h>
/* clang-format on */

#define REM                                                                    \
  ((g_window_width < g_window_height ? g_window_width : g_window_height) /     \
   50.0f)

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
static app_design_language_t g_design_lang = APP_DESIGN_INHERIT;
static cmp_databinding_t *g_title_binding = NULL;
static app_theme_palette_t g_palette_idx = APP_PALETTE_DEFAULT;
static float g_scale_factor = 1.0f; /* NATIVE DPI SCALE FIX */
static float g_window_width = 1280.0f;
static float g_window_height = 768.0f;

static const uint32_t surface_light[] = {0xFFFEF7FF, 0xFFF8FDFF, 0xFFFFFBF7,
                                         0xFFFFF8FB};
static const uint32_t surface_dark[] = {0xFF141218, 0xFF0E1419, 0xFF121411,
                                        0xFF1A110F};

static int build_ui(void);

static int g_needs_rebuild = 0;

static void update_title_binding(void) {
  char buf[128];
  const char *design_str = "Inherit";

  if (!g_title_binding) {
    return;
  }

  switch (g_design_lang) {
  case APP_DESIGN_INHERIT:
    design_str = "Inherit";
    break;
  case APP_DESIGN_MATERIAL3:
    design_str = "Material 3";
    break;
  case APP_DESIGN_FLUENT2:
    design_str = "Fluent 2";
    break;
  case APP_DESIGN_CUPERTINO:
    design_str = "Cupertino";
    break;
  case APP_DESIGN_UNSTYLED:
    design_str = "Unstyled";
    break;
  }

  if (g_lang == APP_LANG_HE) {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "\xD7\xA2\xD7\x99\xD7\xA6\xD7\x95\xD7\x91 %s",
              design_str);
#else
    sprintf(buf, "\xD7\xA2\xD7\x99\xD7\xA6\xD7\x95\xD7\x91 %s", design_str);
#endif
  } else if (g_lang == APP_LANG_AR) {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "\xD8\xAA\xD8\xB5\xD9\x85\xD9\x8A\xD9\x85 %s",
              design_str);
#else
    sprintf(buf, "\xD8\xAA\xD8\xB5\xD9\x85\xD9\x8A\xD9\x85 %s", design_str);
#endif
  } else {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%s Design", design_str);
#else
    sprintf(buf, "%s Design", design_str);
#endif
  }

  (void)cmp_databinding_set_string(g_title_binding, buf);
}

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
  update_title_binding();
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

static void on_design_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  int rc;
  rc = 0;
  if (evt && evt->action != CMP_ACTION_UP) {
    LOG_DEBUG("Ignoring non-UP event for design click\n");
    return;
  }
  if (node || ctx) {
    LOG_DEBUG("Clicked design node\n");
  }

  g_design_lang = (app_design_language_t)((g_design_lang + 1) % 5);

  update_title_binding();

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

  /* Force the button to render natively in all design languages.
     Fluent 2 specifically requires the "type" parameter to be exactly 3.
     Rely completely on the inherited design language override for
     colors, contrast, and borders. */
  btn_node->layout->margin[3] = 0.0f;

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
  return rc;
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
  cmp_ui_app_bar_t *app_bar_obj = NULL;
  cmp_ui_node_t *title = NULL;
  cmp_ui_node_t *btn_lang = NULL;
  cmp_ui_node_t *btn_theme = NULL;
  cmp_ui_node_t *btn_palette = NULL;
  cmp_ui_node_t *btn_design = NULL;
  const char *title_text = "Multi-Design System Toolbar Example";

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
  g_ui_tree->layout->overflow_x = 2; /* Hidden */
  g_ui_tree->layout->overflow_y = 1; /* Scroll */
  g_ui_tree->bg_color = g_is_dark ? 0xFF000000 : 0xFFF0F0F0;

  g_ui_tree->design_language_override = g_design_lang;

  rc = cmp_ui_app_bar_create(&app_bar_obj, CMP_UI_APP_BAR_PLACEMENT_TOP);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_app_bar_create failed: %d\n", rc);
    return rc;
  }

  rc = cmp_ui_app_bar_get_node(app_bar_obj, &app_bar);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_app_bar_get_node failed: %d\n", rc);
    return rc;
  }

  app_bar->bg_color =
      g_is_dark ? surface_dark[g_palette_idx] : surface_light[g_palette_idx];
  app_bar->elevation = 2.0f;

  if (g_lang == APP_LANG_HE) {
    title_text = "\xD7\x94\xD7\x99\xD7\x99 \xD7\xA1\xD7\xA8\xD7\x92\xD7\x9C "
                 "\xD7\xA0\xD7\x99\xD7\x95\xD7\x95\xD7\x98";
  } else if (g_lang == APP_LANG_AR) {
    title_text = "\xD9\x85\xD8\xB1\xD8\xAD\xD8\xA8\xD8\xA7 navbar";
  }

  (void)cmp_ui_app_bar_set_title(app_bar_obj, title_text);
  if (app_bar->child_count > 0) {
    title = app_bar->children[0];
    if (g_title_binding) {
      (void)cmp_ui_node_bind_generic(title, g_title_binding, "text");
    }
    title->text_color = g_is_dark ? 0xFFE6E0E9 : 0xFF1D1B20;
    title->font_size = -1.0f; /* Let system scale font */
  }

  rc = create_simple_button(&btn_lang, "EN", "Switch Language", on_lang_click);
  rc = create_simple_button(&btn_theme, g_is_dark ? "D/L" : "L/D",
                            "Toggle dark mode", on_theme_click);
  rc = create_simple_button(&btn_palette, "CLR", "Change theme color",
                            on_palette_click);
  rc = create_simple_button(&btn_design, "DSG", "Change design system",
                            on_design_click);

  if (btn_lang) {
    btn_lang->layout->width = -1.0f;
    btn_lang->layout->height = -1.0f;
  }
  if (btn_theme) {
    btn_theme->layout->width = -1.0f;
    btn_theme->layout->height = -1.0f;
    btn_theme->layout->margin[3] = 1.0f * REM;
  }
  if (btn_palette) {
    btn_palette->layout->width = -1.0f;
    btn_palette->layout->height = -1.0f;
    btn_palette->layout->margin[3] = 1.0f * REM;
  }
  if (btn_design) {
    btn_design->layout->width = -1.0f;
    btn_design->layout->height = -1.0f;
    btn_design->layout->margin[3] = 1.0f * REM;
  }

  if (btn_lang)
    (void)cmp_ui_app_bar_add_action(app_bar_obj, btn_lang);
  if (btn_theme)
    (void)cmp_ui_app_bar_add_action(app_bar_obj, btn_theme);
  if (btn_palette)
    (void)cmp_ui_app_bar_add_action(app_bar_obj, btn_palette);
  if (btn_design)
    (void)cmp_ui_app_bar_add_action(app_bar_obj, btn_design);
  rc = cmp_ui_node_add_child(g_ui_tree, app_bar);
  if (rc != CMP_SUCCESS)
    return rc;

  return rc;
}

int app_init(void) {
  int rc = 0;
  cmp_window_config_t config;
  cmp_dpi_t *dpi = NULL;

  rc = cmp_databinding_create(&g_title_binding, CMP_DATA_TYPE_STRING);
  if (rc == CMP_SUCCESS) {
    (void)cmp_databinding_set_string(g_title_binding, "Inherit");
  }

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

  config.title = "Multi-Design System Toolbar Example";
  config.width = (int)g_window_width;
  config.height = (int)g_window_height;
  g_ui_tree->layout->display = CMP_DISPLAY_FLEX;
  config.root_layout = g_ui_tree->layout;
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

  return rc;
}

int app_run(void) {
  int rc = 0;
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

  return rc;
}

int app_shutdown(void) {
  int rc = 0;
  rc = 0;

  if (g_title_binding) {
    (void)cmp_databinding_destroy(g_title_binding);
    g_title_binding = NULL;
  }

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

  return rc;
}
