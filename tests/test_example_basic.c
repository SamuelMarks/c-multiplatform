/**
 * @file test_example_basic.c
 * @brief Unit test and OOM stress testing for basic example.
 */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CI_TEST_RUN
#define CI_TEST_RUN 1
#endif

#define OMIT_MAIN 1
#include "../examples/basic/main.c"
#undef OMIT_MAIN

extern int g_malloc_fail_countdown;
extern int g_malloc_called;
extern int g_mock_gles2_destroy_fail;
extern int g_mock_gles2_flush_fail;
extern int g_mock_lock_contention;
extern int g_mock_strcpy_fail;
extern int g_mock_thread_fail;
extern int g_ui_timer_clock_gettime_fail;
extern int g_mock_cg_fail;
extern int g_mock_cf_fail;
extern int g_mock_cf_string_create_fail;
extern int g_mock_dlopen_fail;

/**
 * @brief Tests that basic example displays a horizontal row at wide width and a vertical column at narrow width.
 * @return 0 on success, non-zero on failure.
 */
static int test_basic_layout_row_to_column(void) {
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *box1 = NULL;
  struct ui_dom_node *box2 = NULL;
  struct ui_dom_node *box3 = NULL;
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_layout_node *layout = NULL;
  struct ui_layout_node *l_b1 = NULL;
  struct ui_layout_node *l_b2 = NULL;
  struct ui_layout_node *l_b3 = NULL;
  ui_error_t rc;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_dom_node_set_tag_name(root, "body");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 2;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &box1);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 3;
  }
  rc = ui_dom_node_set_attribute(box1, "class", "box");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box1);
    ui_dom_node_destroy(root);
    return 4;
  }
  rc = ui_dom_node_append_child(root, box1);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box1);
    ui_dom_node_destroy(root);
    return 5;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &box2);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 6;
  }
  rc = ui_dom_node_set_attribute(box2, "class", "box");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box2);
    ui_dom_node_destroy(root);
    return 7;
  }
  rc = ui_dom_node_append_child(root, box2);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box2);
    ui_dom_node_destroy(root);
    return 8;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &box3);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 9;
  }
  rc = ui_dom_node_set_attribute(box3, "class", "box");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box3);
    ui_dom_node_destroy(root);
    return 10;
  }
  rc = ui_dom_node_append_child(root, box3);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box3);
    ui_dom_node_destroy(root);
    return 11;
  }

  rc = ui_css_parse_stylesheet(BASIC_CSS, &sheet);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 12;
  }

  /* Wide window (800x600): should be a single row */
  rc = ui_layout_tree_generate(root, sheet, &layout);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
    return 13;
  }
  rc = ui_layout_solve_viewport(layout, 800.0f, 600.0f);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(layout);
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
    return 14;
  }

  l_b1 = layout->first_child;
  if (!l_b1 || !l_b1->next_sibling || !l_b1->next_sibling->next_sibling) {
    ui_layout_tree_destroy(layout);
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
    return 15;
  }
  l_b2 = l_b1->next_sibling;
  l_b3 = l_b2->next_sibling;

  /* Check that boxes form a horizontal row: X coordinates increase, Y coordinates match */
  if (!(l_b1->x < l_b2->x && l_b2->x < l_b3->x)) {
    printf("Expected row layout at 800px width: x1=%f, x2=%f, x3=%f\n",
           l_b1->x, l_b2->x, l_b3->x);
    ui_layout_tree_destroy(layout);
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
    return 16;
  }
  if (l_b1->y != l_b2->y || l_b2->y != l_b3->y) {
    printf("Expected identical Y for row at 800px width: y1=%f, y2=%f, y3=%f\n",
           l_b1->y, l_b2->y, l_b3->y);
    ui_layout_tree_destroy(layout);
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
    return 17;
  }

  ui_layout_tree_destroy(layout);
  layout = NULL;

  /* Narrow window (200x600): should wrap into a single column */
  rc = ui_layout_tree_generate(root, sheet, &layout);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
    return 18;
  }
  rc = ui_layout_solve_viewport(layout, 200.0f, 600.0f);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(layout);
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
    return 19;
  }

  l_b1 = layout->first_child;
  if (!l_b1 || !l_b1->next_sibling || !l_b1->next_sibling->next_sibling) {
    ui_layout_tree_destroy(layout);
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
    return 20;
  }
  l_b2 = l_b1->next_sibling;
  l_b3 = l_b2->next_sibling;

  /* Check that boxes form a vertical column: Y coordinates increase, X coordinates match */
  if (!(l_b1->y < l_b2->y && l_b2->y < l_b3->y)) {
    printf("Expected column layout at 200px width: y1=%f, y2=%f, y3=%f\n",
           l_b1->y, l_b2->y, l_b3->y);
    ui_layout_tree_destroy(layout);
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
    return 21;
  }
  if (l_b1->x != l_b2->x || l_b2->x != l_b3->x) {
    printf("Expected identical X for column at 200px width: x1=%f, x2=%f, x3=%f\n",
           l_b1->x, l_b2->x, l_b3->x);
    ui_layout_tree_destroy(layout);
    ui_css_stylesheet_destroy(sheet);
    ui_dom_node_destroy(root);
    return 22;
  }

  ui_layout_tree_destroy(layout);
  ui_css_stylesheet_destroy(sheet);
  ui_dom_node_destroy(root);
  return 0;
}

