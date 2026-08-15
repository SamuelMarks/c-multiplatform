/* clang-format off */
#include "ui_virtual_scroll_base.h"
#include "ui_dom_node.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

extern int g_malloc_fail_countdown;

static float variable_size_getter(size_t index, void *user_data) {
  (void)user_data;
  /* Every even item is 20px, odd item is 30px */
  if (index % 2 == 0)
    return 20.0f;
  return 30.0f;
}

static ui_error_t create_mock_node(size_t index, struct ui_dom_node **out_node,
                                   void *user_data) {
  (void)index;
  (void)user_data;
  return ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, out_node);
}

static ui_error_t update_mock_node(size_t index, struct ui_dom_node *node,
                                   void *user_data) {
  char buf[32];
  (void)user_data;
#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "Item %lu", (unsigned long)index);
#else
  sprintf(buf, "Item %lu", (unsigned long)index);
#endif
  return ui_dom_node_set_attribute(node, "data-index", buf);
}

static int test_fixed_size_math(void) {
  struct ui_virtual_scroll_base *vs;
  struct ui_virtual_scroll_config config;
  size_t start, end;
  float offset_y;
  ui_error_t rc;
  int failed = 0;

  memset(&config, 0, sizeof(config));
  config.strategy = UI_VIRTUAL_SCROLL_FIXED_SIZE;
  config.orientation = UI_VIRTUAL_SCROLL_ORIENTATION_VERTICAL;
  config.fixed_item_size = 50.0f;
  config.create_node = create_mock_node;
  config.update_node = update_mock_node;

  rc = ui_virtual_scroll_base_create(&vs, &config);
  failed |= (rc != UI_ERROR_NONE);

  ui_virtual_scroll_base_set_item_count(vs, 100);
  ui_virtual_scroll_base_set_viewport_size(vs, 200.0f, 200.0f);

  {
    float height = 0.0f;
    ui_virtual_scroll_base_get_total_height(vs, &height);
    failed |= (height != 5000.0f);
  }

  rc = ui_virtual_scroll_base_get_visible_range(vs, 0.0f, &start, &end,
                                                &offset_y);
  failed |= (rc != UI_ERROR_NONE || start != 0 || end < 4 || offset_y != 0.0f);

  rc = ui_virtual_scroll_base_get_visible_range(vs, 120.0f, &start, &end,
                                                &offset_y);
  failed |= (rc != UI_ERROR_NONE || start != 2 || offset_y != 100.0f);

  rc = ui_virtual_scroll_base_get_visible_range(vs, -10.0f, &start, &end,
                                                &offset_y);
  failed |= (rc != UI_ERROR_NONE || start != 0 || offset_y != 0.0f);

  rc = ui_virtual_scroll_base_get_visible_range(vs, 10000.0f, &start, &end,
                                                &offset_y);
  failed |= (rc != UI_ERROR_NONE || start != 99 || end != 99);

  ui_virtual_scroll_base_set_item_count(vs, 0);
  rc = ui_virtual_scroll_base_get_visible_range(vs, 10.0f, &start, &end,
                                                &offset_y);
  failed |= (rc != UI_ERROR_NONE || start != 0 || end != 0);

  (void)ui_virtual_scroll_base_destroy(vs);
  if (failed)
    printf("test_fixed_size_math failed\n");
  return failed;
}

static int test_variable_size_math(void) {
  struct ui_virtual_scroll_base *vs;
  struct ui_virtual_scroll_config config;
  size_t start, end;
  float offset_y;
  ui_error_t rc;
  float total_expected;
  int failed = 0;

  memset(&config, 0, sizeof(config));
  config.strategy = UI_VIRTUAL_SCROLL_VARIABLE_SIZE;
  config.get_item_size = variable_size_getter;
  config.create_node = create_mock_node;
  config.update_node = update_mock_node;

  rc = ui_virtual_scroll_base_create(&vs, &config);
  failed |= (rc != UI_ERROR_NONE);

  ui_virtual_scroll_base_set_item_count(vs, 100);
  ui_virtual_scroll_base_set_viewport_size(vs, 200.0f, 200.0f);

  total_expected = 2500.0f;
  {
    float height = 0.0f;
    ui_virtual_scroll_base_get_total_height(vs, &height);
    failed |= (height != total_expected);
  }

  rc = ui_virtual_scroll_base_get_visible_range(vs, 80.0f, &start, &end,
                                                &offset_y);
  failed |= (rc != UI_ERROR_NONE || start != 3 || offset_y != 70.0f);

  rc = ui_virtual_scroll_base_get_visible_range(vs, 10000.0f, &start, &end,
                                                &offset_y);
  failed |= (rc != UI_ERROR_NONE || start != 99 || end != 99);

  rc = ui_virtual_scroll_base_get_visible_range(vs, -10.0f, &start, &end,
                                                &offset_y);
  failed |= (rc != UI_ERROR_NONE || start != 0 || end < 2);

  ui_virtual_scroll_base_set_item_count(vs, 0);
  rc = ui_virtual_scroll_base_get_visible_range(vs, 10.0f, &start, &end,
                                                &offset_y);
  failed |= (rc != UI_ERROR_NONE || start != 0 || end != 0);

  (void)ui_virtual_scroll_base_destroy(vs);
  if (failed)
    printf("test_variable_size_math failed\n");
  return failed;
}

