/* clang-format off */
#include <assert.h>
#include <string.h>

#include "ui_search_bar_base.h"
#include "ui_error.h"
/* clang-format on */

struct ui_component {
  int id;
};

static enum ui_error test_search_bar_init(void) {
  struct ui_search_bar_base sb;
  struct ui_component comp;
  enum ui_error err;

  err = ui_search_bar_base_init(NULL, &comp, NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_search_bar_base_init(&sb, &comp, NULL);
  assert(err == UI_ERROR_NONE);
  assert(sb.component == &comp);
  assert(sb.query == NULL);
  assert(sb.is_loading == 0);
  return UI_ERROR_NONE;
}

static enum ui_error test_search_bar_operations(void) {
  struct ui_search_bar_base sb;
  struct ui_component comp;
  enum ui_error err;

  ui_search_bar_base_init(&sb, &comp, NULL);

  err = ui_search_bar_base_set_query(&sb, "hello world");
  assert(err == UI_ERROR_NONE);
  assert(strcmp(sb.query, "hello world") == 0);

  ui_search_bar_base_set_loading(&sb, 1);
  assert(sb.is_loading == 1);

  ui_search_bar_base_set_loading(&sb, 0);
  assert(sb.is_loading == 0);

  ui_search_bar_base_cleanup(&sb);
  assert(sb.query == NULL);
  return UI_ERROR_NONE;
}

int main(void) {
  test_search_bar_init();
  test_search_bar_operations();
  return 0;
}
