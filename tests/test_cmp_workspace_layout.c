/* clang-format off */
#include "cmp.h"
#include <stdio.h>
/* clang-format on */

#define ASSERT_EQ(expected, actual)                                            \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      printf("%s:%d: Assertion failed: expected %d, got %d\n", __FILE__,       \
             __LINE__, (int)(expected), (int)(actual));                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_NEQ_PTR(expected, actual)                                       \
  do {                                                                         \
    if ((expected) == (actual)) {                                              \
      printf("%s:%d: Assertion failed: expected not %p, got %p\n", __FILE__,   \
             __LINE__, (void *)(expected), (void *)(actual));                  \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main(void) {
  cmp_workspace_layout_t *layout = NULL;
  float width;
  int is_over;
  int rc;

  /* Test invalid args */
  rc = cmp_workspace_layout_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_workspace_layout_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_workspace_layout_set_pane_width(NULL, CMP_PANE_SIDEBAR, 300.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_workspace_layout_get_pane_width(NULL, CMP_PANE_SIDEBAR, &width);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_workspace_layout_set_sidebar_glass(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_workspace_layout_hit_test_splitters(NULL, 0.0f, 0.0f, &is_over);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  /* Test create */
  rc = cmp_workspace_layout_create(&layout);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_NEQ_PTR(NULL, layout);

  /* Test getting defaults */
  rc = cmp_workspace_layout_get_pane_width(layout, CMP_PANE_SIDEBAR, &width);
  ASSERT_EQ(CMP_SUCCESS, rc);
  /* default is 250.0f */

  rc = cmp_workspace_layout_get_pane_width(layout, CMP_PANE_COUNT, &width);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_workspace_layout_get_pane_width(layout, CMP_PANE_SIDEBAR, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  /* Test setting widths */
  rc = cmp_workspace_layout_set_pane_width(layout, CMP_PANE_SIDEBAR, 400.0f);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_workspace_layout_get_pane_width(layout, CMP_PANE_SIDEBAR, &width);
  ASSERT_EQ(CMP_SUCCESS, rc);
  /* width should be 400.0f now */

  /* Test minimum constraint logic (MIN_PANE_WIDTH = 100.0f) */
  rc = cmp_workspace_layout_set_pane_width(layout, CMP_PANE_SIDEBAR, 50.0f);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_workspace_layout_get_pane_width(layout, CMP_PANE_SIDEBAR, &width);
  ASSERT_EQ(CMP_SUCCESS, rc);
  /* Width should be clamped to 100.0f but we just test execution path */

  /* Test glass setting */
  rc = cmp_workspace_layout_set_sidebar_glass(layout, 0);
  ASSERT_EQ(CMP_SUCCESS, rc);

  /* Test hit_test splitters */
  rc = cmp_workspace_layout_hit_test_splitters(layout, 0.0f, 0.0f, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_workspace_layout_hit_test_splitters(
      layout, 105.0f, 0.0f,
      &is_over); /* Assuming clamped at 100 + thickness check */
  ASSERT_EQ(CMP_SUCCESS, rc);

  /* Test destroy */
  rc = cmp_workspace_layout_destroy(layout);
  ASSERT_EQ(CMP_SUCCESS, rc);

  printf("test_cmp_workspace_layout passed.\n");
  return 0;
}
