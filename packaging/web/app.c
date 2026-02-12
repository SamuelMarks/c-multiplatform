#include "m3/m3_core.h"

#include <stdio.h>

static int m3_web_app_print(void) {
  if (puts("LibM3C web packaging stub") < 0) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

int main(void) { return m3_web_app_print(); }
