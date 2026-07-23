/* clang-format off */
#include "../include/ui_data_source.h"
#include "../include/ui_error.h"
#include "../include/ui_signal.h"
#include "../include/ui_arena.h"
#include <stdio.h>
/* clang-format on */

static enum ui_error dummy_fetch_page(struct ui_data_source *ds,
                                      ui_uint32 offset, ui_uint32 limit,
                                      void *user_data) {
  int *called = (int *)user_data;
  *called = 1;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static enum ui_error dummy_apply_sort(struct ui_data_source *ds,
                                      const struct ui_sort_descriptor *sorts,
                                      ui_uint32 num_sorts, void *user_data) {
  int *called = (int *)user_data;
  *called = 1;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static enum ui_error
dummy_apply_filter(struct ui_data_source *ds,
                   const struct ui_filter_descriptor *filters,
                   ui_uint32 num_filters, void *user_data) {
  int *called = (int *)user_data;
  *called = 1;
  return UI_ERROR_NONE;
}

static int test_data_source_lifecycle(void) {
  struct ui_data_source *ds = NULL;
  enum ui_error rc;

  rc = ui_data_source_create(&ds);
  if (rc != UI_ERROR_NONE || ds == NULL)
    return 1;

  rc = ui_data_source_destroy(ds);
  if (rc != UI_ERROR_NONE)
    return 1;

  return 0;
}

static int test_data_source_callbacks(void) {
  struct ui_data_source *ds = NULL;
  enum ui_error rc;
  int fetch_called = 0;
  int sort_called = 0;
  int filter_called = 0;

  rc = ui_data_source_create(&ds);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_data_source_set_fetch_page_callback(ds, dummy_fetch_page,
                                              &fetch_called);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_data_source_set_apply_sort_callback(ds, dummy_apply_sort,
                                              &sort_called);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_data_source_set_apply_filter_callback(ds, dummy_apply_filter,
                                                &filter_called);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_data_source_fetch_page(ds, 0, 10);
  if (rc != UI_ERROR_NONE || fetch_called != 1)
    return 1;

  rc = ui_data_source_apply_sort(ds, NULL, 0);
  if (rc != UI_ERROR_NONE || sort_called != 1)
    return 1;

  rc = ui_data_source_apply_filter(ds, NULL, 0);
  if (rc != UI_ERROR_NONE || filter_called != 1)
    return 1;

  ui_data_source_destroy(ds);
  return 0;
}

static int test_data_source_signals(void) {
  struct ui_data_source *ds = NULL;
  struct ui_signal *state_sig =
      (struct ui_signal *)0x1234; /* Mock signal ptr */
  struct ui_signal *data_sig = (struct ui_signal *)0x5678;
  enum ui_error rc;

  rc = ui_data_source_create(&ds);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_data_source_bind_state(ds, state_sig);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_data_source_bind_data(ds, data_sig);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_data_source_destroy(ds);
  return 0;
}

extern int g_malloc_fail_countdown;

static int test_data_source_errors(void) {
  struct ui_data_source *ds = NULL;
  struct ui_signal *sig = (struct ui_signal *)0x1234;

  if (ui_data_source_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  g_malloc_fail_countdown = 0;
  if (ui_data_source_create(&ds) != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return 1;
  }
  g_malloc_fail_countdown = -1;

  if (ui_data_source_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_data_source_set_fetch_page_callback(NULL, dummy_fetch_page, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_data_source_set_apply_sort_callback(NULL, dummy_apply_sort, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_data_source_set_apply_filter_callback(
          NULL, dummy_apply_filter, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_data_source_fetch_page(NULL, 0, 10) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_data_source_apply_sort(NULL, NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_data_source_apply_filter(NULL, NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_data_source_create(&ds);

  if (ui_data_source_fetch_page(ds, 0, 10) != UI_ERROR_UNSUPPORTED)
    return 1;
  if (ui_data_source_apply_sort(ds, NULL, 0) != UI_ERROR_UNSUPPORTED)
    return 1;
  if (ui_data_source_apply_filter(ds, NULL, 0) != UI_ERROR_UNSUPPORTED)
    return 1;

  if (ui_data_source_bind_state(NULL, sig) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_data_source_bind_state(ds, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_data_source_bind_data(NULL, sig) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_data_source_bind_data(ds, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_data_source_destroy(ds);
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_data_source tests...\n");

  failed |= test_data_source_lifecycle();
  failed |= test_data_source_callbacks();
  failed |= test_data_source_signals();
  failed |= test_data_source_errors();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
