/* clang-format off */
#include "ui_rich_text_editor_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* clang-format on */

static enum ui_error test_rte_creation(void) {
  struct ui_rich_text_editor_base *rte = NULL;
  enum ui_error rc = ui_rich_text_editor_base_create(&rte, NULL);
  assert(rc == UI_ERROR_NONE);
  assert(rte != NULL);
  ui_rich_text_editor_base_destroy(rte);
  printf("test_rte_creation passed\n");
  return UI_ERROR_NONE;
}

static enum ui_error test_rte_operations(void) {
  struct ui_rich_text_editor_base *rte = NULL;
  enum ui_error rc;

  rc = ui_rich_text_editor_base_create(&rte, NULL);
  assert(rc == UI_ERROR_NONE);

  rc = ui_rich_text_editor_base_insert_text(rte, "Hello");
  assert(rc == UI_ERROR_NONE);

  rc = ui_rich_text_editor_base_set_caret_from_point(rte, 10.0f, 20.0f);
  assert(rc == UI_ERROR_NONE);

  rc = ui_rich_text_editor_base_undo(rte);
  assert(rc == UI_ERROR_NONE);

  rc = ui_rich_text_editor_base_redo(rte);
  assert(rc == UI_ERROR_NONE);

  rc = ui_rich_text_editor_base_ime_start(rte);
  assert(rc == UI_ERROR_NONE);

  rc = ui_rich_text_editor_base_ime_update(rte, "nihao");
  assert(rc == UI_ERROR_NONE);

  rc = ui_rich_text_editor_base_ime_end(rte);
  assert(rc == UI_ERROR_NONE);

  ui_rich_text_editor_base_destroy(rte);
  printf("test_rte_operations passed\n");
  return UI_ERROR_NONE;
}

int main(void) {
  test_rte_creation();
  test_rte_operations();
  return 0;
}
