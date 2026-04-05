/* clang-format off */
#include "cmp.h"
#include "cmp.h"
/* clang-format on */

static cmp_app_region_t *g_chrome_region = NULL;

int cmp_custom_chrome_init(void) {
  int result;

  if (g_chrome_region) {
    /* Already initialized */
    return CMP_SUCCESS;
  }

  result = cmp_app_region_create(&g_chrome_region);
  if (result != CMP_SUCCESS) {
    return result;
  }

  /* Example: Map the top 30 pixels as a draggable titlebar mimic */
  result = cmp_app_region_add_rect(g_chrome_region, 0.0f, 0.0f, 9999.0f, 30.0f,
                                   CMP_APP_REGION_DRAG);
  if (result != CMP_SUCCESS) {
    return result;
  }

  return CMP_SUCCESS;
}

int cmp_custom_chrome_cleanup(void) {
  if (g_chrome_region) {
    int result = cmp_app_region_destroy(g_chrome_region);
    g_chrome_region = NULL;
    return result;
  }
  return CMP_SUCCESS;
}
