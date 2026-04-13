/* clang-format off */
#include "app.h"
#include "cmp.h"
#include "themes/cmp_f2_button.h"
#include "themes/cmp_f2_inputs.h"
#include "themes/cmp_f2_text_inputs.h"
#include "themes/cmp_f2_theme.h"
#include "themes/cmp_f2_data_display.h"
#include "themes/cmp_f2_surfaces.h"
#include <stdio.h>
/* clang-format on */

typedef enum {
  SCREEN_HOME = 0,
  SCREEN_BUTTONS,
  SCREEN_INPUTS,
  SCREEN_TEXT_INPUTS,
  SCREEN_DATA_DISPLAY
} app_screen_t;

static cmp_window_t *g_window = NULL;
static cmp_ui_node_t *g_ui_tree = NULL;
static int g_is_dark = 0;
static app_screen_t g_current_screen = SCREEN_HOME;

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

static int build_header(cmp_ui_node_t *parent, const char *title) {
  cmp_ui_node_t *header = NULL, *btn_back = NULL, *btn_theme = NULL,
                *title_node = NULL;
  (void)title;
  cmp_ui_box_create(&header);
  header->layout->direction = CMP_FLEX_ROW;
  header->layout->align_items = CMP_FLEX_ALIGN_CENTER;
  header->layout->width = 800.0f;
  header->layout->padding[3] = 24.0f;

  if (g_current_screen != SCREEN_HOME) {
    cmp_f2_button_create(&btn_back, "Back", NULL);
    btn_back->layout->id = 100;
    btn_back->layout->margin[1] = 16.0f;
    cmp_ui_node_add_child(header, btn_back);
  }

  cmp_ui_box_create(&title_node);
  /* TODO: Use actual typography for title once mapped, but simple box/text for
   * now */
  /* cmp_ui_text_create(&title_node, title, NULL); */
  cmp_ui_node_add_child(header, title_node);

  cmp_f2_button_create(&btn_theme, g_is_dark ? "Light Mode" : "Dark Mode",
                       NULL);
  btn_theme->layout->id = 101;
  btn_theme->layout->margin[1] = 16.0f;
  cmp_ui_node_add_child(header, btn_theme);

  cmp_ui_node_add_child(parent, header);
  return CMP_SUCCESS;
}

static int build_home_screen(cmp_ui_node_t *main_box) {
  cmp_ui_node_t *grid = NULL;
  cmp_ui_node_t *card1 = NULL, *card2 = NULL, *card3 = NULL, *card4 = NULL;

  cmp_ui_box_create(&grid);
  grid->layout->direction = CMP_FLEX_ROW;
  grid->layout->flex_wrap = CMP_FLEX_WRAP;
  grid->layout->padding[0] = 24.0f;

  /* Card 1: Buttons */
  cmp_f2_card_create(&card1);
  cmp_f2_card_set_clickable(card1, 1);
  card1->layout->id = 201;
  card1->layout->width = 200.0f;
  card1->layout->height = 150.0f;
  card1->layout->margin[1] = 16.0f;
  card1->layout->margin[2] = 16.0f;
  /* (Would add label inside card1) */
  cmp_ui_node_add_child(grid, card1);

  /* Card 2: Inputs */
  cmp_f2_card_create(&card2);
  cmp_f2_card_set_clickable(card2, 1);
  card2->layout->id = 202;
  card2->layout->width = 200.0f;
  card2->layout->height = 150.0f;
  card2->layout->margin[1] = 16.0f;
  card2->layout->margin[2] = 16.0f;
  cmp_ui_node_add_child(grid, card2);

  /* Card 3: Text Inputs */
  cmp_f2_card_create(&card3);
  cmp_f2_card_set_clickable(card3, 1);
  card3->layout->id = 203;
  card3->layout->width = 200.0f;
  card3->layout->height = 150.0f;
  card3->layout->margin[1] = 16.0f;
  card3->layout->margin[2] = 16.0f;
  cmp_ui_node_add_child(grid, card3);

  /* Card 4: Data Display */
  cmp_f2_card_create(&card4);
  cmp_f2_card_set_clickable(card4, 1);
  card4->layout->id = 204;
  card4->layout->width = 200.0f;
  card4->layout->height = 150.0f;
  card4->layout->margin[1] = 16.0f;
  card4->layout->margin[2] = 16.0f;
  cmp_ui_node_add_child(grid, card4);

  cmp_ui_node_add_child(main_box, grid);
  return CMP_SUCCESS;
}

