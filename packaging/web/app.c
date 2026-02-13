#include "cmpc/cmp_core.h"

#include <stdio.h>

static int cmp_web_app_print(void) {
  if (puts("LibCMPC web packaging stub") < 0) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

int main(void) { return cmp_web_app_print(); }
