/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
/* clang-format on */

static cmp_app_region_t *g_chrome_region = NULL;

int cmp_custom_chrome_init(void) {
  int rc = CMP_SUCCESS;

  if (g_chrome_region) {
    /* Already initialized */
    return rc;
  }

  rc = cmp_app_region_create(&g_chrome_region);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_custom_chrome_init: Failed to create app region\n");
    return rc;
  }

  /* Example: Map the top 30 pixels as a draggable titlebar mimic */
  rc = cmp_app_region_add_rect(g_chrome_region, 0.0f, 0.0f, 9999.0f, 30.0f,
                               CMP_APP_REGION_DRAG);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_custom_chrome_init: Failed to add rect to app region\n");
    return rc;
  }

  return rc;
}

int cmp_custom_chrome_cleanup(void) {
  int rc = CMP_SUCCESS;

  if (g_chrome_region) {
    rc = cmp_app_region_destroy(g_chrome_region);
    g_chrome_region = NULL;
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_custom_chrome_cleanup: Failed to destroy app region\n");
    }
  }
  return rc;
}
