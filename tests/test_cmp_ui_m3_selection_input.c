/* clang-format off */
#include "greatest.h"
#include "cmp_ui_switch.h"
#include "cmp_ui_text_field.h"
#include "cmp_ui_chip.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_switch(void) {
  cmp_ui_switch_t *sw;
  cmp_ui_node_t *node;

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_switch_create(&sw), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_switch_get_node(sw, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_switch_set_on(sw, 1), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_switch_set_on(sw, 0), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_switch_destroy(sw), "%d");
  PASS();
}

TEST test_text_field(void) {
  cmp_ui_text_field_t *field;
  cmp_ui_node_t *node;

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_text_field_create(&field, "Username"),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_text_field_get_node(field, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_text_field_set_value(field, "John Doe"),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_text_field_set_value(field, NULL), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_text_field_destroy(field), "%d");
  PASS();
}

TEST test_checkbox_radio_slider(void) {
  cmp_ui_node_t *checkbox = NULL;
  cmp_ui_node_t *radio = NULL;
  cmp_ui_node_t *slider = NULL;

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_checkbox_create(&checkbox, "Agree"), "%d");
  ASSERT(checkbox != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_radio_create(&radio, 1), "%d");
  ASSERT(radio != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_slider_create(&slider, 0.0f, 100.0f), "%d");
  ASSERT(slider != NULL);

  cmp_ui_node_destroy(checkbox);
  cmp_ui_node_destroy(radio);
  cmp_ui_node_destroy(slider);

  PASS();
}

TEST test_chip(void) {
  cmp_ui_chip_t *chip;
  cmp_ui_node_t *node;

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_ui_chip_create(NULL, "Filter", 0xFF000000, 0xFFFFFFFF),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_chip_create(&chip, "Filter", 0xFF000000, 0xFFFFFFFF),
                "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_chip_get_node(NULL, &node), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_chip_get_node(chip, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_chip_get_node(chip, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_chip_set_text(NULL, "Updated"),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_chip_set_text(chip, "Updated"), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_chip_set_text(chip, NULL), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_chip_destroy(NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_chip_destroy(chip), "%d");
  PASS();
}

SUITE(m3_selection_input_suite) {
  RUN_TEST(test_switch);
  RUN_TEST(test_text_field);
  RUN_TEST(test_checkbox_radio_slider);
  RUN_TEST(test_chip);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(m3_selection_input_suite);
  GREATEST_MAIN_END();
}