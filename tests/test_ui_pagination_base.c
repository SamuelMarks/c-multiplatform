/* clang-format off */
#include "../include/ui_pagination_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_pagination_basic(void) {
  struct ui_pagination_base *p = NULL;
  size_t total_pages, current, start, end;

  if (ui_pagination_base_create(&p) != UI_ERROR_NONE)
    return 1;

  if (ui_pagination_base_set_config(p, 55, 10) != UI_ERROR_NONE)
    return 1;

  if (ui_pagination_base_get_total_pages(p, &total_pages) != UI_ERROR_NONE)
    return 1;
  if (total_pages != 6)
    return 1;

  if (ui_pagination_base_get_current_page(p, &current) != UI_ERROR_NONE)
    return 1;
  if (current != 0)
    return 1;

  if (ui_pagination_base_get_bounds(p, &start, &end) != UI_ERROR_NONE)
    return 1;
  if (start != 0 || end != 10)
    return 1;

  ui_pagination_base_next(p);
  ui_pagination_base_get_current_page(p, &current);
  if (current != 1)
    return 1;

  ui_pagination_base_get_bounds(p, &start, &end);
  if (start != 10 || end != 20)
    return 1;

  ui_pagination_base_last(p);
  ui_pagination_base_get_current_page(p, &current);
  if (current != 5)
    return 1;

  ui_pagination_base_get_bounds(p, &start, &end);
  if (start != 50 || end != 55)
    return 1;

  /* Next should not go past the end */
  ui_pagination_base_next(p);
  ui_pagination_base_get_current_page(p, &current);
  if (current != 5)
    return 1;

  ui_pagination_base_first(p);
  ui_pagination_base_get_current_page(p, &current);
  if (current != 0)
    return 1;

  /* Previous should not go past the start */
  ui_pagination_base_previous(p);
  ui_pagination_base_get_current_page(p, &current);
  if (current != 0)
    return 1;

  ui_pagination_base_next(p);
  ui_pagination_base_previous(p);
  ui_pagination_base_get_current_page(p, &current);
  if (current != 0)
    return 1;

  ui_pagination_base_destroy(p);
  return 0;
}

static int test_pagination_edge_cases(void) {
  struct ui_pagination_base *p = NULL;
  size_t total_pages, current, start, end;

  if (ui_pagination_base_create(&p) != UI_ERROR_NONE)
    return 1;

  /* 0 items */
  if (ui_pagination_base_set_config(p, 0, 10) != UI_ERROR_NONE)
    return 1;
  ui_pagination_base_get_total_pages(p, &total_pages);
  if (total_pages != 0)
    return 1;
  ui_pagination_base_get_bounds(p, &start, &end);
  if (start != 0 || end != 0)
    return 1;
  ui_pagination_base_next(p);
  ui_pagination_base_get_current_page(p, &current);
  if (current != 0)
    return 1;

  ui_pagination_base_last(p);
  ui_pagination_base_get_current_page(p, &current);
  if (current != 0)
    return 1;

  ui_pagination_base_set_current_page(p, 10);
  ui_pagination_base_get_current_page(p, &current);
  if (current != 0)
    return 1;

  /* Page size > total items */
  if (ui_pagination_base_set_config(p, 5, 10) != UI_ERROR_NONE)
    return 1;
  ui_pagination_base_get_total_pages(p, &total_pages);
  if (total_pages != 1)
    return 1;
  ui_pagination_base_get_bounds(p, &start, &end);
  if (start != 0 || end != 5)
    return 1;
  ui_pagination_base_next(p);
  ui_pagination_base_get_current_page(p, &current);
  if (current != 0)
    return 1;

  /* Exact multiple */
  if (ui_pagination_base_set_config(p, 20, 10) != UI_ERROR_NONE)
    return 1;
  ui_pagination_base_get_total_pages(p, &total_pages);
  if (total_pages != 2)
    return 1;

  ui_pagination_base_set_current_page(p, 100); /* Out of bounds */
  ui_pagination_base_get_current_page(p, &current);
  if (current != 1)
    return 1; /* Clamped to last page */

  ui_pagination_base_set_current_page(p, 0);
  ui_pagination_base_get_current_page(p, &current);
  if (current != 0)
    return 1;

  /* test get_bounds start > total_items via manual tampering, actually we can't
     tamper it. But we can just rely on the existing tests. Wait, if
     current_page is somehow huge. Let's skip tampering for now, see if it hits.
   */

  /* Invalid argument */
  if (ui_pagination_base_set_config(p, 20, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_pagination_base_destroy(p);
  return 0;
}

static int test_pagination_nulls(void) {
  struct ui_pagination_base *p = NULL;
  size_t val1, val2;

  if (ui_pagination_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_pagination_base_destroy(NULL);

  if (ui_pagination_base_set_config(NULL, 10, 10) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pagination_base_get_total_pages(NULL, &val1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_pagination_base_create(&p) != UI_ERROR_NONE)
    return 1;
  if (ui_pagination_base_get_total_pages(p, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_pagination_base_set_current_page(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_pagination_base_get_current_page(NULL, &val1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pagination_base_get_current_page(p, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_pagination_base_next(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pagination_base_previous(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pagination_base_first(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pagination_base_last(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_pagination_base_get_bounds(NULL, &val1, &val2) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pagination_base_get_bounds(p, NULL, &val2) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pagination_base_get_bounds(p, &val1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_pagination_base_bind_current_page(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_pagination_base_bind_current_page(p, NULL) != UI_ERROR_NONE)
    return 1;

  ui_pagination_base_destroy(p);
  return 0;
}

static int test_pagination_oom(void) {
  struct ui_pagination_base *p = NULL;
  g_malloc_fail_countdown = 0;
  if (ui_pagination_base_create(&p) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;
  return 0;
}

static int test_pagination_config_shrink(void) {
  struct ui_pagination_base *p = NULL;
  if (ui_pagination_base_create(&p) != UI_ERROR_NONE)
    return 1;

  /* Set 10 pages */
  ui_pagination_base_set_config(p, 100, 10);
  ui_pagination_base_set_current_page(p, 5);

  /* Shrink to 2 pages */
  ui_pagination_base_set_config(p, 20, 10);

  /* current_page should be clamped to 1 (total_pages - 1) */
  size_t current;
  ui_pagination_base_get_current_page(p, &current);
  if (current != 1)
    return 1;

  ui_pagination_base_destroy(p);
  return 0;
}

int main(void) {
  int failed = 0;

  printf("Running ui_pagination_base tests...\n");

  failed |= test_pagination_basic();
  failed |= test_pagination_edge_cases();
  failed |= test_pagination_nulls();
  failed |= test_pagination_oom();
  failed |= test_pagination_config_shrink();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
