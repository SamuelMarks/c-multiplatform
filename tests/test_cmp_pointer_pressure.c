/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_pointer_pressure_create_destroy(void) {
  cmp_pointer_pressure_t *pressure = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_pressure_create(&pressure));
  ASSERT_NEQ(NULL, pressure);
  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_pressure_destroy(pressure));

  PASS();
}

TEST test_pointer_pressure_process_get(void) {
  cmp_pointer_pressure_t *pressure = NULL;
  cmp_event_t event = {0};
  cmp_stylus_data_t data;

  event.pressure = 0.75f;
  event.modifiers = 0x3; /* Both eraser and button */

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_pressure_create(&pressure));
  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_pressure_process_event(pressure, &event));
  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_pressure_get_data(pressure, &data));

  ASSERT_EQ(0.75f, data.pressure);
  ASSERT_EQ(1, data.is_eraser);
  ASSERT_EQ(1, data.button_pressed);

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_pressure_destroy(pressure));
  PASS();
}

TEST test_pointer_pressure_edge_cases(void) {
  cmp_pointer_pressure_t *pressure = NULL;
  cmp_event_t event = {0};
  cmp_stylus_data_t data;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pointer_pressure_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pointer_pressure_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_pressure_create(&pressure));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_pointer_pressure_process_event(NULL, &event));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_pointer_pressure_process_event(pressure, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pointer_pressure_get_data(NULL, &data));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_pointer_pressure_get_data(pressure, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_pressure_destroy(pressure));
  PASS();
}

SUITE(cmp_pointer_pressure_suite) {
  RUN_TEST(test_pointer_pressure_create_destroy);
  RUN_TEST(test_pointer_pressure_process_get);
  RUN_TEST(test_pointer_pressure_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_pointer_pressure_suite);
  GREATEST_MAIN_END();
}
