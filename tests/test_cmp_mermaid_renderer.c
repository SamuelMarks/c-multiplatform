/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_mermaid_renderer_lifecycle(void) {
  cmp_mermaid_renderer_t *ctx = NULL;
  char *svg = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_mermaid_renderer_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_mermaid_renderer_generate_svg(ctx, "graph TD; A-->B;", &svg));

  ASSERT_NEQ(NULL, svg);
  ASSERT(strstr(svg, "<svg") != NULL);
  ASSERT(strstr(svg, "<text x=\"10\"") != NULL);
  ASSERT(strstr(svg, "graph TD; A-->B;") != NULL);

  ASSERT_EQ(CMP_SUCCESS, cmp_mermaid_renderer_free_svg(svg));
  ASSERT_EQ(CMP_SUCCESS, cmp_mermaid_renderer_destroy(ctx));
  PASS();
}

TEST test_mermaid_renderer_null_args(void) {
  cmp_mermaid_renderer_t *ctx = NULL;
  char *svg = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mermaid_renderer_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mermaid_renderer_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_mermaid_renderer_create(&ctx));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_mermaid_renderer_generate_svg(NULL, "graph", &svg));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_mermaid_renderer_generate_svg(ctx, NULL, &svg));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_mermaid_renderer_generate_svg(ctx, "graph", NULL));

  /* Test syntax validator */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_mermaid_renderer_generate_svg(ctx, "invalid format", &svg));

  ASSERT_EQ(CMP_SUCCESS, cmp_mermaid_renderer_destroy(ctx));

  /* free_svg accepts NULL or valid ptr, normally we return SUCCESS for NULL */
  ASSERT_EQ(CMP_SUCCESS, cmp_mermaid_renderer_free_svg(NULL));
  PASS();
}

SUITE(mermaid_renderer_suite) {
  RUN_TEST(test_mermaid_renderer_lifecycle);
  RUN_TEST(test_mermaid_renderer_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(mermaid_renderer_suite);
  GREATEST_MAIN_END();
}
