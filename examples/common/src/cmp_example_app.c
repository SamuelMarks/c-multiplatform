/* clang-format off */
#include "cmp_example_app.h"
#include "cmp.h"
#include "cmp_log.h"
#include "cmp_ui_app_bar.h"
#include "cmp_ui_icon_button.h"
#include "cmp_ui_tooltip.h"
#include "cmp_ui_badge.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * \file cmp_example_app.c
 * \brief Implementation of the modular, ejectable application shell.
 */

static cmp_window_t *g_window = NULL;
static cmp_ui_node_t *g_ui_tree = NULL;
static cmp_ui_node_t *g_body_container = NULL;
static cmp_a11y_tree_t *g_a11y_tree = NULL;

static int g_is_dark = 1;
static int g_is_rtl = 0;

static cmp_example_design_system_t g_design_lang = CMP_EXAMPLE_DESIGN_MATERIAL3;
static cmp_databinding_t *g_title_binding = NULL;
static float g_scale_factor = 1.0f;
static float g_window_width = 1280.0f;
static float g_window_height = 768.0f;
static int g_needs_rebuild = 0;

static const cmp_example_app_callbacks_t *g_callbacks = NULL;

/**
 * \brief Triggers a layout recalculation.
 * \return 0 on success, non-zero error code otherwise.
 */
int cmp_example_app_recalculate_layout(void) {
  int rc;
  if (!g_ui_tree || !g_window) {
    return CMP_ERROR_INVALID_STATE;
  }
  rc = cmp_layout_calculate(g_ui_tree->layout, g_window_width, g_window_height);
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  rc = cmp_window_set_ui_tree(g_window, g_ui_tree);
  if (rc != CMP_SUCCESS)
    return rc;
  return rc;
}

static void update_title_binding(void) {
  char buf[128];
  const char *design_str = "Inherit";

  if (!g_title_binding) {
    return;
  }

  switch (g_design_lang) {
  case CMP_EXAMPLE_DESIGN_MATERIAL3:
    design_str = "Material 3";
    break;
  case CMP_EXAMPLE_DESIGN_FLUENT2:
    design_str = "Fluent 2";
    break;
  case CMP_EXAMPLE_DESIGN_CUPERTINO:
    design_str = "Cupertino";
    break;
  case CMP_EXAMPLE_DESIGN_UNSTYLED:
    design_str = "Unstyled";
    break;
  default:
    break;
  }
#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%s Examples", design_str);
#else
  sprintf(buf, "%s Examples", design_str);
#endif

  (void)cmp_databinding_set_string(g_title_binding, buf);
}

static void on_lang_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  int rc;
  (void)node;
  (void)ctx;
  if (evt && evt->action != CMP_ACTION_UP) {
    return;
  }
  g_is_rtl = !g_is_rtl;
  rc = cmp_i18n_set_bidi_direction(g_is_rtl ? CMP_TEXT_DIR_RTL
                                            : CMP_TEXT_DIR_LTR);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_i18n_set_bidi_direction failed: %d\n", rc);
  }
  update_title_binding();
  g_needs_rebuild = 1;
}

static void on_theme_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  (void)node;
  (void)ctx;
  if (evt && evt->action != CMP_ACTION_UP) {
    return;
  }
  g_is_dark = !g_is_dark;
  g_needs_rebuild = 1;
}

static void on_design_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  (void)node;
  (void)ctx;
  if (evt && evt->action != CMP_ACTION_UP) {
    return;
  }
  g_design_lang = (cmp_example_design_system_t)((g_design_lang % 4) + 1);
  update_title_binding();
  g_needs_rebuild = 1;
}

static void on_palette_click(cmp_event_t *evt, cmp_ui_node_t *node, void *ctx) {
  (void)node;
  (void)ctx;
  if (evt && evt->action != CMP_ACTION_UP) {
    return;
  }
  LOG_DEBUG("Clicked color palette button\n");
}

static void on_window_resize(int width, int height, void *ctx) {
  (void)width;
  (void)height;
  (void)ctx;
  /* Ignored. We rely on CMP_EVENT_TYPE_RESIZE for logical dimensions. */
}

