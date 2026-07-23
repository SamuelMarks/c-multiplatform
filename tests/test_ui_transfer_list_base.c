/* clang-format off */
#include <stddef.h>
#include <stdio.h>

#include "ui_transfer_list_base.h"
#include "ui_error.h"
/* clang-format on */

struct ui_component {
  int id;
};

static int test_transfer_list_init(void) {
  struct ui_transfer_list_base list;
  struct ui_component comp;
  enum ui_error err;
  int failed = 0;

  err = ui_transfer_list_base_init(NULL, &comp, NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_transfer_list_base_init(&list, NULL, NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_transfer_list_base_init(&list, &comp, NULL);
  failed |= (err != UI_ERROR_NONE);
  failed |= (list.component != &comp);
  failed |= (list.left_list != NULL);
  failed |= (list.right_list != NULL);
  return failed;
}

static int test_transfer_list_operations(void) {
  struct ui_transfer_list_base list;
  struct ui_component comp;
  enum ui_error err;
  int failed = 0;

  ui_transfer_list_base_init(&list, &comp, NULL);

  /* Add items to left */
  failed |= (ui_transfer_list_base_add_item(NULL, 0, 1, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_transfer_list_base_add_item(&list, 0, 1, NULL) != UI_ERROR_NONE);
  failed |=
      (ui_transfer_list_base_add_item(&list, 0, 2, NULL) != UI_ERROR_NONE);
  failed |=
      (ui_transfer_list_base_add_item(&list, 0, 3, NULL) != UI_ERROR_NONE);

  /* Add items to right */
  failed |=
      (ui_transfer_list_base_add_item(&list, 1, 4, NULL) != UI_ERROR_NONE);

  failed |= (list.left_list == NULL);
  failed |= (list.right_list == NULL);
  failed |= (list.right_list->id != 4);

  /* Select item 2 on left and move to right */
  err = ui_transfer_list_base_set_selected(NULL, 2, 1);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_transfer_list_base_set_selected(&list, 2, 1);
  failed |= (err != UI_ERROR_NONE);

  /* invalid move direction */
  err = ui_transfer_list_base_move_selected(NULL, 1);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_transfer_list_base_move_selected(&list, 1);
  failed |= (err != UI_ERROR_NONE);

  /* Verify right list has item 2 and 4 */
  failed |= (list.right_list == NULL);
  failed |= (list.right_list->id != 2 && list.right_list->next->id != 2);

  /* Move all to left */
  err = ui_transfer_list_base_move_all(NULL, 0);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);
  err = ui_transfer_list_base_move_all(&list, 0);
  failed |= (err != UI_ERROR_NONE);
  failed |= (list.right_list != NULL);

  ui_transfer_list_base_cleanup(NULL);
  ui_transfer_list_base_cleanup(&list);
  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_transfer_list_init();
  failed |= test_transfer_list_operations();
  if (!failed) {
    printf("test_ui_transfer_list_base passed\n");
  }
  return failed;
}
