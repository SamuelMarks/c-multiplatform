#include "cupertino/cupertino_blur.h"
#include "../phase1/test_utils.h"
#include <stddef.h>

static int test_cupertino_blur(void) {
  CupertinoBlurEffect effect;

  CMP_TEST_EXPECT(
      cupertino_blur_get_effect(CUPERTINO_BLUR_STYLE_NORMAL, CMP_FALSE, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cupertino_blur_get_effect((CupertinoBlurStyle)999, CMP_FALSE, &effect),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cupertino_blur_get_effect(CUPERTINO_BLUR_STYLE_NORMAL, CMP_FALSE,
                                        &effect));
  CMP_TEST_ASSERT(effect.blur_radius == 30.0f);
  CMP_TEST_ASSERT(effect.tint_color.a == 0.55f);
  CMP_TEST_ASSERT(effect.base_color.r == 0.85f);

  CMP_TEST_OK(cupertino_blur_get_effect(CUPERTINO_BLUR_STYLE_ULTRA_THIN,
                                        CMP_TRUE, &effect));
  CMP_TEST_ASSERT(effect.blur_radius == 10.0f);
  CMP_TEST_ASSERT(effect.tint_color.a == 0.25f);
  CMP_TEST_ASSERT(effect.tint_color.r == 0.15f);

  CMP_TEST_OK(cupertino_blur_get_effect(CUPERTINO_BLUR_STYLE_CHROME, CMP_FALSE,
                                        &effect));
  CMP_TEST_ASSERT(effect.blur_radius == 30.0f);
  CMP_TEST_ASSERT(effect.tint_color.a == 0.85f);

  return 0;
}

int main(void) {
  if (test_cupertino_blur() != 0)
    return 1;
  return 0;
}
