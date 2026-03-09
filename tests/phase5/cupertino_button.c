#include "cupertino/cupertino_button.h"
#include "../phase1/test_utils.h"
#include <stddef.h>

/* Mock text backend for testing */
static int CMP_CALL mock_create_font(void *text, const char *utf8_family,
                                     cmp_i32 size_px, cmp_i32 weight,
                                     CMPBool italic, CMPHandle *out_font) {
  if (out_font) {
    out_font->id = 1;
  }
  return CMP_OK;
}

static int CMP_CALL mock_destroy_font(void *text, CMPHandle font) {
  return CMP_OK;
}

static int CMP_CALL mock_measure_text(void *ctx, CMPHandle font,
                                      const char *utf8, cmp_usize len,
                                      cmp_u32 direction, CMPScalar *out_width,
                                      CMPScalar *out_height,
                                      CMPScalar *out_baseline) {
  if (out_width)
    *out_width = (CMPScalar)len * 8.0f;
  if (out_height)
    *out_height = 16.0f;
  if (out_baseline)
    *out_baseline = 12.0f;
  return CMP_OK;
}

static const CMPTextVTable mock_text_vtable = {
    mock_create_font,
    mock_destroy_font,
    mock_measure_text,
    NULL, /* draw_text */
    NULL, /* shape_text */
    NULL, /* free_layout */
    NULL  /* draw_layout */
};

static int test_cupertino_button(void) {
  CupertinoButton button;
  CupertinoButtonStyle style;
  CMPTextBackend text_backend;
  CMPMeasureSpec exact = {CMP_MEASURE_EXACTLY, 100.0f};
  CMPMeasureSpec at_most = {CMP_MEASURE_AT_MOST, 100.0f};
  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};
  CMPSize size;

  text_backend.ctx = NULL;
  text_backend.vtable = &mock_text_vtable;

  CMP_TEST_EXPECT(cupertino_button_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_button_style_init(&style));
  CMP_TEST_EXPECT(style.variant, CUPERTINO_BUTTON_VARIANT_PLAIN);

  CMP_TEST_EXPECT(cupertino_button_init(NULL, &text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_button_init(&button, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_button_init(&button, &text_backend));

  CMP_TEST_EXPECT(cupertino_button_set_label(NULL, "Test", 4),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_button_set_label(&button, "Test", 4));

  CMP_TEST_EXPECT(cupertino_button_set_style(NULL, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_button_set_style(&button, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_button_set_style(&button, &style));

  CMP_TEST_EXPECT(cupertino_button_measure(NULL, unspec, unspec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_button_measure(&button, unspec, unspec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test measure unspec medium plain */
  CMP_TEST_OK(cupertino_button_measure(&button, unspec, unspec, &size));
  CMP_TEST_ASSERT(size.width == 32.0f); /* 4 * 8 + 0 padding */
  CMP_TEST_ASSERT(size.height ==
                  44.0f); /* 16 + 0 padding, bumped to 44 hit target */

  /* Test measure exactly */
  CMP_TEST_OK(cupertino_button_measure(&button, exact, exact, &size));
  CMP_TEST_ASSERT(size.width == 100.0f);
  CMP_TEST_ASSERT(size.height == 100.0f);

  /* Test layout */
  CMP_TEST_EXPECT(cupertino_button_layout(NULL, button.bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  {
    CMPRect r = {10, 10, 100, 50};
    CMP_TEST_OK(cupertino_button_layout(&button, r));
    CMP_TEST_ASSERT(button.bounds.x == 10.0f);
    CMP_TEST_ASSERT(button.bounds.width == 100.0f);
  }

  return 0;
}

int main(void) {
  if (test_cupertino_button() != 0)
    return 1;
  return 0;
}
