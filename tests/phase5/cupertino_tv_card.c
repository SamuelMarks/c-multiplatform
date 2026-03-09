#include "cupertino/cupertino_tv_card.h"
#include "../phase1/test_utils.h"
#include <stddef.h>

static int test_cupertino_tv_card(void) {
  CupertinoTVCard card;
  CMPTextBackend text_backend = {0};
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_tv_card_init(NULL, &text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_tv_card_init(&card, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_tv_card_init(&card, &text_backend), CMP_OK);

  CMP_TEST_EXPECT(cupertino_tv_card_set_title(NULL, "Title"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_tv_card_set_title(&card, "Title"), CMP_OK);

  CMP_TEST_EXPECT(cupertino_tv_card_set_focused(NULL, CMP_TRUE, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_tv_card_set_focused(&card, CMP_TRUE, CMP_FALSE),
                  CMP_OK);

  CMP_TEST_EXPECT(cupertino_tv_card_update(NULL, 0.1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_tv_card_update(&card, 0.1), CMP_OK);

  CMP_TEST_EXPECT(cupertino_tv_card_paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_tv_card_paint(&card, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_tv_card_paint(&card, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_tv_card();
  return fails > 0 ? 1 : 0;
}
