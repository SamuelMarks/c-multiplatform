/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_clipboard_overlay {
  cmp_clipboard_state_t state;
};

int cmp_clipboard_overlay_create(cmp_clipboard_overlay_t **out_overlay) {
  cmp_clipboard_overlay_t *overlay;

  if (!out_overlay)
    return CMP_ERROR_INVALID_ARG;

  overlay = (cmp_clipboard_overlay_t *)malloc(sizeof(cmp_clipboard_overlay_t));
  if (!overlay)
    return CMP_ERROR_OOM;

  overlay->state = CMP_CLIPBOARD_STATE_IDLE;

  *out_overlay = overlay;
  return CMP_SUCCESS;
}

int cmp_clipboard_overlay_destroy(cmp_clipboard_overlay_t *overlay) {
  if (!overlay)
    return CMP_ERROR_INVALID_ARG;
  free(overlay);
  return CMP_SUCCESS;
}

int cmp_clipboard_overlay_render(cmp_clipboard_overlay_t *overlay,
                                 cmp_code_block_t *block,
                                 cmp_clipboard_state_t state) {
  if (!overlay || !block)
    return CMP_ERROR_INVALID_ARG;

  overlay->state = state;
  /* Visual rendering logic goes here. E.g. update button opacity, icon, etc. */
  return CMP_SUCCESS;
}

int cmp_clipboard_overlay_copy(cmp_clipboard_overlay_t *overlay,
                               cmp_window_t *window, const char *raw_code) {
  int result;

  if (!overlay || !window || !raw_code)
    return CMP_ERROR_INVALID_ARG;

  /* Actually copy to the OS clipboard via cmp */
  result =
      cmp_window_set_clipboard_text(window, CMP_CLIPBOARD_PRIMARY, raw_code);
  if (result == CMP_SUCCESS) {
    overlay->state = CMP_CLIPBOARD_STATE_COPIED;
  }

  return result;
}
