/**
 * @file main.c
 * @brief Entry point for the WebAssembly hello world example.
 */

/* clang-format off */
#include "ui_engine.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
/* clang-format on */

/**
 * @brief Main entry point.
 * @return Exit status code.
 */
int main(void) {
  printf("Hello from WebAssembly (C-side)!\n");
  return 0;
}
