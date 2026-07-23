static int run_edge_cases(void);
/* clang-format off */
#include "ui_split_button_base.h"
#include "ui_error.h"
#include "ui_button_base.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    enum ui_error err = (expr);                                                \
    if (err != UI_ERROR_NONE) {                                                \
      printf("Failed at line %d: %d\n", __LINE__, err);                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    enum ui_error err = (expr);                                                \
    if (err != (expected)) {                                                   \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             err);                                                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)

static int run_normal_tests(void) {
  struct ui_split_button_base *sb = NULL;
  struct ui_button_base *main_btn = NULL;
  struct ui_button_base *trigger_btn = NULL;
  struct ui_component *comp = NULL;

  /* NULL checks */
  ASSERT_EQ(ui_split_button_base_create(NULL), UI_ERROR_INVALID_ARGUMENT);
  ui_split_button_base_destroy(NULL);

  ASSERT_EQ(ui_split_button_base_set_disabled(NULL, 1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_button_base_get_main_button(NULL, &main_btn),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_button_base_get_main_button(sb, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_button_base_get_trigger_button(NULL, &trigger_btn),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_button_base_get_trigger_button(sb, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_button_base_get_component(NULL, &comp),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_button_base_get_component(sb, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_button_base_bind_disabled(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_button_base_bind_text(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_split_button_base_create(&sb));

  ASSERT_SUCCESS(ui_split_button_base_get_main_button(sb, &main_btn));
  if (!main_btn)
    return 1;
  ASSERT_SUCCESS(ui_split_button_base_get_trigger_button(sb, &trigger_btn));
  if (!trigger_btn)
    return 1;
  ASSERT_SUCCESS(ui_split_button_base_get_component(sb, &comp));
  if (!comp)
    return 1;

  ASSERT_SUCCESS(ui_split_button_base_set_disabled(sb, 1));
  ASSERT_SUCCESS(ui_split_button_base_set_disabled(sb, 0));

  ASSERT_SUCCESS(ui_split_button_base_bind_disabled(sb, NULL));
  ASSERT_SUCCESS(ui_split_button_base_bind_text(sb, NULL));

  ui_split_button_base_destroy(sb);
  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_split_button_base *sb = NULL;
  int i;
  for (i = 0; i < 20; i++) {
    if (i == 1)
      continue;

    g_malloc_fail_countdown = i;
    if (ui_split_button_base_create(&sb) != UI_ERROR_NONE) {
      if (sb != NULL) {
        printf("Leaked sb in OOM tests\n");
        return 1;
      }
    } else {
      ui_split_button_base_destroy(sb);
      break; /* We succeeded, stop failing */
    }
  }
  g_malloc_fail_countdown = -1;
#endif
  return 0;
}

int main(void) {
  if (run_normal_tests() != 0)
    return 1;
  if (run_edge_cases() != 0)
    return 1;
  if (run_oom_tests() != 0)
    return 1;
  printf("All ui_split_button_base tests passed.\n");
  return 0;
}

static int run_edge_cases(void) {
  struct ui_split_button_base *sb = NULL;
  ui_split_button_base_create(&sb);
  if (!sb)
    return 1;

  struct ui_button_base *main_btn;
  struct ui_button_base *trigger_btn;
  ui_split_button_base_get_main_button(sb, &main_btn);
  ui_split_button_base_get_trigger_button(sb, &trigger_btn);

  /* Tamper with internals */
  struct ui_component *tmp;
  ui_button_base_get_component(main_btn, &tmp);
  struct ui_dom_node *tmp_node = tmp->shadow_root;
  tmp->shadow_root = NULL;

  if (ui_split_button_base_set_disabled(sb, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  tmp->shadow_root = tmp_node;

  ui_button_base_get_component(trigger_btn, &tmp);
  tmp_node = tmp->shadow_root;
  tmp->shadow_root = NULL;

  if (ui_split_button_base_set_disabled(sb, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  tmp->shadow_root = tmp_node;

  ui_split_button_base_destroy(sb);
  return 0;
}
