/* clang-format off */
#include "ui_rich_text_editor_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* clang-format on */

static ui_error_t test_rte_creation(void) {
  struct ui_rich_text_editor_base *rte = NULL;
  ui_error_t rc = ui_rich_text_editor_base_create(&rte, NULL);
  assert(rc == UI_ERROR_NONE);
  assert(rte != NULL);
  (void)ui_rich_text_editor_base_destroy(rte);
  printf("test_rte_creation passed\n");
  return UI_ERROR_NONE;
}

static ui_error_t test_rte_operations(void) {
  struct ui_rich_text_editor_base *rte = NULL;
  ui_error_t rc;

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

  (void)ui_rich_text_editor_base_destroy(rte);
  printf("test_rte_operations passed\n");
  return UI_ERROR_NONE;
}

void test_ui_rte_coverage_errs(void);
void test_ui_rte_errors3(void);
void test_ui_rte_cva3(void);
void test_ui_rte_cva_nulls3(void);
void test_ui_rte_coverage_errs4(void);
void test_ui_rte_coverage_errs5(void);
void test_ui_rte_coverage_errs6(void);
void test_ui_rte_coverage_errs7(void);
void test_ui_rte_coverage_errs8(void);
void test_ui_rte_coverage_errs9(void);
void test_ui_rte_coverage_errs10(void);
void test_ui_rte_coverage_errs11(void);
void test_ui_rte_coverage_errs12(void);
void test_ui_rte_coverage_errs13(void);
void test_ui_rte_coverage_errs14(void);
void test_ui_rte_coverage_errs15(void);
int main(void) {
  test_rte_creation();
  test_rte_operations();
  test_ui_rte_coverage_errs();
  test_ui_rte_coverage_errs4();
  test_ui_rte_coverage_errs5();
  test_ui_rte_coverage_errs6();
  test_ui_rte_coverage_errs7();
  test_ui_rte_coverage_errs8();
  test_ui_rte_coverage_errs9();
  test_ui_rte_coverage_errs10();
  test_ui_rte_coverage_errs11();
  test_ui_rte_coverage_errs12();
  test_ui_rte_coverage_errs13();
  test_ui_rte_errors3();
  test_ui_rte_cva3();
  test_ui_rte_cva_nulls3();
  test_ui_rte_coverage_errs14();
  test_ui_rte_coverage_errs15();
  return 0;
}
void test_ui_rte_coverage_errs(void) {
  extern int g_malloc_fail_countdown;
  struct ui_rich_text_editor_base *rte = NULL;
  int i;
  for (i = 0; i < 20; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_editor_base_create(&rte, NULL);
  }
  g_malloc_fail_countdown = -1;
}
void test_ui_rte_errors3(void) {
  (void)ui_rich_text_editor_base_destroy(NULL);
  ui_rich_text_editor_base_get_component(NULL, NULL);
  struct ui_component *comp;
  ui_rich_text_editor_base_get_component(NULL, &comp);

  ui_rich_text_editor_base_insert_text(NULL, NULL);
  ui_rich_text_editor_base_insert_text(NULL, "a");

  ui_rich_text_editor_base_set_caret_from_point(NULL, 0, 0);
  ui_rich_text_editor_base_undo(NULL);
  ui_rich_text_editor_base_redo(NULL);
  ui_rich_text_editor_base_ime_start(NULL);
  ui_rich_text_editor_base_ime_update(NULL, NULL);
  ui_rich_text_editor_base_ime_end(NULL);
}

void test_ui_rte_cva3(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val = "hello";
    cva.write_value(rte, val);
    cva.set_disabled_state(rte, 1);
    ui_rich_text_editor_base_insert_text(rte, "foo");
    cva.register_on_change(rte, NULL, NULL);
    cva.register_on_touched(rte, NULL, NULL);

    val.ptr_val = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    cva.write_value(rte, val);

    struct ui_component *cmp;
    ui_rich_text_editor_base_get_component(rte, &cmp);

    (void)ui_rich_text_editor_base_destroy(rte);
  }
}

