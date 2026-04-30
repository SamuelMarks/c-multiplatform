/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_spellcheck {
  int enabled;
};

/**
 * @brief Create a spellcheck context.
 *
 * @param out_spellcheck Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spellcheck_create(cmp_spellcheck_t **out_spellcheck) {
  int rc = CMP_SUCCESS;
  struct cmp_spellcheck *spellcheck;

  rc = CMP_SUCCESS;

  if (out_spellcheck == NULL) {
    LOG_DEBUG("Invalid argument: out_spellcheck is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_spellcheck), (void **)&spellcheck);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  memset(spellcheck, 0, sizeof(struct cmp_spellcheck));
  spellcheck->enabled = 1;

  *out_spellcheck = (cmp_spellcheck_t *)spellcheck;
  return rc;
}

/**
 * @brief Destroy a spellcheck context.
 *
 * @param spellcheck Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spellcheck_destroy(cmp_spellcheck_t *spellcheck) {
  int rc = CMP_SUCCESS;
  struct cmp_spellcheck *internal_spellcheck;

  rc = CMP_SUCCESS;
  internal_spellcheck = (struct cmp_spellcheck *)spellcheck;

  if (internal_spellcheck == NULL) {
    LOG_DEBUG("Invalid argument: spellcheck is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(internal_spellcheck);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return rc;
}

/**
 * @brief Verify if a word is spelled correctly.
 *
 * @param spellcheck Parameter description.
 * @param word Parameter description.
 * @param out_is_correct Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_spellcheck_verify_word(cmp_spellcheck_t *spellcheck, const char *word,
                               int *out_is_correct) {
  int rc = CMP_SUCCESS;
  struct cmp_spellcheck *internal_spellcheck;

  internal_spellcheck = (struct cmp_spellcheck *)spellcheck;

  if (internal_spellcheck == NULL || word == NULL || out_is_correct == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (!internal_spellcheck->enabled) {
    *out_is_correct = 1;
    return rc;
  }

  /* Dummy logic: anything with 'x' is misspelled */
  if (strchr(word, 'x') != NULL || strchr(word, 'X') != NULL) {
    *out_is_correct = 0;
  } else {
    *out_is_correct = 1;
  }

  return rc;
}
