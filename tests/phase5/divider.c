#include "m3/m3_divider.h"
#include "test_utils.h"

static int test_divider_style(void) {
  M3DividerStyle style;

  CMP_TEST_EXPECT(m3_divider_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_divider_style_init(&style));

  CMP_TEST_EXPECT(style.direction == CMP_LAYOUT_DIRECTION_ROW ? CMP_OK
                                                              : CMP_ERR_UNKNOWN,
                  CMP_OK);
  CMP_TEST_EXPECT(style.thickness == M3_DIVIDER_DEFAULT_THICKNESS
                      ? CMP_OK
                      : CMP_ERR_UNKNOWN,
                  CMP_OK);
  CMP_TEST_EXPECT(style.inset_start == 0.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);
  CMP_TEST_EXPECT(style.inset_end == 0.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  return 0;
}

static int test_divider_init(void) {
  M3Divider divider;
  M3DividerStyle style;

  m3_divider_style_init(&style);

  CMP_TEST_EXPECT(m3_divider_init(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_divider_init(&divider, NULL), CMP_ERR_INVALID_ARGUMENT);

  style.thickness = -1.0f;
  CMP_TEST_EXPECT(m3_divider_init(&divider, &style), CMP_ERR_INVALID_ARGUMENT);
  style.thickness = 1.0f;

  style.inset_start = -1.0f;
  CMP_TEST_EXPECT(m3_divider_init(&divider, &style), CMP_ERR_INVALID_ARGUMENT);
  style.inset_start = 0.0f;

  style.inset_end = -1.0f;
  CMP_TEST_EXPECT(m3_divider_init(&divider, &style), CMP_ERR_INVALID_ARGUMENT);
  style.inset_end = 0.0f;

  CMP_TEST_OK(m3_divider_init(&divider, &style));

  CMP_TEST_EXPECT(m3_divider_set_style(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_divider_set_style(&divider, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  style.thickness = -1.0f;
  CMP_TEST_EXPECT(m3_divider_set_style(&divider, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  style.thickness = 1.0f;

  CMP_TEST_OK(m3_divider_set_style(&divider, &style));

  return 0;
}

static int test_divider_widget(void) {
  M3Divider divider;
  M3DividerStyle style;
  CMPSize size;
  CMPRect bounds = {0, 0, 100, 100};
  CMPInputEvent event = {0};
  CMPBool handled;
  CMPSemantics semantics;
  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};
  CMPMeasureSpec exact100 = {CMP_MEASURE_EXACTLY, 100.0f};

  m3_divider_style_init(&style);
  m3_divider_init(&divider, &style);

  /* Measure */
  CMP_TEST_EXPECT(divider.widget.vtable->measure(NULL, unspec, unspec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      divider.widget.vtable->measure(&divider, unspec, unspec, NULL),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(
      divider.widget.vtable->measure(&divider, exact100, unspec, &size));
  CMP_TEST_EXPECT(size.width == 100.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);
  CMP_TEST_EXPECT(size.height == 1.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  m3_divider_set_style(&divider, &style);
  CMP_TEST_OK(
      divider.widget.vtable->measure(&divider, unspec, exact100, &size));
  CMP_TEST_EXPECT(size.width == 1.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);
  CMP_TEST_EXPECT(size.height == 100.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  /* Layout */
  CMP_TEST_EXPECT(divider.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(divider.widget.vtable->layout(&divider, bounds));
  CMP_TEST_EXPECT(divider.bounds.width == 100.0f ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);

  /* Paint */
  CMP_TEST_EXPECT(divider.widget.vtable->paint(NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  /* Can't easily test valid paint without a mock ctx, but we cover nulls */

  /* Event */
  CMP_TEST_EXPECT(divider.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(divider.widget.vtable->event(&divider, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(divider.widget.vtable->event(&divider, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(divider.widget.vtable->event(&divider, &event, &handled));
  CMP_TEST_EXPECT(handled == CMP_FALSE ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  /* Semantics */
  CMP_TEST_EXPECT(divider.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(divider.widget.vtable->get_semantics(&divider, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(divider.widget.vtable->get_semantics(&divider, &semantics));
  CMP_TEST_EXPECT(
      semantics.role == CMP_SEMANTIC_NONE ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  /* Destroy */
  CMP_TEST_EXPECT(divider.widget.vtable->destroy(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(divider.widget.vtable->destroy(&divider));

  return 0;
}

int main(void) {
  CMP_TEST_ASSERT(test_divider_style() == 0);
  CMP_TEST_ASSERT(test_divider_init() == 0);
  CMP_TEST_ASSERT(test_divider_widget() == 0);
  return 0;
}
