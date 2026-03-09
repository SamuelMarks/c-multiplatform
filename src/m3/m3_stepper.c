#include "m3/m3_stepper.h"

#include <string.h>

#ifdef CMP_TESTING
static cmp_u32 g_m3_stepper_test_fail_point = 0u;

CMP_API int CMP_CALL m3_stepper_test_set_fail_point(cmp_u32 fail_point);
CMP_API int CMP_CALL m3_stepper_test_clear_fail_points(void);

int CMP_CALL m3_stepper_test_set_fail_point(cmp_u32 fail_point) {
  g_m3_stepper_test_fail_point = fail_point;
  return CMP_OK;
}

int CMP_CALL m3_stepper_test_clear_fail_points(void) {
  g_m3_stepper_test_fail_point = 0u;
  return CMP_OK;
}
#endif

CMP_API int CMP_CALL m3_stepper_init(M3Stepper *stepper) {
  if (stepper == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_m3_stepper_test_fail_point == 1u) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
#endif

  stepper->bounds.x = 0;
  stepper->bounds.y = 0;
  stepper->bounds.width = 0;
  stepper->bounds.height = 0;
  stepper->steps = NULL;
  stepper->step_count = 0;
  stepper->current_step = 0;
  stepper->vertical = CMP_FALSE;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_stepper_draw(CMPPaintContext *ctx,
                                     const M3Stepper *stepper) {
  if (ctx == NULL || stepper == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_m3_stepper_test_fail_point == 2u) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
#endif

  if (stepper->step_count == 0 || stepper->steps == NULL) {
    return CMP_OK; /* Nothing to draw */
  }

  /* Dummy drawing for a stepper */

  return CMP_OK;
}

CMP_API int CMP_CALL m3_stepper_cleanup(M3Stepper *stepper) {
  if (stepper == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_m3_stepper_test_fail_point == 3u) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
#endif

  /* User owns steps, nothing to free internally */
  stepper->steps = NULL;

  return CMP_OK;
}