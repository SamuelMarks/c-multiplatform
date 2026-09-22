/* clang-format off */
#include "../include/ui_accordion_base.h"
#include "../include/ui_disclosure_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

static int g_change_count = 0;
static struct ui_disclosure_base *g_last_active = NULL;

static ui_error_t on_accordion_change(struct ui_accordion_base *accordion,
                                      struct ui_disclosure_base *active,
                                      void *user_data) {
  if (accordion || user_data) {
  }
  g_change_count++;
  g_last_active = active;
  return UI_ERROR_NONE;
}

static int test_accordion_lifecycle(void) {
  struct ui_accordion_base *accordion = NULL;
  struct ui_disclosure_base *d1 = NULL;
  struct ui_disclosure_base *d2 = NULL;
  struct ui_disclosure_base *d3 = NULL;
  ui_error_t rc;

  rc = ui_accordion_base_create(&accordion);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_accordion_base_set_on_change(accordion, on_accordion_change, NULL);

  ui_disclosure_base_create(&d1);
  ui_disclosure_base_create(&d2);
  ui_disclosure_base_create(&d3);

  rc = ui_accordion_base_add_disclosure(accordion, d1);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_accordion_base_add_disclosure(accordion, d2);
  ui_accordion_base_add_disclosure(accordion, d3);

  g_change_count = 0;

  /* Expand d1 */
  ui_accordion_base_set_active(accordion, d1);
  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(d1, &is_expanded);
    if (!is_expanded)
      return 1;
  }
  {
    struct ui_disclosure_base *tmp_active;
    if (ui_accordion_base_get_active(accordion, &tmp_active) != UI_ERROR_NONE ||
        tmp_active != d1)
      return 1;
  }
  if (g_change_count != 1 || g_last_active != d1)
    return 1;

  /* Expand d2, should collapse d1 */
  ui_accordion_base_set_active(accordion, d2);
  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(d1, &is_expanded);
    if (is_expanded)
      return 1;
  }
  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(d2, &is_expanded);
    if (!is_expanded)
      return 1;
  }
  {
    struct ui_disclosure_base *tmp_active;
    if (ui_accordion_base_get_active(accordion, &tmp_active) != UI_ERROR_NONE ||
        tmp_active != d2)
      return 1;
  }
  if (g_change_count != 2 || g_last_active != d2)
    return 1;

  /* Programmatically setting via disclosure should work too */
  ui_disclosure_base_set_expanded(d3, 1);
  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(d2, &is_expanded);
    if (is_expanded)
      return 1;
  }
  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(d3, &is_expanded);
    if (!is_expanded)
      return 1;
  }
  {
    struct ui_disclosure_base *tmp_active;
    if (ui_accordion_base_get_active(accordion, &tmp_active) != UI_ERROR_NONE ||
        tmp_active != d3)
      return 1;
  }
  if (g_change_count != 3 || g_last_active != d3)
    return 1;

  /* Collapse all */
  ui_accordion_base_set_active(accordion, NULL);
  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(d3, &is_expanded);
    if (is_expanded)
      return 1;
  }
  {
    struct ui_disclosure_base *tmp_active;
    if (ui_accordion_base_get_active(accordion, &tmp_active) != UI_ERROR_NONE ||
        tmp_active != NULL)
      return 1;
  }
  if (g_change_count != 4 || g_last_active != NULL)
    return 1;

  /* Remove d2 */
  rc = ui_accordion_base_remove_disclosure(accordion, d2);
  if (rc != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_accordion_base_destroy(accordion);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_disclosure_base_destroy(d1);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_disclosure_base_destroy(d2);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_disclosure_base_destroy(d3);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  return 0;
}

