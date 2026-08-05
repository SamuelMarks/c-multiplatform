/* clang-format off */
#include "ui_css_view_transition.h"
#include <stdio.h>
#include <stdlib.h>
#include "ui_types.h"
#include <string.h>
/* clang-format on */

#define EXPECT_EQ(expected, actual)                                            \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      fprintf(stderr,                                                          \
              "%s:%d: EXPECT_EQ failed: %" UI_PRId64 " != %" UI_PRId64 "\n",   \
              __FILE__, __LINE__, (ui_int64)(expected), (ui_int64)(actual));   \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define EXPECT_STR_EQ(expected, actual)                                        \
  do {                                                                         \
    if (strcmp((expected), (actual)) != 0) {                                   \
      fprintf(stderr, "%s:%d: EXPECT_STR_EQ failed: %s != %s\n", __FILE__,     \
              __LINE__, (expected), (actual));                                 \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

static ui_error_t test_parse_view_transition_name(void) {
  struct ui_css_view_transition_name name;
  ui_error_t err;

  err = ui_css_parse_view_transition_name("none", &name);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(1, name.is_none);

  err = ui_css_parse_view_transition_name("main-header", &name);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(0, name.is_none);
  EXPECT_STR_EQ("main-header", name.name);
  return UI_ERROR_NONE;
}

static ui_error_t test_parse_view_transition_class(void) {
  struct ui_css_view_transition_class vt_class;
  ui_error_t err;

  err = ui_css_parse_view_transition_class("none", &vt_class);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(1, vt_class.is_none);
  EXPECT_EQ(NULL, vt_class.names);

  err = ui_css_parse_view_transition_class("card list-item interactive",
                                           &vt_class);
  EXPECT_EQ(UI_ERROR_NONE, err);
  EXPECT_EQ(0, vt_class.is_none);

  if (!vt_class.names) {
    fprintf(stderr, "Expected class names list\n");
    exit(1);
    return UI_ERROR_NONE;
  }

  EXPECT_STR_EQ("card", vt_class.names->name);
  EXPECT_STR_EQ("list-item", vt_class.names->next->name);
  EXPECT_STR_EQ("interactive", vt_class.names->next->next->name);
  EXPECT_EQ(NULL, vt_class.names->next->next->next);

  ui_css_view_transition_class_destroy(&vt_class);
}

extern int g_malloc_fail_countdown;

static ui_error_t test_parse_view_transition_more(void) {
  struct ui_css_view_transition_name name;
  struct ui_css_view_transition_class vt_class;
  ui_error_t err;

  /* Invalid arguments */
  err = ui_css_parse_view_transition_name(NULL, &name);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, err);
  err = ui_css_parse_view_transition_name("test", NULL);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, err);

  err = ui_css_parse_view_transition_class(NULL, &vt_class);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, err);
  err = ui_css_parse_view_transition_class("test", NULL);
  EXPECT_EQ(UI_ERROR_INVALID_ARGUMENT, err);

  /* OOM */
  g_malloc_fail_countdown = 0;
  err = ui_css_parse_view_transition_class("a b c", &vt_class);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, err);
  g_malloc_fail_countdown = 1;
  err = ui_css_parse_view_transition_class("a b c", &vt_class);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, err);
  g_malloc_fail_countdown = 2;
  err = ui_css_parse_view_transition_class("a b c", &vt_class);
  EXPECT_EQ(UI_ERROR_OUT_OF_MEMORY, err);
  g_malloc_fail_countdown = -1;

  /* Destroy NULL */
  ui_css_view_transition_class_destroy(NULL);

  return UI_ERROR_NONE;
}

static void test_whitespace(void) {
  struct ui_css_view_transition_name name;
  ui_css_parse_view_transition_name("   my-name", &name);
}
int main(void) {

  test_parse_view_transition_more();
  test_whitespace();

  test_parse_view_transition_name();
  test_parse_view_transition_class();
  printf("All View Transition tests passed.\n");
  return 0;
}
