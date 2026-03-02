#include "m3/m3_segmented.h"
#include "test_utils.h"

static int test_on_change(void *ctx, M3SegmentedGroup *group, cmp_usize index) {
  int *called = (int *)ctx;
  *called = 1;
  return CMP_OK;
}

static int test_create_font(void *text, const char *utf8_family,
                            cmp_i32 size_px, cmp_i32 weight, CMPBool italic,
                            CMPHandle *out_font) {
  if (out_font) {
    out_font->id = 1;
    out_font->generation = 1;
  }
  return CMP_OK;
}
static int test_destroy_font(void *text, CMPHandle font) { return CMP_OK; }
static int test_measure_text(void *text, CMPHandle font, const char *utf8,
                             cmp_usize utf8_len, CMPScalar *w, CMPScalar *h,
                             CMPScalar *baseline) {
  if (w)
    *w = 20.0f;
  if (h)
    *h = 10.0f;
  if (baseline)
    *baseline = 8.0f;
  return CMP_OK;
}
static int test_draw_text(void *text, CMPHandle font, const char *utf8,
                          cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                          CMPColor color) {
  return CMP_OK;
}

static const CMPTextVTable mock_vtable = {test_create_font, test_destroy_font,
                                          test_measure_text, test_draw_text};

int main(void) {
  M3SegmentedStyle style;
  M3SegmentedGroup group;
  CMPTextBackend text_backend;
  int called = 0;

  CMP_TEST_EXPECT(m3_segmented_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_style_init(&style));
  CMP_TEST_EXPECT(style.corner_radius == 20.0f ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);

  text_backend.ctx = NULL;
  text_backend.vtable = &mock_vtable;

  CMP_TEST_EXPECT(m3_segmented_init(NULL, &style, text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_init(&group, NULL, text_backend),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_segmented_init(&group, &style, text_backend));

  CMP_TEST_EXPECT(m3_segmented_add(NULL, "Opt1"), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_add(&group, "Opt1"));
  CMP_TEST_OK(m3_segmented_add(&group, "Opt2"));
  CMP_TEST_OK(m3_segmented_add(&group, "Opt3"));

  CMP_TEST_EXPECT(m3_segmented_set_on_change(NULL, test_on_change, &called),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_set_on_change(&group, test_on_change, &called));

  CMP_TEST_EXPECT(m3_segmented_set_selected(NULL, 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_set_selected(&group, 1));
  CMP_TEST_EXPECT(group.selected_index, 1);

  CMP_TEST_OK(m3_segmented_test_helper());

  {
    CMPSize size;
    CMPRect bounds = {0, 0, 300, 40};
    CMPInputEvent event = {0};
    CMPBool handled;
    CMPSemantics semantics;
    CMPMeasureSpec m300 = {1, 300.0f};

    CMP_TEST_EXPECT(group.widget.vtable->measure(NULL, m300, m300, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(group.widget.vtable->measure(&group, m300, m300, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(group.widget.vtable->measure(&group, m300, m300, &size));

    CMP_TEST_EXPECT(group.widget.vtable->layout(NULL, bounds),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(group.widget.vtable->layout(&group, bounds));

    CMP_TEST_EXPECT(group.widget.vtable->event(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(group.widget.vtable->event(&group, NULL, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(group.widget.vtable->event(&group, &event, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    event.type = CMP_INPUT_POINTER_DOWN;
    CMP_TEST_OK(group.widget.vtable->event(&group, &event, &handled));

    CMP_TEST_EXPECT(group.widget.vtable->get_semantics(NULL, &semantics),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(group.widget.vtable->get_semantics(&group, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(group.widget.vtable->get_semantics(&group, &semantics));

    CMP_TEST_EXPECT(group.widget.vtable->destroy(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(group.widget.vtable->destroy(&group));
  }

  return 0;
}
