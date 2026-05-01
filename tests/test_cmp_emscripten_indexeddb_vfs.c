/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_emscripten_indexeddb_vfs_lifecycle(void) {
  cmp_indexeddb_vfs_t *vfs = NULL;
  int res;

  res = cmp_indexeddb_vfs_create(&vfs);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, vfs);

  res = cmp_indexeddb_vfs_destroy(vfs);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_emscripten_indexeddb_vfs_null_args(void) {
  cmp_indexeddb_vfs_t *vfs = NULL;
  int res;

  res = cmp_indexeddb_vfs_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_indexeddb_vfs_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_indexeddb_vfs_create(&vfs);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_indexeddb_vfs_mount(NULL, "/data", "mydb");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_indexeddb_vfs_mount(vfs, NULL, "mydb");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_indexeddb_vfs_mount(vfs, "/data", NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_indexeddb_vfs_sync(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_indexeddb_vfs_destroy(vfs);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_emscripten_indexeddb_vfs_operations(void) {
  cmp_indexeddb_vfs_t *vfs = NULL;
  int res;

  res = cmp_indexeddb_vfs_create(&vfs);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_indexeddb_vfs_mount(vfs, "/data", "app_db");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_indexeddb_vfs_sync(vfs);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_indexeddb_vfs_destroy(vfs);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(emscripten_indexeddb_vfs_suite) {
  RUN_TEST(test_emscripten_indexeddb_vfs_lifecycle);
  RUN_TEST(test_emscripten_indexeddb_vfs_null_args);
  RUN_TEST(test_emscripten_indexeddb_vfs_operations);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(emscripten_indexeddb_vfs_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
