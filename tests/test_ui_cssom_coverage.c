/* clang-format off */
#include "../include/ui_css_parser.h"
#include "../include/ui_css_values.h"
#include "../include/ui_cssom.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static void test_cssom_coverage(void) {
  struct ui_css_stylesheet *sheet;
  int order = 1, order2 = 2, order3 = 3;

  ui_css_stylesheet_create(&sheet);

  /* Cover while (curr->next) for namespaces */
  ui_css_stylesheet_register_namespace(sheet, "n1", "http://1");
  ui_css_stylesheet_register_namespace(sheet, "n2", "http://2");
  ui_css_stylesheet_register_namespace(sheet, "n3", "http://3");

  /* Cover while (curr->next) for layers */
  ui_css_stylesheet_register_layer(sheet, "L1", &order);
  ui_css_stylesheet_register_layer(sheet, "L2", &order2);
  ui_css_stylesheet_register_layer(sheet, "L3", &order3);

  ui_css_stylesheet_destroy(sheet);
}
