/* clang-format off */
#include "cupertino/cupertino_shape.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_shape_squircle(void) {
  CMPPath path = {0};
  CMPRect bounds = {10.0f, 10.0f, 100.0f, 100.0f};
  CMPAllocator alloc;

  CMP_TEST_EXPECT(
      cupertino_shape_append_squircle(NULL, bounds, 10.0f, 10.0f, 10.0f, 10.0f),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));
  CMP_TEST_OK(cmp_path_init(&path, &alloc, 16));

  CMP_TEST_OK(cupertino_shape_append_squircle(&path, bounds, 10.0f, 10.0f,
                                              10.0f, 10.0f));
  CMP_TEST_OK(cupertino_shape_append_squircle(
      &path, bounds, 60.0f, 60.0f, 60.0f, 60.0f)); /* Test clamping */
  CMP_TEST_OK(cupertino_shape_append_squircle(&path, bounds, 0.0f, 0.0f, 0.0f,
                                              0.0f)); /* Test zero radii */

  CMP_TEST_OK(cmp_path_shutdown(&path));
  return 0;
}

int main(void) {
  if (test_cupertino_shape_squircle() != 0)
    return 1;
  return 0;
}