static void dump_layout(cmp_ui_node_t *node, int depth, FILE *f) {
  int i;
  const char *props = "";
  if (!node)
    return;
  for (i = 0; i < depth; i++)
    fprintf(f, "  ");

  if ((node->type == 4 || node->type == 3 || node->type == 2) &&
      node->properties) {
    props = (const char *)node->properties;
  }

  fprintf(
      f, "Node %p (Type %d): {x: %.1f, y: %.1f, w: %.1f, h: %.1f} text: '%s'\n",
      (void *)node, node->type, node->layout->computed_rect.x,
      node->layout->computed_rect.y, node->layout->computed_rect.width,
      node->layout->computed_rect.height, props);
  for (i = 0; i < (int)node->child_count; i++) {
    dump_layout(node->children[i], depth + 1, f);
  }
}

static int build_ui(void) {
  int rc;
  size_t c;
  cmp_ui_app_bar_t *app_bar_cmp = NULL;
  cmp_ui_node_t *app_bar_node = NULL;
  cmp_ui_icon_button_t *btn_lang = NULL;
  cmp_ui_node_t *btn_lang_node = NULL;
  cmp_ui_icon_button_t *btn_theme = NULL;
  cmp_ui_node_t *btn_theme_node = NULL;
  cmp_ui_icon_button_t *btn_design = NULL;
  cmp_ui_node_t *btn_design_node = NULL;
  cmp_ui_node_t *actions_row = NULL;

  if (g_a11y_tree) {
    (void)cmp_a11y_tree_destroy(g_a11y_tree);
    g_a11y_tree = NULL;
  }
  rc = cmp_a11y_tree_create(&g_a11y_tree);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_a11y_tree_create failed: %d\n", rc);
  }

  if (g_ui_tree != NULL) {
    rc = cmp_ui_node_destroy(g_ui_tree);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_node_destroy failed: %d\n", rc);
    }
    g_ui_tree = NULL;
  }

  rc = cmp_ui_box_create(&g_ui_tree);
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  g_ui_tree->layout->direction = CMP_FLEX_COLUMN;
  g_ui_tree->layout->width = -1.0f;
  g_ui_tree->layout->height = -1.0f;
  g_ui_tree->layout->margin[0] = 0.0f;
  g_ui_tree->layout->margin[1] = 0.0f;
  g_ui_tree->layout->margin[2] = 0.0f;
  g_ui_tree->layout->margin[3] = 0.0f;
  g_ui_tree->layout->align_items = CMP_FLEX_ALIGN_STRETCH;
  g_ui_tree->bg_color = g_is_dark ? 0xFF000000 : 0xFFFFFFFF;
  g_ui_tree->design_language_override = g_design_lang;

  /* Use App Bar */
  rc = cmp_ui_app_bar_create(&app_bar_cmp, CMP_UI_APP_BAR_PLACEMENT_TOP);
  if (rc == CMP_SUCCESS) {
    rc = cmp_ui_app_bar_get_node(app_bar_cmp, &app_bar_node);
    if (rc == CMP_SUCCESS) {
      app_bar_node->layout->width = -1.0f;
      app_bar_node->layout->height = -1.0f;
      app_bar_node->layout->flex_grow = 1.0f;
      app_bar_node->layout->flex_shrink = 0.0f;
      app_bar_node->layout->justify_content = CMP_FLEX_ALIGN_SPACE_BETWEEN;
      app_bar_node->layout->align_items = CMP_FLEX_ALIGN_CENTER;
      app_bar_node->layout->padding[1] = 16.0f;
      app_bar_node->layout->padding[3] = 16.0f;
      app_bar_node->bg_color = 0xFF4A4458;
      app_bar_node->text_color = 0xFFFFFFFF;
      rc = cmp_ui_node_add_child(g_ui_tree, app_bar_node);
      if (rc != CMP_SUCCESS)
        return rc;
    }
  }

  if (app_bar_cmp) {
    (void)cmp_ui_app_bar_set_title(app_bar_cmp, "Example App");
    app_bar_node->text_color = 0xFFFFFFFF;
    if (app_bar_node->children && app_bar_node->child_count > 0) {
      app_bar_node->children[0]->text_color = 0xFFFFFFFF;
      app_bar_node->children[0]->layout->flex_grow = 0.0f;
      app_bar_node->children[0]->layout->flex_shrink = 0.0f;
      app_bar_node->children[0]->layout->width = -1.0f; /* auto stretch */
      /* flex layout adjusted */
      app_bar_node->children[0]->layout->height = -1.0f;
      app_bar_node->children[0]->layout->flex_grow = 1.0f;
      app_bar_node->children[0]->font_size = -1.0f; /* Let system scale font */

      if (app_bar_node->child_count > 1) {
        app_bar_node->children[1]->layout->flex_shrink = 0.0f;
      }
    }
    if (g_a11y_tree)
      (void)cmp_ui_app_bar_bind_a11y(app_bar_cmp, g_a11y_tree);
  }

  /* Actions */
  (void)cmp_ui_box_create(&actions_row);
  actions_row->layout->direction = CMP_FLEX_ROW;
  actions_row->layout->flex_shrink = 0.0f;
  actions_row->layout->width = -1.0f; /* auto stretch */
  /* flex layout adjusted */
  actions_row->layout->height = -1.0f;
  actions_row->layout->flex_grow = 1.0f;
  actions_row->layout->justify_content = CMP_FLEX_ALIGN_END;

  {
    rc = cmp_ui_icon_button_create(&btn_lang, "LNG",
                                   CMP_UI_ICON_BUTTON_STYLE_STANDARD);
    if (rc == CMP_SUCCESS) {
      (void)cmp_ui_icon_button_get_node(btn_lang, &btn_lang_node);
      btn_lang_node->text_color = g_is_dark ? 0xFFFFFFFF : 0xFF000000;
      btn_lang_node->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
      btn_lang_node->text_color = 0xFFFFFFFF;
      for (c = 0; c < btn_lang_node->child_count; c++) {
        btn_lang_node->children[c]->text_color = 0xFFFFFFFF;
        btn_lang_node->children[c]->font_size =
            -1.0f; /* Let system scale font */
        btn_lang_node->children[c]->layout->flex_shrink = 0.0f;
        btn_lang_node->children[c]->layout->width = -1.0f; /* auto stretch */
        /* flex layout adjusted */
      }
      btn_lang_node->layout->width = -1.0f; /* auto stretch */
                                            /* flex layout adjusted */
      btn_lang_node->layout->height = -1.0f;
      btn_lang_node->layout->flex_grow = 1.0f;
      btn_lang_node->layout->flex_shrink = 0.0f;
      btn_lang_node->bg_color = 0xFF2B2930;
      btn_lang_node->border_radius = 8.0f;
      btn_lang_node->border_width = 1.0f;
      btn_lang_node->border_color = 0xFFFFFFFF;
      btn_lang_node->layout->margin[1] = 4.0f;
      btn_lang_node->layout->margin[3] = 4.0f;
      cmp_ui_node_add_event_listener(btn_lang_node, CMP_EVENT_TYPE_MOUSE, 0,
                                     on_lang_click, NULL);
      (void)cmp_ui_node_add_child(actions_row, btn_lang_node);
      if (g_a11y_tree)
        (void)cmp_ui_icon_button_bind_a11y(btn_lang, g_a11y_tree);
    }
  }

  {
    rc = cmp_ui_icon_button_create(&btn_theme, g_is_dark ? "LGT" : "DRK",
                                   CMP_UI_ICON_BUTTON_STYLE_STANDARD);
    if (rc == CMP_SUCCESS) {
      (void)cmp_ui_icon_button_get_node(btn_theme, &btn_theme_node);
      btn_theme_node->text_color = 0xFFFFFFFF;
      btn_theme_node->bg_color = 0xFF2B2930;
      btn_theme_node->border_radius = 8.0f;
      btn_theme_node->layout->margin[1] = 4.0f;
      btn_theme_node->layout->margin[3] = 4.0f;
      btn_theme_node->layout->justify_content = CMP_FLEX_ALIGN_CENTER;
      btn_theme_node->layout->align_items = CMP_FLEX_ALIGN_CENTER;
      for (c = 0; c < btn_theme_node->child_count; c++) {
        btn_theme_node->children[c]->text_color = 0xFFFFFFFF;
        btn_theme_node->children[c]->font_size =
            -1.0f; /* Let system scale font */
        btn_theme_node->children[c]->layout->flex_shrink = 0.0f;
      }
      btn_theme_node->layout->width = -1.0f; /* auto stretch */
                                             /* flex layout adjusted */
      btn_theme_node->layout->height = -1.0f;
      btn_theme_node->layout->flex_grow = 1.0f;
      btn_theme_node->layout->flex_shrink = 0.0f;
      cmp_ui_node_add_event_listener(btn_theme_node, CMP_EVENT_TYPE_MOUSE, 0,
                                     on_theme_click, NULL);
      (void)cmp_ui_node_add_child(actions_row, btn_theme_node);
      if (g_a11y_tree)
        (void)cmp_ui_icon_button_bind_a11y(btn_theme, g_a11y_tree);
    }
  }

  {
    cmp_ui_icon_button_t *btn_palette = NULL;
    cmp_ui_node_t *btn_palette_node = NULL;
    rc = cmp_ui_icon_button_create(&btn_palette, "CLR",
                                   CMP_UI_ICON_BUTTON_STYLE_STANDARD);
    if (rc == CMP_SUCCESS) {
      (void)cmp_ui_icon_button_get_node(btn_palette, &btn_palette_node);
      btn_palette_node->text_color = 0xFFFFFFFF;
      btn_palette_node->bg_color = 0xFF2B2930;
      btn_palette_node->border_radius = 8.0f;
      btn_palette_node->layout->margin[1] = 4.0f;
      btn_palette_node->layout->margin[3] = 4.0f;
      for (c = 0; c < btn_palette_node->child_count; c++) {
        btn_palette_node->children[c]->text_color = 0xFFFFFFFF;
        btn_palette_node->children[c]->font_size =
            -1.0f; /* Let system scale font */
        btn_palette_node->children[c]->layout->flex_shrink = 0.0f;
      }
      btn_palette_node->layout->width = -1.0f; /* auto stretch */
                                               /* flex layout adjusted */
      btn_palette_node->layout->height = -1.0f;
      btn_palette_node->layout->flex_grow = 1.0f;
      btn_palette_node->layout->flex_shrink = 0.0f;
      cmp_ui_node_add_event_listener(btn_palette_node, CMP_EVENT_TYPE_MOUSE, 0,
                                     on_palette_click, NULL);
      (void)cmp_ui_node_add_child(actions_row, btn_palette_node);
      if (g_a11y_tree)
        (void)cmp_ui_icon_button_bind_a11y(btn_palette, g_a11y_tree);
    }
  }

  {
    cmp_ui_badge_t *badge_design = NULL;
    cmp_ui_node_t *badge_node = NULL;
    rc = cmp_ui_icon_button_create(&btn_design, "SYS",
                                   CMP_UI_ICON_BUTTON_STYLE_STANDARD);
    if (rc == CMP_SUCCESS) {
      (void)cmp_ui_icon_button_get_node(btn_design, &btn_design_node);
      btn_design_node->text_color = 0xFFFFFFFF;
      btn_design_node->bg_color = 0xFF2B2930;
      btn_design_node->border_radius = 8.0f;
      btn_design_node->layout->margin[1] = 4.0f;
      btn_design_node->layout->margin[3] = 4.0f;
      for (c = 0; c < btn_design_node->child_count; c++) {
        btn_design_node->children[c]->text_color = 0xFFFFFFFF;
        btn_design_node->children[c]->font_size =
            -1.0f; /* Let system scale font */
        btn_design_node->children[c]->layout->flex_shrink = 0.0f;
      }
      btn_design_node->layout->width = -1.0f; /* auto stretch */
                                              /* flex layout adjusted */
      btn_design_node->layout->height = -1.0f;
      btn_design_node->layout->flex_grow = 1.0f;
      btn_design_node->layout->flex_shrink = 0.0f;
      cmp_ui_node_add_event_listener(btn_design_node, CMP_EVENT_TYPE_MOUSE, 0,
                                     on_design_click, NULL);

      (void)cmp_ui_badge_create(&badge_design, "DS", 0xFFFF0000, 0xFFFFFFFF);
      (void)cmp_ui_badge_get_node(badge_design, &badge_node);
      (void)cmp_ui_node_add_child(btn_design_node, badge_node);
      (void)cmp_ui_node_add_child(actions_row, btn_design_node);
      if (g_a11y_tree)
        (void)cmp_ui_icon_button_bind_a11y(btn_design, g_a11y_tree);
      if (g_a11y_tree)
        (void)cmp_ui_badge_bind_a11y(badge_design, g_a11y_tree);
    }
  }

  if (app_bar_cmp) {
    (void)cmp_ui_app_bar_add_action(app_bar_cmp, actions_row);
  }

  /* Body container */
  rc = cmp_ui_box_create(&g_body_container);
  if (rc == CMP_SUCCESS) {
    g_body_container->layout->flex_grow = 1.0f;
    g_body_container->layout->width = -1.0f;
    g_body_container->layout->height = -1.0f;
    g_body_container->layout->direction = CMP_FLEX_COLUMN;
    rc = cmp_ui_node_add_child(g_ui_tree, g_body_container);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_node_add_child failed\n");
    }
  }

  if (g_callbacks && g_callbacks->build_body_ui) {
    rc = g_callbacks->build_body_ui(g_design_lang, g_body_container);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("build_body_ui callback failed: %d\n", rc);
    }
  }

  return CMP_SUCCESS;
}

