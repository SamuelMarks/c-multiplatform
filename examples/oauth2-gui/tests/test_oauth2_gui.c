/* clang-format off */
#include "greatest.h"
#include "oauth2_gui.h"
#include <cmpc/cmp_core.h>
/* clang-format on */

TEST test_init(void) {
  OAuth2Gui gui;
  int rc;

  rc = oauth2_gui_init(&gui);
  ASSERT_EQ(CMP_OK, rc);

  rc = oauth2_gui_tick(&gui);
  ASSERT_EQ(CMP_OK, rc);

  rc = oauth2_gui_shutdown(&gui);
  ASSERT_EQ(CMP_OK, rc);

  PASS();
}

SUITE(oauth2_gui_suite) { RUN_TEST(test_init); }
