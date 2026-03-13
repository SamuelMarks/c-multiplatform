/* clang-format off */
#include "m3/m3_divider.h"
#include "test_utils.h"
#include <string.h>
/* clang-format on */

static int dummy_draw_line(void *ctx, CMPScalar x1, CMPScalar y1, CMPScalar x2,
                           CMPScalar y2, CMPColor color, CMPScalar thickness) {
  return CMP_OK;
}

static int dummy_draw_rect(void *ctx, const CMPRect *bounds, CMPColor color,
                           CMPScalar corner_radius) {
  return CMP_OK;
}

int main(void) {
  M3DividerStyle style;
  M3Divider div;
  CMPRect bounds = {0, 0, 100, 100};
  CMPMeasureSpec spec = {CMP_MEASURE_EXACTLY, 100};
  CMPSize size;

  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};
  CMPMeasureSpec exact = {CMP_MEASURE_EXACTLY, 100.0f};

  CMP_TEST_EXPECT(m3_divider_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_divider_style_init(&style));

  CMP_TEST_EXPECT(m3_divider_init(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_divider_init(&div, NULL), CMP_ERR_INVALID_ARGUMENT);
  style.thickness = -1.0f;
  CMP_TEST_EXPECT(m3_divider_init(&div, &style), CMP_ERR_INVALID_ARGUMENT);
  style.thickness = 1.0f;
  style.inset_start = -1.0f;
  CMP_TEST_EXPECT(m3_divider_init(&div, &style), CMP_ERR_INVALID_ARGUMENT);
  style.inset_start = 0.0f;
  style.inset_end = -1.0f;
  CMP_TEST_EXPECT(m3_divider_init(&div, &style), CMP_ERR_INVALID_ARGUMENT);
  style.inset_end = 0.0f;
  CMP_TEST_OK(m3_divider_init(&div, &style));

  CMP_TEST_EXPECT(m3_divider_set_style(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_divider_set_style(&div, NULL), CMP_ERR_INVALID_ARGUMENT);
  style.thickness = -1.0f;
  CMP_TEST_EXPECT(m3_divider_set_style(&div, &style), CMP_ERR_INVALID_ARGUMENT);
  style.thickness = 1.0f;
  style.inset_start = -1.0f;
  CMP_TEST_EXPECT(m3_divider_set_style(&div, &style), CMP_ERR_INVALID_ARGUMENT);
  style.inset_start = 0.0f;
  style.inset_end = -1.0f;
  CMP_TEST_EXPECT(m3_divider_set_style(&div, &style), CMP_ERR_INVALID_ARGUMENT);
  style.inset_end = 0.0f;
  CMP_TEST_OK(m3_divider_set_style(&div, &style));

  /* Measure Vertical */
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  CMP_TEST_OK(m3_divider_init(&div, &style));
  CMP_TEST_OK(div.widget.vtable->measure(&div, unspec, unspec, &size));
  CMP_TEST_OK(div.widget.vtable->measure(&div, exact, unspec, &size));
  CMP_TEST_OK(div.widget.vtable->measure(&div, unspec, exact, &size));
  CMP_TEST_OK(div.widget.vtable->measure(&div, exact, exact, &size));

  /* Measure Horizontal with exact height */
  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  CMP_TEST_OK(m3_divider_set_style(&div, &style));
  CMP_TEST_OK(div.widget.vtable->measure(&div, unspec, exact, &size));

  CMP_TEST_EXPECT(div.widget.vtable->measure(NULL, spec, spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(div.widget.vtable->measure(&div, spec, spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(div.widget.vtable->measure(&div, spec, spec, &size));

  CMP_TEST_EXPECT(div.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(div.widget.vtable->layout(&div, bounds));

  CMPPaintContext pctx;
  memset(&pctx, 0, sizeof(pctx));
  CMPGfx gfx;
  memset(&gfx, 0, sizeof(gfx));
  pctx.gfx = &gfx;

  CMP_TEST_EXPECT(div.widget.vtable->paint(NULL, &pctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(div.widget.vtable->paint(&div, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  pctx.gfx = NULL;
  CMP_TEST_EXPECT(div.widget.vtable->paint(&div, &pctx), CMP_OK);

  CMPGfxVTable gvt;
  memset(&gvt, 0, sizeof(gvt));
  gfx.vtable = &gvt;

  gvt.draw_rect = dummy_draw_rect;
  pctx.gfx = &gfx;
  CMP_TEST_EXPECT(div.widget.vtable->paint(&div, &pctx), CMP_OK);

  style.color.a = 0.0f;
  m3_divider_init(&div, &style);
  CMP_TEST_OK(div.widget.vtable->paint(&div, &pctx));

  style.color.a = 1.0f;
  style.thickness = 0.0f;
  m3_divider_init(&div, &style);
  CMP_TEST_OK(div.widget.vtable->paint(&div, &pctx));
  style.thickness = 1.0f;

  style.color.a = 1.0f;
  style.direction = CMP_LAYOUT_DIRECTION_COLUMN;
  style.inset_start = 10.0f;
  style.inset_end = 1000.0f;
  m3_divider_init(&div, &style);
  CMP_TEST_OK(div.widget.vtable->paint(&div, &pctx));

  style.direction = CMP_LAYOUT_DIRECTION_ROW;
  style.inset_end = 1000.0f;
  m3_divider_init(&div, &style);
  CMP_TEST_OK(div.widget.vtable->paint(&div, &pctx));

  CMPInputEvent ev;
  memset(&ev, 0, sizeof(ev));
  CMPBool handled;
  CMP_TEST_EXPECT(div.widget.vtable->event(NULL, &ev, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(div.widget.vtable->event(&div, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(div.widget.vtable->event(&div, &ev, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(div.widget.vtable->event(&div, &ev, &handled));

  CMPSemantics sem;
  CMP_TEST_EXPECT(div.widget.vtable->get_semantics(NULL, &sem),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(div.widget.vtable->get_semantics(&div, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(div.widget.vtable->get_semantics(&div, &sem));

  CMP_TEST_EXPECT(div.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(div.widget.vtable->destroy(&div));

  return CMP_OK;
}
