/* clang-format off */
#include "cupertino/cupertino_menu.h"
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

static int CMP_CALL mock_draw_text(void *text, CMPHandle font, const char *utf8,
                                   cmp_usize utf8_len, cmp_u32 base_direction,
                                   CMPScalar x, CMPScalar y, CMPColor color) {
  return CMP_OK;
}

static const CMPTextVTable mock_text_vtable = {
    mock_create_font,
    mock_destroy_font,
    mock_measure_text,
    mock_draw_text, /* draw_text */
    NULL,           /* shape_text */
    NULL,           /* free_layout */
    NULL            /* draw_layout */
};

static int test_cupertino_menu(void) {
  CupertinoMenu menu;
  CupertinoMenuStyle style;
  CMPTextBackend text_backend;
  CMPRect anchor = {100.0f, 100.0f, 50.0f, 20.0f};
  CMPRect screen = {0.0f, 0.0f, 1024.0f, 768.0f};
  CupertinoMenuItem items[3] = {0};
  CMPSize size;

  text_backend.ctx = NULL;
  text_backend.vtable = &mock_text_vtable;

  CMP_TEST_EXPECT(cupertino_menu_style_init(NULL, CUPERTINO_MENU_MAC_OS),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_menu_style_init(&style, CUPERTINO_MENU_MAC_OS));
  CMP_TEST_EXPECT(style.variant, CUPERTINO_MENU_MAC_OS);

  CMP_TEST_EXPECT(
      cupertino_menu_init(NULL, &text_backend, CUPERTINO_MENU_MAC_OS),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_menu_init(&menu, NULL, CUPERTINO_MENU_MAC_OS),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_menu_init(&menu, &text_backend, CUPERTINO_MENU_MAC_OS));

  items[0].title_utf8 = "Copy";
  items[0].title_len = 4;
  items[0].shortcut_utf8 = "Cmd C";
  items[0].shortcut_len = 5;
  items[0].style = CUPERTINO_MENU_ITEM_DEFAULT;

  items[1].title_utf8 = NULL;
  items[1].title_len = 0;
  items[1].shortcut_utf8 = NULL;
  items[1].shortcut_len = 0;
  items[1].style = CUPERTINO_MENU_ITEM_SEPARATOR;

  items[2].title_utf8 = "Delete";
  items[2].title_len = 6;
  items[2].shortcut_utf8 = NULL;
  items[2].shortcut_len = 0;
  items[2].style = CUPERTINO_MENU_ITEM_DESTRUCTIVE;

  CMP_TEST_EXPECT(cupertino_menu_set_items(NULL, items, 3),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_menu_set_items(&menu, items, 3));
  CMP_TEST_EXPECT(menu.item_count, 3);

  CMP_TEST_EXPECT(cupertino_menu_set_hover(NULL, 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_menu_set_hover(&menu, 0));
  CMP_TEST_EXPECT(menu.hovered_index, 0);

  /* Measure */
  CMP_TEST_EXPECT(cupertino_menu_measure(NULL, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_menu_measure(&menu, &size));

  /* Expected Height MAC: padding(12) + (2 * 22) + 9 = 65 */
  CMP_TEST_ASSERT(size.height == 65.0f);
  CMP_TEST_ASSERT(size.width >= 120.0f); /* Min width */

  /* Layout MAC */
  CMP_TEST_EXPECT(cupertino_menu_layout(NULL, anchor, screen),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cupertino_menu_layout(&menu, anchor, screen));
  CMP_TEST_ASSERT(menu.bounds.x == 100.0f);
  CMP_TEST_ASSERT(menu.bounds.y == 120.0f); /* anchor y + height */

  /* Re-init as iOS */
  CMP_TEST_OK(cupertino_menu_init(&menu, &text_backend, CUPERTINO_MENU_IOS));
  CMP_TEST_OK(cupertino_menu_set_items(&menu, items, 3));
  CMP_TEST_OK(cupertino_menu_measure(&menu, &size));

  /* Expected Height IOS: padding(12) + (2 * 44) + 9 = 109 */
  CMP_TEST_ASSERT(size.height == 109.0f);

  /* Layout IOS */
  CMP_TEST_OK(cupertino_menu_layout(&menu, anchor, screen));
  CMP_TEST_ASSERT(menu.bounds.x == 100.0f);
  CMP_TEST_ASSERT(menu.bounds.y == 128.0f); /* anchor y + height + 8.0f gap */

  return 0;
}
static int CMP_CALL mock_draw_rect(void *gfx, const CMPRect *rect,
                                   CMPColor color, CMPScalar corner_radius) {
  return CMP_OK;
}
static int CMP_CALL mock_draw_line(void *gfx, CMPScalar x0, CMPScalar y0,
                                   CMPScalar x1, CMPScalar y1, CMPColor color,
                                   CMPScalar thickness) {
  return CMP_OK;
}
static int CMP_CALL mock_draw_path(void *gfx, const CMPPath *path,
                                   CMPColor color) {
  return CMP_OK;
}

static const CMPGfxVTable mock_gfx_vtable = {
    NULL,           NULL, NULL, mock_draw_rect, mock_draw_line,
    mock_draw_path, NULL, NULL, NULL,           NULL,
    NULL,           NULL, NULL};

static int test_cupertino_menu_paint(void) {
  CupertinoMenu menu;
  CMPTextBackend text_backend;
  CupertinoMenuItem items[3] = {0};
  CMPPaintContext pctx = {0};
  CMPGfx gfx = {0};

  text_backend.ctx = NULL;
  text_backend.vtable = &mock_text_vtable;

  gfx.vtable = &mock_gfx_vtable;
  gfx.text_vtable = &mock_text_vtable;
  pctx.gfx = &gfx;

  CMP_TEST_OK(cupertino_menu_init(&menu, &text_backend, CUPERTINO_MENU_MAC_OS));

  items[0].title_utf8 = "Copy";
  items[0].title_len = 4;
  items[0].style = CUPERTINO_MENU_ITEM_DEFAULT;

  items[1].style = CUPERTINO_MENU_ITEM_SEPARATOR;

  items[2].title_utf8 = "Delete";
  items[2].title_len = 6;
  items[2].style = CUPERTINO_MENU_ITEM_DESTRUCTIVE;
  items[2].is_disabled = CMP_TRUE;

  CMP_TEST_OK(cupertino_menu_set_items(&menu, items, 3));
  CMP_TEST_OK(cupertino_menu_set_hover(&menu, 0));

  menu.bounds.x = 0;
  menu.bounds.y = 0;
  menu.bounds.width = 150;
  menu.bounds.height = 100;
  menu.animation_progress = 1.0f;

  CMP_TEST_EXPECT(cupertino_menu_paint(NULL, &pctx), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_menu_paint(&menu, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cupertino_menu_paint(&menu, &pctx));

  /* Test dark mode, iOS */
  menu.style.is_dark_mode = CMP_TRUE;
  menu.style.variant = CUPERTINO_MENU_IOS;
  CMP_TEST_OK(cupertino_menu_paint(&menu, &pctx));

  /* Test edge cases */
  menu.animation_progress = 0.0f;
  CMP_TEST_OK(cupertino_menu_paint(&menu, &pctx));
  menu.animation_progress = 1.0f;

  menu.item_count = 0;
  CMP_TEST_OK(cupertino_menu_paint(&menu, &pctx));
  menu.item_count = 3;

  return 0;
}

int main(void) {
  if (test_cupertino_menu() != 0)
    return 1;
  if (test_cupertino_menu_paint() != 0)
    return 1;
  return 0;
}