/**
 * @brief Tests dynamic resize callback and layout switching between row and column.
 * @return 0 on success, non-zero on failure.
 */
static int test_basic_dynamic_resize_render(void) {
  struct app_context app_ctx;
  struct render_context rctx;
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *box1 = NULL;
  struct ui_dom_node *box2 = NULL;
  struct ui_dom_node *box3 = NULL;
  struct ui_layout_node *l_b1 = NULL;
  struct ui_layout_node *l_b2 = NULL;
  struct ui_layout_node *l_b3 = NULL;
  int is_col;
  const char *css;
  ui_error_t rc;

  memset(&app_ctx, 0, sizeof(app_ctx));
  memset(&rctx, 0, sizeof(rctx));
  rctx.app_ctx = &app_ctx;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_dom_node_set_tag_name(root, "body");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 2;
  }
  app_ctx.root = root;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &box1);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 3;
  }
  rc = ui_dom_node_set_attribute(box1, "class", "box");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box1);
    ui_dom_node_destroy(root);
    return 4;
  }
  rc = ui_dom_node_append_child(root, box1);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box1);
    ui_dom_node_destroy(root);
    return 5;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &box2);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 6;
  }
  rc = ui_dom_node_set_attribute(box2, "class", "box");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box2);
    ui_dom_node_destroy(root);
    return 7;
  }
  rc = ui_dom_node_append_child(root, box2);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box2);
    ui_dom_node_destroy(root);
    return 8;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &box3);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 9;
  }
  rc = ui_dom_node_set_attribute(box3, "class", "box");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box3);
    ui_dom_node_destroy(root);
    return 10;
  }
  rc = ui_dom_node_append_child(root, box3);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(box3);
    ui_dom_node_destroy(root);
    return 11;
  }

  /* 1. Initial 800x600 landscape window */
  rc = on_resize_callback(&rctx, 800, 600);
  if (rc != UI_ERROR_NONE || app_ctx.window_width != 800.0f ||
      app_ctx.window_height != 600.0f || !app_ctx.needs_layout) {
    ui_dom_node_destroy(root);
    return 12;
  }

  is_col = (app_ctx.window_width < 600.0f ||
            app_ctx.window_width < app_ctx.window_height);
  if (is_col != 0) {
    ui_dom_node_destroy(root);
    return 13;
  }
  css = is_col ? COL_CSS : ROW_CSS;

  rc = ui_css_parse_stylesheet(css, &app_ctx.stylesheet);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 14;
  }
  rc = ui_layout_tree_generate(app_ctx.root, app_ctx.stylesheet,
                               &app_ctx.layout_tree);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 15;
  }
  rc = ui_layout_solve_viewport(app_ctx.layout_tree, app_ctx.window_width,
                                app_ctx.window_height);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 16;
  }

  l_b1 = app_ctx.layout_tree->first_child;
  l_b2 = l_b1 ? l_b1->next_sibling : NULL;
  l_b3 = l_b2 ? l_b2->next_sibling : NULL;
  if (!l_b1 || !l_b2 || !l_b3) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 17;
  }

  /* Must be a horizontal row */
  if (!(l_b1->x < l_b2->x && l_b2->x < l_b3->x) ||
      l_b1->y != l_b2->y || l_b2->y != l_b3->y) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 18;
  }

  /* 2. Resize to 400x600 (narrow window) -> turns into column */
  rc = on_resize_callback(&rctx, 400, 600);
  if (rc != UI_ERROR_NONE || app_ctx.window_width != 400.0f ||
      !app_ctx.needs_layout) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 19;
  }

  is_col = (app_ctx.window_width < 600.0f ||
            app_ctx.window_width < app_ctx.window_height);
  if (is_col != 1) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 20;
  }
  css = is_col ? COL_CSS : ROW_CSS;

  ui_layout_tree_destroy(app_ctx.layout_tree);
  app_ctx.layout_tree = NULL;
  ui_css_stylesheet_destroy(app_ctx.stylesheet);
  app_ctx.stylesheet = NULL;

  rc = ui_css_parse_stylesheet(css, &app_ctx.stylesheet);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 21;
  }
  rc = ui_layout_tree_generate(app_ctx.root, app_ctx.stylesheet,
                               &app_ctx.layout_tree);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 22;
  }
  rc = ui_layout_solve_viewport(app_ctx.layout_tree, app_ctx.window_width,
                                app_ctx.window_height);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 23;
  }

  l_b1 = app_ctx.layout_tree->first_child;
  l_b2 = l_b1 ? l_b1->next_sibling : NULL;
  l_b3 = l_b2 ? l_b2->next_sibling : NULL;
  if (!l_b1 || !l_b2 || !l_b3) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 24;
  }

  /* Must be a vertical column */
  if (!(l_b1->y < l_b2->y && l_b2->y < l_b3->y) ||
      l_b1->x != l_b2->x || l_b2->x != l_b3->x) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 25;
  }

  /* 3. Resize back to 900x600 (wide window) -> turns back into row */
  rc = on_resize_callback(&rctx, 900, 600);
  if (rc != UI_ERROR_NONE || app_ctx.window_width != 900.0f) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 26;
  }

  is_col = (app_ctx.window_width < 600.0f ||
            app_ctx.window_width < app_ctx.window_height);
  if (is_col != 0) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 27;
  }
  css = is_col ? COL_CSS : ROW_CSS;

  ui_layout_tree_destroy(app_ctx.layout_tree);
  app_ctx.layout_tree = NULL;
  ui_css_stylesheet_destroy(app_ctx.stylesheet);
  app_ctx.stylesheet = NULL;

  rc = ui_css_parse_stylesheet(css, &app_ctx.stylesheet);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root);
    return 28;
  }
  rc = ui_layout_tree_generate(app_ctx.root, app_ctx.stylesheet,
                               &app_ctx.layout_tree);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 29;
  }
  rc = ui_layout_solve_viewport(app_ctx.layout_tree, app_ctx.window_width,
                                app_ctx.window_height);
  if (rc != UI_ERROR_NONE) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 30;
  }

  l_b1 = app_ctx.layout_tree->first_child;
  l_b2 = l_b1 ? l_b1->next_sibling : NULL;
  l_b3 = l_b2 ? l_b2->next_sibling : NULL;

  if (!(l_b1->x < l_b2->x && l_b2->x < l_b3->x) ||
      l_b1->y != l_b2->y || l_b2->y != l_b3->y) {
    ui_layout_tree_destroy(app_ctx.layout_tree);
    ui_css_stylesheet_destroy(app_ctx.stylesheet);
    ui_dom_node_destroy(root);
    return 31;
  }

  ui_layout_tree_destroy(app_ctx.layout_tree);
  ui_css_stylesheet_destroy(app_ctx.stylesheet);
  ui_dom_node_destroy(root);
  return 0;
}

/**
 * @brief Test entry point for basic example execution and OOM simulation.
 * @return 0 on success, non-zero on failure.
 */
int main(void) {
    int i;
    int rc;

    rc = test_basic_layout_row_to_column();
    if (rc != 0) {
        return rc;
    }

    rc = test_basic_dynamic_resize_render();
    if (rc != 0) {
        return rc;
    }

    printf("Running OOM loop for basic...\n");
    for (i = 1; i < 5; i++) {
        g_malloc_called = 0;
        g_malloc_fail_countdown = i;
        rc = example_basic_main();
        if (rc != 0) {
            /* Expected failure on simulated memory exhaustion */
        }
        if (g_malloc_fail_countdown > 0) {
            break;
        }
    }
    g_malloc_fail_countdown = -1;
    /* Run once successfully */
    rc = example_basic_main();
    if (rc != 0) {
        return rc;
    }
    return 0;
}
/* clang-format on */
