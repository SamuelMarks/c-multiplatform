#include "m3/m3_math.h"
#include "test_utils.h"

#include <string.h>

#define M3_TEST_EPS 1.0e-4f

static int m3_float_near(M3Scalar a, M3Scalar b) {
  M3Scalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }

  return (diff <= M3_TEST_EPS) ? 1 : 0;
}

int main(void) {
  M3Rect a = {0};
  M3Rect b = {0};
  M3Rect out;
  M3Bool hit;
  M3Mat3 mat;
  M3Mat3 mat2;
  M3Mat3 mat3;
  M3Scalar out_x;
  M3Scalar out_y;
  M3Scalar angle;

  M3_TEST_EXPECT(m3_rect_intersect(NULL, &b, &out, &hit),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_rect_intersect(&a, NULL, &out, &hit),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_rect_intersect(&a, &b, NULL, &hit),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_rect_intersect(&a, &b, &out, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  a.x = 0.0f;
  a.y = 0.0f;
  a.width = -1.0f;
  a.height = 1.0f;
  b = a;
  b.width = 1.0f;
  M3_TEST_EXPECT(m3_rect_intersect(&a, &b, &out, &hit), M3_ERR_RANGE);

  a.x = 0.0f;
  a.y = 0.0f;
  a.width = 2.0f;
  a.height = 2.0f;
  b.x = 5.0f;
  b.y = 5.0f;
  b.width = 1.0f;
  b.height = 1.0f;
  M3_TEST_OK(m3_rect_intersect(&a, &b, &out, &hit));
  M3_TEST_ASSERT(hit == M3_FALSE);
  M3_TEST_ASSERT(out.width == 0.0f);

  b.x = 1.0f;
  b.y = 1.0f;
  b.width = 2.0f;
  b.height = 2.0f;
  M3_TEST_OK(m3_rect_intersect(&a, &b, &out, &hit));
  M3_TEST_ASSERT(hit == M3_TRUE);
  M3_TEST_ASSERT(m3_float_near(out.x, 1.0f));
  M3_TEST_ASSERT(m3_float_near(out.y, 1.0f));
  M3_TEST_ASSERT(m3_float_near(out.width, 1.0f));
  M3_TEST_ASSERT(m3_float_near(out.height, 1.0f));

  b.x = 2.0f;
  b.y = 0.0f;
  b.width = 1.0f;
  b.height = 1.0f;
  M3_TEST_OK(m3_rect_intersect(&a, &b, &out, &hit));
  M3_TEST_ASSERT(hit == M3_FALSE);

  M3_TEST_EXPECT(m3_rect_union(NULL, &b, &out), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_rect_union(&a, NULL, &out), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_rect_union(&a, &b, NULL), M3_ERR_INVALID_ARGUMENT);

  a.width = -1.0f;
  b.width = 1.0f;
  M3_TEST_EXPECT(m3_rect_union(&a, &b, &out), M3_ERR_RANGE);

  a.x = 0.0f;
  a.y = 0.0f;
  a.width = 2.0f;
  a.height = 2.0f;
  b.x = -1.0f;
  b.y = -2.0f;
  b.width = 1.0f;
  b.height = 1.0f;
  M3_TEST_OK(m3_rect_union(&a, &b, &out));
  M3_TEST_ASSERT(m3_float_near(out.x, -1.0f));
  M3_TEST_ASSERT(m3_float_near(out.y, -2.0f));
  M3_TEST_ASSERT(m3_float_near(out.width, 3.0f));
  M3_TEST_ASSERT(m3_float_near(out.height, 4.0f));

  M3_TEST_EXPECT(m3_rect_contains_point(NULL, 0.0f, 0.0f, &hit),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_rect_contains_point(&a, 0.0f, 0.0f, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  a.width = -1.0f;
  a.height = 1.0f;
  M3_TEST_EXPECT(m3_rect_contains_point(&a, 0.0f, 0.0f, &hit), M3_ERR_RANGE);

  a.x = 0.0f;
  a.y = 0.0f;
  a.width = 10.0f;
  a.height = 10.0f;
  M3_TEST_OK(m3_rect_contains_point(&a, 0.0f, 0.0f, &hit));
  M3_TEST_ASSERT(hit == M3_TRUE);
  M3_TEST_OK(m3_rect_contains_point(&a, -1.0f, 5.0f, &hit));
  M3_TEST_ASSERT(hit == M3_FALSE);
  M3_TEST_OK(m3_rect_contains_point(&a, 9.0f, 9.0f, &hit));
  M3_TEST_ASSERT(hit == M3_TRUE);
  M3_TEST_OK(m3_rect_contains_point(&a, 10.0f, 10.0f, &hit));
  M3_TEST_ASSERT(hit == M3_FALSE);

  a.width = 0.0f;
  a.height = 0.0f;
  M3_TEST_OK(m3_rect_contains_point(&a, 0.0f, 0.0f, &hit));
  M3_TEST_ASSERT(hit == M3_FALSE);

  M3_TEST_EXPECT(m3_mat3_identity(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_mat3_identity(&mat));
  M3_TEST_ASSERT(m3_float_near(mat.m[0], 1.0f));
  M3_TEST_ASSERT(m3_float_near(mat.m[4], 1.0f));
  M3_TEST_ASSERT(m3_float_near(mat.m[8], 1.0f));

  M3_TEST_EXPECT(m3_mat3_mul(NULL, &mat, &mat2), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_mat3_mul(&mat, NULL, &mat2), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_mat3_mul(&mat, &mat2, NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_mat3_translate(2.0f, 3.0f, &mat2));
  M3_TEST_OK(m3_mat3_mul(&mat, &mat2, &mat3));
  M3_TEST_ASSERT(m3_float_near(mat3.m[6], 2.0f));
  M3_TEST_ASSERT(m3_float_near(mat3.m[7], 3.0f));

  M3_TEST_OK(m3_mat3_mul(&mat2, &mat, &mat3));
  M3_TEST_ASSERT(m3_float_near(mat3.m[6], 2.0f));
  M3_TEST_ASSERT(m3_float_near(mat3.m[7], 3.0f));

  M3_TEST_OK(m3_mat3_scale(2.0f, 3.0f, &mat2));
  M3_TEST_OK(m3_mat3_translate(5.0f, 7.0f, &mat3));
  M3_TEST_OK(m3_mat3_mul(&mat3, &mat2, &mat));
  M3_TEST_OK(m3_mat3_transform_point(&mat, 1.0f, 1.0f, &out_x, &out_y));
  M3_TEST_ASSERT(m3_float_near(out_x, 7.0f));
  M3_TEST_ASSERT(m3_float_near(out_y, 10.0f));

  M3_TEST_EXPECT(m3_mat3_translate(0.0f, 0.0f, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_mat3_scale(0.0f, 0.0f, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_mat3_rotate(0.0f, NULL), M3_ERR_INVALID_ARGUMENT);

  angle = (M3Scalar)(3.14159265f / 2.0f);
  M3_TEST_OK(m3_mat3_rotate(angle, &mat));
  M3_TEST_OK(m3_mat3_transform_point(&mat, 1.0f, 0.0f, &out_x, &out_y));
  M3_TEST_ASSERT(m3_float_near(out_x, 0.0f));
  M3_TEST_ASSERT(m3_float_near(out_y, 1.0f));

  M3_TEST_EXPECT(m3_mat3_transform_point(NULL, 0.0f, 0.0f, &out_x, &out_y),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_mat3_transform_point(&mat, 0.0f, 0.0f, NULL, &out_y),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_mat3_transform_point(&mat, 0.0f, 0.0f, &out_x, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  memset(&mat, 0, sizeof(mat));
  M3_TEST_EXPECT(m3_mat3_transform_point(&mat, 1.0f, 1.0f, &out_x, &out_y),
                 M3_ERR_RANGE);

  return 0;
}
