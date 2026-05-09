/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_plugin_loader_suite);

TEST test_plugin_loader_lifecycle(void) {
  cmp_plugin_loader_t *loader = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_plugin_loader_create(&loader));
  ASSERT_NEQ(NULL, loader);

  ASSERT_EQ(CMP_SUCCESS, cmp_plugin_loader_destroy(loader));

  /* Null arguments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_plugin_loader_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_plugin_loader_destroy(NULL));
  PASS();
}

TEST test_plugin_loader_load_unload(void) {
  cmp_plugin_loader_t *loader = NULL;
  int plugin_id = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_plugin_loader_create(&loader));

  /* Null arguments for load */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_plugin_loader_load(NULL, "path", &plugin_id));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_plugin_loader_load(loader, NULL, &plugin_id));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_plugin_loader_load(loader, "path", NULL));

  /* Null arguments for unload */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_plugin_loader_unload(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_plugin_loader_unload(loader, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_plugin_loader_unload(loader, -1));

  /* Successful load */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_plugin_loader_load(loader, "dummy_plugin", &plugin_id));
  ASSERT_NEQ(0, plugin_id);

  /* Successful unload */
  ASSERT_EQ(CMP_SUCCESS, cmp_plugin_loader_unload(loader, plugin_id));

  ASSERT_EQ(CMP_SUCCESS, cmp_plugin_loader_destroy(loader));
  PASS();
}

TEST test_plugin_loader_execute(void) {
  cmp_plugin_loader_t *loader = NULL;
  int plugin_id = 0;
  char *response = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_plugin_loader_create(&loader));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_plugin_loader_load(loader, "dummy_plugin", &plugin_id));

  /* Null arguments for execute */
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_plugin_loader_execute(NULL, plugin_id, "func", "payload", &response));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_plugin_loader_execute(loader, 0, "func", "payload", &response));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_plugin_loader_execute(loader, plugin_id, NULL, "payload", &response));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_plugin_loader_execute(loader, plugin_id, "func", "payload", NULL));

  /* Payload can be NULL depending on the API but typically it's tested if valid
   */

  /* Successful execute */
  ASSERT_EQ(CMP_SUCCESS, cmp_plugin_loader_execute(loader, plugin_id, "func",
                                                   "payload", &response));
  ASSERT_NEQ(NULL, response);

  /* Null arguments for free_response */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_plugin_loader_free_response(NULL));

  /* Successful free */
  ASSERT_EQ(CMP_SUCCESS, cmp_plugin_loader_free_response(response));

  ASSERT_EQ(CMP_SUCCESS, cmp_plugin_loader_unload(loader, plugin_id));
  ASSERT_EQ(CMP_SUCCESS, cmp_plugin_loader_destroy(loader));
  PASS();
}

SUITE(cmp_plugin_loader_suite) {
  RUN_TEST(test_plugin_loader_lifecycle);
  RUN_TEST(test_plugin_loader_load_unload);
  RUN_TEST(test_plugin_loader_execute);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_plugin_loader_suite);
  GREATEST_MAIN_END();
}
