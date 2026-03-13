/* clang-format off */
#include "cmpc/cmp_list.h"
#include "test_utils.h"
/* clang-format on */

int main(void) {
  CMPColor color;
  CMP_TEST_EXPECT(cmp_list_test_validate_color(NULL), CMP_ERR_INVALID_ARGUMENT);
  color.r = -1.0f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 0.0f;
  CMP_TEST_OK(cmp_list_test_validate_color(&color));

  CMPLayoutEdges edges;
  CMP_TEST_EXPECT(cmp_list_test_validate_edges(NULL), CMP_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.left = 0.0f;
  edges.right = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.right = 0.0f;
  edges.top = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.top = 0.0f;
  edges.bottom = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.bottom = 0.0f;
  CMP_TEST_OK(cmp_list_test_validate_edges(&edges));

  CMPRect rect;
  CMP_TEST_EXPECT(cmp_list_test_validate_rect(NULL), CMP_ERR_INVALID_ARGUMENT);
  rect.width = -1.0f;
  rect.height = 0.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 0.0f;
  rect.height = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.height = 0.0f;
  CMP_TEST_OK(cmp_list_test_validate_rect(&rect));

  CMPMeasureSpec spec;
  spec.mode = 99;
  CMP_TEST_EXPECT(cmp_list_test_validate_measure_spec(spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(cmp_list_test_validate_measure_spec(spec), CMP_ERR_RANGE);
  spec.size = 0.0f;
  CMP_TEST_OK(cmp_list_test_validate_measure_spec(spec));

  return CMP_OK;
}
