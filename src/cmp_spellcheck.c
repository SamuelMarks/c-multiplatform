/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_spellcheck {
  int enabled;
};

int cmp_spellcheck_create(cmp_spellcheck_t **out_spellcheck) {
  struct cmp_spellcheck *spellcheck;

  if (!out_spellcheck)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_spellcheck), (void **)&spellcheck) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(spellcheck, 0, sizeof(struct cmp_spellcheck));
  spellcheck->enabled = 1;

  *out_spellcheck = (cmp_spellcheck_t *)spellcheck;
  return CMP_SUCCESS;
}

int cmp_spellcheck_destroy(cmp_spellcheck_t *spellcheck) {
  struct cmp_spellcheck *internal_spellcheck =
      (struct cmp_spellcheck *)spellcheck;
  if (!internal_spellcheck)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_spellcheck);
  return CMP_SUCCESS;
}

int cmp_spellcheck_verify_word(cmp_spellcheck_t *spellcheck, const char *word,
                               int *out_is_correct) {
  struct cmp_spellcheck *internal_spellcheck =
      (struct cmp_spellcheck *)spellcheck;

  if (!internal_spellcheck || !word || !out_is_correct)
    return CMP_ERROR_INVALID_ARG;

  if (!internal_spellcheck->enabled) {
    *out_is_correct = 1;
    return CMP_SUCCESS;
  }

  /* Dummy logic: anything with 'x' is misspelled */
  if (strchr(word, 'x') != NULL || strchr(word, 'X') != NULL) {
    *out_is_correct = 0;
  } else {
    *out_is_correct = 1;
  }

  return CMP_SUCCESS;
}