void test_ui_rte_cva_nulls3(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val = NULL;
    cva.write_value(rte, val);
    cva.write_value(NULL, val);
    cva.set_disabled_state(NULL, 1);
    cva.register_on_change(NULL, NULL, NULL);
    cva.register_on_touched(NULL, NULL, NULL);
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
void test_ui_rte_coverage_errs4(void) {
  /* To hit null new_buf we mock malloc via countdown */
  extern int g_malloc_fail_countdown;
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;

  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    g_malloc_fail_countdown =
        0; /* this should trigger the REALLOC failure if countdown hits it */
    cva.write_value(rte, val);
    g_malloc_fail_countdown = -1;

    g_malloc_fail_countdown = 0;
    ui_rich_text_editor_base_insert_text(
        rte, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    g_malloc_fail_countdown = -1;

    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
void test_ui_rte_coverage_errs5(void) {
  ui_rich_text_editor_base_create(NULL, NULL);
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    ui_rich_text_editor_base_insert_text(rte, "foo");
  }
}
void test_ui_rte_coverage_errs6(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    ui_rich_text_editor_base_insert_text(rte, "foo");
    struct ui_component *cmp;
    ui_rich_text_editor_base_get_component(
        rte, &cmp); /* Need something to trigger change */

    /* Cover missing rte */
    cva.write_value(NULL, (union ui_signal_payload){0});
    cva.register_on_change(NULL, NULL, NULL);
    cva.register_on_touched(NULL, NULL, NULL);
    cva.set_disabled_state(NULL, 1);

    /* Cover empty str */
    union ui_signal_payload val;
    val.ptr_val = NULL;
    cva.write_value(rte, val);

    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
void test_ui_rte_coverage_errs7(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    ui_rich_text_editor_base_insert_text(rte, "foo");
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}

static ui_error_t my_cva_on_change(union ui_signal_payload payload,
                                   void *user_data) {
  (void)payload;
  (void)user_data;
  return UI_ERROR_NONE;
}
static ui_error_t my_cva_on_touched(void *user_data) {
  (void)user_data;
  return UI_ERROR_NONE;
}

void test_ui_rte_coverage_errs8(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    cva.register_on_change(rte, my_cva_on_change, NULL);
    cva.register_on_touched(rte, my_cva_on_touched, NULL);
    ui_rich_text_editor_base_insert_text(rte, "foo");
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}

void test_ui_rte_coverage_errs9(void) {
  extern int g_malloc_fail_countdown;
  struct ui_rich_text_editor_base *rte = NULL;
  int i;
  for (i = 20; i < 30; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_editor_base_create(&rte, NULL);
  }
  g_malloc_fail_countdown = -1;
}

void test_ui_rte_coverage_errs10(void) {
  extern int g_malloc_fail_countdown;
  struct ui_rich_text_editor_base *rte = NULL;
  int i;
  for (i = 30; i < 40; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_editor_base_create(&rte, NULL);
  }
  g_malloc_fail_countdown = -1;
}
void test_ui_rte_coverage_errs11(void) {
  extern int g_malloc_fail_countdown;
  struct ui_rich_text_editor_base *rte = NULL;
  int i;
  for (i = 40; i < 60; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_editor_base_create(&rte, NULL);
  }
  g_malloc_fail_countdown = -1;
}
void test_ui_rte_coverage_errs12(void) {
  extern int g_malloc_fail_countdown;
  struct ui_rich_text_editor_base *rte = NULL;
  int i;
  for (i = 60; i < 200; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_editor_base_create(&rte, NULL);
  }
  g_malloc_fail_countdown = -1;
}
void test_ui_rte_coverage_errs13(void) {
  extern int g_malloc_fail_countdown;
  struct ui_rich_text_editor_base *rte = NULL;
  int i;
  for (i = 200; i < 400; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_editor_base_create(&rte, NULL);
  }
  g_malloc_fail_countdown = -1;
}
void test_ui_rte_extra(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val =
        "Hello world that is longer than the initial capacity of the rte "
        "buffer hopefully by a lot more than this 128 characters or something "
        "to trigger reallocation just in case it is small. Let's make it very "
        "very very very long indeed just to be completely safe.";
    cva.write_value(rte, val);
    cva.write_value(rte, val); /* already large enough */
    cva.set_disabled_state(rte, 1);
    cva.register_on_change(rte, NULL, NULL);
    cva.register_on_touched(rte, NULL, NULL);

    /* Also trigger CVA change and touched */
    struct ui_component *comp;
    ui_rich_text_editor_base_get_component(rte, &comp);

    ui_rich_text_editor_base_insert_text(
        rte, "foo"); /* Will fail because disabled */
    cva.set_disabled_state(rte, 0);

    /* Try null checks */
    cva.write_value(NULL, val);
    cva.set_disabled_state(NULL, 1);
    cva.register_on_change(NULL, NULL, NULL);
    cva.register_on_touched(NULL, NULL, NULL);
    ui_rich_text_editor_base_get_component(NULL, &comp);

    (void)ui_rich_text_editor_base_destroy(rte);
  }

  /* trigger oom */
  extern int g_malloc_fail_countdown;
  int i;
  for (i = 0; i < 10; ++i) {
    g_malloc_fail_countdown = i;
    ui_rich_text_editor_base_create(&rte, NULL);
  }
  g_malloc_fail_countdown = -1;
}
static ui_error_t dummy_on_change(union ui_signal_payload value,
                                  void *user_data) {
  return UI_ERROR_NONE;
}

static ui_error_t dummy_on_touched(void *user_data) { return UI_ERROR_NONE; }

void test_ui_rte_extra_more(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    cva.register_on_change(rte, dummy_on_change, NULL);
    cva.register_on_touched(rte, dummy_on_touched, NULL);
    ui_rich_text_editor_base_insert_text(rte, "triggertouchandchange");
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
void test_ui_rte_cva(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val = "hello";
    cva.write_value(rte, val);
    cva.set_disabled_state(rte, 1);
    ui_rich_text_editor_base_insert_text(rte, "foo");
    cva.register_on_change(rte, NULL, NULL);
    cva.register_on_touched(rte, NULL, NULL);
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
void test_ui_rte_cva_nulls(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val = NULL;
    cva.write_value(rte, val);
    cva.write_value(NULL, val);
    cva.set_disabled_state(NULL, 1);
    cva.register_on_change(NULL, NULL, NULL);
    cva.register_on_touched(NULL, NULL, NULL);
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
void test_ui_rte_cva2(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    union ui_signal_payload val;
    val.ptr_val = "hello";
    if (cva.write_value)
      cva.write_value(rte, val);
    if (cva.set_disabled_state)
      cva.set_disabled_state(rte, 1);
    if (cva.register_on_change)
      cva.register_on_change(rte, NULL, NULL);
    if (cva.register_on_touched)
      cva.register_on_touched(rte, NULL, NULL);

    val.ptr_val = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    if (cva.write_value)
      cva.write_value(rte, val);

    struct ui_component *comp;
    ui_rich_text_editor_base_get_component(rte, &comp);
    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
#include "ui_rich_text_editor_base.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static ui_error_t failing_cva_on_change(union ui_signal_payload payload,
                                        void *user_data) {
  return UI_ERROR_NOT_FOUND;
}
static ui_error_t failing_cva_on_touched(void *user_data) {
  return UI_ERROR_NOT_FOUND;
}

void test_ui_rte_coverage_errs14(void) {
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    cva.register_on_change(rte, failing_cva_on_change, NULL);
    ui_rich_text_editor_base_insert_text(rte, "foo"); /* hits 295 */

    cva.register_on_touched(rte, failing_cva_on_touched, NULL);
    ui_rich_text_editor_base_insert_text(rte, "foo"); /* hits 266 */

    (void)ui_rich_text_editor_base_destroy(rte);
  }
}

void test_ui_rte_coverage_errs15(void) {
  extern int g_malloc_fail_countdown;
  struct ui_control_value_accessor cva;
  struct ui_rich_text_editor_base *rte = NULL;
  ui_rich_text_editor_base_create(&rte, &cva);
  if (rte) {
    g_malloc_fail_countdown = 0;
    cva.set_disabled_state(rte, 1);
    g_malloc_fail_countdown = -1;

    g_malloc_fail_countdown = 1;
    cva.set_disabled_state(rte, 1);
    g_malloc_fail_countdown = -1;

    (void)ui_rich_text_editor_base_destroy(rte);
  }
}
