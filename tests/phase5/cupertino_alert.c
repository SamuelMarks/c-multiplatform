#include "cupertino/cupertino_alert.h"
#include "../phase1/test_utils.h"
#include <stddef.h>

static int test_cupertino_alert(void) {
  CupertinoAlert alert;
  CMPTextBackend text_backend = {0};
  CMPPaintContext ctx = {0};
  CMPGfx gfx = {0};
  CMPGfxVTable vtable = {0};

  gfx.vtable = &vtable;
  ctx.gfx = &gfx;

  CMP_TEST_EXPECT(cupertino_alert_init(NULL, &text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_alert_init(&alert, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_alert_init(&alert, &text_backend), CMP_OK);

  CMP_TEST_EXPECT(cupertino_alert_set_content(NULL, "Title", "Message"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_alert_set_content(&alert, "Title", "Message"),
                  CMP_OK);

  CMP_TEST_EXPECT(cupertino_alert_add_action(NULL, "OK", CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_alert_add_action(&alert, NULL, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_alert_add_action(&alert, "OK", CMP_FALSE), CMP_OK);
  CMP_TEST_EXPECT(alert.action_count, 1);

  CMP_TEST_EXPECT(cupertino_alert_paint(NULL, &ctx), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_alert_paint(&alert, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cupertino_alert_paint(&alert, &ctx), CMP_OK);

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_cupertino_alert();
  return fails > 0 ? 1 : 0;
}
