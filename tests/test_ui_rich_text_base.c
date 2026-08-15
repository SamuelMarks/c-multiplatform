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

struct ui_rich_text_run_mock {
  char *text;
  int format_flags;
  struct ui_rich_text_run_mock *next;
};

struct ui_rich_text_history_entry_mock {
  char *state_snapshot;
  struct ui_rich_text_history_entry_mock *next;
  struct ui_rich_text_history_entry_mock *prev;
};

struct ui_rich_text_base_mock {
  struct ui_component *component;
  struct ui_rich_text_run_mock *document_head;
  int selection_start;
  int selection_end;
  struct ui_rich_text_history_entry_mock *history_head;
  struct ui_rich_text_history_entry_mock *history_current;
  char *ime_composition;
  struct ui_signal *text_signal;
};

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

  /* Test non-keydown event (line 290) */
  ev.type = UI_EVENT_MOUSE_UP;
  (void)ui_rich_text_base_process_event(editor, &ev);

  /* Test toggle_format with document_head == NULL (line 258) */
  {
    struct ui_rich_text_base_mock *meditor =
        (struct ui_rich_text_base_mock *)editor;
    struct ui_rich_text_run_mock *saved_head = meditor->document_head;
    meditor->document_head = NULL;
    (void)ui_rich_text_base_toggle_format(editor, UI_RICH_TEXT_FORMAT_BOLD);
    meditor->document_head = saved_head;
  }

  /* Test get_text and free_runs with current->text == NULL (lines 114, 227,
   * 239) */
  {
    struct ui_rich_text_base_mock *meditor =
        (struct ui_rich_text_base_mock *)editor;
    struct ui_rich_text_run_mock *new_run =
        malloc(sizeof(struct ui_rich_text_run_mock));
    new_run->text = NULL;
    new_run->format_flags = 0;
    new_run->next = meditor->document_head;
    meditor->document_head = new_run;

    char *txt = NULL;
    (void)ui_rich_text_base_get_text(editor, &txt);
    if (txt)
      free(txt);
  }

  /* Test set_text with NULL text (line 179) */
  (void)ui_rich_text_base_set_text(editor, NULL);
  (void)ui_rich_text_base_set_text(editor, "");

  /* Test set_text with NULL component/shadow_root (line 194) */
  {
    struct ui_rich_text_base_mock *meditor =
        (struct ui_rich_text_base_mock *)editor;
    struct ui_dom_node *saved_sr = meditor->component->shadow_root;
    meditor->component->shadow_root = NULL;
    (void)ui_rich_text_base_set_text(editor, "NoShadowRoot");

    struct ui_component *saved_comp = meditor->component;
    meditor->component = NULL;
    (void)ui_rich_text_base_set_text(editor, "NoComponent");

    meditor->component = saved_comp;
    meditor->component->shadow_root = saved_sr;
  }

  /* Test undo with no prev, redo with no next (lines 267, 277) */
  {
    /* Normal undo/redo boundary hitting */
    ui_rich_text_base_undo(editor);
    ui_rich_text_base_undo(editor);
    ui_rich_text_base_undo(editor);
    ui_rich_text_base_undo(editor); /* definitely hits prev == NULL */

    ui_rich_text_base_redo(editor);
    ui_rich_text_base_redo(editor);
    ui_rich_text_base_redo(editor);
    ui_rich_text_base_redo(editor); /* hits next == NULL */

    struct ui_rich_text_base_mock *meditor =
        (struct ui_rich_text_base_mock *)editor;
    struct ui_rich_text_history_entry_mock *saved_hist =
        meditor->history_current;

    meditor->history_current = NULL;
    (void)ui_rich_text_base_undo(editor);
    (void)ui_rich_text_base_redo(editor);

    meditor->history_current = saved_hist;
  }

  /* Test OOM on history entry creation (line 294) */
  g_malloc_fail_countdown = 0;
  ev.type = UI_EVENT_KEY_DOWN;
  (void)ui_rich_text_base_process_event(editor, &ev);
  g_malloc_fail_countdown = -1;

  /* Test destroy with ime_composition, NULL component, NULL shadow_root (lines
   * 140, 144, 145) */
  {
    struct ui_rich_text_base_mock *meditor =
        (struct ui_rich_text_base_mock *)editor;
    ui_rich_text_base_set_ime_composition(editor, "composed");
    ui_dom_node_destroy(meditor->component->shadow_root);
    meditor->component->shadow_root = NULL;
  }
  (void)ui_rich_text_base_destroy(editor);

  /* Also test destroy with NULL component entirely */
  ui_rich_text_base_create(&editor);
  {
    struct ui_rich_text_base_mock *meditor =
        (struct ui_rich_text_base_mock *)editor;
    ui_component_destroy(meditor->component);
    meditor->component = NULL;
  }
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
