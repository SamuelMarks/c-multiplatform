/* clang-format off */
#include <stdio.h>
#include "ui_css_logical.h"
/* clang-format on */

static int test_logical_to_physical_edge(void) {
  /* Horizontal TB, LTR */
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_START, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_TOP)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_END, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_BOTTOM)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_START, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_LEFT)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_END, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_RIGHT)
    return 1;

  /* Horizontal TB, RTL */
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_START, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_TOP)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_END, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_BOTTOM)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_START, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_RIGHT)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_END, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_LEFT)
    return 1;

  /* Vertical RL, LTR */
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_START, UI_CSS_WRITING_MODE_VERTICAL_RL,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_RIGHT)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_END, UI_CSS_WRITING_MODE_VERTICAL_RL,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_LEFT)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_START, UI_CSS_WRITING_MODE_VERTICAL_RL,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_TOP)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_END, UI_CSS_WRITING_MODE_VERTICAL_RL,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_BOTTOM)
    return 1;

  /* Vertical RL, RTL */
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_START, UI_CSS_WRITING_MODE_VERTICAL_RL,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_RIGHT)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_END, UI_CSS_WRITING_MODE_VERTICAL_RL,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_LEFT)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_START, UI_CSS_WRITING_MODE_VERTICAL_RL,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_BOTTOM)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_END, UI_CSS_WRITING_MODE_VERTICAL_RL,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_TOP)
    return 1;

  /* Vertical LR, LTR */
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_START, UI_CSS_WRITING_MODE_VERTICAL_LR,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_LEFT)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_END, UI_CSS_WRITING_MODE_VERTICAL_LR,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_RIGHT)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_START, UI_CSS_WRITING_MODE_VERTICAL_LR,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_TOP)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_END, UI_CSS_WRITING_MODE_VERTICAL_LR,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_BOTTOM)
    return 1;

  /* Vertical LR, RTL */
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_START, UI_CSS_WRITING_MODE_VERTICAL_LR,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_LEFT)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_BLOCK_END, UI_CSS_WRITING_MODE_VERTICAL_LR,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_RIGHT)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_START, UI_CSS_WRITING_MODE_VERTICAL_LR,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_BOTTOM)
    return 1;
  if (ui_css_logical_to_physical_edge(
          UI_CSS_LOGICAL_EDGE_INLINE_END, UI_CSS_WRITING_MODE_VERTICAL_LR,
          UI_CSS_DIRECTION_RTL) != UI_CSS_PHYSICAL_EDGE_TOP)
    return 1;

  /* Fallback */
  if (ui_css_logical_to_physical_edge(
          (enum ui_css_logical_edge)99, (enum ui_css_writing_mode)99,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_TOP)
    return 1;
  if (ui_css_logical_to_physical_edge(
          (enum ui_css_logical_edge)99, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_TOP)
    return 1;
  if (ui_css_logical_to_physical_edge(
          (enum ui_css_logical_edge)99, UI_CSS_WRITING_MODE_VERTICAL_RL,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_TOP)
    return 1;
  if (ui_css_logical_to_physical_edge(
          (enum ui_css_logical_edge)99, UI_CSS_WRITING_MODE_VERTICAL_LR,
          UI_CSS_DIRECTION_LTR) != UI_CSS_PHYSICAL_EDGE_TOP)
    return 1;

  return 0;
}

static int test_logical_to_physical_size(void) {
  int is_width = 0;
  if (ui_css_logical_to_physical_size(1, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
                                      NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_css_logical_to_physical_size(1, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
                                  &is_width);
  if (is_width != 1)
    return 1;

  ui_css_logical_to_physical_size(0, UI_CSS_WRITING_MODE_HORIZONTAL_TB,
                                  &is_width);
  if (is_width != 0)
    return 1;

  ui_css_logical_to_physical_size(1, UI_CSS_WRITING_MODE_VERTICAL_RL,
                                  &is_width);
  if (is_width != 0)
    return 1;

  ui_css_logical_to_physical_size(0, UI_CSS_WRITING_MODE_VERTICAL_RL,
                                  &is_width);
  if (is_width != 1)
    return 1;

  return 0;
}

int main(void) {
  int failures = 0;

  failures += test_logical_to_physical_edge();
  failures += test_logical_to_physical_size();

  if (failures == 0) {
    printf("test_ui_css_logical passed\n");
  } else {
    printf("test_ui_css_logical failed with %d errors\n", failures);
  }

  return failures;
}
