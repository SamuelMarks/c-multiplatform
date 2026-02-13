#include "cmpc/cmp_layout.h"
#include "test_utils.h"

int main(void) {
  CMPLayoutDirection direction;
  CMPLayoutDirection direction_ltr;
  CMPLayoutDirection direction_rtl;
  CMPLayoutMeasureSpec spec_exact_100;
  CMPLayoutMeasureSpec spec_exact_60;
  CMPLayoutMeasureSpec spec_exact_50;
  CMPLayoutStyle style;
  CMPLayoutNode container;
  CMPLayoutNode child1;
  CMPLayoutNode child2;
  CMPLayoutNode *children[2];
  CMPRect rect;
  int rc;

  CMP_TEST_EXPECT(cmp_layout_direction_init(NULL, CMP_DIRECTION_LTR),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_layout_direction_init(&direction, 99),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_layout_direction_init(&direction_ltr, CMP_DIRECTION_LTR));
  CMP_TEST_OK(cmp_layout_direction_init(&direction_rtl, CMP_DIRECTION_RTL));

  CMP_TEST_OK(cmp_layout_measure_spec_init(&spec_exact_100,
                                           CMP_LAYOUT_MEASURE_EXACTLY, 100.0f));
  CMP_TEST_OK(cmp_layout_measure_spec_init(&spec_exact_60,
                                           CMP_LAYOUT_MEASURE_EXACTLY, 60.0f));
  CMP_TEST_OK(cmp_layout_measure_spec_init(&spec_exact_50,
                                           CMP_LAYOUT_MEASURE_EXACTLY, 50.0f));

  CMP_TEST_OK(cmp_layout_style_init(&style));
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  direction.flow = 99;
  rc =
      cmp_layout_compute(&container, &direction, spec_exact_100, spec_exact_50);
  CMP_TEST_ASSERT(rc == CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_layout_compute(&container, NULL, spec_exact_100, spec_exact_50),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_START;
  style.padding.left = 5.0f;
  style.padding.right = 15.0f;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  child1.style.width = 10.0f;
  child1.style.height = 10.0f;
  CMP_TEST_OK(cmp_layout_node_init(&child2, &style));
  child2.style.width = 20.0f;
  child2.style.height = 10.0f;
  children[0] = &child1;
  children[1] = &child2;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));

  CMP_TEST_OK(cmp_layout_compute(&container, &direction_rtl, spec_exact_100,
                                 spec_exact_50));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 85.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 65.0f);

  CMP_TEST_OK(cmp_layout_compute(&container, &direction_ltr, spec_exact_100,
                                 spec_exact_50));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 5.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 15.0f);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.align_main = CMP_LAYOUT_ALIGN_END;
  style.align_cross = CMP_LAYOUT_ALIGN_START;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  child1.style.width = 10.0f;
  child1.style.height = 10.0f;
  CMP_TEST_OK(cmp_layout_node_init(&child2, &style));
  child2.style.width = 20.0f;
  child2.style.height = 10.0f;
  children[0] = &child1;
  children[1] = &child2;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));

  CMP_TEST_OK(cmp_layout_compute(&container, &direction_rtl, spec_exact_100,
                                 spec_exact_50));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 20.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 0.0f);

  CMP_TEST_OK(cmp_layout_compute(&container, &direction_ltr, spec_exact_100,
                                 spec_exact_50));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 70.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 80.0f);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.wrap = CMP_LAYOUT_WRAP_YES;
  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_CENTER;
  style.padding.left = 4.0f;
  style.padding.right = 6.0f;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  child1.style.width = 10.0f;
  child1.style.height = 10.0f;
  CMP_TEST_OK(cmp_layout_node_init(&child2, &style));
  child2.style.width = 20.0f;
  child2.style.height = 10.0f;
  children[0] = &child1;
  children[1] = &child2;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));

  CMP_TEST_OK(cmp_layout_compute(&container, &direction_rtl, spec_exact_100,
                                 spec_exact_60));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 81.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 76.0f);

  CMP_TEST_OK(cmp_layout_compute(&container, &direction_ltr, spec_exact_100,
                                 spec_exact_60));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 9.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 4.0f);

  CMP_TEST_OK(cmp_layout_style_init(&style));
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.wrap = CMP_LAYOUT_WRAP_NO;
  style.align_main = CMP_LAYOUT_ALIGN_START;
  style.align_cross = CMP_LAYOUT_ALIGN_END;
  CMP_TEST_OK(cmp_layout_node_init(&container, &style));
  CMP_TEST_OK(cmp_layout_node_init(&child1, &style));
  child1.style.width = 10.0f;
  child1.style.height = 10.0f;
  CMP_TEST_OK(cmp_layout_node_init(&child2, &style));
  child2.style.width = 20.0f;
  child2.style.height = 10.0f;
  children[0] = &child1;
  children[1] = &child2;
  CMP_TEST_OK(cmp_layout_node_set_children(&container, children, 2));

  CMP_TEST_OK(cmp_layout_compute(&container, &direction_rtl, spec_exact_100,
                                 spec_exact_60));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 0.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 0.0f);

  CMP_TEST_OK(cmp_layout_compute(&container, &direction_ltr, spec_exact_100,
                                 spec_exact_60));
  CMP_TEST_OK(cmp_layout_node_get_layout(&child1, &rect));
  CMP_TEST_ASSERT(rect.x == 90.0f);
  CMP_TEST_OK(cmp_layout_node_get_layout(&child2, &rect));
  CMP_TEST_ASSERT(rect.x == 80.0f);

  return 0;
}
