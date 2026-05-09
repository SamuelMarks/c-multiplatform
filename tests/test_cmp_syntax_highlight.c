/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

SUITE(cmp_syntax_highlight_suite);

TEST test_syntax_highlighter_create_destroy(void) {
  cmp_syntax_highlighter_t *hl = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_syntax_highlighter_create(&hl));
  ASSERT_NEQ(NULL, hl);
  ASSERT_EQ(CMP_SUCCESS, cmp_syntax_highlighter_destroy(hl));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_syntax_highlighter_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_syntax_highlighter_destroy(NULL));
  PASS();
}

TEST test_syntax_highlighter_parse(void) {
  cmp_syntax_highlighter_t *hl = NULL;
  cmp_highlight_span_t *spans = NULL;
  size_t count = 0;
  const char *code = "int main() { /* comment */ return 0; } /* line */\n";

  ASSERT_EQ(CMP_SUCCESS, cmp_syntax_highlighter_create(&hl));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_syntax_highlighter_parse(hl, code, "c", &spans, &count));
  ASSERT_NEQ(NULL, spans);
  ASSERT(count > 0);

  ASSERT_EQ(CMP_SUCCESS, cmp_syntax_highlighter_free_spans(spans));

  /* Test invalid args */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_syntax_highlighter_parse(NULL, code, "c", &spans, &count));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_syntax_highlighter_parse(hl, NULL, "c", &spans, &count));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_syntax_highlighter_parse(hl, code, "c", NULL, &count));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_syntax_highlighter_parse(hl, code, "c", &spans, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_syntax_highlighter_destroy(hl));
  PASS();
}

SUITE(cmp_syntax_highlight_suite) {
  RUN_TEST(test_syntax_highlighter_create_destroy);
  RUN_TEST(test_syntax_highlighter_parse);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_syntax_highlight_suite);
  GREATEST_MAIN_END();
}