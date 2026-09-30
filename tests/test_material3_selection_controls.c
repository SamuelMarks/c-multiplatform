/**
 * @file test_material3_selection_controls.c
 * @brief Unit tests for Material 3 Selection & Advanced Input Controls.
 */

/* clang-format off */
#include "material3/md3_selection_controls.h"
#include "ui_engine.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_md3_autocomplete_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_autocomplete *ac = NULL;
  struct ui_control_value_accessor cva;
  char buf[64];
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;
  memset(&cva, 0, sizeof(cva));

  /* Null checks */
  rc = md3_autocomplete_create(NULL, MD3_TEXT_FIELD_OUTLINED, &ac, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_autocomplete_create(dummy_engine, MD3_TEXT_FIELD_OUTLINED, NULL,
                               NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Normal creation with CVA */
  rc =
      md3_autocomplete_create(dummy_engine, MD3_TEXT_FIELD_OUTLINED, &ac, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ac != NULL);
  ASSERT_EQ(MD3_TEXT_FIELD_OUTLINED, ac->variant);
  ASSERT_EQ(0, ac->is_open);

  /* Query manipulation */
  rc = md3_autocomplete_set_query(NULL, "test");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_autocomplete_set_query(ac, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_autocomplete_set_query(ac, "Material Design");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Very long query exceeding buffer */
  {
    char long_query[300];
    memset(long_query, 'a', sizeof(long_query));
    long_query[sizeof(long_query) - 1] = '\0';
    rc = md3_autocomplete_set_query(ac, long_query);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Query retrieval with small buffer triggering truncation */
  {
    char small_buf[5];
    rc = md3_autocomplete_get_query(ac, small_buf, sizeof(small_buf));
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(4, (int)strlen(small_buf));
  }

  rc = md3_autocomplete_set_query(ac, "Material Design");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_autocomplete_get_query(NULL, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_autocomplete_get_query(ac, NULL, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_autocomplete_get_query(ac, buf, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_autocomplete_get_query(ac, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Material Design", buf);

  rc = md3_autocomplete_destroy(ac);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_autocomplete_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_md3_select_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_select *sel = NULL;
  struct ui_control_value_accessor cva;
  int sel_idx = -1;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;
  memset(&cva, 0, sizeof(cva));

  /* Null checks */
  rc = md3_select_create(NULL, MD3_TEXT_FIELD_FILLED, &sel, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_select_create(dummy_engine, MD3_TEXT_FIELD_FILLED, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Normal creation without and with CVA */
  rc = md3_select_create(dummy_engine, MD3_TEXT_FIELD_FILLED, &sel, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sel != NULL);
  md3_select_destroy(sel);
  sel = NULL;

  rc = md3_select_create(dummy_engine, MD3_TEXT_FIELD_FILLED, &sel, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sel != NULL);
  ASSERT_EQ(0, sel->is_multi);
  ASSERT_EQ(0, sel->is_open);

  /* Add options */
  rc = md3_select_add_option(NULL, "Apple", "appl");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_select_add_option(sel, NULL, "appl");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_select_add_option(sel, "Apple", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_select_add_option(sel, "Apple", "appl");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_select_add_option(sel, "Banana", "bana");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, (int)sel->option_count);

  /* Add up to capacity 64, then test out of memory */
  {
    int opt_i;
    for (opt_i = 2; opt_i < 64; opt_i++) {
      rc = md3_select_add_option(sel, "Item", "val");
      ASSERT_EQ(UI_ERROR_NONE, rc);
    }
    rc = md3_select_add_option(sel, "Overflow", "overflow");
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  /* Multi-select toggle */
  rc = md3_select_set_multiselect(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_select_set_multiselect(sel, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, sel->is_multi);

  /* Open / close */
  rc = md3_select_set_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_select_set_open(sel, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, sel->is_open);
  ASSERT_EQ(180.0f, sel->arrow_rotation_deg);

  rc = md3_select_set_open(sel, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, sel->is_open);
  ASSERT_EQ(0.0f, sel->arrow_rotation_deg);

  /* Selection index */
  rc = md3_select_set_selected_index(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_select_set_selected_index(sel, 100);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_select_set_selected_index(sel, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Deselect with negative index */
  rc = md3_select_set_selected_index(sel, -1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(-1, sel->selected_index);

  rc = md3_select_get_selected_index(NULL, &sel_idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_select_get_selected_index(sel, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_select_get_selected_index(sel, &sel_idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(-1, sel_idx);

  rc = md3_select_destroy(sel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_select_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_md3_pin_input_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_pin_input *pin = NULL;
  struct ui_control_value_accessor cva;
  char buf[32];
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;
  memset(&cva, 0, sizeof(cva));

  /* Null and argument checks */
  rc = md3_pin_input_create(NULL, 6, &pin, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_create(dummy_engine, 0, &pin, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_create(dummy_engine, 20, &pin, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_create(dummy_engine, 6, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Normal creation */
  rc = md3_pin_input_create(dummy_engine, 6, &pin, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(pin != NULL);
  ASSERT_EQ(6, pin->length);
  ASSERT_EQ(0, pin->is_masked);

  /* Masking */
  rc = md3_pin_input_set_masked(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_set_masked(pin, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, pin->is_masked);

  /* Character input */
  rc = md3_pin_input_on_input(NULL, 0, "4");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_on_input(pin, -1, "4");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_on_input(pin, 6, "4");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_on_input(pin, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_pin_input_on_input(pin, 0, "1");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_pin_input_on_input(pin, 1, "2");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_pin_input_on_input(pin, 2, "3");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Input at last index length - 1 */
  rc = md3_pin_input_on_input(pin, 5, "9");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_pin_input_get_value(NULL, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_get_value(pin, NULL, sizeof(buf));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_get_value(pin, buf, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_pin_input_get_value(pin, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("1239", buf);

  /* Backspace at index 0 */
  rc = md3_pin_input_on_backspace(pin, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Backspace */
  rc = md3_pin_input_on_backspace(NULL, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_on_backspace(pin, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_on_backspace(pin, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_pin_input_on_backspace(pin, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_pin_input_get_value(pin, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("29", buf);

  /* Paste shorter than length */
  rc = md3_pin_input_on_paste(pin, "12");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Paste */
  rc = md3_pin_input_on_paste(NULL, "987654");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_pin_input_on_paste(pin, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_pin_input_on_paste(pin, "987654");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_pin_input_get_value(pin, buf, sizeof(buf));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("987654", buf);

  rc = md3_pin_input_destroy(pin);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_pin_input_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_md3_rating_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_rating *rating = NULL;
  struct ui_control_value_accessor cva;
  float val = 0.0f;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;
  memset(&cva, 0, sizeof(cva));

  /* Null checks */
  rc = md3_rating_create(NULL, 5, &rating, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rating_create(dummy_engine, 0, &rating, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rating_create(dummy_engine, 5, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Normal creation */
  rc = md3_rating_create(dummy_engine, 5, &rating, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(rating != NULL);
  ASSERT_EQ(5, rating->max_rating);

  /* Value manipulation */
  rc = md3_rating_set_value(NULL, 3.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rating_set_value(rating, 3.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_rating_get_value(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rating_get_value(rating, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_rating_get_value(rating, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3.5f, val);

  /* Value clamping */
  rc = md3_rating_set_value(rating, -1.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, rating->value);

  rc = md3_rating_set_value(rating, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5.0f, rating->value);

  /* Hover preview and read only */
  rc = md3_rating_set_hover_preview(NULL, 2.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rating_set_hover_preview(rating, 2.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2.0f, rating->hover_preview_value);

  /* Hover preview clamping */
  rc = md3_rating_set_hover_preview(rating, -1.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, rating->hover_preview_value);

  rc = md3_rating_set_hover_preview(rating, 10.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5.0f, rating->hover_preview_value);

  rc = md3_rating_set_read_only(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_rating_set_read_only(rating, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, rating->is_read_only);

  rc = md3_rating_destroy(rating);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_rating_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_md3_spin_button_lifecycle(void) {
  struct ui_engine *dummy_engine;
  struct md3_spin_button *spin = NULL;
  struct ui_control_value_accessor cva;
  double val = 0.0;
  ui_error_t rc;

  dummy_engine = (struct ui_engine *)0x1234;
  memset(&cva, 0, sizeof(cva));

  /* Null and invalid args */
  rc = md3_spin_button_create(NULL, 0.0, 100.0, 1.0, &spin, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_spin_button_create(dummy_engine, 100.0, 0.0, 1.0, &spin, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_spin_button_create(dummy_engine, 0.0, 100.0, 0.0, &spin, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_spin_button_create(dummy_engine, 0.0, 100.0, 1.0, NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Normal creation */
  rc = md3_spin_button_create(dummy_engine, 10.0, 50.0, 2.5, &spin, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(spin != NULL);

  /* Value get/set */
  rc = md3_spin_button_get_value(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_spin_button_get_value(spin, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_spin_button_get_value(spin, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10.0, val);

  rc = md3_spin_button_set_value(NULL, 25.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_spin_button_set_value(spin, 25.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(25.0, spin->current_val);

  /* Step */
  rc = md3_spin_button_step(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_spin_button_step(spin, 2); /* 25.0 + 2 * 2.5 = 30.0 */
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(30.0, spin->current_val);

  rc = md3_spin_button_step(spin, -1); /* 30.0 - 2.5 = 27.5 */
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(27.5, spin->current_val);

  /* Clamping on step */
  rc = md3_spin_button_step(spin, 100);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(50.0, spin->current_val);

  rc = md3_spin_button_step(spin, -100);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10.0, spin->current_val);

  rc = md3_spin_button_destroy(spin);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_spin_button_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

SUITE(md3_selection_controls_suite) {
  RUN_TEST(test_md3_autocomplete_lifecycle);
  RUN_TEST(test_md3_select_lifecycle);
  RUN_TEST(test_md3_pin_input_lifecycle);
  RUN_TEST(test_md3_rating_lifecycle);
  RUN_TEST(test_md3_spin_button_lifecycle);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  int result;
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_selection_controls_suite);
  GREATEST_MAIN_END();
  return result;
}
