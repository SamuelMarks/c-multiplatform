#include "m3/m3_badge.h"
#include "test_utils.h"

static int mock_create_font(void *text, const char *utf8_family,
                            cmp_i32 size_px, cmp_i32 weight, CMPBool italic,
                            CMPHandle *out_font) {
  if (out_font) {
    out_font->id = 1;
    out_font->generation = 1;
  }
  return CMP_OK;
}
static int mock_destroy_font(void *text, CMPHandle font) { return CMP_OK; }
static int mock_measure_text(void *text, CMPHandle font, const char *utf8,
                             cmp_usize utf8_len, CMPScalar *out_width,
                             CMPScalar *out_height, CMPScalar *out_baseline) {
  if (out_width)
    *out_width = 10.0f;
  if (out_height)
    *out_height = 10.0f;
  if (out_baseline)
    *out_baseline = 8.0f;
  return CMP_OK;
}
static int mock_draw_text(void *text, CMPHandle font, const char *utf8,
                          cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                          CMPColor color) {
  return CMP_OK;
}
static const CMPTextVTable mock_text_vtable = {
    mock_create_font, mock_destroy_font, mock_measure_text, mock_draw_text};

static void cmp_test_text_backend_init(CMPTextBackend *backend) {
  backend->ctx = NULL;
  backend->vtable = &mock_text_vtable;
}

static int test_badge_style(void) {
  M3BadgeStyle style;

  CMP_TEST_EXPECT(m3_badge_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_badge_style_init(&style));

  CMP_TEST_EXPECT(style.dot_diameter == M3_BADGE_DEFAULT_DOT_DIAMETER
                      ? CMP_OK
                      : CMP_ERR_UNKNOWN,
                  CMP_OK);
  CMP_TEST_EXPECT(
      style.min_width == M3_BADGE_DEFAULT_MIN_WIDTH ? CMP_OK : CMP_ERR_UNKNOWN,
      CMP_OK);
  CMP_TEST_EXPECT(style.height == M3_BADGE_DEFAULT_HEIGHT ? CMP_OK
                                                          : CMP_ERR_UNKNOWN,
                  CMP_OK);
  CMP_TEST_EXPECT(
      style.padding_x == M3_BADGE_DEFAULT_PADDING_X ? CMP_OK : CMP_ERR_UNKNOWN,
      CMP_OK);

  return 0;
}

static int test_badge_init(void) {
  M3Badge badge;
  M3BadgeStyle style;
  CMPTextBackend backend;

  m3_badge_style_init(&style);
  cmp_test_text_backend_init(&backend);

  CMP_TEST_EXPECT(m3_badge_init(NULL, &backend, &style, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_badge_init(&badge, &backend, NULL, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_badge_init(&badge, &backend, &style, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);

  style.dot_diameter = -1.0f;
  CMP_TEST_EXPECT(m3_badge_init(&badge, &backend, &style, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  style.dot_diameter = 6.0f;

  CMP_TEST_OK(m3_badge_init(&badge, &backend, &style, NULL, 0));
  CMP_TEST_OK(m3_badge_init(&badge, &backend, &style, "1", 1));

  CMP_TEST_EXPECT(m3_badge_set_label(NULL, "2", 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_badge_set_label(&badge, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_badge_set_label(&badge, "2", 1));

  CMP_TEST_EXPECT(m3_badge_set_style(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_badge_set_style(&badge, NULL), CMP_ERR_INVALID_ARGUMENT);
  style.height = -1.0f;
  CMP_TEST_EXPECT(m3_badge_set_style(&badge, &style), CMP_ERR_INVALID_ARGUMENT);
  style.height = 16.0f;
  CMP_TEST_OK(m3_badge_set_style(&badge, &style));

  return 0;
}

static int test_badge_widget(void) {
  M3Badge badge;
  M3BadgeStyle style;
  CMPTextBackend backend;
  CMPSize size;
  CMPRect bounds = {0, 0, 100, 100};
  CMPInputEvent event = {0};
  CMPBool handled;
  CMPSemantics semantics;
  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};
  CMPMeasureSpec exact100 = {CMP_MEASURE_EXACTLY, 100.0f};

  m3_badge_style_init(&style);
  cmp_test_text_backend_init(&backend);
  m3_badge_init(&badge, &backend, &style, "3", 1);

  /* Measure */
  CMP_TEST_EXPECT(badge.widget.vtable->measure(NULL, unspec, unspec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(badge.widget.vtable->measure(&badge, unspec, unspec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(badge.widget.vtable->measure(&badge, unspec, unspec, &size));
  CMP_TEST_EXPECT(size.width >= style.min_width ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);
  CMP_TEST_EXPECT(size.height == style.height ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);

  CMP_TEST_OK(badge.widget.vtable->measure(&badge, exact100, exact100, &size));
  CMP_TEST_EXPECT(size.width == 100.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);
  CMP_TEST_EXPECT(size.height == 100.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  /* Layout */
  CMP_TEST_EXPECT(badge.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(badge.widget.vtable->layout(&badge, bounds));
  CMP_TEST_EXPECT(badge.bounds.width == 100.0f ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);

  /* Paint */
  CMP_TEST_EXPECT(badge.widget.vtable->paint(NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Event */
  CMP_TEST_EXPECT(badge.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(badge.widget.vtable->event(&badge, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(badge.widget.vtable->event(&badge, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(badge.widget.vtable->event(&badge, &event, &handled));
  CMP_TEST_EXPECT(handled == CMP_FALSE ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  /* Semantics */
  CMP_TEST_EXPECT(badge.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(badge.widget.vtable->get_semantics(&badge, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(badge.widget.vtable->get_semantics(&badge, &semantics));
  CMP_TEST_EXPECT(
      semantics.role == CMP_SEMANTIC_TEXT ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  /* Destroy */
  CMP_TEST_EXPECT(badge.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(badge.widget.vtable->destroy(&badge));

  return 0;
}

static int test_badge_dot(void) {
  M3Badge badge;
  M3BadgeStyle style;
  CMPSize size;
  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};

  m3_badge_style_init(&style);
  m3_badge_init(&badge, NULL, &style, NULL, 0);

  CMP_TEST_OK(badge.widget.vtable->measure(&badge, unspec, unspec, &size));
  CMP_TEST_EXPECT(size.width == style.dot_diameter ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);
  CMP_TEST_EXPECT(size.height == style.dot_diameter ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);

  CMP_TEST_OK(badge.widget.vtable->destroy(&badge));
  return 0;
}

int main(void) {
  CMP_TEST_ASSERT(test_badge_style() == 0);
  CMP_TEST_ASSERT(test_badge_init() == 0);
  CMP_TEST_ASSERT(test_badge_widget() == 0);
  CMP_TEST_ASSERT(test_badge_dot() == 0);
  return 0;
}
