/* clang-format off */
#include "../include/ui_rich_text_base.h"
#include "../include/ui_error.h"
#include "../include/ui_component.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define EXPECT(cond) failed |= !(cond)

static int test_rich_text(void) {
  int failed = 0;
  struct ui_rich_text_base *editor = NULL;
  struct ui_component *comp = NULL;
  char *text = NULL;
  struct ui_event ev;

  EXPECT(ui_rich_text_base_create(&editor) == UI_ERROR_NONE);

  EXPECT(ui_rich_text_base_get_component(editor, &comp) == UI_ERROR_NONE);
  EXPECT(comp != NULL);

  EXPECT(ui_rich_text_base_set_text(editor, "Hello World") == UI_ERROR_NONE);

  EXPECT(ui_rich_text_base_get_text(editor, &text) == UI_ERROR_NONE);
  EXPECT(text != NULL);
  if (text) {
    EXPECT(strcmp(text, "Hello World") == 0);
    free(text);
    text = NULL;
  }

  ui_rich_text_base_toggle_format(editor, UI_RICH_TEXT_FORMAT_BOLD);

  ui_rich_text_base_undo(editor);
  ui_rich_text_base_redo(editor);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ui_rich_text_base_process_event(editor, &ev);

  ev.event_data.keyboard.key_code = 'H';
  ui_rich_text_base_process_event(editor, &ev);
  ui_rich_text_base_process_event(
      editor, &ev); /* Add a second one so prev/next exist */
  ui_rich_text_base_undo(editor);
  ui_rich_text_base_redo(editor);

  /* Trigger OOM on snapshot allocation in process_event */
  g_malloc_fail_countdown = 1;
  ui_rich_text_base_process_event(editor, &ev);
  g_malloc_fail_countdown = -1;

  ui_rich_text_base_set_ime_composition(editor, "nihao");
  ui_rich_text_base_set_ime_composition(editor, NULL);
  ui_rich_text_base_set_ime_composition(editor, "nihaoma");

  ui_rich_text_base_bind_text(editor, NULL);

  ui_rich_text_base_destroy(editor);

  /* nulls */
  EXPECT(ui_rich_text_base_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  ui_rich_text_base_destroy(NULL);
  EXPECT(ui_rich_text_base_get_component(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_get_component(NULL, &comp) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_get_component(editor, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_set_text(NULL, NULL) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_get_text(NULL, NULL) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_get_text(editor, NULL) == UI_ERROR_INVALID_ARGUMENT);
  ui_rich_text_base_toggle_format(NULL, 0);
  EXPECT(ui_rich_text_base_undo(NULL) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_redo(NULL) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_process_event(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_process_event(editor, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_rich_text_base_set_ime_composition(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  ui_rich_text_base_bind_text(NULL, NULL);

  /* alloc failures */
  g_malloc_fail_countdown = 0;
  EXPECT(ui_rich_text_base_create(&editor) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  EXPECT(ui_rich_text_base_create(&editor) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 2;
  EXPECT(ui_rich_text_base_create(&editor) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_rich_text_base_create(&editor);
  g_malloc_fail_countdown = 0;
  EXPECT(ui_rich_text_base_set_text(editor, "Test") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  EXPECT(ui_rich_text_base_set_text(editor, "Test") == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
  EXPECT(ui_rich_text_base_set_text(editor, "Test") == UI_ERROR_NONE);

  g_malloc_fail_countdown = 0;
  EXPECT(ui_rich_text_base_get_text(editor, &text) == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  EXPECT(ui_rich_text_base_set_ime_composition(editor, "test") ==
         UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_rich_text_base_destroy(editor);

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_rich_text();
  return failed;
}
