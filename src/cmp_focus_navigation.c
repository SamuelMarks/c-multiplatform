/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_focus_nav {
  cmp_a11y_tree_t *tree;
  cmp_focus_ring_t *ring;
  int current_focus_id;
};

int cmp_focus_nav_create(cmp_a11y_tree_t *tree, cmp_focus_nav_t **out_nav) {
  cmp_focus_nav_t *nav;
  int result;

  if (!tree || !out_nav) {
    return CMP_ERROR_INVALID_ARG;
  }

  nav = (cmp_focus_nav_t *)malloc(sizeof(cmp_focus_nav_t));
  if (!nav) {
    return CMP_ERROR_OOM;
  }

  nav->tree = tree;
  nav->current_focus_id = -1;

  result = cmp_focus_ring_create(tree, &nav->ring);
  if (result != CMP_SUCCESS) {
    free(nav);
    return result;
  }

  *out_nav = nav;
  return CMP_SUCCESS;
}

int cmp_focus_nav_destroy(cmp_focus_nav_t *nav) {
  if (!nav) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (nav->ring) {
    cmp_focus_ring_destroy(nav->ring);
  }
  free(nav);
  return CMP_SUCCESS;
}

int cmp_focus_nav_handle_tab(cmp_focus_nav_t *nav, int is_shift_pressed) {
  if (!nav) {
    return CMP_ERROR_INVALID_ARG;
  }

  /*
   * In a complete implementation, this would query the accessibility tree
   * for the next/previous focusable node and set its state.
   */
  (void)is_shift_pressed;

  /* Tell the focus ring to render keyboard navigation visual cues */
  cmp_focus_ring_set_keyboard_mode(nav->ring, 1);

  /* Dummy advancement, in reality this queries the DOM/Tree */
  nav->current_focus_id++;

  /* Mirror the focus into the native focus ring */
  cmp_focus_ring_node_focused(nav->ring, nav->current_focus_id);

  return CMP_SUCCESS;
}
