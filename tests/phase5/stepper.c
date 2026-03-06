#include "m3/m3_stepper.h"
#include "test_utils.h"

extern int CMP_CALL m3_stepper_test_set_fail_point(cmp_u32 fail_point);
extern int CMP_CALL m3_stepper_test_clear_fail_points(void);

static int test_stepper_init(void) {
  M3Stepper stepper;

  CMP_TEST_EXPECT(m3_stepper_init(NULL), CMP_ERR_INVALID_ARGUMENT);

  m3_stepper_test_set_fail_point(1u);
  CMP_TEST_EXPECT(m3_stepper_init(&stepper), CMP_ERR_OUT_OF_MEMORY);
  m3_stepper_test_clear_fail_points();

  CMP_TEST_OK(m3_stepper_init(&stepper));
  CMP_TEST_ASSERT(stepper.step_count == 0);
  CMP_TEST_ASSERT(stepper.current_step == 0);

  return 0;
}

static int test_stepper_draw(void) {
  M3Stepper stepper;
  CMPPaintContext ctx;

  CMP_TEST_OK(m3_stepper_init(&stepper));

  CMP_TEST_EXPECT(m3_stepper_draw(NULL, &stepper), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_stepper_draw(&ctx, NULL), CMP_ERR_INVALID_ARGUMENT);

  m3_stepper_test_set_fail_point(2u);
  CMP_TEST_EXPECT(m3_stepper_draw(&ctx, &stepper), CMP_ERR_OUT_OF_MEMORY);
  m3_stepper_test_clear_fail_points();

  /* Empty stepper */
  CMP_TEST_OK(m3_stepper_draw(&ctx, &stepper));

  /* Non-empty stepper */
  stepper.step_count = 1;
  M3Step step = {"Step 1", NULL, M3_STEP_STATE_INCOMPLETE};
  stepper.steps = &step;

  CMP_TEST_OK(m3_stepper_draw(&ctx, &stepper));

  return 0;
}

static int test_stepper_cleanup(void) {
  M3Stepper stepper;
  CMP_TEST_OK(m3_stepper_init(&stepper));

  CMP_TEST_EXPECT(m3_stepper_cleanup(NULL), CMP_ERR_INVALID_ARGUMENT);

  m3_stepper_test_set_fail_point(3u);
  CMP_TEST_EXPECT(m3_stepper_cleanup(&stepper), CMP_ERR_OUT_OF_MEMORY);
  m3_stepper_test_clear_fail_points();

  CMP_TEST_OK(m3_stepper_cleanup(&stepper));

  return 0;
}

int main(void) {
  int result = 0;
  result |= test_stepper_init();
  result |= test_stepper_draw();
  result |= test_stepper_cleanup();
  return result == 0 ? 0 : 1;
}