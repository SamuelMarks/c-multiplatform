#include "cmpc/cmp_math.h"
#include "test_utils.h"

#include <string.h>

#define CMP_TEST_EPS 1.0e-4f

static int cmp_float_near(CMPScalar a, CMPScalar b) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }

  return (diff <= CMP_TEST_EPS) ? 1 : 0;
}

int main(void) {
  CMPRect a = {0};
  CMPRect b = {0};
  CMPRect out;
  CMPBool hit;
  CMPMat3 mat;
  CMPMat3 mat2;
  CMPMat3 mat3;
  CMPScalar out_x;
  CMPScalar out_y;
  CMPScalar angle;

  CMP_TEST_EXPECT(cmp_rect_intersect(NULL, &b, &out, &hit),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_rect_intersect(&a, NULL, &out, &hit),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_rect_intersect(&a, &b, NULL, &hit),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_rect_intersect(&a, &b, &out, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  a.x = 0.0f;
  a.y = 0.0f;
  a.width = -1.0f;
  a.height = 1.0f;
  b = a;
  b.width = 1.0f;
  CMP_TEST_EXPECT(cmp_rect_intersect(&a, &b, &out, &hit), CMP_ERR_RANGE);

  a.x = 0.0f;
  a.y = 0.0f;
  a.width = 2.0f;
  a.height = 2.0f;
  b.x = 5.0f;
  b.y = 5.0f;
  b.width = 1.0f;
  b.height = 1.0f;
  CMP_TEST_OK(cmp_rect_intersect(&a, &b, &out, &hit));
  CMP_TEST_ASSERT(hit == CMP_FALSE);
  CMP_TEST_ASSERT(out.width == 0.0f);

  b.x = 1.0f;
  b.y = 1.0f;
  b.width = 2.0f;
  b.height = 2.0f;
  CMP_TEST_OK(cmp_rect_intersect(&a, &b, &out, &hit));
  CMP_TEST_ASSERT(hit == CMP_TRUE);
  CMP_TEST_ASSERT(cmp_float_near(out.x, 1.0f));
  CMP_TEST_ASSERT(cmp_float_near(out.y, 1.0f));
  CMP_TEST_ASSERT(cmp_float_near(out.width, 1.0f));
  CMP_TEST_ASSERT(cmp_float_near(out.height, 1.0f));

  b.x = 2.0f;
  b.y = 0.0f;
  b.width = 1.0f;
  b.height = 1.0f;
  CMP_TEST_OK(cmp_rect_intersect(&a, &b, &out, &hit));
  CMP_TEST_ASSERT(hit == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_rect_union(NULL, &b, &out), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_rect_union(&a, NULL, &out), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_rect_union(&a, &b, NULL), CMP_ERR_INVALID_ARGUMENT);

  a.width = -1.0f;
  b.width = 1.0f;
  CMP_TEST_EXPECT(cmp_rect_union(&a, &b, &out), CMP_ERR_RANGE);

  a.x = 0.0f;
  a.y = 0.0f;
  a.width = 2.0f;
  a.height = 2.0f;
  b.x = -1.0f;
  b.y = -2.0f;
  b.width = 1.0f;
  b.height = 1.0f;
  CMP_TEST_OK(cmp_rect_union(&a, &b, &out));
  CMP_TEST_ASSERT(cmp_float_near(out.x, -1.0f));
  CMP_TEST_ASSERT(cmp_float_near(out.y, -2.0f));
  CMP_TEST_ASSERT(cmp_float_near(out.width, 3.0f));
  CMP_TEST_ASSERT(cmp_float_near(out.height, 4.0f));

  CMP_TEST_EXPECT(cmp_rect_contains_point(NULL, 0.0f, 0.0f, &hit),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_rect_contains_point(&a, 0.0f, 0.0f, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  a.width = -1.0f;
  a.height = 1.0f;
  CMP_TEST_EXPECT(cmp_rect_contains_point(&a, 0.0f, 0.0f, &hit), CMP_ERR_RANGE);

  a.x = 0.0f;
  a.y = 0.0f;
  a.width = 10.0f;
  a.height = 10.0f;
  CMP_TEST_OK(cmp_rect_contains_point(&a, 0.0f, 0.0f, &hit));
  CMP_TEST_ASSERT(hit == CMP_TRUE);
  CMP_TEST_OK(cmp_rect_contains_point(&a, -1.0f, 5.0f, &hit));
  CMP_TEST_ASSERT(hit == CMP_FALSE);
  CMP_TEST_OK(cmp_rect_contains_point(&a, 9.0f, 9.0f, &hit));
  CMP_TEST_ASSERT(hit == CMP_TRUE);
  CMP_TEST_OK(cmp_rect_contains_point(&a, 10.0f, 10.0f, &hit));
  CMP_TEST_ASSERT(hit == CMP_FALSE);

  a.width = 0.0f;
  a.height = 0.0f;
  CMP_TEST_OK(cmp_rect_contains_point(&a, 0.0f, 0.0f, &hit));
  CMP_TEST_ASSERT(hit == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_mat3_identity(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_mat3_identity(&mat));
  CMP_TEST_ASSERT(cmp_float_near(mat.m[0], 1.0f));
  CMP_TEST_ASSERT(cmp_float_near(mat.m[4], 1.0f));
  CMP_TEST_ASSERT(cmp_float_near(mat.m[8], 1.0f));

  CMP_TEST_EXPECT(cmp_mat3_mul(NULL, &mat, &mat2), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_mat3_mul(&mat, NULL, &mat2), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_mat3_mul(&mat, &mat2, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_mat3_translate(2.0f, 3.0f, &mat2));
  CMP_TEST_OK(cmp_mat3_mul(&mat, &mat2, &mat3));
  CMP_TEST_ASSERT(cmp_float_near(mat3.m[6], 2.0f));
  CMP_TEST_ASSERT(cmp_float_near(mat3.m[7], 3.0f));

  CMP_TEST_OK(cmp_mat3_mul(&mat2, &mat, &mat3));
  CMP_TEST_ASSERT(cmp_float_near(mat3.m[6], 2.0f));
  CMP_TEST_ASSERT(cmp_float_near(mat3.m[7], 3.0f));

  CMP_TEST_OK(cmp_mat3_scale(2.0f, 3.0f, &mat2));
  CMP_TEST_OK(cmp_mat3_translate(5.0f, 7.0f, &mat3));
  CMP_TEST_OK(cmp_mat3_mul(&mat3, &mat2, &mat));
  CMP_TEST_OK(cmp_mat3_transform_point(&mat, 1.0f, 1.0f, &out_x, &out_y));
  CMP_TEST_ASSERT(cmp_float_near(out_x, 7.0f));
  CMP_TEST_ASSERT(cmp_float_near(out_y, 10.0f));

  CMP_TEST_EXPECT(cmp_mat3_translate(0.0f, 0.0f, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_mat3_scale(0.0f, 0.0f, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_mat3_rotate(0.0f, NULL), CMP_ERR_INVALID_ARGUMENT);

  angle = (CMPScalar)(3.14159265f / 2.0f);
  CMP_TEST_OK(cmp_mat3_rotate(angle, &mat));
  CMP_TEST_OK(cmp_mat3_transform_point(&mat, 1.0f, 0.0f, &out_x, &out_y));
  CMP_TEST_ASSERT(cmp_float_near(out_x, 0.0f));
  CMP_TEST_ASSERT(cmp_float_near(out_y, 1.0f));

  CMP_TEST_EXPECT(cmp_mat3_transform_point(NULL, 0.0f, 0.0f, &out_x, &out_y),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_mat3_transform_point(&mat, 0.0f, 0.0f, NULL, &out_y),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_mat3_transform_point(&mat, 0.0f, 0.0f, &out_x, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  memset(&mat, 0, sizeof(mat));
  CMP_TEST_EXPECT(cmp_mat3_transform_point(&mat, 1.0f, 1.0f, &out_x, &out_y),
                 CMP_ERR_RANGE);

  return 0;
}
