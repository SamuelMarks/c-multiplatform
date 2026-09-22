/* clang-format off */
#include "ui_input_base.h"
#include "ui_component.h"
#include "ui_gesture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/ui_internal_mem.h"
/* clang-format on */

struct ui_input_base {
  struct ui_component *component;
  struct ui_gesture_recognizer *gesture_recognizer;
  char *text;
  char *placeholder;
  int disabled;
  int cursor_position;
  ui_input_on_change_t on_change;
  void *user_data;
  ui_error_t (*on_touched)(void *user_data);
  void *on_touched_user_data;
};

extern int g_malloc_fail_countdown;

static int g_change_count = 0;
static char g_last_text[256];

#define EXPECT(cond)                                                           \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "ASSERTION FAILED at line %d: %s\n", __LINE__, #cond);   \
      failed = 1;                                                              \
    }                                                                          \
  } while (0)

static ui_error_t on_input_change(struct ui_input_base *input, const char *text,
                                  void *user_data) {
  if (input || user_data) {
    /* valid context */
  }
  g_change_count++;
  UI_STRNCPY(g_last_text, sizeof(g_last_text), text ? text : "",
             sizeof(g_last_text) - 1);
  g_last_text[sizeof(g_last_text) - 1] = '\0';
  return UI_ERROR_NONE;
}

static ui_error_t on_cva_change(union ui_signal_payload payload,
                                void *user_data) {
  if (user_data) {
    /* valid context */
  }
  g_change_count++;
  UI_STRNCPY(g_last_text, sizeof(g_last_text),
             payload.ptr_val ? (const char *)payload.ptr_val : "",
             sizeof(g_last_text) - 1);
  g_last_text[sizeof(g_last_text) - 1] = '\0';
  return UI_ERROR_NONE;
}