int cmp_example_app_run(const char *title,
                        const cmp_example_app_callbacks_t *callbacks) {
  int rc;
  cmp_window_config_t config;
  cmp_event_t evt;
  int running = 1;
  cmp_dpi_t *dpi = NULL;

  g_callbacks = callbacks;
  g_design_lang = CMP_EXAMPLE_DESIGN_MATERIAL3;

  rc = cmp_event_system_init();
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  rc = cmp_vfs_init();
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  rc = cmp_window_system_init();
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  rc = build_ui();
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  if (g_ui_tree) {
    rc = cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                              g_window_height);
    if (rc != CMP_SUCCESS) {
      return rc;
    }
  }

  memset(&config, 0, sizeof(config));
  config.title = title ? title : "Example App";
  config.width = (int)g_window_width;
  config.height = (int)g_window_height;
  g_ui_tree->layout->display = CMP_DISPLAY_FLEX;
  config.root_layout = g_ui_tree->layout;
  config.x = -1;
  config.y = -1;

  rc = cmp_window_create(&config, &g_window);
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  rc = cmp_dpi_create(&dpi);
  if (rc == CMP_SUCCESS) {
    rc = cmp_dpi_update_window_scale(dpi, g_window, 0);
    if (rc == CMP_SUCCESS) {
      /* Assume scale worked */
    }
    (void)cmp_dpi_destroy(dpi);
  }

  if (g_ui_tree) {
    rc = cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                              g_window_height);
    if (rc != CMP_SUCCESS) {
      return rc;
    }
  }

  rc = cmp_window_set_resize_callback(g_window, on_window_resize, NULL);
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  rc = cmp_window_set_ui_tree(g_window, g_ui_tree);
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  dump_layout(g_ui_tree, 0, stderr);
  fflush(stderr);

  rc = cmp_window_show(g_window);
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  while (running) {
    rc = cmp_window_poll_events(g_window);
    if (rc != CMP_SUCCESS) {
      /* Log error */
    }

    while (cmp_event_pop(&evt) == CMP_SUCCESS) {
      cmp_hit_test_t *ht = NULL;
      cmp_ui_node_t *target = NULL;

      if (evt.type == CMP_EVENT_TYPE_RESIZE) {
        g_window_width = (float)evt.x;
        g_window_height = (float)evt.y;
        g_needs_rebuild = 1;
        continue;
      }

      rc = cmp_hit_test_create(g_ui_tree, &ht);
      if (rc == CMP_SUCCESS) {
        rc = cmp_hit_test_query(ht, (float)evt.x, (float)evt.y, &target);
        if (rc == CMP_SUCCESS && target != NULL) {
          rc = cmp_event_dispatch_run(g_ui_tree, target, &evt);
          if (rc != CMP_SUCCESS) {
            /* error */
          }
        }
        (void)cmp_hit_test_destroy(ht);
      }
    }

    if (g_needs_rebuild) {
      g_needs_rebuild = 0;
      rc = build_ui();
      if (rc == CMP_SUCCESS && g_ui_tree && g_window) {
        cmp_layout_calculate(g_ui_tree->layout, g_window_width,
                             g_window_height);
        (void)cmp_window_set_ui_tree(g_window, g_ui_tree);
      }
    }

    rc = cmp_window_should_close(g_window);
    if (rc) {
      running = 0;
    }
  }

  if (g_ui_tree) {
    (void)cmp_ui_node_destroy(g_ui_tree);
    g_ui_tree = NULL;
  }

  (void)cmp_window_destroy(g_window);
  (void)cmp_window_system_shutdown();
  (void)cmp_vfs_shutdown();
  (void)cmp_event_system_shutdown();
  return CMP_SUCCESS;
}
