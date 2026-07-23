/* clang-format off */
#include "../include/ui_selection_model.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

static int test_single_select(void) {
  struct ui_selection_model *model = NULL;
  int is_selected = 0;
  int count = 0;
  void *ids[4];

  if (ui_selection_model_create(&model) != UI_ERROR_NONE)
    return 1;

  /* Check initial state */
  ui_selection_model_get_selected_count(model, &count);
  if (count != 0)
    return 1;

  /* Select item 5 */
  if (ui_selection_model_select(model, (void *)5) != UI_ERROR_NONE)
    return 1;
  ui_selection_model_get_selected_count(model, &count);
  if (count != 1)
    return 1;

  ui_selection_model_is_selected(model, (void *)5, &is_selected);
  if (!is_selected)
    return 1;

  /* Select item 10, should replace 5 */
  ui_selection_model_select(model, (void *)10);
  ui_selection_model_get_selected_count(model, &count);
  if (count != 1)
    return 1;

  ui_selection_model_is_selected(model, (void *)5, &is_selected);
  if (is_selected)
    return 1;

  ui_selection_model_is_selected(model, (void *)10, &is_selected);
  if (!is_selected)
    return 1;

  ui_selection_model_get_selected(model, ids, 4);
  if (ids[0] != (void *)10)
    return 1;

  /* Deselect */
  ui_selection_model_deselect(model, (void *)10);
  ui_selection_model_get_selected_count(model, &count);
  if (count != 0)
    return 1;

  ui_selection_model_destroy(model);
  return 0;
}

static int test_multi_select(void) {
  struct ui_selection_model *model = NULL;
  int is_selected = 0;
  int count = 0;
  void *ids[4];
  void *all_ids[] = {(void *)1, (void *)2, (void *)3};

  if (ui_selection_model_create(&model) != UI_ERROR_NONE)
    return 1;
  if (ui_selection_model_set_multi_select(model, 1) != UI_ERROR_NONE)
    return 1;

  ui_selection_model_select(model, (void *)10);
  ui_selection_model_select(model, (void *)20);
  ui_selection_model_select(model,
                            (void *)20); /* Duplicate should be ignored */

  ui_selection_model_get_selected_count(model, &count);
  if (count != 2)
    return 1;

  ui_selection_model_is_selected(model, (void *)10, &is_selected);
  if (!is_selected)
    return 1;
  ui_selection_model_is_selected(model, (void *)20, &is_selected);
  if (!is_selected)
    return 1;

  ui_selection_model_toggle(model, (void *)20);
  ui_selection_model_get_selected_count(model, &count);
  if (count != 1)
    return 1;
  ui_selection_model_is_selected(model, (void *)20, &is_selected);
  if (is_selected)
    return 1;

  ui_selection_model_clear(model);
  ui_selection_model_get_selected_count(model, &count);
  if (count != 0)
    return 1;

  ui_selection_model_select_all(model, all_ids, 3);
  ui_selection_model_get_selected_count(model, &count);
  if (count != 3)
    return 1;

  ui_selection_model_get_selected(model, ids, 4);
  if (ids[0] != (void *)1 || ids[1] != (void *)2 || ids[2] != (void *)3)
    return 1;

  /* Transition back to single select */
  ui_selection_model_set_multi_select(model, 0);
  ui_selection_model_get_selected_count(model, &count);
  if (count != 1)
    return 1;

  ui_selection_model_destroy(model);
  return 0;
}

int main(void) {
  int failed = 0;

  printf("Running ui_selection_model tests...\n");

  failed |= test_single_select();
  failed |= test_multi_select();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
