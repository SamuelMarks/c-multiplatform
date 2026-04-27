/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include "cmp_plugin_loader.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_plugin_loader {
  int dummy;
  int active_plugin_id;
};

/**
 * @brief cmp_plugin_loader_create
 *
 * @param out_loader Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_plugin_loader_create(cmp_plugin_loader_t **out_loader) {
  int rc;
  rc = CMP_SUCCESS;
  cmp_plugin_loader_t *loader = NULL;

  if (!out_loader) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_plugin_loader_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_plugin_loader_t), (void **)&(loader));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_plugin_loader_create: Out of memory\n");
    return CMP_ERROR_OOM;
  }

  loader->dummy = 0;
  loader->active_plugin_id = 0;
  *out_loader = loader;
  return rc;
}

/**
 * @brief cmp_plugin_loader_destroy
 *
 * @param loader Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_plugin_loader_destroy(cmp_plugin_loader_t *loader) {
  int rc;
  rc = CMP_SUCCESS;

  if (!loader) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_plugin_loader_destroy: Invalid argument\n");
    return rc;
  }

  rc = CMP_FREE(loader);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_plugin_loader_destroy: CMP_FREE failed\n");
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_plugin_loader_load
 *
 * @param loader Parameter description.
 * @param path Parameter description.
 * @param out_plugin_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_plugin_loader_load(cmp_plugin_loader_t *loader, const char *path,
                           int *out_plugin_id) {
  int rc;
  rc = CMP_SUCCESS;

  if (!loader || !path || !out_plugin_id) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_plugin_loader_load: Invalid argument\n");
    return rc;
  }

  /* Mock load */
  loader->active_plugin_id++;
  *out_plugin_id = loader->active_plugin_id;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_plugin_loader_unload
 *
 * @param loader Parameter description.
 * @param plugin_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_plugin_loader_unload(cmp_plugin_loader_t *loader, int plugin_id) {
  int rc;
  rc = CMP_SUCCESS;

  if (!loader || plugin_id <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_plugin_loader_unload: Invalid argument\n");
    return rc;
  }

  /* Mock unload */
  if (plugin_id == loader->active_plugin_id) {
    loader->active_plugin_id--;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_plugin_loader_execute
 *
 * @param loader Parameter description.
 * @param plugin_id Parameter description.
 * @param function_name Parameter description.
 * @param payload Parameter description.
 * @param out_response Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_plugin_loader_execute(cmp_plugin_loader_t *loader, int plugin_id,
                              const char *function_name, const char *payload,
                              char **out_response) {
  int rc;
  rc = CMP_SUCCESS;
  const char *dummy_resp = "{\"status\":\"ok\"}";
  size_t len;

  if (!loader || plugin_id <= 0 || !function_name || !out_response) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_plugin_loader_execute: Invalid argument\n");
    return rc;
  }

  (void)payload;

  len = strlen(dummy_resp);
  rc = CMP_MALLOC(len + 1, (void **)&(*out_response));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_plugin_loader_execute: Out of memory\n");
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strncpy_s(*out_response, len + 1, dummy_resp, _TRUNCATE);
#else
  strncpy(*out_response, dummy_resp, len);
#endif
  (*out_response)[len] = '\0';

  if (rc != 0) {

    return rc;
  }

  return rc;
}

/**
 * @brief cmp_plugin_loader_free_response
 *
 * @param response Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_plugin_loader_free_response(char *response) {
  int rc;
  rc = CMP_SUCCESS;

  if (!response) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_plugin_loader_free_response: Invalid argument\n");
    return rc;
  }

  rc = CMP_FREE(response);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_plugin_loader_free_response: CMP_FREE failed\n");
    return rc;
  }

  if (rc != 0) {

    return rc;
  }

  return rc;
}