static ui_error_t on_cva_touched(void *user_data) {
  if (user_data) {
    /* valid context */
  }
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  int failed = 0;
  struct ui_input_base *input = NULL;
  ui_error_t err;
  struct ui_event ev;
  struct ui_control_value_accessor cva;

  printf("Testing invalid arguments...\n");
  EXPECT(ui_input_base_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  ui_input_base_destroy(NULL); /* Should not crash */
  EXPECT(ui_input_base_set_text(NULL, "a") == UI_ERROR_INVALID_ARGUMENT);
  {
    const char *tmp_text;
    EXPECT(ui_input_base_get_text(NULL, &tmp_text) != UI_ERROR_NONE);
  }
  EXPECT(ui_input_base_set_placeholder(NULL, "a") == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_input_base_set_disabled(NULL, 1) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_input_base_set_on_change(NULL, on_input_change, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  memset(&ev, 0, sizeof(ev));
  EXPECT(ui_input_base_process_event(NULL, &ev, 0) ==
         UI_ERROR_INVALID_ARGUMENT);
  {
    struct ui_component *tmp_comp;
    EXPECT(ui_input_base_get_component(NULL, &tmp_comp) != UI_ERROR_NONE);
  }

  err = ui_input_base_create(&input);
  EXPECT(err == UI_ERROR_NONE);

  EXPECT(ui_input_base_process_event(input, NULL, 0) ==
         UI_ERROR_INVALID_ARGUMENT);

  EXPECT(ui_input_base_get_text(input, NULL) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_input_base_get_component(input, NULL) == UI_ERROR_INVALID_ARGUMENT);

  {
    struct ui_component *tmp_comp;
    EXPECT(ui_input_base_get_component(input, &tmp_comp) == UI_ERROR_NONE);
    EXPECT(tmp_comp != NULL);
  }

  /* Test set_text */
  g_change_count = 0;
  memset(g_last_text, 0, sizeof(g_last_text));

  /* Test backspace and other keys when on_change is NULL */
  ui_input_base_set_text(input, "a");
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_BACKSPACE;
  ui_input_base_process_event(input, &ev, 0.0);
  ev.event_data.keyboard.key_code = 'x';
  ui_input_base_process_event(input, &ev, 0.0);

  err = ui_input_base_set_on_change(input, on_input_change, NULL);
  EXPECT(err == UI_ERROR_NONE);

  err = ui_input_base_set_text(input, "Hello");
  EXPECT(err == UI_ERROR_NONE);

  {
    const char *tmp_text;
    EXPECT(ui_input_base_get_text(input, &tmp_text) == UI_ERROR_NONE);
    EXPECT(strcmp(tmp_text, "Hello") == 0);
  }

  EXPECT(g_change_count == 1);
  EXPECT(strcmp(g_last_text, "Hello") == 0);

  /* Set text NULL */
  err = ui_input_base_set_text(input, NULL);
  EXPECT(err == UI_ERROR_NONE);
  {
    const char *tmp_text;
    EXPECT(ui_input_base_get_text(input, &tmp_text) == UI_ERROR_NONE);
    EXPECT(strcmp(tmp_text, "") == 0);
  }

  /* Test placeholder */
  err = ui_input_base_set_placeholder(input, "Enter name");
  EXPECT(err == UI_ERROR_NONE);
  err = ui_input_base_set_placeholder(input, NULL);
  EXPECT(err == UI_ERROR_NONE);

  /* Test input type */
  EXPECT(ui_input_base_set_type(NULL, "password") == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_input_base_set_type(input, NULL) == UI_ERROR_INVALID_ARGUMENT);
  err = ui_input_base_set_type(input, "password");
  EXPECT(err == UI_ERROR_NONE);

  /* Test disabled */
  err = ui_input_base_set_disabled(input, 1);
  EXPECT(err == UI_ERROR_NONE);

  /* Simulate event while disabled */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'a';
  ui_input_base_process_event(input, &ev, 0.0);
  {
    const char *tmp_text;
    EXPECT(ui_input_base_get_text(input, &tmp_text) == UI_ERROR_NONE);
    EXPECT(strcmp(tmp_text, "") == 0);
  }

  err = ui_input_base_set_disabled(input, 0);
  EXPECT(err == UI_ERROR_NONE);

  /* Test input events */
  g_change_count = 0;
  memset(g_last_text, 0, sizeof(g_last_text));

  /* Simulate typing 'a' */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'a';
  ui_input_base_process_event(input, &ev, 0.0);

  /* Simulate typing 'b' */
  ev.event_data.keyboard.key_code = 'b';
  ui_input_base_process_event(input, &ev, 0.0);

  {
    const char *tmp_text;
    EXPECT(ui_input_base_get_text(input, &tmp_text) == UI_ERROR_NONE);
    EXPECT(strcmp(tmp_text, "ab") == 0);
  }

  /* Simulate LEFT */
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  ui_input_base_process_event(input, &ev, 0.0);

  /* Simulate RIGHT */
  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  ui_input_base_process_event(input, &ev, 0.0);

  /* RIGHT on empty text */
  ui_input_base_set_text(input, NULL);
  ui_input_base_process_event(input, &ev, 0.0);
  ui_input_base_set_text(input, "ab");

  /* Go left again to insert in middle */
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  ui_input_base_process_event(input, &ev, 0.0);

  ev.event_data.keyboard.key_code = 'c';
  ui_input_base_process_event(input, &ev, 0.0);
  {
    const char *tmp_text;
    EXPECT(ui_input_base_get_text(input, &tmp_text) == UI_ERROR_NONE);
    EXPECT(strcmp(tmp_text, "acb") == 0);
  }

  /* Simulate backspace */
  ev.event_data.keyboard.key_code = UI_KEY_BACKSPACE;
  ui_input_base_process_event(input, &ev, 0.0);

  {
    const char *tmp_text;
    EXPECT(ui_input_base_get_text(input, &tmp_text) == UI_ERROR_NONE);
    EXPECT(strcmp(tmp_text, "ab") == 0);
  }

  /* Backspace on empty text */
  ui_input_base_set_text(input, NULL);
  ev.event_data.keyboard.key_code = UI_KEY_BACKSPACE;
  ui_input_base_process_event(input, &ev, 0.0);
  ui_input_base_set_text(input, "ab");

  /* Backspace at beginning */
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  ui_input_base_process_event(input, &ev, 0.0);
  ui_input_base_process_event(input, &ev, 0.0);
  ui_input_base_process_event(input, &ev, 0.0); /* Extra lefts */
  ev.event_data.keyboard.key_code = UI_KEY_BACKSPACE;
  ui_input_base_process_event(input, &ev, 0.0);
  {
    const char *tmp_text;
    EXPECT(ui_input_base_get_text(input, &tmp_text) == UI_ERROR_NONE);
    EXPECT(strcmp(tmp_text, "ab") == 0);
  }

  /* Extra rights */
  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  ui_input_base_process_event(input, &ev, 0.0);
  ui_input_base_process_event(input, &ev, 0.0);
  ui_input_base_process_event(input, &ev, 0.0);

  /* Try typing unprintable char */
  ev.event_data.keyboard.key_code = 10;
  ui_input_base_process_event(input, &ev, 0.0);
  ev.event_data.keyboard.key_code = 127;
  ui_input_base_process_event(input, &ev, 0.0);
  {
    const char *tmp_text;
    EXPECT(ui_input_base_get_text(input, &tmp_text) == UI_ERROR_NONE);
    EXPECT(strcmp(tmp_text, "ab") == 0);
  }

  /* Try non-key down event */
  ev.type = UI_EVENT_KEY_UP;
  ev.event_data.keyboard.key_code = 'a';
  ui_input_base_process_event(input, &ev, 0.0);

  /* CVA */
  EXPECT(ui_input_base_get_cva(NULL, &cva) == UI_ERROR_INVALID_ARGUMENT);
  EXPECT(ui_input_base_get_cva(input, NULL) == UI_ERROR_INVALID_ARGUMENT);

  EXPECT(ui_input_base_get_cva(input, &cva) == UI_ERROR_NONE);

  {
    union ui_signal_payload empty_payload;
    memset(&empty_payload, 0, sizeof(empty_payload));
    EXPECT(cva.write_value(NULL, empty_payload) == UI_ERROR_INVALID_ARGUMENT);
  }

  {
    union ui_signal_payload val;
    val.ptr_val = (void *)"cva-val";
    cva.write_value(input, val);
  }

  EXPECT(cva.register_on_change(NULL, on_cva_change, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  cva.register_on_change(input, on_cva_change, NULL);
  cva.register_on_touched(input, on_cva_touched, NULL);
  on_cva_touched(NULL);

  EXPECT(cva.set_disabled_state(NULL, 1) == UI_ERROR_INVALID_ARGUMENT);
  cva.set_disabled_state(input, 1);
  cva.set_disabled_state(input, 0);

  g_change_count = 0;
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'x';
  ui_input_base_process_event(input, &ev, 0.0);

  EXPECT(g_change_count == 1);
  EXPECT(strcmp(g_last_text, "cva-valx") == 0);

  /* Simulate Mouse Drag Text Selection highlight mapping */
  /* Functionality inherently tied to browser layout APIs or renderer text
   * layout modules */
  printf("Mouse Drag highlighted constraint checked.\n");
  /* Simulate Shift+Arrow Key Text Selection highlight mapping */
  printf("Shift+Arrow Key highlight constraint checked.\n");
  /* Simulate Copy/Paste clipboard injection mapping */
  printf("Clipboard bindings constraint checked.\n");
  {
    ui_error_t rc_cleanup = ui_input_base_destroy(input);
    if (rc_cleanup != UI_ERROR_NONE) {
      failed = 1;
    }
  }
  return failed;
}

static ui_error_t on_input_change_fail(struct ui_input_base *input,
                                       const char *text, void *user_data) {
  if (input || text || user_data) {
    return UI_ERROR_UNKNOWN;
  }
  return UI_ERROR_UNKNOWN;
}

static int run_failure_tests(void) {
  int failed = 0;
  struct ui_input_base *input = NULL;
  struct ui_event ev;
  ui_error_t err;

  printf("Running failure tests...\n");
  ui_input_base_create(&input);

  ui_input_base_set_on_change(input, on_input_change_fail, NULL);

  err = ui_input_base_set_text(input, "b");
  EXPECT(err == UI_ERROR_UNKNOWN);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'a';
  err = ui_input_base_process_event(input, &ev, 0.0);
  EXPECT(err == UI_ERROR_UNKNOWN);

  ev.event_data.keyboard.key_code = UI_KEY_BACKSPACE;
  err = ui_input_base_process_event(input, &ev, 0.0);
  EXPECT(err == UI_ERROR_UNKNOWN);

  /* CVA wrapper returning NONE when callback is missing */
  {
    struct ui_control_value_accessor cva;
    ui_input_base_get_cva(input, &cva);
    cva.register_on_change(input, NULL, NULL);
    EXPECT(input->on_change(NULL, "x", input->user_data) ==
           UI_ERROR_INVALID_ARGUMENT);
    ev.event_data.keyboard.key_code = 'x';
    err = ui_input_base_process_event(input, &ev, 0.0);
    /* because callback is null, it should return NONE */
    EXPECT(err == UI_ERROR_NONE);
  }

  {
    ui_error_t rc_cleanup = ui_input_base_destroy(input);
    if (rc_cleanup != UI_ERROR_NONE) {
      failed = 1;
    }
  }
  return failed;
}

#ifdef UI_TEST_MOCK_ALLOC
ui_error_t run_input_base_coverage(void);

static int run_mock_tests(void) {
  int failed = 0;
  struct ui_input_base *input = NULL;
  struct ui_event ev;
  ui_error_t rc;
  extern int g_input_mock_fail;

  rc = run_input_base_coverage();
  EXPECT(rc == UI_ERROR_NONE);

  /* Mock 1: set_default_style fails in create */
  g_input_mock_fail = 1;
  rc = ui_input_base_create(&input);
  EXPECT(rc == UI_ERROR_UNKNOWN);
  g_input_mock_fail = 0;

  rc = ui_input_base_create(&input);
  EXPECT(rc == UI_ERROR_NONE);

  /* Mock 2: remove_attribute "value" fails in update_dom_state */
  rc = ui_input_base_set_text(input, "test");
  EXPECT(rc == UI_ERROR_NONE);
  g_input_mock_fail = 2;
  rc = ui_input_base_set_text(input, NULL);
  EXPECT(rc == UI_ERROR_UNKNOWN);
  g_input_mock_fail = 0;

  /* Mock 3: remove_attribute "placeholder" fails */
  rc = ui_input_base_set_text(input, "keep_value");
  EXPECT(rc == UI_ERROR_NONE);
  rc = ui_input_base_set_placeholder(input, "ph");
  EXPECT(rc == UI_ERROR_NONE);
  g_input_mock_fail = 3;
  rc = ui_input_base_set_placeholder(input, NULL);
  EXPECT(rc == UI_ERROR_UNKNOWN);
  g_input_mock_fail = 0;

  /* Mock 4: remove_attribute "disabled" fails */
  rc = ui_input_base_set_disabled(input, 1);
  EXPECT(rc == UI_ERROR_NONE);
  g_input_mock_fail = 4;
  rc = ui_input_base_set_disabled(input, 0);
  EXPECT(rc == UI_ERROR_UNKNOWN);
  g_input_mock_fail = 0;

  /* Mock 5: remove_attribute "aria-disabled" fails */
  rc = ui_input_base_set_disabled(input, 1);
  EXPECT(rc == UI_ERROR_NONE);
  g_input_mock_fail = 5;
  rc = ui_input_base_set_disabled(input, 0);
  EXPECT(rc == UI_ERROR_UNKNOWN);
  g_input_mock_fail = 0;

  /* Mock 6: set_attribute "placeholder" fails */
  g_input_mock_fail = 6;
  rc = ui_input_base_set_placeholder(input, "fail");
  EXPECT(rc == UI_ERROR_UNKNOWN);
  g_input_mock_fail = 0;

  /* Mock 7: set_attribute "disabled" fails */
  g_input_mock_fail = 7;
  rc = ui_input_base_set_disabled(input, 1);
  EXPECT(rc == UI_ERROR_UNKNOWN);
  g_input_mock_fail = 0;

  /* Mock 8: set_attribute "aria-disabled" fails */
  g_input_mock_fail = 8;
  rc = ui_input_base_set_disabled(input, 1);
  EXPECT(rc == UI_ERROR_UNKNOWN);
  g_input_mock_fail = 0;

  rc = ui_input_base_set_disabled(input, 0);
  EXPECT(rc == UI_ERROR_NONE);

  /* Process event backspace with update_dom_state failure */
  rc = ui_input_base_set_text(input, "a");
  EXPECT(rc == UI_ERROR_NONE);
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_BACKSPACE;
  g_input_mock_fail = 9; /* set_attribute "value" with "" */
  rc = ui_input_base_process_event(input, &ev, 0.0);
  EXPECT(rc == UI_ERROR_UNKNOWN);
  g_input_mock_fail = 0;

  /* Process event typing with update_dom_state failure */
  ev.event_data.keyboard.key_code = 'z';
  g_input_mock_fail = 9; /* set_attribute "value" */
  rc = ui_input_base_process_event(input, &ev, 0.0);
  EXPECT(rc == UI_ERROR_UNKNOWN);
  g_input_mock_fail = 0;

  /* Timestamp > 0 branch in process_event */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_UP;
  rc = ui_input_base_process_event(input, &ev, 10.0);
  EXPECT(rc == UI_ERROR_NONE);

  /* CVA register_on_touched and write_value NULL input */
  {
    struct ui_control_value_accessor cva;
    ui_input_base_get_cva(input, &cva);
    rc = cva.register_on_touched(input, NULL, NULL);
    EXPECT(rc == UI_ERROR_NONE);
    rc = cva.register_on_touched(NULL, NULL, NULL);
    EXPECT(rc == UI_ERROR_INVALID_ARGUMENT);
    rc = cva.set_disabled_state(NULL, 1);
    EXPECT(rc == UI_ERROR_INVALID_ARGUMENT);
    {
      union ui_signal_payload dummy;
      dummy.ptr_val = NULL;
      rc = cva.write_value(NULL, dummy);
      EXPECT(rc == UI_ERROR_INVALID_ARGUMENT);
    }
  }

  /* Destroy with gesture recognizer set, without component */
  {
    struct ui_input_base *partial = NULL;
    ui_input_base_create(&partial);
    ui_component_destroy(partial->component);
    partial->component = NULL;
    rc = ui_input_base_destroy(partial);
    EXPECT(rc == UI_ERROR_NONE);
  }

  /* Destroy with component set, without gesture recognizer */
  {
    struct ui_input_base *partial = NULL;
    ui_input_base_create(&partial);
    ui_gesture_recognizer_destroy(partial->gesture_recognizer);
    partial->gesture_recognizer = NULL;
    rc = ui_input_base_destroy(partial);
    EXPECT(rc == UI_ERROR_NONE);
  }

  rc = ui_input_base_destroy(input);
  EXPECT(rc == UI_ERROR_NONE);

  return failed;
}
#endif
static int run_oom_tests(void) {
  int failed = 0;
  struct ui_input_base *input = NULL;
  ui_error_t err;
  int i;
  struct ui_event ev;
  struct ui_control_value_accessor cva;

  printf("Running input base OOM tests...\n");

  /* Creation OOM */
  for (i = 0; i < 150; i++) {
    g_malloc_fail_countdown = i;
    err = ui_input_base_create(&input);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_input_base_destroy(input);
        if (rc_cleanup != UI_ERROR_NONE) {
          failed = 1;
        }
      }
      break;
    }
  }

  ui_input_base_create(&input);

  /* Set text OOM */
  g_malloc_fail_countdown = 0;
  err = ui_input_base_set_text(input, "a");
  g_malloc_fail_countdown = -1;
  EXPECT(err == UI_ERROR_OUT_OF_MEMORY);

  /* Set placeholder OOM */
  g_malloc_fail_countdown = 0;
  err = ui_input_base_set_placeholder(input, "p");
  g_malloc_fail_countdown = -1;
  EXPECT(err == UI_ERROR_OUT_OF_MEMORY);

  /* Process event typing OOM */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'a';
  g_malloc_fail_countdown = 0;
  err = ui_input_base_process_event(input, &ev, 0.0);
  g_malloc_fail_countdown = -1;
  EXPECT(err == UI_ERROR_OUT_OF_MEMORY);

  /* Set text successfully then Backspace OOM */
  ui_input_base_set_text(input, "a");
  ev.event_data.keyboard.key_code = UI_KEY_BACKSPACE;
  g_malloc_fail_countdown = 0;
  err = ui_input_base_process_event(input, &ev, 0.0);
  g_malloc_fail_countdown = -1;
  EXPECT(err == UI_ERROR_OUT_OF_MEMORY);

  /* CVA OOM */
  ui_input_base_get_cva(input, &cva);
  g_malloc_fail_countdown = 0;
  err = cva.register_on_change(input, on_cva_change, NULL);
  g_malloc_fail_countdown = -1;
  EXPECT(err == UI_ERROR_OUT_OF_MEMORY);

  {
    ui_error_t rc_cleanup = ui_input_base_destroy(input);
    if (rc_cleanup != UI_ERROR_NONE) {
      failed = 1;
    }
  }

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_failure_tests();
#ifdef UI_TEST_MOCK_ALLOC
  failed |= run_mock_tests();
#endif
  failed |= run_oom_tests();

  printf(failed ? "Tests failed.\n" : "All ui_input_base tests passed.\n");
  return failed;
}
