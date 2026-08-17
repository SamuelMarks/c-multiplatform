/* clang-format off */
#include "../include/ui_image_decoder.h"
#include <stddef.h>
#if !defined(_MSC_VER) || _MSC_VER >= 1600
#include <stdint.h>
#else
typedef unsigned char uint8_t;
#endif

/* clang-format on */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  /* Dummy fuzz function */
  return 0;
}
