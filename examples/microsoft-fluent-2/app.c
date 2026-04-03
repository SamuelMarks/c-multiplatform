/* clang-format off */
#include "app.h"
#include "cmp.h"
#include "themes/cmp_f2_button.h"
#include "themes/cmp_f2_inputs.h"
#include "themes/cmp_f2_text_inputs.h"
#include "themes/cmp_f2_theme.h"
#include "themes/cmp_f2_data_display.h"
#include <stdio.h>
/* clang-format on */

static cmp_window_t *g_window = NULL;
static cmp_ui_node_t *g_ui_tree = NULL;
static int g_is_dark = 0;
static cmp_ui_node_t *g_btn_theme = NULL;
static cmp_ui_node_t *g_btn_state = NULL;

static int manual_hit_test(cmp_ui_node_t *node, float x, float y) {
  size_t i;
  if (!node || !node->layout) {
    return -1;
  }
  if (x >= node->layout->computed_rect.x &&
      x <= node->layout->computed_rect.x + node->layout->computed_rect.width &&
      y >= node->layout->computed_rect.y &&
      y <= node->layout->computed_rect.y + node->layout->computed_rect.height) {
    int hit = -1;
    for (i = 0; i < node->child_count; ++i) {
      int child_hit = manual_hit_test(node->children[i], x, y);
      if (child_hit != -1) {
        hit = child_hit;
      }
    }
    if (hit != -1) {
      return hit;
    }
    if (node->layout->id != 0) {
      return node->layout->id;
    }
  }
  return -1;
}

static int build_ui(void) {
  cmp_ui_node_t *main_box = NULL;
  cmp_ui_node_t *btn1 = NULL, *btn2 = NULL, *cb = NULL, *tg = NULL,
                *txt_in = NULL;
  int res;

  if (g_ui_tree != NULL) {
    cmp_ui_node_destroy(g_ui_tree);
    g_ui_tree = NULL;
  }

  res = cmp_ui_box_create(&g_ui_tree);
  if (res != CMP_SUCCESS)
    return res;

  g_ui_tree->layout->direction = CMP_FLEX_COLUMN;
  g_ui_tree->layout->padding[0] = 24.0f;
  g_ui_tree->layout->padding[1] = 24.0f;
  g_ui_tree->layout->padding[2] = 24.0f;
  g_ui_tree->layout->padding[3] = 24.0f;
  g_ui_tree->layout->width = 800.0f;
  g_ui_tree->layout->height = 600.0f;

  if (cmp_ui_box_create(&main_box) != CMP_SUCCESS)
    return CMP_ERROR_NOT_FOUND;
  main_box->layout->direction = CMP_FLEX_COLUMN;
  main_box->layout->align_items = CMP_FLEX_ALIGN_START;

  if (cmp_f2_button_create(&btn1, "Primary Button", NULL) != CMP_SUCCESS)
    return CMP_ERROR_NOT_FOUND;
  cmp_f2_button_set_variant(btn1, CMP_F2_BUTTON_VARIANT_PRIMARY);
  btn1->layout->margin[2] = 16.0f;

  if (cmp_f2_button_create(&btn2, "Secondary Button", NULL) != CMP_SUCCESS)
    return CMP_ERROR_NOT_FOUND;
  btn2->layout->margin[2] = 16.0f;

  if (cmp_f2_checkbox_create(&cb, "Fluent 2 Checkbox") != CMP_SUCCESS)
    return CMP_ERROR_NOT_FOUND;
  cb->layout->margin[2] = 16.0f;

  if (cmp_f2_toggle_create(&tg, "Fluent 2 Toggle") != CMP_SUCCESS)
    return CMP_ERROR_NOT_FOUND;
  tg->layout->margin[2] = 16.0f;

  if (cmp_f2_text_input_create(&txt_in) != CMP_SUCCESS)
    return CMP_ERROR_NOT_FOUND;
  txt_in->layout->margin[2] = 16.0f;
  txt_in->layout->width = 200.0f;
  txt_in->layout->height = 32.0f;

  if (cmp_f2_button_create(&g_btn_theme,
                           g_is_dark ? "Switch to Light Mode"
                                     : "Switch to Dark Mode",
                           NULL) != CMP_SUCCESS)
    return CMP_ERROR_NOT_FOUND;
  g_btn_theme->layout->id = 101;
  g_btn_theme->layout->margin[2] = 16.0f;

  cmp_ui_node_add_child(main_box, btn1);
  cmp_ui_node_add_child(main_box, btn2);
  cmp_ui_node_add_child(main_box, cb);
  cmp_ui_node_add_child(main_box, tg);
  cmp_ui_node_add_child(main_box, txt_in);
  cmp_ui_node_add_child(main_box, g_btn_theme);

  cmp_ui_node_add_child(g_ui_tree, main_box);

  return CMP_SUCCESS;
}

int app_init(void) {
  cmp_window_config_t config;

  cmp_event_system_init();
  cmp_vfs_init();
  cmp_window_system_init();

  config.title = "Microsoft Fluent 2 Component Gallery";
  config.width = 800;
  config.height = 600;
  config.x = -1;
  config.y = -1;
  config.hidden = 0;
  config.frameless = 0;
  config.use_legacy_backend = 0;

  if (cmp_window_create(&config, &g_window) != CMP_SUCCESS) {
    return CMP_ERROR_NOT_FOUND;
  }

  if (build_ui() != CMP_SUCCESS) {
    return CMP_ERROR_NOT_FOUND;
  }

  if (g_ui_tree) {
    cmp_layout_calculate(g_ui_tree->layout, 800.0f, 600.0f);
  }
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
      if (evt.action == CMP_ACTION_DOWN && (evt.type == 1 || evt.type == 2)) {
        int hit_node = manual_hit_test(g_ui_tree, (float)evt.x, (float)evt.y);
        if (hit_node == 101) {
          g_is_dark = !g_is_dark;
          build_ui();
          if (g_ui_tree) {
            cmp_layout_calculate(g_ui_tree->layout, 800.0f, 600.0f);
          }
          cmp_window_set_ui_tree(g_window, g_ui_tree);
        }
      }
    }

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
