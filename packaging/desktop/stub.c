#include "cmpc/cmp_core.h"

#include <stdio.h>

static int cmp_stub_print(const char *message) {
  if (message == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (fprintf(stdout, "%s\n", message) < 0) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

int main(void) { return cmp_stub_print("LibCMPC packaging stub"); }
