#include "m3/m3_layout.h"
#include "test_utils.h"

int main(void) {
  M3LayoutDirection direction;
  M3LayoutDirection direction_ltr;
  M3LayoutDirection direction_rtl;
  M3LayoutMeasureSpec spec_exact_100;
  M3LayoutMeasureSpec spec_exact_60;
  M3LayoutMeasureSpec spec_exact_50;
  M3LayoutStyle style;
  M3LayoutNode container;
  M3LayoutNode child1;
  M3LayoutNode child2;
  M3LayoutNode *children[2];
  M3Rect rect;
  int rc;

  M3_TEST_EXPECT(m3_layout_direction_init(NULL, M3_DIRECTION_LTR),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_layout_direction_init(&direction, 99),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_layout_direction_init(&direction_ltr, M3_DIRECTION_LTR));
  M3_TEST_OK(m3_layout_direction_init(&direction_rtl, M3_DIRECTION_RTL));

  M3_TEST_OK(m3_layout_measure_spec_init(&spec_exact_100,
                                         M3_LAYOUT_MEASURE_EXACTLY, 100.0f));
  M3_TEST_OK(m3_layout_measure_spec_init(&spec_exact_60,
                                         M3_LAYOUT_MEASURE_EXACTLY, 60.0f));
  M3_TEST_OK(m3_layout_measure_spec_init(&spec_exact_50,
                                         M3_LAYOUT_MEASURE_EXACTLY, 50.0f));

  M3_TEST_OK(m3_layout_style_init(&style));
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  direction.flow = 99;
  rc = m3_layout_compute(&container, &direction, spec_exact_100, spec_exact_50);
  M3_TEST_ASSERT(rc == M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_layout_compute(&container, NULL, spec_exact_100, spec_exact_50),
      M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_START;
  style.padding.left = 5.0f;
  style.padding.right = 15.0f;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  child1.style.width = 10.0f;
  child1.style.height = 10.0f;
  M3_TEST_OK(m3_layout_node_init(&child2, &style));
  child2.style.width = 20.0f;
  child2.style.height = 10.0f;
  children[0] = &child1;
  children[1] = &child2;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));

  M3_TEST_OK(m3_layout_compute(&container, &direction_rtl, spec_exact_100,
                               spec_exact_50));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 85.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 65.0f);

  M3_TEST_OK(m3_layout_compute(&container, &direction_ltr, spec_exact_100,
                               spec_exact_50));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 5.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 15.0f);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_ROW;
  style.align_main = M3_LAYOUT_ALIGN_END;
  style.align_cross = M3_LAYOUT_ALIGN_START;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  child1.style.width = 10.0f;
  child1.style.height = 10.0f;
  M3_TEST_OK(m3_layout_node_init(&child2, &style));
  child2.style.width = 20.0f;
  child2.style.height = 10.0f;
  children[0] = &child1;
  children[1] = &child2;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));

  M3_TEST_OK(m3_layout_compute(&container, &direction_rtl, spec_exact_100,
                               spec_exact_50));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 20.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 0.0f);

  M3_TEST_OK(m3_layout_compute(&container, &direction_ltr, spec_exact_100,
                               spec_exact_50));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 70.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 80.0f);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  style.wrap = M3_LAYOUT_WRAP_YES;
  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_CENTER;
  style.padding.left = 4.0f;
  style.padding.right = 6.0f;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  child1.style.width = 10.0f;
  child1.style.height = 10.0f;
  M3_TEST_OK(m3_layout_node_init(&child2, &style));
  child2.style.width = 20.0f;
  child2.style.height = 10.0f;
  children[0] = &child1;
  children[1] = &child2;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));

  M3_TEST_OK(m3_layout_compute(&container, &direction_rtl, spec_exact_100,
                               spec_exact_60));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 81.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 76.0f);

  M3_TEST_OK(m3_layout_compute(&container, &direction_ltr, spec_exact_100,
                               spec_exact_60));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 9.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 4.0f);

  M3_TEST_OK(m3_layout_style_init(&style));
  style.direction = M3_LAYOUT_DIRECTION_COLUMN;
  style.wrap = M3_LAYOUT_WRAP_NO;
  style.align_main = M3_LAYOUT_ALIGN_START;
  style.align_cross = M3_LAYOUT_ALIGN_END;
  M3_TEST_OK(m3_layout_node_init(&container, &style));
  M3_TEST_OK(m3_layout_node_init(&child1, &style));
  child1.style.width = 10.0f;
  child1.style.height = 10.0f;
  M3_TEST_OK(m3_layout_node_init(&child2, &style));
  child2.style.width = 20.0f;
  child2.style.height = 10.0f;
  children[0] = &child1;
  children[1] = &child2;
  M3_TEST_OK(m3_layout_node_set_children(&container, children, 2));

  M3_TEST_OK(m3_layout_compute(&container, &direction_rtl, spec_exact_100,
                               spec_exact_60));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 0.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 0.0f);

  M3_TEST_OK(m3_layout_compute(&container, &direction_ltr, spec_exact_100,
                               spec_exact_60));
  M3_TEST_OK(m3_layout_node_get_layout(&child1, &rect));
  M3_TEST_ASSERT(rect.x == 90.0f);
  M3_TEST_OK(m3_layout_node_get_layout(&child2, &rect));
  M3_TEST_ASSERT(rect.x == 80.0f);

  return 0;
}
