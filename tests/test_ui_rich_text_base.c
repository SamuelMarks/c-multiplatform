/* clang-format off */
#include "../include/ui_rich_text_base.h"
#include "../include/ui_error.h"
#include "../include/ui_component.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define EXPECT(cond)                                                           \
  do {                                                                         \
    if (!(cond)) {                                                             \
      printf("failed %d\n", __LINE__);                                         \
      failed = 1;                                                              \
    }                                                                          \
  } while (0)

static int test_rich_text(void) {
  int failed = 0;
  struct ui_rich_text_base *editor = NULL;
  struct ui_component *comp = NULL;
  char *text = NULL;
  struct ui_event ev;
  int i;

  EXPECT(ui_rich_text_base_create(&editor) == UI_ERROR_NONE);

  EXPECT(ui_rich_text_base_get_component(editor, &comp) == UI_ERROR_NONE);
  EXPECT(comp != NULL);

  EXPECT(ui_rich_text_base_get_text(editor, &text) == UI_ERROR_NONE);
  EXPECT(text != NULL);
  if (text) {
    free(text);
    text = NULL;
  }

  /* successful set text */
  EXPECT(ui_rich_text_base_set_text(editor, "Hello World!") == UI_ERROR_NONE);
  EXPECT(ui_rich_text_base_get_text(editor, &text) == UI_ERROR_NONE);
  if (text) {
    free(text);
    text = NULL;
  }

  /* format */
  (void)ui_rich_text_base_toggle_format(editor, UI_RICH_TEXT_FORMAT_BOLD);

  (void)ui_rich_text_base_undo(editor);
  (void)ui_rich_text_base_redo(editor);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  (void)ui_rich_text_base_process_event(editor, &ev);

  ev.event_data.keyboard.key_code = 'H';
  (void)ui_rich_text_base_process_event(editor, &ev);
  (void)ui_rich_text_base_process_event(
      editor, &ev); /* Add a second one so prev/next exist */
  (void)ui_rich_text_base_undo(editor);
  (void)ui_rich_text_base_redo(editor);

  /* Trigger OOM on snapshot allocation in process_event */
  g_malloc_fail_countdown = 1;
  (void)ui_rich_text_base_process_event(editor, &ev);
  g_malloc_fail_countdown = -1;

  (void)ui_rich_text_base_set_ime_composition(editor, "nihao");
  (void)ui_rich_text_base_set_ime_composition(editor, NULL);
  (void)ui_rich_text_base_set_ime_composition(editor, "nihaoma");

  (void)ui_rich_text_base_bind_text(editor, NULL);

  (void)ui_rich_text_base_destroy(editor);

  /* nulls */
  EXPECT(ui_rich_text_base_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  (void)ui_rich_text_base_destroy(NULL);
  EXPECT(ui_rich_text_base_get_component(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_get_component(NULL, &comp) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_get_component(editor, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_set_text(NULL, NULL) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_get_text(NULL, NULL) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_get_text(editor, NULL) == UI_ERROR_INVALID_ARGUMENT);
  (void)ui_rich_text_base_toggle_format(NULL, 0);
  EXPECT(ui_rich_text_base_undo(NULL) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_redo(NULL) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_process_event(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_process_event(editor, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_set_ime_composition(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  (void)ui_rich_text_base_bind_text(NULL, NULL);

  /* alloc failures */
  for (i = 0; i < 20; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_base_create(&editor);
  }
  g_malloc_fail_countdown = -1;

  (void)ui_rich_text_base_create(&editor);

  for (i = 0; i < 15; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_base_set_text(editor, "Test");
  }
  g_malloc_fail_countdown = -1;
  ui_rich_text_base_set_text(editor, "Test");

  for (i = 0; i < 5; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_base_get_text(editor, &text);
    if (text) {
      free(text);
      text = NULL;
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 5; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_base_set_ime_composition(editor, "test");
  }
  g_malloc_fail_countdown = -1;

  (void)ui_rich_text_base_destroy(editor);

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_rich_text();
  return failed;
}
