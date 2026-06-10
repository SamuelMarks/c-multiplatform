/* clang-format off */
#include "cmp_css_device_adaptation.h"
#include <stddef.h>
/* clang-format on */

int cmp_viewport_width_init(cmp_viewport_width_t *vw) {
  if (!vw) {
    return -1;
  }
  if (cmp_prop_size_init_auto(&vw->min_width) != 0) return -1;
  if (cmp_prop_size_init_auto(&vw->max_width) != 0) return -1;
  if (cmp_prop_size_init_auto(&vw->width) != 0) return -1;
  return 0;
}

int cmp_viewport_height_init(cmp_viewport_height_t *vh) {
  if (!vh) {
    return -1;
  }
  if (cmp_prop_size_init_auto(&vh->min_height) != 0) return -1;
  if (cmp_prop_size_init_auto(&vh->max_height) != 0) return -1;
  if (cmp_prop_size_init_auto(&vh->height) != 0) return -1;
  return 0;
}

int cmp_viewport_zoom_init(cmp_viewport_zoom_t *zoom) {
  if (!zoom) {
    return -1;
  }
  zoom->zoom = 1.0f;
  zoom->min_zoom = 0.0f; /* auto */
  zoom->max_zoom = 0.0f; /* auto */
  zoom->user_zoom = CMP_VIEWPORT_USER_ZOOM_ZOOM;
  return 0;
}
