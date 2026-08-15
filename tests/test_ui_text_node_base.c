/* clang-format off */
#include "ui_text_node_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
int g_mock_font_fail = 0;

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

static int test_oom(void) {
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_text_node_base *node;
  int i;
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    if (ui_text_node_base_create(&node) == UI_ERROR_NONE) {
      (void)ui_text_node_base_destroy(node);
    }
  }
  g_malloc_fail_countdown = -1;

  ui_text_node_base_create(&node);
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    ui_text_node_base_set_font_family(node, "Font");
  }
  g_malloc_fail_countdown = -1;
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    ui_text_node_base_set_text(node, "Hello");
  }
  g_malloc_fail_countdown = -1;

  ui_text_node_base_set_text(node, NULL);
  for (i = 0; i < 2; i++) {
    g_malloc_fail_countdown = i;
    ui_text_node_base_update_layout(node);
  }
  g_malloc_fail_countdown = -1;

  ui_text_node_base_set_text(node, "Hello World");
  ui_text_node_base_set_overflow(node, UI_TEXT_NODE_OVERFLOW_ELLIPSIS);
  ui_text_node_base_set_max_lines(node, 2);
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    ui_text_node_base_update_layout(node);
  }
  g_malloc_fail_countdown = -1;

  (void)ui_text_node_base_destroy(node);
#endif
  return failed;
}

