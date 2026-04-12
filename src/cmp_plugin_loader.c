/* clang-format off */
#include "cmp_plugin_loader.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_plugin_loader {
  int dummy;
  int active_plugin_id;
};

int cmp_plugin_loader_create(cmp_plugin_loader_t **out_loader) {
  cmp_plugin_loader_t *loader;

  if (!out_loader) {
    return -1;
  }

  loader = (cmp_plugin_loader_t *)malloc(sizeof(cmp_plugin_loader_t));
  if (!loader) {
    return -2;
  }

  loader->dummy = 0;
  loader->active_plugin_id = 0;
  *out_loader = loader;
  return 0;
}

int cmp_plugin_loader_destroy(cmp_plugin_loader_t *loader) {
  if (!loader) {
    return -1;
  }
  free(loader);
  return 0;
}

int cmp_plugin_loader_load(cmp_plugin_loader_t *loader, const char *path,
                           int *out_plugin_id) {
  if (!loader || !path || !out_plugin_id) {
    return -1;
  }

  /* Mock load */
  loader->active_plugin_id++;
  *out_plugin_id = loader->active_plugin_id;
  return 0;
}

int cmp_plugin_loader_unload(cmp_plugin_loader_t *loader, int plugin_id) {
  if (!loader || plugin_id <= 0) {
    return -1;
  }

  /* Mock unload */
  if (plugin_id == loader->active_plugin_id) {
    loader->active_plugin_id--;
  }

  return 0;
}

int cmp_plugin_loader_execute(cmp_plugin_loader_t *loader, int plugin_id,
                              const char *function_name, const char *payload,
                              char **out_response) {
  const char *dummy_resp = "{\"status\":\"ok\"}";
  size_t len;

  if (!loader || plugin_id <= 0 || !function_name || !out_response) {
    return -1;
  }

  (void)payload;

  len = strlen(dummy_resp);
  *out_response = (char *)malloc(len + 1);
  if (!*out_response) {
    return -2;
  }
  strncpy(*out_response, dummy_resp, len);
  (*out_response)[len] = '\0';

  return 0;
}

int cmp_plugin_loader_free_response(char *response) {
  if (response) {
    free(response);
  }
  return 0;
}
