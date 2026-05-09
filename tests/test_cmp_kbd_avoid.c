/* clang-format off */
#include "greatest.h"
#include "cmp.h"
/* clang-format on */

TEST test_kbd_avoid_lifecycle(void) {
  cmp_keyboard_avoidance_t *avoider = NULL;
  float offset;

  ASSERT_EQ(CMP_SUCCESS, cmp_keyboard_avoidance_create(&avoider));
  ASSERT_NEQ(NULL, avoider);

  /* Hidden keyboard */
  ASSERT_EQ(CMP_SUCCESS, cmp_keyboard_avoidance_compute_offset(
                             avoider, 0.0f, 500.0f, 800.0f, &offset));
  ASSERT_EQ(0.0f, offset);

  /* Visible keyboard, overlapping input */
  /* Screen = 800, Kbd = 300, Input Y = 600.
     Visible space = 800 - 300 = 500.
     Input is at 600. Overlap = (600 + 16) - 500 = 116. */
  ASSERT_EQ(CMP_SUCCESS, cmp_keyboard_avoidance_compute_offset(
                             avoider, 300.0f, 600.0f, 800.0f, &offset));
  ASSERT_EQ(116.0f, offset);

  ASSERT_EQ(CMP_SUCCESS, cmp_keyboard_avoidance_destroy(avoider));
  PASS();
}

TEST test_kbd_avoid_null_args(void) {
  cmp_keyboard_avoidance_t *avoider = NULL;
  float offset;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_keyboard_avoidance_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_keyboard_avoidance_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_keyboard_avoidance_create(&avoider));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_keyboard_avoidance_compute_offset(
                                       NULL, 100.0f, 100.0f, 800.0f, &offset));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_keyboard_avoidance_compute_offset(
                                       avoider, 100.0f, 100.0f, 800.0f, NULL));

  /* invalid internal args tests iOS logic fallback */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_keyboard_avoidance_compute_offset(avoider, 100.0f, -1.0f,
                                                  800.0f, &offset));

  cmp_keyboard_avoidance_destroy(avoider);
  PASS();
}

SUITE(cmp_kbd_avoid_suite) {
  RUN_TEST(test_kbd_avoid_lifecycle);
  RUN_TEST(test_kbd_avoid_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_kbd_avoid_suite);
  GREATEST_MAIN_END();
}
