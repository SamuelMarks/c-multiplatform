/* clang-format off */
#include "../include/ui_css_parser.h"
#include "../include/ui_css_ui.h"
#include "../include/ui_css_values.h"
#include "../include/ui_cssom.h"
#include "../include/ui_dom_node.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

void test_oom(void) {
  int i;
  for (i = 0; i < 2000; i++) {
    struct ui_css_stylesheet *sheet = NULL;
    const char *css =
        "div, .class, #id { color: red; margin: 10px; font-family: 'Arial'; } "
        "@media screen { div { width: 100px; } }";
    g_malloc_fail_countdown = i;
    ui_css_parse_stylesheet(css, &sheet);
    g_malloc_fail_countdown = -1;
    if (sheet) {
      ui_css_stylesheet_destroy(sheet);
      break;
    }
  }
}
