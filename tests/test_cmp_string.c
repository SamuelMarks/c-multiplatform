/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_string_init(void) {
  cmp_string_t str;
  int res = cmp_string_init(&str);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(str.data == NULL);
  ASSERT_EQ_FMT((size_t)0, str.length, "%zd");
  ASSERT_EQ_FMT((size_t)0, str.capacity, "%zd");
  PASS();
}

TEST test_string_append(void) {
  cmp_string_t str;
  int res;

  cmp_string_init(&str);

  res = cmp_string_append(&str, "Hello ");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(str.data != NULL);
  ASSERT_EQ_FMT((size_t)6, str.length, "%zd");
  ASSERT(str.capacity >= 7); /* 6 chars + null terminator */
  ASSERT_STR_EQ("Hello ", str.data);

  res = cmp_string_append(&str, "World!");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT((size_t)12, str.length, "%zd");
  ASSERT_STR_EQ("Hello World!", str.data);

  cmp_string_destroy(&str);
  PASS();
}

TEST test_string_append_realloc(void) {
  cmp_string_t str;
  int res;
  int i;
  char buf[16];

  cmp_string_init(&str);

  /* Force multiple reallocations */
  for (i = 0; i < 100; i++) {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%d,", i);
#else
    sprintf(buf, "%d,", i);
#endif
    res = cmp_string_append(&str, buf);
    ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  }

  ASSERT(str.length > 200);
  ASSERT(str.capacity > str.length);
  ASSERT(str.data != NULL);

  cmp_string_destroy(&str);
  PASS();
}

TEST test_string_destroy(void) {
  cmp_string_t str;
  int res;

  cmp_string_init(&str);
  cmp_string_append(&str, "Test");

  res = cmp_string_destroy(&str);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(str.data == NULL);
  ASSERT_EQ_FMT((size_t)0, str.length, "%zd");
  ASSERT_EQ_FMT((size_t)0, str.capacity, "%zd");

  PASS();
}

SUITE(string_suite) {
  RUN_TEST(test_string_init);
  RUN_TEST(test_string_append);
  RUN_TEST(test_string_append_realloc);
  RUN_TEST(test_string_destroy);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(string_suite);
  GREATEST_MAIN_END();
}
