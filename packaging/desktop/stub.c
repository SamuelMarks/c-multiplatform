#include "m3/m3_core.h"

#include <stdio.h>

static int m3_stub_print(const char *message) {
  if (message == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (fprintf(stdout, "%s\n", message) < 0) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

int main(void) { return m3_stub_print("LibM3C packaging stub"); }
