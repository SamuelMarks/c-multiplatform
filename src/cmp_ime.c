/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ime {
  char composition_buffer[128];
};

/**
 * @brief cmp_ime_create
 *
 * @param out_ime Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ime_create(cmp_ime_t **out_ime) {
  int rc = CMP_SUCCESS;
  struct cmp_ime *ime = NULL;

  if (!out_ime) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ime_create: Invalid argument (out_ime=NULL)\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_ime), (void **)&ime);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ime_create: Out of memory\n");

    return rc;
  }

  memset(ime, 0, sizeof(struct cmp_ime));

  *out_ime = (cmp_ime_t *)ime;

  return rc;
}

/**
 * @brief cmp_ime_destroy
 *
 * @param ime Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ime_destroy(cmp_ime_t *ime) {
  int rc = CMP_SUCCESS;
  struct cmp_ime *internal_ime = (struct cmp_ime *)ime;

  if (!internal_ime) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ime_destroy: Invalid argument (ime=NULL)\n");

    return rc;
  }

  CMP_FREE(internal_ime);

  return rc;
}

/**
 * @brief cmp_ime_update_composition
 *
 * @param ime Parameter description.
 * @param composition_string Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ime_update_composition(cmp_ime_t *ime, const char *composition_string) {
  int rc = CMP_SUCCESS;
  struct cmp_ime *internal_ime = (struct cmp_ime *)ime;

  if (!internal_ime || !composition_string) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ime_update_composition: Invalid argument\n");

    return rc;
  }

  if (strlen(composition_string) >= sizeof(internal_ime->composition_buffer)) {
    rc = CMP_ERROR_BOUNDS;
    LOG_DEBUG(
        "Error in cmp_ime_update_composition: Composition string too long\n");

    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(internal_ime->composition_buffer,
           sizeof(internal_ime->composition_buffer), composition_string);
#else
  strcpy(internal_ime->composition_buffer, composition_string);
#endif

  return rc;
}
