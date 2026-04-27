/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
/* clang-format on */

static cmp_app_region_t *g_chrome_region = NULL;

/**
 * @brief cmp_custom_chrome_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_custom_chrome_init(void) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (g_chrome_region != NULL) {
    /* Already initialized */
    cmp_log_debug("cmp_custom_chrome_init: Already initialized\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = cmp_app_region_create(&g_chrome_region);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_custom_chrome_init: Failed to create app region: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Example: Map the top 30 pixels as a draggable titlebar mimic */
  rc = cmp_app_region_add_rect(g_chrome_region, 0.0f, 0.0f, 9999.0f, 30.0f,
                               CMP_APP_REGION_DRAG);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_custom_chrome_init: Failed to add rect to app region: %s\n",
        err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  cmp_log_debug("cmp_custom_chrome_init: Initialized custom chrome\n");
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_custom_chrome_cleanup
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_custom_chrome_cleanup(void) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (g_chrome_region != NULL) {
    rc = cmp_app_region_destroy(g_chrome_region);
    g_chrome_region = NULL;
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug(
          "cmp_custom_chrome_cleanup: Failed to destroy app region: %s\n",
          err_str);
    }
  }

  cmp_log_debug("cmp_custom_chrome_cleanup: Cleanup completed\n");
  if (rc != 0) {
    return rc;
  }
  return rc;
}
