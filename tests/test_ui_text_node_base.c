/* clang-format off */
#include "greatest.h"
#include "ui_text_node_base.h"
#include "ui_font_manager.h"
#include "ui_text_layout.h"
#include "ui_dom_node.h"
#include "ui_component.h"
#include "ui_error.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
int g_mock_font_fail = 0;
extern int g_text_node_mock_fail;

struct ui_text_node_base {
  struct ui_component *component;
  struct ui_text_layout *layout;
  struct ui_font_manager *font_manager;
  char *text;
  char *font_family;
  float font_size;
  float max_width;
  int max_lines;
  enum ui_text_node_overflow overflow;
  float computed_width;
  float computed_height;
  struct ui_signal *text_signal;
};

TEST test_text_node_invalid_args(void) {
  struct ui_text_node_base *node = NULL;
  const char *text_out = NULL;
  struct ui_text_layout *layout_out = NULL;
  struct ui_component *comp_out = NULL;
  ui_error_t rc;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_text_node_base_create(NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_text_node_base_destroy(NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_text_node_base_set_text(NULL, "txt"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_text_node_base_get_text(NULL, &text_out));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_text_node_base_set_font_manager(NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_text_node_base_set_font_family(NULL, "Arial"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_text_node_base_set_font_size(NULL, 14.0f));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_text_node_base_set_max_width(NULL, 200.0f));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_text_node_base_set_max_lines(NULL, 2));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_text_node_base_set_overflow(NULL, UI_TEXT_NODE_OVERFLOW_CLIP));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_text_node_base_update_layout(NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_text_node_base_get_layout(NULL, &layout_out));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_text_node_base_get_component(NULL, &comp_out));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_text_node_base_bind_text(NULL, NULL));

  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_text_node_base_get_text(node, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_text_node_base_get_layout(node, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_text_node_base_get_component(node, NULL));

  rc = ui_text_node_base_destroy(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_text_node_lifecycle_and_properties(void) {
  struct ui_text_node_base *node = NULL;
  struct ui_text_layout *layout = NULL;
  struct ui_component *comp = NULL;
  const char *text_out = NULL;
  ui_error_t rc;

  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_text_node_base_get_layout(node, &layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(layout != NULL);

  rc = ui_text_node_base_get_component(node, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);

  rc = ui_text_node_base_bind_text(node, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set and get text */
  rc = ui_text_node_base_set_text(node, "Hello World");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_get_text(node, &text_out);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Hello World", text_out);

  /* Overwrite text */
  rc = ui_text_node_base_set_text(node, "New Text");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set text to NULL (clears) */
  rc = ui_text_node_base_set_text(node, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_get_text(node, &text_out);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(text_out == NULL);

  /* Set font family, size, max_width, max_lines, overflow */
  rc = ui_text_node_base_set_font_family(node, "Roboto");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_set_font_family(node, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_set_font_family(node, "Helvetica");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_text_node_base_set_font_size(node, 24.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_set_max_width(node, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_set_max_lines(node, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_set_overflow(node, UI_TEXT_NODE_OVERFLOW_ELLIPSIS);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy with font_family non-NULL (covers line 304) */
  rc = ui_text_node_base_destroy(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy with shadow_root == NULL (covers line 306) */
  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(node->component->shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  node->component->shadow_root = NULL;
  rc = ui_text_node_base_destroy(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_text_node_layout_and_font_fallbacks(void) {
  struct ui_text_node_base *node = NULL;
  struct ui_font_manager *font_mgr = NULL;
  ui_error_t rc;

  rc = ui_font_manager_create(&font_mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Update layout with no text and no font_manager (early return, zeros bounds)
   */
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, node->computed_width);
  ASSERT_EQ(0.0f, node->computed_height);

  /* Attach font manager and set text */
  rc = ui_text_node_base_set_font_manager(node, font_mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_set_text(
      node, "Typography line clamp test string long enough");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Standard layout update */
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(node->computed_width > 0.0f);
  ASSERT(node->computed_height > 0.0f);

  /* Font fallback to system-ui (g_mock_font_fail = 1) */
  g_mock_font_fail = 1;
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_mock_font_fail = 0;

  /* Font fallback completely fails (g_mock_font_fail = 7) */
  g_mock_font_fail = 7;
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_mock_font_fail = 0;

  /* Line clamp / Ellipsis truncation */
  rc = ui_text_node_base_set_max_lines(node, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_set_overflow(node, UI_TEXT_NODE_OVERFLOW_ELLIPSIS);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Line clamp hard clip (overflow = CLIP) */
  rc = ui_text_node_base_set_overflow(node, UI_TEXT_NODE_OVERFLOW_CLIP);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Update layout failures: shape fail (fail = 2), get_bounds fail (fail = 3),
     vmetrics fail (fail = 6), ellipsis shape fail (fail = 4), ellipsis bounds
     fail (fail = 5) */
  g_mock_font_fail = 2;
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_font_fail = 0;

  g_mock_font_fail = 3;
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_font_fail = 0;

  g_mock_font_fail = 6;
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_font_fail = 0;

  rc = ui_text_node_base_set_overflow(node, UI_TEXT_NODE_OVERFLOW_ELLIPSIS);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_mock_font_fail = 4;
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_font_fail = 0;

  g_mock_font_fail = 5;
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_font_fail = 0;

  /* max_lines = 100 so computed_height <= max_allowed_height (line 484 false)
   */
  rc = ui_text_node_base_set_max_lines(node, 100);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* target_len <= 3 branch (line 489 false) */
  rc = ui_text_node_base_set_max_lines(node, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_set_text(node, "A");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Primary font non-null (line 444 true branch) */
  rc = ui_text_node_base_set_font_family(node, "CustomFont");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock append_child failure in update_dom_text (line 424) */
  {
    struct ui_text_node_base *n_app = NULL;
    rc = ui_text_node_base_create(&n_app);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_text_node_mock_fail = 6;
    rc = ui_text_node_base_update_layout(n_app);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_text_node_mock_fail = 0;
    rc = ui_text_node_base_destroy(n_app);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* update_layout when first_child is not a text node */
  {
    struct ui_text_node_base *n_elem = NULL;
    struct ui_dom_node *elem_child = NULL;
    rc = ui_text_node_base_create(&n_elem);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &elem_child);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_append_child(n_elem->component->shadow_root, elem_child);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_text_node_base_update_layout(n_elem);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_text_node_base_destroy(n_elem);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* update_layout when shadow_root is NULL and component is NULL (line 410) */
  rc = ui_dom_node_destroy(node->component->shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  node->component->shadow_root = NULL;
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(node->component);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  node->component = NULL;
  rc = ui_text_node_base_update_layout(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_text_node_base_destroy(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_font_manager_destroy(font_mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_text_node_error_branches(void) {
  struct ui_text_node_base *node = NULL;
  ui_error_t rc;

  /* 1. Layout create failure in create */
  g_text_node_mock_fail = 4;
  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_text_node_mock_fail = 0;

  /* 2. Set tag name failure in create */
  g_text_node_mock_fail = 5;
  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_text_node_mock_fail = 0;

  /* 3. Cleanup failures in create */
  /* component_destroy failure after dom_node_create fails */
  g_malloc_fail_countdown = 2; /* dom_node_create fails */
  g_text_node_mock_fail = 2;   /* component_destroy fails */
  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_malloc_fail_countdown = -1;
  g_text_node_mock_fail = 0;

  /* dom_node_destroy failure after set_tag_name fails */
  g_text_node_mock_fail =
      11; /* set_tag_name fails and dom_node_destroy fails (lines 277-279) */
  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_text_node_mock_fail = 0;

  /* component_destroy failure after set_tag_name fails */
  g_text_node_mock_fail =
      12; /* set_tag_name fails and component_destroy fails (lines 281-283) */
  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_text_node_mock_fail = 0;

  /* dom_node_destroy failure after layout_create fails (lines 292-294) */
  g_text_node_mock_fail = 9;
  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_text_node_mock_fail = 0;

  /* component_destroy failure after layout_create fails (lines 297-299) */
  g_text_node_mock_fail = 10;
  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_text_node_mock_fail = 0;

  /* 4. Destroy mock failures */
  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_text_node_mock_fail = 3; /* layout destroy fails */
  rc = ui_text_node_base_destroy(node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_text_node_mock_fail = 0;

  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_text_node_mock_fail = 1; /* shadow_root destroy fails */
  rc = ui_text_node_base_destroy(node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_text_node_mock_fail = 0;

  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_text_node_mock_fail = 2; /* component destroy fails */
  rc = ui_text_node_base_destroy(node);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_text_node_mock_fail = 0;

  /* Destroy with node->component == NULL, node->layout == NULL, shadow_root ==
   * NULL */
  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_layout_destroy(node->layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  node->layout = NULL;
  rc = ui_dom_node_destroy(node->component->shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  node->component->shadow_root = NULL;
  rc = ui_component_destroy(node->component);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  node->component = NULL;
  rc = ui_text_node_base_destroy(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_text_node_oom(void) {
  struct ui_text_node_base *node = NULL;
  struct ui_font_manager *font_mgr = NULL;
  ui_error_t rc;
  int i;

  rc = ui_font_manager_create(&font_mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Creation OOM loop */
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    node = NULL;
    rc = ui_text_node_base_create(&node);
    if (rc == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      rc = ui_text_node_base_destroy(node);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    ASSERT(node == NULL);
  }
  g_malloc_fail_countdown = -1;

  rc = ui_text_node_base_create(&node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* set_font_family OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_text_node_base_set_font_family(node, "Roboto");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* set_text OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_text_node_base_set_text(node, "Hello World");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* update_layout truncation OOM */
  rc = ui_text_node_base_set_font_manager(node, font_mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_set_text(node, "Very long text string to clamp");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_set_max_lines(node, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_text_node_base_set_overflow(node, UI_TEXT_NODE_OVERFLOW_ELLIPSIS);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    ui_text_node_base_update_layout(node);
  }
  g_malloc_fail_countdown = -1;

  rc = ui_text_node_base_destroy(node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_font_manager_destroy(font_mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(ui_text_node_base_suite) {
  RUN_TEST(test_text_node_invalid_args);
  RUN_TEST(test_text_node_lifecycle_and_properties);
  RUN_TEST(test_text_node_layout_and_font_fallbacks);
  RUN_TEST(test_text_node_error_branches);
  RUN_TEST(test_text_node_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_text_node_base_suite);
  GREATEST_MAIN_END();
}
