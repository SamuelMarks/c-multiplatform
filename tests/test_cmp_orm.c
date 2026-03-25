/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdio.h>

/* clang-format on */

TEST test_orm_lifecycle(void) {
  int res;

  res = cmp_orm_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_orm_shutdown();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_orm_db_connection(void) {
  c_orm_db_t *db = NULL;
  int res;

  cmp_vfs_init();
  cmp_orm_init();

  cmp_vfs_mount("virt:/test_db", ".");

  res = cmp_orm_connect("virt:/test_db/test.sqlite", &db);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(db != NULL);

  res = cmp_orm_execute(
      db, "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT);");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_orm_execute(db, "INSERT INTO test (name) VALUES ('hello');");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_orm_disconnect(db);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  remove("test.sqlite");

  cmp_orm_shutdown();
  cmp_vfs_shutdown();
  PASS();
}

TEST test_orm_default_path(void) {
  c_orm_db_t *db = NULL;
  cmp_string_t exe_dir;
  cmp_string_t expected_path;
  int res;

  cmp_vfs_init();
  cmp_orm_init();

  /* A plain filename should resolve to executable directory */
  res = cmp_orm_connect("auto_default.sqlite", &db);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(db != NULL);

  res = cmp_orm_disconnect(db);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  /* Clean up the file created in the executable directory */
  res = cmp_vfs_get_standard_path(5, &exe_dir);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  cmp_string_init(&expected_path);
  cmp_string_append(&expected_path, exe_dir.data);
  cmp_string_append(&expected_path, "/auto_default.sqlite");

  remove(expected_path.data);

  cmp_string_destroy(&expected_path);
  cmp_string_destroy(&exe_dir);

  cmp_orm_shutdown();
  cmp_vfs_shutdown();
  PASS();
}

TEST test_orm_features(void) {
  c_orm_db_t *db = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_orm_observable_t *obs = NULL;
  int res;

  cmp_vfs_init();
  cmp_orm_init();

  cmp_vfs_mount("virt:/test_db", ".");
  cmp_vfs_mount("virt:/migrations", ".");

  res = cmp_orm_connect("virt:/test_db/test_features.sqlite", &db);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_orm_migrate(db, "virt:/migrations");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_orm_set_encryption_key(db, "my_secret_key");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_ui_text_input_create(&node);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_orm_observable_create(db, "SELECT name FROM test WHERE id = 1;",
                                  &obs);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_ui_node_bind(node, obs, "text");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_orm_observable_destroy(obs);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_ui_node_destroy(node);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_orm_disconnect(db);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  cmp_orm_shutdown();
  cmp_vfs_shutdown();
  PASS();
}

SUITE(orm_suite) {
  RUN_TEST(test_orm_lifecycle);
  RUN_TEST(test_orm_db_connection);
  RUN_TEST(test_orm_default_path);
  RUN_TEST(test_orm_features);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(orm_suite);
  GREATEST_MAIN_END();
}