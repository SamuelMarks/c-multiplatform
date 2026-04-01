/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ime {
  char composition_buffer[128];
};

int cmp_ime_create(cmp_ime_t **out_ime) {
  struct cmp_ime *ime;

  if (!out_ime)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_ime), (void **)&ime) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ime, 0, sizeof(struct cmp_ime));

  *out_ime = (cmp_ime_t *)ime;
  return CMP_SUCCESS;
}

int cmp_ime_destroy(cmp_ime_t *ime) {
  struct cmp_ime *internal_ime = (struct cmp_ime *)ime;
  if (!internal_ime)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_ime);
  return CMP_SUCCESS;
}

int cmp_ime_update_composition(cmp_ime_t *ime, const char *composition_string) {
  struct cmp_ime *internal_ime = (struct cmp_ime *)ime;

  if (!internal_ime || !composition_string)
    return CMP_ERROR_INVALID_ARG;

  if (strlen(composition_string) >= sizeof(internal_ime->composition_buffer))
    return CMP_ERROR_BOUNDS;

  strcpy(internal_ime->composition_buffer, composition_string);

  return CMP_SUCCESS;
}
