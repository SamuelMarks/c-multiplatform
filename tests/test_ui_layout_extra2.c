#include "../include/ui_layout.h"
#include <stdio.h>

void test_coverage_layout(void) {
  struct ui_layout_node root;
  ui_layout_tree_generate(NULL, NULL, NULL);

  root.width = 100.0f;
  root.max_width = 50.0f;
  root.min_width = 0.0f;
  root.height = 10.0f;
  root.min_height = 50.0f;
  root.max_height = 100.0f;
  root.padding[0] = root.padding[1] = root.padding[2] = root.padding[3] = 0.0f;
  root.border[0] = root.border[1] = root.border[2] = root.border[3] = 0.0f;

  /* We can't really call compute_box_model directly because it's static.
     Instead, we can construct css rules that trigger it in generate or compute.
  */
}