static int test_accordion_edge_cases(void) {
  struct ui_accordion_base *accordion = NULL;
  struct ui_disclosure_base *d1 = NULL;
  struct ui_disclosure_base *d2 = NULL;
  struct ui_disclosure_base *d3 = NULL;
  struct ui_disclosure_base *tmp_active = NULL;
  ui_error_t rc;

  /* 1. NULL pointer args */
  if (ui_accordion_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_accordion_base_destroy(NULL); /* Should not crash */

  rc = ui_accordion_base_create(&accordion);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_accordion_base_add_disclosure(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_accordion_base_add_disclosure(accordion, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_accordion_base_remove_disclosure(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_accordion_base_remove_disclosure(accordion, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_accordion_base_set_active(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_accordion_base_get_active(NULL, &tmp_active) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_accordion_base_get_active(accordion, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_accordion_base_set_on_change(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_accordion_base_bind_data(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* 2. OOM in create */
  extern int g_malloc_fail_countdown;
  g_malloc_fail_countdown = 0;
  if (ui_accordion_base_create(&accordion) != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return 1;
  }
  g_malloc_fail_countdown = -1;

  /* 3. Not found errors */
  ui_disclosure_base_create(&d1);
  ui_disclosure_base_create(&d2);

  if (ui_accordion_base_remove_disclosure(accordion, d1) != UI_ERROR_NOT_FOUND)
    return 1;
  if (ui_accordion_base_set_active(accordion, d1) != UI_ERROR_NOT_FOUND)
    return 1;

  /* 4. Add disclosure twice */
  rc = ui_accordion_base_add_disclosure(accordion, d1);
  if (rc != UI_ERROR_NONE)
    return 1;
  if (ui_accordion_base_add_disclosure(accordion, d1) != UI_ERROR_NONE)
    return 1;

  /* 5. Add expanded disclosure */
  ui_disclosure_base_set_expanded(d2, 1);
  if (ui_accordion_base_add_disclosure(accordion, d2) != UI_ERROR_NONE)
    return 1;
  if (ui_accordion_base_get_active(accordion, &tmp_active) != UI_ERROR_NONE ||
      tmp_active != d2)
    return 1;

  /* 6. OOM in add (realloc) */
  {
    struct ui_disclosure_base *d3 = NULL, *d4 = NULL, *d5 = NULL;
    ui_disclosure_base_create(&d3);
    ui_disclosure_base_create(&d4);
    ui_disclosure_base_create(&d5);

    ui_accordion_base_add_disclosure(accordion, d3);
    ui_accordion_base_add_disclosure(accordion, d4);

    /* count is now 4. Next add will realloc. */
    g_malloc_fail_countdown = 0;
    if (ui_accordion_base_add_disclosure(accordion, d5) !=
        UI_ERROR_OUT_OF_MEMORY) {
      g_malloc_fail_countdown = -1;
      return 1;
    }
    g_malloc_fail_countdown = -1;

    ui_accordion_base_remove_disclosure(accordion, d3);
    ui_accordion_base_remove_disclosure(accordion, d4);

    {
      ui_error_t rc_cleanup = ui_disclosure_base_destroy(d3);
      if (rc_cleanup != UI_ERROR_NONE) {
        return 1;
      }
    }
    {
      ui_error_t rc_cleanup = ui_disclosure_base_destroy(d4);
      if (rc_cleanup != UI_ERROR_NONE) {
        return 1;
      }
    }
    {
      ui_error_t rc_cleanup = ui_disclosure_base_destroy(d5);
      if (rc_cleanup != UI_ERROR_NONE) {
        return 1;
      }
    }
  }

  /* 7. Remove active disclosure */
  if (ui_accordion_base_remove_disclosure(accordion, d2) != UI_ERROR_NONE)
    return 1;
  if (ui_accordion_base_get_active(accordion, &tmp_active) != UI_ERROR_NONE ||
      tmp_active != NULL)
    return 1;

  /* 8. Bind data */
  if (ui_accordion_base_bind_data(accordion, (struct ui_computed *)0x1234) !=
      UI_ERROR_NONE)
    return 1;

  /* 9. Collapse without on_change handler */
  ui_disclosure_base_create(&d3);
  ui_accordion_base_add_disclosure(accordion, d3);
  ui_accordion_base_set_active(accordion, d3);
  ui_disclosure_base_set_expanded(
      d3, 0); /* triggers on_child_disclosure_toggle(!is_expanded) without
                 on_change */
  ui_accordion_base_remove_disclosure(accordion, d3);
  {
    ui_error_t rc_cleanup = ui_disclosure_base_destroy(d3);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  {
    ui_error_t rc_cleanup = ui_accordion_base_destroy(accordion);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_disclosure_base_destroy(d1);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }
  {
    ui_error_t rc_cleanup = ui_disclosure_base_destroy(d2);
    if (rc_cleanup != UI_ERROR_NONE) {
      return 1;
    }
  }

  return 0;
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_accordion_mock_fail;

static ui_error_t test_accordion_fail_cb(struct ui_accordion_base *a,
                                         struct ui_disclosure_base *d,
                                         void *u) {
  if (a || d || u) {
  }
  return UI_ERROR_UNKNOWN;
}

static int test_accordion_mock_coverage(void) {
  ui_error_t rc;
  struct ui_accordion_base *accordion = NULL;
  struct ui_disclosure_base *d1 = NULL;
  struct ui_disclosure_base *d2 = NULL;
  struct ui_disclosure_base *d3 = NULL;

  rc = ui_accordion_base_create(&accordion);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_disclosure_base_create(&d1);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_disclosure_base_create(&d2);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  rc = ui_accordion_base_add_disclosure(accordion, d1);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_accordion_base_add_disclosure(accordion, d2);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  /* 1. on_child_disclosure_toggle(!is_expanded) with on_change failing */
  rc = ui_accordion_base_set_active(accordion, d1);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_accordion_base_set_on_change(accordion, test_accordion_fail_cb, NULL);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  /* Toggling d1 to 0 calls on_child_disclosure_toggle(d1, 0) */
  rc = ui_disclosure_base_set_expanded(d1, 0);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }

  /* 2. on_child_disclosure_toggle(is_expanded) with on_change failing */
  rc = ui_disclosure_base_set_expanded(d1, 1);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_accordion_base_set_on_change(accordion, NULL, NULL);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  /* 3. is_expanded fails inside on_child_disclosure_toggle */
  /* For this, d1 must expand while another item exists. But wait, d1 was set
   * expanded to 1 above (though it failed in on_change). Let us collapse d1
   * first. */
  rc = ui_disclosure_base_set_expanded(d1, 0);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 2;
  rc = ui_disclosure_base_set_expanded(d1, 1);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 0;

  /* 4. set_expanded fails inside on_child_disclosure_toggle */
  /* d2 must be expanded=1, and then d1 expands */
  rc = ui_disclosure_base_set_expanded(d1, 0);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_disclosure_base_set_expanded(d2, 1);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 3;
  rc = ui_disclosure_base_set_expanded(d1, 1);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 0;

  /* 5. set_on_toggle fails in ui_accordion_base_destroy */
  g_accordion_mock_fail = 1;
  rc = ui_accordion_base_destroy(accordion);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 0;

  /* Destroy accordion for real */
  rc = ui_accordion_base_destroy(accordion);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  /* 6. set_on_toggle fails in add_disclosure */
  rc = ui_accordion_base_create(&accordion);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 1;
  rc = ui_accordion_base_add_disclosure(accordion, d1);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 0;

  /* 7. is_expanded fails in add_disclosure */
  g_accordion_mock_fail = 2;
  rc = ui_accordion_base_add_disclosure(accordion, d2);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 0;

  /* 8. on_child_disclosure_toggle fails in add_disclosure */
  rc = ui_disclosure_base_create(&d3);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_disclosure_base_set_expanded(d3, 1);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_accordion_base_set_on_change(accordion, test_accordion_fail_cb, NULL);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_accordion_base_add_disclosure(accordion, d3);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_accordion_base_set_on_change(accordion, NULL, NULL);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_disclosure_base_destroy(d3);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  /* 9. set_on_toggle fails in remove_disclosure */
  rc = ui_accordion_base_add_disclosure(accordion, d1);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 1;
  rc = ui_accordion_base_remove_disclosure(accordion, d1);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 0;

  /* 10. set_expanded fails in set_active(accordion, d1) */
  rc = ui_accordion_base_add_disclosure(accordion, d1);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 3;
  rc = ui_accordion_base_set_active(accordion, d1);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 0;

  /* 11. set_expanded fails in set_active(accordion, NULL) */
  g_accordion_mock_fail = 3;
  rc = ui_accordion_base_set_active(accordion, NULL);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_accordion_mock_fail = 0;

  /* test_accordion_fail_cb NULL checks */
  test_accordion_fail_cb(NULL, NULL, NULL);

  rc = ui_accordion_base_destroy(accordion);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_disclosure_base_destroy(d1);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_disclosure_base_destroy(d2);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  return 0;
}
#endif

int main(void) {
  int failed = 0;
  printf("Running ui_accordion_base tests...\n");

  failed |= test_accordion_lifecycle();
  failed |= test_accordion_edge_cases();
#ifdef UI_TEST_MOCK_ALLOC
  failed |= test_accordion_mock_coverage();
#endif

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
