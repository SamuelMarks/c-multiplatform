/* clang-format off */
#include "cupertino/cupertino_action_sheet.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int CMP_CALL mock_create_font(void *text, const char *utf8_family,
                                     cmp_i32 size_px, cmp_i32 weight,
                                     CMPBool italic, CMPHandle *out_font) {
  if (out_font)
    out_font->id = 1;
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

static int test_cupertino_action_sheet(void) {
  CupertinoActionSheet sheet;
  CupertinoActionSheetStyle style;
  CMPTextBackend text_backend;
  CMPRect bounds = {0.0f, 0.0f, 320.0f, 480.0f};
  CMPRect safe_area = {0.0f, 0.0f, 0.0f, 34.0f};
  CupertinoAction action1, action2;

  text_backend.ctx = NULL;
  text_backend.vtable = &mock_text_vtable;

  CMP_TEST_EXPECT(cupertino_action_sheet_style_init(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_action_sheet_style_init(&style));

  CMP_TEST_EXPECT(cupertino_action_sheet_init(NULL, &text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_action_sheet_init(&sheet, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_action_sheet_init(&sheet, &text_backend));

  CMP_TEST_EXPECT(cupertino_action_sheet_set_text(NULL, "T", 1, "M", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      cupertino_action_sheet_set_text(&sheet, "Title", 5, "Message", 7));

  action1.title_utf8 = "Delete";
  action1.title_len = 6;
  action1.style = CUPERTINO_ACTION_STYLE_DESTRUCTIVE;
  action1.user_data = NULL;

  action2.title_utf8 = "Cancel";
  action2.title_len = 6;
  action2.style = CUPERTINO_ACTION_STYLE_CANCEL;
  action2.user_data = NULL;

  CMP_TEST_EXPECT(cupertino_action_sheet_add_action(NULL, &action1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_action_sheet_add_action(&sheet, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_action_sheet_add_action(&sheet, &action1));
  CMP_TEST_OK(cupertino_action_sheet_add_action(&sheet, &action2));

  CMP_TEST_EXPECT(sheet.action_count, 2);

  /* Layout */
  CMP_TEST_EXPECT(cupertino_action_sheet_layout(NULL, bounds, safe_area),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_action_sheet_layout(&sheet, bounds, safe_area));

  CMP_TEST_ASSERT(sheet.bounds.width == 320.0f);
  CMP_TEST_ASSERT(sheet.bounds.height == 480.0f);

  return 0;
}

int main(void) {
  if (test_cupertino_action_sheet() != 0)
    return 1;
  return 0;
}
