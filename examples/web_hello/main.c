/* clang-format off */
#include "ui_engine.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
/* clang-format on */

int main(void) {
  printf("Hello from WebAssembly (C-side)!\n");
  return 0;
}