int main(void) {
  struct ui_text_node_base *node = NULL;
  ui_error_t rc;
  struct ui_font_manager *font_mgr = NULL;
  struct ui_text_layout *layout = NULL;
  struct ui_component *comp;
  int failed = 0;

  rc = ui_font_manager_create(&font_mgr);
  if (rc != UI_ERROR_NONE)
    failed |= 1;

  if (ui_text_node_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;

  rc = ui_text_node_base_create(&node);
  if (rc != UI_ERROR_NONE || node == NULL)
    failed |= 1;

  /* Bind text test */
  if (ui_text_node_base_bind_text(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  if (ui_text_node_base_bind_text(node, (struct ui_signal *)1) != UI_ERROR_NONE)
    failed |= 1;

  if (ui_text_node_base_set_text(NULL, "Hello") != UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  if (ui_text_node_base_set_text(node, NULL) != UI_ERROR_NONE)
    failed |= 1;

  rc = ui_text_node_base_set_text(
      node, "Hello World Hello World Hello World Hello World");
  if (rc != UI_ERROR_NONE)
    failed |= 1;

  /* Setting again to free old text */
  ui_text_node_base_set_text(node,
                             "Hello World Hello World Hello World Hello World");

  {
    const char *tmp_text;
    if (ui_text_node_base_get_text(NULL, &tmp_text) !=
        UI_ERROR_INVALID_ARGUMENT)
      failed |= 1;
    if (ui_text_node_base_get_text(node, NULL) != UI_ERROR_INVALID_ARGUMENT)
      failed |= 1;

    rc = ui_text_node_base_get_text(node, &tmp_text);
    if (rc != UI_ERROR_NONE)
      failed |= 1;
  }

  if (ui_text_node_base_set_font_manager(NULL, font_mgr) !=
      UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  if (ui_text_node_base_set_font_manager(node, NULL) != UI_ERROR_NONE)
    failed |= 1;
  rc = ui_text_node_base_set_font_manager(node, font_mgr);
  if (rc != UI_ERROR_NONE)
    failed |= 1;

  if (ui_text_node_base_set_font_family(NULL, "Arial") !=
      UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  if (ui_text_node_base_set_font_family(node, NULL) != UI_ERROR_NONE)
    failed |= 1;
  rc = ui_text_node_base_set_font_family(node, "Arial");
  if (rc != UI_ERROR_NONE)
    failed |= 1;

  /* set again to free old family */
  ui_text_node_base_set_font_family(node, "Arial");

  if (ui_text_node_base_set_font_size(NULL, 24.0f) != UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  rc = ui_text_node_base_set_font_size(node, 24.0f);
  if (rc != UI_ERROR_NONE)
    failed |= 1;

  if (ui_text_node_base_set_max_width(NULL, 100.0f) !=
      UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  rc = ui_text_node_base_set_max_width(node, 100.0f);
  if (rc != UI_ERROR_NONE)
    failed |= 1;

  if (ui_text_node_base_set_max_lines(NULL, 2) != UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  rc = ui_text_node_base_set_max_lines(
      node, 2); /* line height 10, max lines 2 -> height limit 20. But mock
                   bounds return height 50 -> overflow! */
  if (rc != UI_ERROR_NONE)
    failed |= 1;

  if (ui_text_node_base_set_overflow(NULL, UI_TEXT_NODE_OVERFLOW_ELLIPSIS) !=
      UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  rc = ui_text_node_base_set_overflow(node, UI_TEXT_NODE_OVERFLOW_ELLIPSIS);
  if (rc != UI_ERROR_NONE)
    failed |= 1;

  if (ui_text_node_base_update_layout(NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;

  g_mock_font_fail = 0;
  rc = ui_text_node_base_update_layout(node); /* hits ellipsis */

#ifdef UI_TEST_MOCK_ALLOC
  /* trigger malloc failure during ellipsis string creation */
  g_malloc_fail_countdown = 0;
  ui_text_node_base_update_layout(node);
  g_malloc_fail_countdown = -1;
#endif

  g_mock_font_fail = 4; /* shape fails during ellipsis */
  ui_text_node_base_update_layout(node);

  g_mock_font_fail = 5; /* get_bounds fails during ellipsis */
  ui_text_node_base_update_layout(node);

  g_mock_font_fail = 6; /* vmetrics fails */
  ui_text_node_base_update_layout(node);

  g_mock_font_fail = 3; /* get_bounds fails immediately */
  ui_text_node_base_update_layout(node);

  ui_text_node_base_set_overflow(node, UI_TEXT_NODE_OVERFLOW_CLIP);
  g_mock_font_fail = 0;
  rc = ui_text_node_base_update_layout(node); /* hits hard clip */

  ui_text_node_base_set_overflow(node, UI_TEXT_NODE_OVERFLOW_ELLIPSIS);

  /* Triggers target_len <= 3 */
  ui_text_node_base_set_text(node, "A");
  ui_text_node_base_set_max_lines(node, 1);
  rc = ui_text_node_base_update_layout(node);

  /* Triggers target_len >= len by hacking mock bounds */
  ui_text_node_base_set_text(node, "Hello World Hello World Hello World Hello "
                                   "World Hello World Hello World");
  ui_text_node_base_set_max_lines(node, 4);
  rc = ui_text_node_base_update_layout(node);

  /* Target hard clip in clip mode */
  ui_text_node_base_set_overflow(node, UI_TEXT_NODE_OVERFLOW_CLIP);
  ui_text_node_base_set_text(node,
                             "Hello World Hello World Hello World Hello World "
                             "Hello World Hello World Hello World Hello World");
  ui_text_node_base_set_max_lines(node, 1);
  rc = ui_text_node_base_update_layout(node);

  ui_text_node_base_set_max_lines(node, 0); /* skip truncation block */
  rc = ui_text_node_base_update_layout(node);

  ui_text_node_base_set_max_lines(
      node, 20); /* large enough so height < max allowed */
  rc = ui_text_node_base_update_layout(node);

  g_mock_font_fail = 2; /* shape fails */
  rc = ui_text_node_base_update_layout(node);

  g_mock_font_fail = 1; /* font not found */
  rc = ui_text_node_base_update_layout(node);

  ui_text_node_base_set_font_family(node, NULL);
  rc = ui_text_node_base_update_layout(node); /* fallback to system-ui */

  if (ui_text_node_base_get_layout(NULL, &layout) != UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  if (ui_text_node_base_get_layout(node, NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  rc = ui_text_node_base_get_layout(node, &layout);

  if (ui_text_node_base_get_component(NULL, &comp) != UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  if (ui_text_node_base_get_component(node, NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed |= 1;
  rc = ui_text_node_base_get_component(node, &comp);
  if (rc != UI_ERROR_NONE || comp == NULL)
    failed |= 1;

  (void)ui_text_node_base_destroy(node);

  ui_text_node_base_create(&node);
  if (node) {
    struct ui_component *comp = NULL;
    ui_text_node_base_get_component(node, &comp);
    ui_text_node_base_set_font_manager(node, font_mgr);
    ui_text_node_base_set_text(node, "A component update");
    ui_text_node_base_set_font_family(node, "Arial");
    g_mock_font_fail = 0;
    ui_text_node_base_update_layout(node);
    ui_text_node_base_set_text(node, NULL);
    ui_text_node_base_update_layout(node);

    /* Test missing component branches */
    ui_dom_node_destroy(node->component->shadow_root);
    node->component->shadow_root = NULL;
    ui_text_node_base_set_text(node, "Shadow root missing");
    ui_text_node_base_update_layout(node); /* hits 348 */
    ui_text_node_base_set_text(node, NULL);
    ui_text_node_base_update_layout(node); /* hits 279 */

    ui_component_destroy(node->component);
    node->component = NULL;
    ui_text_node_base_set_text(node, "Component missing");
    ui_text_node_base_update_layout(node); /* hits 348 */
    ui_text_node_base_set_text(node, NULL);
    ui_text_node_base_update_layout(node); /* hits 279 */

    ui_text_node_base_set_font_family(node, NULL);
    ui_text_layout_destroy(node->layout);
    node->layout = NULL;

    ui_text_node_base_destroy(node); /* hits 160, 154, etc. */
  }

  /* Test missing shadow root branch in destroy */
  ui_text_node_base_create(&node);
  if (node) {
    ui_dom_node_destroy(node->component->shadow_root);
    node->component->shadow_root = NULL;
    ui_text_node_base_destroy(node);
  }

  (void)ui_font_manager_destroy(font_mgr);

  (void)ui_text_node_base_destroy(NULL);

  failed |= test_oom();

  printf("test_ui_text_node_base finished\n");
  return failed;
}
