/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_list_metrics(void) {
  cmp_list_t *ctx = NULL;
  float margin, radius, offset_x;

  /* Plain */
  ASSERT_EQ(CMP_SUCCESS, cmp_list_create(&ctx, CMP_LIST_STYLE_PLAIN));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_list_resolve_metrics(ctx, &margin, &radius, &offset_x));
  ASSERT_EQ(0.0f, margin);
  ASSERT_EQ(0.0f, radius);
  ASSERT_EQ(0.0f, offset_x);

  /* Enter Edit Mode */
  ASSERT_EQ(CMP_SUCCESS, cmp_list_set_edit_mode(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_list_resolve_metrics(ctx, &margin, &radius, &offset_x));
  ASSERT_EQ(38.0f, offset_x); /* Shifted right for delete circles */

  ASSERT_EQ(CMP_SUCCESS, cmp_list_destroy(ctx));

  /* Inset Grouped */
  ASSERT_EQ(CMP_SUCCESS, cmp_list_create(&ctx, CMP_LIST_STYLE_INSET_GROUPED));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_list_resolve_metrics(ctx, &margin, &radius, &offset_x));
  ASSERT_EQ(16.0f, margin);
  ASSERT_EQ(10.0f, radius);

  ASSERT_EQ(CMP_SUCCESS, cmp_list_destroy(ctx));
  PASS();
}

TEST test_row_actions(void) {
  cmp_list_t *list = NULL;
  cmp_list_row_t *row1 = NULL;
  cmp_list_row_t *row2 = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_list_create(&list, CMP_LIST_STYLE_PLAIN));
  ASSERT_EQ(CMP_SUCCESS, cmp_list_row_create(&row1, "Item 1"));
  ASSERT_EQ(CMP_SUCCESS, cmp_list_row_create(&row2, "Item 2"));

  /* Configure Row 1: Custom Inset */
  ASSERT_EQ(CMP_SUCCESS, cmp_list_row_set_separator_inset(
                             row1, 56.0f)); /* Room for large icon */

  /* Configure Row 2: Swipe Actions */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_list_row_add_swipe_action(
                row2, 0, "Delete", CMP_SWIPE_ACTION_STYLE_DESTRUCTIVE, 1));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_list_row_add_swipe_action(row2, 0, "Flag",
                                          CMP_SWIPE_ACTION_STYLE_NORMAL, 0));

  ASSERT_EQ(CMP_SUCCESS, cmp_list_add_row(list, row1));
  ASSERT_EQ(CMP_SUCCESS, cmp_list_add_row(list, row2));

  /* Destroying the list will cascade delete the rows */
  ASSERT_EQ(CMP_SUCCESS, cmp_list_destroy(list));
  PASS();
}

TEST test_null_args(void) {
  cmp_list_t *list = NULL;
  cmp_list_row_t *row = NULL;
  float f;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_list_create(NULL, CMP_LIST_STYLE_PLAIN));
  cmp_list_create(&list, CMP_LIST_STYLE_PLAIN);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_list_row_create(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_list_row_create(&row, NULL));
  cmp_list_row_create(&row, "A");

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_list_add_row(NULL, row));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_list_add_row(list, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_list_row_set_separator_inset(NULL, 10.0f));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_list_row_add_swipe_action(
                NULL, 0, "Delete", CMP_SWIPE_ACTION_STYLE_DESTRUCTIVE, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_list_row_add_swipe_action(
                row, 0, NULL, CMP_SWIPE_ACTION_STYLE_DESTRUCTIVE, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_list_set_edit_mode(NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_list_resolve_metrics(NULL, &f, &f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_list_resolve_metrics(list, NULL, &f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_list_resolve_metrics(list, &f, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_list_resolve_metrics(list, &f, &f, NULL));

  cmp_list_add_row(list, row);
  cmp_list_destroy(list);
  PASS();
}

SUITE(lists_suite) {
  RUN_TEST(test_list_metrics);
  RUN_TEST(test_row_actions);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(lists_suite);
  GREATEST_MAIN_END();
}
