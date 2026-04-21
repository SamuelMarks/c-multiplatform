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

int cmp_plugin_loader_create(cmp_plugin_loader_t **out_loader) {
  int rc = CMP_SUCCESS;
  cmp_plugin_loader_t *loader = NULL;

  if (!out_loader) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_plugin_loader_create: Invalid argument\n");
    return rc;
  }

  loader = (cmp_plugin_loader_t *)malloc(sizeof(cmp_plugin_loader_t));
  if (!loader) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_plugin_loader_create: Out of memory\n");
    return rc;
  }

  loader->dummy = 0;
  loader->active_plugin_id = 0;
  *out_loader = loader;
  return rc;
}

int cmp_plugin_loader_destroy(cmp_plugin_loader_t *loader) {
  int rc = CMP_SUCCESS;

  if (!loader) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_plugin_loader_destroy: Invalid argument\n");
    return rc;
  }
  free(loader);
  return rc;
}

int cmp_plugin_loader_load(cmp_plugin_loader_t *loader, const char *path,
                           int *out_plugin_id) {
  int rc = CMP_SUCCESS;

  if (!loader || !path || !out_plugin_id) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_plugin_loader_load: Invalid argument\n");
    return rc;
  }

  /* Mock load */
  loader->active_plugin_id++;
  *out_plugin_id = loader->active_plugin_id;
  return rc;
}

int cmp_plugin_loader_unload(cmp_plugin_loader_t *loader, int plugin_id) {
  int rc = CMP_SUCCESS;

  if (!loader || plugin_id <= 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_plugin_loader_unload: Invalid argument\n");
    return rc;
  }

  /* Mock unload */
  if (plugin_id == loader->active_plugin_id) {
    loader->active_plugin_id--;
  }

  return rc;
}

int cmp_plugin_loader_execute(cmp_plugin_loader_t *loader, int plugin_id,
                              const char *function_name, const char *payload,
                              char **out_response) {
  int rc = CMP_SUCCESS;
  const char *dummy_resp = "{\"status\":\"ok\"}";
  size_t len;

  if (!loader || plugin_id <= 0 || !function_name || !out_response) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_plugin_loader_execute: Invalid argument\n");
    return rc;
  }

  (void)payload;

  len = strlen(dummy_resp);
  *out_response = (char *)malloc(len + 1);
  if (!*out_response) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_plugin_loader_execute: Out of memory\n");
    return rc;
  }
#if defined(_MSC_VER)
  strncpy_s(*out_response, len + 1, dummy_resp, _TRUNCATE);
#else
  strncpy(*out_response, dummy_resp, len);
#endif
  (*out_response)[len] = '\0';

  return rc;
}

int cmp_plugin_loader_free_response(char *response) {
  int rc = CMP_SUCCESS;

  if (response) {
    free(response);
  }
  return rc;
}
