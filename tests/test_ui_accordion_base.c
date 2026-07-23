/* clang-format off */
#include "../include/ui_accordion_base.h"
#include "../include/ui_disclosure_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

static int g_change_count = 0;
static struct ui_disclosure_base *g_last_active = NULL;

static enum ui_error on_accordion_change(struct ui_accordion_base *accordion,
                                         struct ui_disclosure_base *active,
                                         void *user_data) {
  (void)accordion;
  (void)user_data;
  g_change_count++;
  g_last_active = active;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static int test_accordion_lifecycle(void) {
  struct ui_accordion_base *accordion = NULL;
  struct ui_disclosure_base *d1 = NULL;
  struct ui_disclosure_base *d2 = NULL;
  struct ui_disclosure_base *d3 = NULL;
  enum ui_error rc;

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

  ui_accordion_base_destroy(accordion);
  ui_disclosure_base_destroy(d1);
  ui_disclosure_base_destroy(d2);
  ui_disclosure_base_destroy(d3);

  return 0;
}

static int test_accordion_edge_cases(void) {
  struct ui_accordion_base *accordion = NULL;
  struct ui_disclosure_base *d1 = NULL;
  struct ui_disclosure_base *d2 = NULL;
  struct ui_disclosure_base *d3 = NULL;
  struct ui_disclosure_base *tmp_active = NULL;
  enum ui_error rc;

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

    ui_disclosure_base_destroy(d3);
    ui_disclosure_base_destroy(d4);
    ui_disclosure_base_destroy(d5);
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
  ui_disclosure_base_destroy(d3);

  ui_accordion_base_destroy(accordion);
  ui_disclosure_base_destroy(d1);
  ui_disclosure_base_destroy(d2);

  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_accordion_base tests...\n");

  failed |= test_accordion_lifecycle();
  failed |= test_accordion_edge_cases();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
