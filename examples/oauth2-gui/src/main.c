/* clang-format off */
#include "oauth2_gui.h"
#include <cmpc/cmp_event.h>
#include <cmpc/cmp_core.h>
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

int main(void) {
  OAuth2Gui gui;
  int rc;

  rc = oauth2_gui_init(&gui);
  if (rc != CMP_OK) {
    printf("Failed to init GUI\n");
    return 1;
  }

  /* Event loop stub */
  while (1) {
    rc = oauth2_gui_tick(&gui);
    if (rc != CMP_OK) {
      break;
    }
    /* Break immediately for test */
    break;
  }

  oauth2_gui_shutdown(&gui);
  return 0;
}
