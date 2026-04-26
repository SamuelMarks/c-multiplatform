/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_ime_lifecycle(void) {
  cmp_ime_t *ime = NULL;
  int res = cmp_ime_create(&ime);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, ime);

  res = cmp_ime_destroy(ime);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ime_null_args(void) {
  int res = cmp_ime_create(NULL);
  cmp_ime_t *ime = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ime_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ime_update_composition(NULL, "test");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_ime_create(&ime);
  res = cmp_ime_update_composition(ime, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_ime_destroy(ime);
  PASS();
}

TEST test_ime_update(void) {
  cmp_ime_t *ime = NULL;
  int res;
  char long_str[256];
  int i;

  for (i = 0; i < 255; i++) {
    long_str[i] = 'A';
  }
  long_str[255] = '\0';

  cmp_ime_create(&ime);

  res = cmp_ime_update_composition(ime, "test string");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ime_update_composition(ime, long_str);
  ASSERT_EQ(CMP_ERROR_BOUNDS, res);

  cmp_ime_destroy(ime);
  PASS();
}

SUITE(ime_suite) {
  RUN_TEST(test_ime_lifecycle);
  RUN_TEST(test_ime_null_args);
  RUN_TEST(test_ime_update);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ime_suite);
  GREATEST_MAIN_END();
}
