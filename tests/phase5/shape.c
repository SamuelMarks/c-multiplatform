/* clang-format off */
#include "m3/m3_shape.h"
#include "test_utils.h"
/* clang-format on */

static int test_shape_init(void) {
  M3Shape shape;
  M3CornerSize tl = {M3_SHAPE_SIZE_ABSOLUTE, 10.0f};
  M3CornerSize tr = {M3_SHAPE_SIZE_PERCENT, 0.5f};
  M3CornerSize br = {M3_SHAPE_SIZE_ABSOLUTE, 20.0f};
  M3CornerSize bl = {M3_SHAPE_SIZE_PERCENT, 0.25f};

  CMP_TEST_EXPECT(m3_shape_init_absolute(NULL, 10.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_shape_init_absolute(&shape, 10.0f));
  CMP_TEST_ASSERT(shape.top_left.type == M3_SHAPE_SIZE_ABSOLUTE);
  CMP_TEST_ASSERT(shape.top_left.value == 10.0f);
  CMP_TEST_ASSERT(shape.bottom_right.value == 10.0f);

  CMP_TEST_EXPECT(m3_shape_init_percent(NULL, 0.5f), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_shape_init_percent(&shape, 0.5f));
  CMP_TEST_ASSERT(shape.top_left.type == M3_SHAPE_SIZE_PERCENT);
  CMP_TEST_ASSERT(shape.top_left.value == 0.5f);
  CMP_TEST_ASSERT(shape.bottom_right.value == 0.5f);

  CMP_TEST_EXPECT(m3_shape_init_asymmetrical(NULL, tl, tr, br, bl),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_shape_init_asymmetrical(&shape, tl, tr, br, bl));
  CMP_TEST_ASSERT(shape.top_left.type == M3_SHAPE_SIZE_ABSOLUTE &&
                  shape.top_left.value == 10.0f);
  CMP_TEST_ASSERT(shape.top_right.type == M3_SHAPE_SIZE_PERCENT &&
                  shape.top_right.value == 0.5f);
  CMP_TEST_ASSERT(shape.bottom_right.type == M3_SHAPE_SIZE_ABSOLUTE &&
                  shape.bottom_right.value == 20.0f);
  CMP_TEST_ASSERT(shape.bottom_left.type == M3_SHAPE_SIZE_PERCENT &&
                  shape.bottom_left.value == 0.25f);

  return 0;
}

static int test_shape_resolve(void) {
  M3Shape shape;
  CMPRect bounds = {0, 0, 100.0f, 200.0f}; /* shortest edge is 100 */
  CMPRect bad_bounds = {0, 0, -10.0f, -20.0f};
  CMPScalar tl, tr, br, bl;

  M3CornerSize c_tl = {M3_SHAPE_SIZE_ABSOLUTE, 10.0f};
  M3CornerSize c_tr = {M3_SHAPE_SIZE_PERCENT, 0.5f};
  M3CornerSize c_br = {M3_SHAPE_SIZE_ABSOLUTE, 20.0f};
  M3CornerSize c_bl = {M3_SHAPE_SIZE_PERCENT, 0.25f};

  m3_shape_init_asymmetrical(&shape, c_tl, c_tr, c_br, c_bl);

  CMP_TEST_EXPECT(m3_shape_resolve(NULL, bounds, &tl, &tr, &br, &bl),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_shape_resolve(&shape, bounds, &tl, &tr, &br, &bl));
  CMP_TEST_ASSERT(tl == 10.0f);
  CMP_TEST_ASSERT(tr == 50.0f); /* 100 * 0.5 */
  CMP_TEST_ASSERT(br == 20.0f);
  CMP_TEST_ASSERT(bl == 25.0f); /* 100 * 0.25 */

  /* Handle negative bounds gracefully by clamping to 0 */
  CMP_TEST_OK(m3_shape_resolve(&shape, bad_bounds, &tl, &tr, &br, &bl));
  CMP_TEST_ASSERT(tr == 0.0f);
  CMP_TEST_ASSERT(bl == 0.0f);

  /* Allow NULL pointers for out parameters */
  CMP_TEST_OK(m3_shape_resolve(&shape, bounds, NULL, NULL, NULL, NULL));

  return 0;
}

static int test_shape_morph(void) {
  M3Shape shape1, shape2, out;
  M3CornerSize c_tl1 = {M3_SHAPE_SIZE_ABSOLUTE, 10.0f};
  M3CornerSize c_tr1 = {M3_SHAPE_SIZE_PERCENT, 0.0f};
  M3CornerSize c_tl2 = {M3_SHAPE_SIZE_ABSOLUTE, 30.0f};
  M3CornerSize c_tr2 = {M3_SHAPE_SIZE_ABSOLUTE, 50.0f}; /* different type */

  m3_shape_init_asymmetrical(&shape1, c_tl1, c_tr1, c_tl1, c_tr1);
  m3_shape_init_asymmetrical(&shape2, c_tl2, c_tr2, c_tl2, c_tr2);

  CMP_TEST_EXPECT(m3_shape_morph(NULL, &shape2, 0.5f, &out),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_shape_morph(&shape1, NULL, 0.5f, &out),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_shape_morph(&shape1, &shape2, 0.5f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_shape_morph(&shape1, &shape2, 0.5f, &out));

  /* Same type interpolation */
  CMP_TEST_ASSERT(out.top_left.type == M3_SHAPE_SIZE_ABSOLUTE);
  CMP_TEST_ASSERT(out.top_left.value == 20.0f); /* (10 + 30) / 2 */

  /* Mixed type interpolation */
  CMP_TEST_ASSERT(out.top_right.type ==
                  M3_SHAPE_SIZE_ABSOLUTE);       /* Swaps at 0.5 */
  CMP_TEST_ASSERT(out.top_right.value == 25.0f); /* (0 + 50) / 2 */

  /* Bounds check fraction */
  CMP_TEST_OK(m3_shape_morph(&shape1, &shape2, -1.0f, &out));
  CMP_TEST_ASSERT(out.top_left.value == 10.0f);
  CMP_TEST_OK(m3_shape_morph(&shape1, &shape2, 2.0f, &out));
  CMP_TEST_ASSERT(out.top_left.value == 30.0f);

  return 0;
}

static int test_shape_scales(void) {
  M3Shape shape;

  CMP_TEST_OK(m3_shape_scale_none(&shape));
  CMP_TEST_ASSERT(shape.top_left.value == 0.0f &&
                  shape.top_left.type == M3_SHAPE_SIZE_ABSOLUTE);

  CMP_TEST_OK(m3_shape_scale_extra_small(&shape));
  CMP_TEST_ASSERT(shape.top_left.value == 4.0f &&
                  shape.top_left.type == M3_SHAPE_SIZE_ABSOLUTE);

  CMP_TEST_OK(m3_shape_scale_small(&shape));
  CMP_TEST_ASSERT(shape.top_left.value == 8.0f &&
                  shape.top_left.type == M3_SHAPE_SIZE_ABSOLUTE);

  CMP_TEST_OK(m3_shape_scale_medium(&shape));
  CMP_TEST_ASSERT(shape.top_left.value == 12.0f &&
                  shape.top_left.type == M3_SHAPE_SIZE_ABSOLUTE);

  CMP_TEST_OK(m3_shape_scale_large(&shape));
  CMP_TEST_ASSERT(shape.top_left.value == 16.0f &&
                  shape.top_left.type == M3_SHAPE_SIZE_ABSOLUTE);

  CMP_TEST_OK(m3_shape_scale_extra_large(&shape));
  CMP_TEST_ASSERT(shape.top_left.value == 28.0f &&
                  shape.top_left.type == M3_SHAPE_SIZE_ABSOLUTE);

  CMP_TEST_OK(m3_shape_scale_full(&shape));
  CMP_TEST_ASSERT(shape.top_left.value == 0.5f &&
                  shape.top_left.type == M3_SHAPE_SIZE_PERCENT);

  return 0;
}

int main(void) {
  CMP_TEST_ASSERT(test_shape_init() == 0);
  CMP_TEST_ASSERT(test_shape_resolve() == 0);
  CMP_TEST_ASSERT(test_shape_morph() == 0);
  CMP_TEST_ASSERT(test_shape_scales() == 0);
  return 0;
}