static int build_buttons_screen(cmp_ui_node_t *main_box) {
  cmp_ui_node_t *btn1 = NULL, *btn2 = NULL;
  cmp_f2_button_create(&btn1, "Primary Button", NULL);
  cmp_f2_button_set_variant(btn1, CMP_F2_BUTTON_VARIANT_PRIMARY);
  btn1->layout->margin[2] = 16.0f;

  cmp_f2_button_create(&btn2, "Secondary Button", NULL);
  btn2->layout->margin[2] = 16.0f;

  cmp_ui_node_add_child(main_box, btn1);
  cmp_ui_node_add_child(main_box, btn2);
  return CMP_SUCCESS;
}

static int build_inputs_screen(cmp_ui_node_t *main_box) {
  cmp_ui_node_t *cb = NULL, *tg = NULL;
  cmp_f2_checkbox_create(&cb, "Fluent 2 Checkbox");
  cb->layout->margin[2] = 16.0f;

  cmp_f2_toggle_create(&tg, "Fluent 2 Toggle");
  tg->layout->margin[2] = 16.0f;

  cmp_ui_node_add_child(main_box, cb);
  cmp_ui_node_add_child(main_box, tg);
  return CMP_SUCCESS;
}

static int build_text_inputs_screen(cmp_ui_node_t *main_box) {
  cmp_ui_node_t *txt_in = NULL;
  cmp_f2_text_input_create(&txt_in);
  txt_in->layout->margin[2] = 16.0f;
  txt_in->layout->width = 200.0f;
  txt_in->layout->height = 32.0f;

  cmp_ui_node_add_child(main_box, txt_in);
  return CMP_SUCCESS;
}

static int build_data_display_screen(cmp_ui_node_t *main_box) {
  cmp_ui_node_t *card = NULL;
  cmp_f2_card_create(&card);
  card->layout->width = 300.0f;
  card->layout->height = 200.0f;
  card->layout->margin[2] = 16.0f;
  cmp_ui_node_add_child(main_box, card);
  return CMP_SUCCESS;
}

static int build_ui(void) {
  cmp_ui_node_t *main_box = NULL;

  if (g_ui_tree != NULL) {
    cmp_ui_node_destroy(g_ui_tree);
    g_ui_tree = NULL;
  }

  cmp_ui_box_create(&g_ui_tree);
  g_ui_tree->layout->direction = CMP_FLEX_COLUMN;
  g_ui_tree->layout->padding[0] = 24.0f;
  g_ui_tree->layout->padding[1] = 24.0f;
  g_ui_tree->layout->padding[2] = 24.0f;
  g_ui_tree->layout->padding[3] = 24.0f;
  g_ui_tree->layout->width = 800.0f;
  g_ui_tree->layout->height = 600.0f;

  cmp_ui_box_create(&main_box);
  main_box->layout->direction = CMP_FLEX_COLUMN;
  main_box->layout->align_items = CMP_FLEX_ALIGN_START;

  build_header(main_box, "Fluent 2 Catalog");

  switch (g_current_screen) {
  case SCREEN_HOME:
    build_home_screen(main_box);
    break;
  case SCREEN_BUTTONS:
    build_buttons_screen(main_box);
    break;
  case SCREEN_INPUTS:
    build_inputs_screen(main_box);
    break;
  case SCREEN_TEXT_INPUTS:
    build_text_inputs_screen(main_box);
    break;
  case SCREEN_DATA_DISPLAY:
    build_data_display_screen(main_box);
    break;
  }

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

  build_ui();
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
        int changed = 0;

        if (hit_node == 101) {
          g_is_dark = !g_is_dark;
          changed = 1;
        } else if (hit_node == 100) {
          g_current_screen = SCREEN_HOME;
          changed = 1;
        } else if (hit_node == 201) {
          g_current_screen = SCREEN_BUTTONS;
          changed = 1;
        } else if (hit_node == 202) {
          g_current_screen = SCREEN_INPUTS;
          changed = 1;
        } else if (hit_node == 203) {
          g_current_screen = SCREEN_TEXT_INPUTS;
          changed = 1;
        } else if (hit_node == 204) {
          g_current_screen = SCREEN_DATA_DISPLAY;
          changed = 1;
        }

        if (changed) {
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