static int test_dom_recycling(void) {
  struct ui_virtual_scroll_base *vs;
  struct ui_virtual_scroll_config config;
  struct ui_dom_node *container;
  ui_error_t rc;
  int failed = 0;

  memset(&config, 0, sizeof(config));
  config.strategy = UI_VIRTUAL_SCROLL_FIXED_SIZE;
  config.orientation = UI_VIRTUAL_SCROLL_ORIENTATION_VERTICAL;
  config.fixed_item_size = 50.0f;
  config.create_node = create_mock_node;
  config.update_node = update_mock_node;

  ui_virtual_scroll_base_create(&vs, &config);
  ui_virtual_scroll_base_set_item_count(vs, 1000);
  ui_virtual_scroll_base_set_viewport_size(vs, 200.0f, 200.0f);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ui_virtual_scroll_base_mount(vs, container);

  rc = ui_virtual_scroll_base_render(vs, 0.0f);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_virtual_scroll_base_render(vs, 1000.0f);
  failed |= (rc != UI_ERROR_NONE);

  ui_virtual_scroll_base_set_viewport_size(vs, 50.0f, 50.0f);
  rc = ui_virtual_scroll_base_render(vs, 1000.0f);
  failed |= (rc != UI_ERROR_NONE);

  ui_virtual_scroll_base_set_item_count(vs, 0);
  rc = ui_virtual_scroll_base_render(vs, 0.0f);
  failed |= (rc != UI_ERROR_NONE);

  (void)ui_virtual_scroll_base_destroy(vs);
  (void)ui_dom_node_destroy(container);
  if (failed)
    printf("test_dom_recycling failed\n");
  return failed;
}

static int test_dom_recycling_variable(void) {
  struct ui_virtual_scroll_base *vs;
  struct ui_virtual_scroll_config config;
  struct ui_dom_node *container;
  ui_error_t rc;
  int failed = 0;

  memset(&config, 0, sizeof(config));
  config.strategy = UI_VIRTUAL_SCROLL_VARIABLE_SIZE;
  config.orientation = UI_VIRTUAL_SCROLL_ORIENTATION_VERTICAL;
  config.get_item_size = variable_size_getter;
  config.create_node = create_mock_node;
  config.update_node = update_mock_node;

  ui_virtual_scroll_base_create(&vs, &config);
  ui_virtual_scroll_base_set_item_count(vs, 1000);
  ui_virtual_scroll_base_set_viewport_size(vs, 200.0f, 200.0f);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ui_virtual_scroll_base_mount(vs, container);

  rc = ui_virtual_scroll_base_render(vs, 0.0f);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_virtual_scroll_base_render(vs, 1000.0f);
  failed |= (rc != UI_ERROR_NONE);

  ui_virtual_scroll_base_set_viewport_size(vs, 50.0f, 50.0f);
  rc = ui_virtual_scroll_base_render(vs, 1000.0f);
  failed |= (rc != UI_ERROR_NONE);

  ui_virtual_scroll_base_set_item_count(vs, 0);
  rc = ui_virtual_scroll_base_render(vs, 0.0f);
  failed |= (rc != UI_ERROR_NONE);

  (void)ui_virtual_scroll_base_destroy(vs);
  (void)ui_dom_node_destroy(container);
  if (failed)
    printf("test_dom_recycling_variable failed\n");
  return failed;
}

#define CHECK_FAIL(cond)                                                       \
  do {                                                                         \
    if (cond) {                                                                \
      printf("test_error_handling failed at %d\n", __LINE__);                  \
      failed = 1;                                                              \
    }                                                                          \
  } while (0)

