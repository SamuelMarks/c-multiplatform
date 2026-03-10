/* clang-format off */
#include "f2/f2_searchbox.h"
#include <string.h>
/* clang-format on */

CMP_API int CMP_CALL f2_search_box_style_init(F2SearchBoxStyle *style) {
  if (!style) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(style, 0, sizeof(*style));

  f2_text_field_style_init(&style->text_field_style);
  f2_button_style_init_transparent(&style->clear_button_style);

  style->icon_color = (CMPColor){0.4f, 0.4f, 0.4f, 1.0f};
  style->icon_size = 16.0f;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_search_box_init(F2SearchBox *search_box,
                                        const CMPTextBackend *backend,
                                        const F2SearchBoxStyle *style,
                                        CMPAllocator *allocator) {
  int res;
  if (!search_box || !backend || !style || !allocator) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(search_box, 0, sizeof(*search_box));
  search_box->style = *style;
  search_box->has_text = CMP_FALSE;

  res = f2_text_field_init(&search_box->text_field, backend,
                           &style->text_field_style, allocator);
  if (res != CMP_OK)
    return res;

  res = f2_button_init(&search_box->clear_button, backend,
                       &style->clear_button_style, "X", 1);
  if (res != CMP_OK)
    return res;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_search_box_set_text(F2SearchBox *search_box,
                                            const char *utf8_text) {
  if (!search_box)
    return CMP_ERR_INVALID_ARGUMENT;

  if (utf8_text && utf8_text[0] != '\0') {
    search_box->has_text = CMP_TRUE;
  } else {
    search_box->has_text = CMP_FALSE;
  }

  return f2_text_field_set_text(&search_box->text_field, utf8_text);
}

CMP_API int CMP_CALL f2_search_box_set_callbacks(F2SearchBox *search_box,
                                                 F2SearchBoxOnChange on_change,
                                                 void *change_ctx,
                                                 F2SearchBoxOnSearch on_search,
                                                 void *search_ctx) {
  if (!search_box)
    return CMP_ERR_INVALID_ARGUMENT;
  search_box->on_change = on_change;
  search_box->on_change_ctx = change_ctx;
  search_box->on_search = on_search;
  search_box->on_search_ctx = search_ctx;
  return CMP_OK;
}

CMP_API int CMP_CALL f2_search_box_cleanup(F2SearchBox *search_box) {
  if (!search_box)
    return CMP_ERR_INVALID_ARGUMENT;
  /* cmp_text_field cleanup is unneeded/does not exist */
  return CMP_OK;
}
