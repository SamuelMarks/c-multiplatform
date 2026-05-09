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
  ASSERT_EQ_FMT((unsigned long)0, (unsigned long)(str.length), "%lu");
  ASSERT_EQ_FMT((unsigned long)0, (unsigned long)(str.capacity), "%lu");
  PASS();
}

TEST test_string_init_invalid(void) {
  int res = cmp_string_init(NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");
  PASS();
}

TEST test_string_append(void) {
  cmp_string_t str;
  int res;

  cmp_string_init(&str);

  res = cmp_string_append(&str, "Hello ");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(str.data != NULL);
  ASSERT_EQ_FMT((unsigned long)6, (unsigned long)(str.length), "%lu");
  ASSERT(str.capacity >= 7); /* 6 chars + null terminator */
  ASSERT_STR_EQ("Hello ", str.data);

  res = cmp_string_append(&str, "World!");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT((unsigned long)12, (unsigned long)(str.length), "%lu");
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

TEST test_string_append_invalid(void) {
  cmp_string_t str;
  int res;

  cmp_string_init(&str);

  res = cmp_string_append(NULL, "Test");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  res = cmp_string_append(&str, NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");

  /* Appending empty string should succeed but do nothing */
  res = cmp_string_append(&str, "");
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT_EQ_FMT((unsigned long)0, (unsigned long)(str.length), "%lu");

  cmp_string_destroy(&str);
  PASS();
}

TEST test_string_append_overflow(void) {
  cmp_string_t str;
  int res;

  cmp_string_init(&str);
  str.length = (size_t)-1; /* Max out length to trigger overflow */

  res = cmp_string_append(&str, "A");
  ASSERT_EQ_FMT(CMP_ERROR_BOUNDS, res, "%d");

  cmp_string_destroy(&str); /* Safe since data is NULL initially */
  PASS();
}

TEST test_string_append_oom_simulation(void) {
  cmp_string_t str;
  int res;

  cmp_string_init(&str);

  /* Setting capacity to SIZE_MAX/2 to simulate enormous malloc */
  str.capacity = ((size_t)-1) / 2;
  str.length = str.capacity - 1;
  /* Next append triggers doubling, attempting to allocate SIZE_MAX,
     which fails returning OOM or alloc failure */
  res = cmp_string_append(&str, "A");
  /* We expect some error here, either CMP_ERROR_BOUNDS or something from MALLOC
   */
  ASSERT(res != CMP_SUCCESS);

  str.length = 0;
  str.capacity = 0;
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
  ASSERT_EQ_FMT((unsigned long)0, (unsigned long)(str.length), "%lu");
  ASSERT_EQ_FMT((unsigned long)0, (unsigned long)(str.capacity), "%lu");

  PASS();
}

TEST test_string_destroy_invalid(void) {
  int res = cmp_string_destroy(NULL);
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, res, "%d");
  PASS();
}

TEST test_strtok_r_normal(void) {
  char buf[] = "apple,banana,cherry";
  char *saveptr = NULL;
  char *token = NULL;

  cmp_strtok_r(buf, ", ", &saveptr, &token);
  ASSERT_STR_EQ("apple", token);

  token = NULL;
  cmp_strtok_r(NULL, ", ", &saveptr, &token);
  ASSERT_STR_EQ("banana", token);

  token = NULL;
  cmp_strtok_r(NULL, ", ", &saveptr, &token);
  ASSERT_STR_EQ("cherry", token);

  token = NULL;
  cmp_strtok_r(NULL, ", ", &saveptr, &token);
  ASSERT(token == NULL);

  PASS();
}

TEST test_strtok_r_edge_cases(void) {
  char buf1[] = ",,"; /* Only delimiters */
  char *saveptr1 = NULL;
  char *token = NULL;

  cmp_strtok_r(buf1, ", ", &saveptr1, &token);
  ASSERT(token == NULL);

  PASS();
}

SUITE(string_suite) {
  RUN_TEST(test_string_init);
  RUN_TEST(test_string_init_invalid);
  RUN_TEST(test_string_append);
  RUN_TEST(test_string_append_realloc);
  RUN_TEST(test_string_append_invalid);
  RUN_TEST(test_string_append_overflow);
  RUN_TEST(test_string_append_oom_simulation);
  RUN_TEST(test_string_destroy);
  RUN_TEST(test_string_destroy_invalid);
  RUN_TEST(test_strtok_r_normal);
  RUN_TEST(test_strtok_r_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  /* For avoiding unused warnings */
  (void)argc;
  (void)argv;

  GREATEST_MAIN_BEGIN();
  RUN_SUITE(string_suite);
  GREATEST_MAIN_END();
}
