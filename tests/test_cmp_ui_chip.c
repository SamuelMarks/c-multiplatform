/* clang-format off */
#include "cmp.h"
#include "cmp_ui_chip.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_chip_suite);

TEST test_ui_chip_lifecycle(void) {
  cmp_ui_chip_t *chip = NULL;
  cmp_ui_node_t *node = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_chip_create(NULL, "Chip", 0xFF000000, 0xFFFFFFFF));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_chip_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_chip_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_chip_set_text(NULL, "new text"));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_chip_create(&chip, "Test Chip", 0xFF000000, 0xFFFFFFFF));
  ASSERT_NEQ(NULL, chip);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_chip_get_node(chip, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_chip_get_node(chip, &node));
  ASSERT_NEQ(NULL, node);

  /* Set text */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_chip_set_text(chip, "Updated"));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_chip_set_text(chip, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_chip_destroy(chip));

  PASS();
}

TEST test_ui_chip_null_text_init(void) {
  cmp_ui_chip_t *chip = NULL;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_chip_create(&chip, NULL, 0xFF000000, 0xFFFFFFFF));
  ASSERT_NEQ(NULL, chip);
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_chip_destroy(chip));

  PASS();
}

SUITE(cmp_ui_chip_suite) {
  RUN_TEST(test_ui_chip_lifecycle);
  RUN_TEST(test_ui_chip_null_text_init);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_chip_suite);
  GREATEST_MAIN_END();
}