static int test_error_handling(void) {
  struct ui_virtual_scroll_base *vs;
  struct ui_virtual_scroll_config config;
  float th;
  size_t start, end;
  float offset;
  int failed = 0;
  struct ui_dom_node *container;

  memset(&config, 0, sizeof(config));
  CHECK_FAIL(ui_virtual_scroll_base_create(NULL, &config) !=
             UI_ERROR_INVALID_ARGUMENT);
  CHECK_FAIL(ui_virtual_scroll_base_create(&vs, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);

  config.strategy = UI_VIRTUAL_SCROLL_FIXED_SIZE;
  config.fixed_item_size = 0.0f;
  CHECK_FAIL(ui_virtual_scroll_base_create(&vs, &config) !=
             UI_ERROR_INVALID_ARGUMENT);

  config.strategy = UI_VIRTUAL_SCROLL_FIXED_SIZE;
  config.fixed_item_size = 50.0f;
  CHECK_FAIL(ui_virtual_scroll_base_create(&vs, &config) !=
             UI_ERROR_INVALID_ARGUMENT);

  config.create_node = create_mock_node;
  config.update_node = NULL;
  CHECK_FAIL(ui_virtual_scroll_base_create(&vs, &config) !=
             UI_ERROR_INVALID_ARGUMENT);

  config.create_node = create_mock_node;
  config.update_node = update_mock_node;
  config.strategy = UI_VIRTUAL_SCROLL_VARIABLE_SIZE;
  CHECK_FAIL(ui_virtual_scroll_base_create(&vs, &config) !=
             UI_ERROR_INVALID_ARGUMENT);

  config.strategy = UI_VIRTUAL_SCROLL_FIXED_SIZE;
  config.fixed_item_size = 50.0f;
  CHECK_FAIL(ui_virtual_scroll_base_create(&vs, &config) != UI_ERROR_NONE);

  (void)ui_virtual_scroll_base_destroy(NULL);

  CHECK_FAIL(ui_virtual_scroll_base_set_item_count(NULL, 10) !=
             UI_ERROR_INVALID_ARGUMENT);
  CHECK_FAIL(ui_virtual_scroll_base_set_viewport_size(NULL, 100.0f, 100.0f) !=
             UI_ERROR_INVALID_ARGUMENT);

  CHECK_FAIL(ui_virtual_scroll_base_set_viewport_size(vs, -10.0f, -20.0f) !=
             UI_ERROR_NONE);

  CHECK_FAIL(ui_virtual_scroll_base_get_total_height(NULL, &th) !=
             UI_ERROR_INVALID_ARGUMENT);
  CHECK_FAIL(ui_virtual_scroll_base_get_total_height(vs, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);

  CHECK_FAIL(ui_virtual_scroll_base_get_visible_range(NULL, 0.0f, &start, &end,
                                                      &offset) !=
             UI_ERROR_INVALID_ARGUMENT);
  CHECK_FAIL(ui_virtual_scroll_base_get_visible_range(
                 vs, 0.0f, NULL, &end, &offset) != UI_ERROR_INVALID_ARGUMENT);
  CHECK_FAIL(ui_virtual_scroll_base_get_visible_range(
                 vs, 0.0f, &start, NULL, &offset) != UI_ERROR_INVALID_ARGUMENT);
  CHECK_FAIL(ui_virtual_scroll_base_get_visible_range(
                 vs, 0.0f, &start, &end, NULL) != UI_ERROR_INVALID_ARGUMENT);

  CHECK_FAIL(ui_virtual_scroll_base_render(NULL, 0.0f) !=
             UI_ERROR_INVALID_ARGUMENT);
  CHECK_FAIL(ui_virtual_scroll_base_render(vs, 0.0f) !=
             UI_ERROR_INVALID_ARGUMENT); /* container is NULL since mount wasn't
                                            called */

  CHECK_FAIL(ui_virtual_scroll_base_bind_data(NULL, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);

  CHECK_FAIL(ui_virtual_scroll_base_bind_data(vs, NULL) != UI_ERROR_NONE);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  CHECK_FAIL(ui_virtual_scroll_base_mount(NULL, container) !=
             UI_ERROR_INVALID_ARGUMENT);
  CHECK_FAIL(ui_virtual_scroll_base_mount(vs, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  ui_dom_node_destroy(container);

  (void)ui_virtual_scroll_base_destroy(vs);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  CHECK_FAIL(ui_virtual_scroll_base_create(&vs, &config) !=
             UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  config.strategy = UI_VIRTUAL_SCROLL_VARIABLE_SIZE;
  config.get_item_size = variable_size_getter;
  ui_virtual_scroll_base_create(&vs, &config);

  ui_virtual_scroll_base_set_item_count(vs, 100);
  ui_virtual_scroll_base_set_viewport_size(vs, 200.0f, 200.0f);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ui_virtual_scroll_base_mount(vs, container);

  g_malloc_fail_countdown = 0;
  CHECK_FAIL(ui_virtual_scroll_base_render(vs, 0.0f) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  CHECK_FAIL(ui_virtual_scroll_base_render(vs, 0.0f) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  th = -1.0f;
  ui_virtual_scroll_base_set_item_count(vs, 0);

  /* Test when new_cache == NULL and count == 0 */
  g_malloc_fail_countdown = 0;
  ui_virtual_scroll_base_set_item_count(vs, 0);
  g_malloc_fail_countdown = -1;

  /* Setup a non-NULL cache */
  ui_virtual_scroll_base_set_item_count(vs, 1);

  /* Test when cached_prefix_heights_count <= item_count */
  g_malloc_fail_countdown = 0;
  ui_virtual_scroll_base_set_item_count(vs, 200);
  g_malloc_fail_countdown = -1;
  ui_virtual_scroll_base_get_total_height(vs, &th);

  /* Test when cached_prefix_heights == NULL */
  {
    struct ui_virtual_scroll_base *vs_empty = NULL;
    struct ui_virtual_scroll_config cfg2 = config;
    cfg2.strategy = UI_VIRTUAL_SCROLL_VARIABLE_SIZE;
    ui_virtual_scroll_base_create(&vs_empty, &cfg2);
    g_malloc_fail_countdown = 0;
    ui_virtual_scroll_base_set_item_count(vs_empty, 10);
    g_malloc_fail_countdown = -1;
    ui_virtual_scroll_base_get_total_height(vs_empty, &th);
    ui_virtual_scroll_base_destroy(vs_empty);
  }

  (void)ui_virtual_scroll_base_destroy(vs);
  (void)ui_dom_node_destroy(container);
#endif

  if (failed)
    printf("test_error_handling failed\n");
  return failed;
}

static int test_empty_scroll(void) {
  struct ui_virtual_scroll_base *vs;
  struct ui_virtual_scroll_config config;
  size_t start, end;
  float offset_y;
  int failed = 0;

  memset(&config, 0, sizeof(config));
  config.strategy = UI_VIRTUAL_SCROLL_FIXED_SIZE;
  config.fixed_item_size = 50.0f;
  config.create_node = create_mock_node;
  config.update_node = update_mock_node;

  ui_virtual_scroll_base_create(&vs, &config);
  ui_virtual_scroll_base_set_item_count(vs, 0);

  ui_virtual_scroll_base_get_visible_range(vs, 0.0f, &start, &end, &offset_y);
  failed |= (start != 0 || end != 0 || offset_y != 0.0f);

  ui_virtual_scroll_base_get_total_height(vs, &offset_y);

  (void)ui_virtual_scroll_base_destroy(vs);
  if (failed)
    printf("test_empty_scroll failed\n");
  return failed;
}

static int test_empty_scroll_variable(void) {
  struct ui_virtual_scroll_base *vs;
  struct ui_virtual_scroll_config config;
  size_t start, end;
  float offset_y;
  int failed = 0;

  memset(&config, 0, sizeof(config));
  config.strategy = UI_VIRTUAL_SCROLL_VARIABLE_SIZE;
  config.get_item_size = variable_size_getter;
  config.create_node = create_mock_node;
  config.update_node = update_mock_node;

  ui_virtual_scroll_base_create(&vs, &config);
  ui_virtual_scroll_base_set_item_count(vs, 0);

  ui_virtual_scroll_base_get_visible_range(vs, 0.0f, &start, &end, &offset_y);
  failed |= (start != 0 || end != 0 || offset_y != 0.0f);

  ui_virtual_scroll_base_get_total_height(vs, &offset_y);

  (void)ui_virtual_scroll_base_destroy(vs);
  if (failed)
    printf("test_empty_scroll_variable failed\n");
  return failed;
}

static ui_error_t create_mock_node_fail(size_t index,
                                        struct ui_dom_node **out_node,
                                        void *user_data) {
  (void)index;
  (void)user_data;
  return UI_ERROR_OUT_OF_MEMORY;
}

static int test_render_fail(void) {
  struct ui_virtual_scroll_base *vs;
  struct ui_virtual_scroll_config config;
  struct ui_dom_node *container;
  ui_error_t rc;
  int failed = 0;

  memset(&config, 0, sizeof(config));
  config.strategy = UI_VIRTUAL_SCROLL_FIXED_SIZE;
  config.orientation = UI_VIRTUAL_SCROLL_ORIENTATION_VERTICAL;
  config.fixed_item_size = 50.0f;
  config.create_node = create_mock_node_fail;
  config.update_node = update_mock_node;

  ui_virtual_scroll_base_create(&vs, &config);
  ui_virtual_scroll_base_set_item_count(vs, 1000);
  ui_virtual_scroll_base_set_viewport_size(vs, 200.0f, 200.0f);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ui_virtual_scroll_base_mount(vs, container);

  rc = ui_virtual_scroll_base_render(vs, 0.0f);
  failed |= (rc != UI_ERROR_OUT_OF_MEMORY);

  (void)ui_virtual_scroll_base_destroy(vs);
  (void)ui_dom_node_destroy(container);
  if (failed)
    printf("test_render_fail failed\n");
  return failed;
}

static ui_error_t update_mock_node_fail(size_t index, struct ui_dom_node *node,
                                        void *user_data) {
  (void)index;
  (void)node;
  (void)user_data;
  return UI_ERROR_OUT_OF_MEMORY;
}

static int test_render_update_fail(void) {
  struct ui_virtual_scroll_base *vs;
  struct ui_virtual_scroll_config config;
  struct ui_dom_node *container;
  ui_error_t rc;
  int failed = 0;

  memset(&config, 0, sizeof(config));
  config.strategy = UI_VIRTUAL_SCROLL_FIXED_SIZE;
  config.orientation = UI_VIRTUAL_SCROLL_ORIENTATION_VERTICAL;
  config.fixed_item_size = 50.0f;
  config.create_node = create_mock_node;
  config.update_node = update_mock_node_fail;

  ui_virtual_scroll_base_create(&vs, &config);
  ui_virtual_scroll_base_set_item_count(vs, 1000);
  ui_virtual_scroll_base_set_viewport_size(vs, 200.0f, 200.0f);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ui_virtual_scroll_base_mount(vs, container);

  rc = ui_virtual_scroll_base_render(vs, 0.0f);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    failed = 1;

  (void)ui_virtual_scroll_base_destroy(vs);
  (void)ui_dom_node_destroy(container);
  if (failed)
    printf("test_render_update_fail failed\n");
  return failed;
}

static int test_horizontal_orientation(void) {
  struct ui_virtual_scroll_base *vs;
  struct ui_virtual_scroll_config config;
  struct ui_dom_node *container;
  ui_error_t rc;
  int failed = 0;

  memset(&config, 0, sizeof(config));
  config.strategy = UI_VIRTUAL_SCROLL_FIXED_SIZE;
  config.orientation = UI_VIRTUAL_SCROLL_ORIENTATION_HORIZONTAL;
  config.fixed_item_size = 50.0f;
  config.create_node = create_mock_node;
  config.update_node = update_mock_node;

  ui_virtual_scroll_base_create(&vs, &config);
  ui_virtual_scroll_base_set_item_count(vs, 100);
  ui_virtual_scroll_base_set_viewport_size(vs, 200.0f, 200.0f);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &container);
  ui_virtual_scroll_base_mount(vs, container);

  rc = ui_virtual_scroll_base_render(vs, 0.0f);
  if (rc != UI_ERROR_NONE)
    failed = 1;

  (void)ui_virtual_scroll_base_destroy(vs);
  (void)ui_dom_node_destroy(container);
  if (failed)
    printf("test_horizontal_orientation failed\n");
  return failed;
}

int main(void) {
  int result = 0;
  printf("Running ui_virtual_scroll_base tests...\n");

  result |= test_fixed_size_math();
  result |= test_variable_size_math();
  result |= test_dom_recycling();
  result |= test_dom_recycling_variable();
  result |= test_error_handling();
  result |= test_empty_scroll();
  result |= test_empty_scroll_variable();
  result |= test_render_fail();
  result |= test_render_update_fail();
  result |= test_horizontal_orientation();

  if (result == 0) {
    printf("All virtual scroll base tests PASSED\n");
  }

  return result;
}
