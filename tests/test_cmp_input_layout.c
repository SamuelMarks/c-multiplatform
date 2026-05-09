/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_input_layout_lifecycle(void) {
  cmp_input_layout_t *layout = NULL;
  size_t count = 0;
  float height = 0.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_input_layout_create(&layout));
  ASSERT_NEQ(NULL, layout);

  ASSERT_EQ(CMP_SUCCESS, cmp_input_layout_get_attachment_count(layout, &count));
  ASSERT_EQ(0, count);

  ASSERT_EQ(CMP_SUCCESS, cmp_input_layout_add_attachment(layout, "test.png"));
  ASSERT_EQ(CMP_SUCCESS, cmp_input_layout_get_attachment_count(layout, &count));
  ASSERT_EQ(1, count);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_input_layout_update_text(layout, "hello\nworld", &height));
  ASSERT_NEQ(0.0f, height);

  ASSERT_EQ(CMP_SUCCESS, cmp_input_layout_destroy(layout));
  PASS();
}

TEST test_input_layout_null_args(void) {
  cmp_input_layout_t *layout = NULL;
  size_t count = 0;
  float height = 0.0f;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_input_layout_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_input_layout_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_input_layout_create(&layout));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_input_layout_add_attachment(NULL, "test.png"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_input_layout_add_attachment(layout, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_input_layout_get_attachment_count(NULL, &count));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_input_layout_get_attachment_count(layout, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_input_layout_update_text(NULL, "test", &height));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_input_layout_update_text(layout, NULL, &height));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_input_layout_update_text(layout, "test", NULL));

  cmp_input_layout_destroy(layout);
  PASS();
}

TEST test_input_layout_bounds(void) {
  cmp_input_layout_t *layout = NULL;
  int i;
  ASSERT_EQ(CMP_SUCCESS, cmp_input_layout_create(&layout));

  for (i = 0; i < 16; i++) {
    ASSERT_EQ(CMP_SUCCESS, cmp_input_layout_add_attachment(layout, "test.png"));
  }
  ASSERT_EQ(CMP_ERROR_BOUNDS,
            cmp_input_layout_add_attachment(layout, "test.png"));

  cmp_input_layout_destroy(layout);
  PASS();
}

SUITE(input_layout_suite) {
  RUN_TEST(test_input_layout_lifecycle);
  RUN_TEST(test_input_layout_null_args);
  RUN_TEST(test_input_layout_bounds);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(input_layout_suite);
  GREATEST_MAIN_END();
}
