/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

static int g_listener_called = 0;
static void *g_listener_user_data = NULL;

static void test_listener(cmp_databinding_t *binding, void *user_data) {
  (void)binding;
  g_listener_called++;
  g_listener_user_data = user_data;
}

TEST test_databinding_lifecycle(void) {
  cmp_databinding_t *binding = NULL;
  int res;

  res = cmp_databinding_create(&binding, CMP_DATA_TYPE_STRING);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, binding);

  res = cmp_databinding_destroy(binding);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_databinding_null_args(void) {
  cmp_databinding_t *binding = NULL;
  const char *str_val = NULL;
  int int_val = 0;
  cmp_ui_node_t *node = NULL;
  int res;

  res = cmp_databinding_create(NULL, CMP_DATA_TYPE_STRING);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_databinding_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_databinding_create(&binding, CMP_DATA_TYPE_INT);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_databinding_set_string(NULL, "test");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_databinding_get_string(NULL, &str_val);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_databinding_get_string(binding, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_databinding_set_int(NULL, 5);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_databinding_get_int(NULL, &int_val);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_databinding_get_int(binding, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_databinding_add_listener(NULL, test_listener, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_databinding_add_listener(binding, NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_text_create(&node, "", 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_node_bind_generic(NULL, binding, "text");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_node_bind_generic(node, NULL, "text");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_node_bind_generic(node, binding, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_node_destroy(node);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_databinding_destroy(binding);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_databinding_string(void) {
  cmp_databinding_t *binding = NULL;
  const char *val = NULL;
  int res;

  res = cmp_databinding_create(&binding, CMP_DATA_TYPE_STRING);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Set up listener */
  g_listener_called = 0;
  res = cmp_databinding_add_listener(binding, test_listener, (void *)0x1234);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Set string */
  res = cmp_databinding_set_string(binding, "Hello World");
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, g_listener_called);
  ASSERT_EQ((void *)0x1234, g_listener_user_data);

  /* Get string */
  res = cmp_databinding_get_string(binding, &val);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("Hello World", val);

  /* Clear string */
  res = cmp_databinding_set_string(binding, NULL);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(2, g_listener_called);

  res = cmp_databinding_get_string(binding, &val);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(NULL, val);

  /* Invalid type check */
  res = cmp_databinding_set_int(binding, 42);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_databinding_destroy(binding);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_databinding_int(void) {
  cmp_databinding_t *binding = NULL;
  int val = 0;
  int res;

  res = cmp_databinding_create(&binding, CMP_DATA_TYPE_INT);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Set up listener */
  g_listener_called = 0;
  res = cmp_databinding_add_listener(binding, test_listener, (void *)0x5678);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Set int */
  res = cmp_databinding_set_int(binding, 42);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, g_listener_called);
  ASSERT_EQ((void *)0x5678, g_listener_user_data);

  /* Get int */
  res = cmp_databinding_get_int(binding, &val);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(42, val);

  /* Invalid type check */
  res = cmp_databinding_set_string(binding, "Error");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_databinding_destroy(binding);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_databinding_node_bind(void) {
  cmp_databinding_t *binding = NULL;
  cmp_ui_node_t *node = NULL;
  int res;

  res = cmp_databinding_create(&binding, CMP_DATA_TYPE_STRING);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_text_create(&node, "", 0);
  ASSERT_EQ(CMP_SUCCESS, res);
  /* Type 2 is text node in generic ui implementation */
  node->type = 2;

  res = cmp_ui_node_bind_generic(node, binding, "text");
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Update binding, should propagate to node properties */
  res = cmp_databinding_set_string(binding, "Bound Text");
  ASSERT_EQ(CMP_SUCCESS, res);

  /* node->properties should be set to "Bound Text" */
  ASSERT_NEQ(NULL, node->properties);
  ASSERT_STR_EQ("Bound Text", (const char *)node->properties);

  /* Clear binding */
  res = cmp_databinding_set_string(binding, NULL);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(NULL, node->properties);

  /* Destroying node cleans up its generic internal state if set appropriately
   */
  res = cmp_ui_node_destroy(node);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_databinding_destroy(binding);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(databinding_suite) {
  RUN_TEST(test_databinding_lifecycle);
  RUN_TEST(test_databinding_null_args);
  RUN_TEST(test_databinding_string);
  RUN_TEST(test_databinding_int);
  RUN_TEST(test_databinding_node_bind);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(databinding_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
