/* clang-format off */
#include "cmp.h"
#include "cmp_sse_parser.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

SUITE(cmp_sse_parser_suite);

static void test_sse_cb(const char *event_name, const char *data,
                        void *user_data) {
  int *calls = (int *)user_data;
  if (calls) {
    (*calls)++;
  }
  (void)event_name;
  (void)data;
}

TEST test_sse_parser_lifecycle(void) {
  cmp_sse_parser_t *parser = NULL;
  int calls = 0;

  /* Null arguments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_sse_parser_create(NULL, test_sse_cb, &calls));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_sse_parser_create(&parser, test_sse_cb, &calls));
  ASSERT_NEQ(NULL, parser);

  /* Null argument for destroy */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_sse_parser_destroy(NULL));

  /* Successful destroy */
  ASSERT_EQ(CMP_SUCCESS, cmp_sse_parser_destroy(parser));

  PASS();
}

TEST test_sse_parser_feed(void) {
  cmp_sse_parser_t *parser = NULL;
  int calls = 0;
  const char *chunk1 = "data: hello\n";
  const char *chunk2 = "data: world\n\n";

  ASSERT_EQ(CMP_SUCCESS, cmp_sse_parser_create(&parser, test_sse_cb, &calls));

  /* Null arguments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_sse_parser_feed(NULL, chunk1, (unsigned int)strlen(chunk1)));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_sse_parser_feed(parser, NULL, (unsigned int)strlen(chunk1)));

  /* Feed partial */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_sse_parser_feed(parser, chunk1, (unsigned int)strlen(chunk1)));
  ASSERT_EQ(0, calls);

  /* Feed rest to trigger callback */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_sse_parser_feed(parser, chunk2, (unsigned int)strlen(chunk2)));
  ASSERT_EQ(1, calls);

  ASSERT_EQ(CMP_SUCCESS, cmp_sse_parser_destroy(parser));

  PASS();
}

SUITE(cmp_sse_parser_suite) {
  RUN_TEST(test_sse_parser_lifecycle);
  RUN_TEST(test_sse_parser_feed);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_sse_parser_suite);
  GREATEST_MAIN_END();
}