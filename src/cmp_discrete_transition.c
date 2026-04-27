/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_discrete_transition {
  int is_active;
};

/**
 * @brief cmp_discrete_transition_create
 *
 * @param out_transition Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_discrete_transition_create(cmp_discrete_transition_t **out_transition) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_discrete_transition *transition = NULL;

  if (out_transition == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_discrete_transition_create: Invalid argument "
                  "(out_transition=NULL): %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_discrete_transition), (void **)&transition);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_discrete_transition_create: Out of memory: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(transition, 0, sizeof(struct cmp_discrete_transition));

  *out_transition = (cmp_discrete_transition_t *)transition;
  cmp_log_debug("cmp_discrete_transition_create: Successfully created "
                "transition context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_discrete_transition_destroy
 *
 * @param transition Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_discrete_transition_destroy(cmp_discrete_transition_t *transition) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_discrete_transition *internal_transition =
      (struct cmp_discrete_transition *)transition;

  if (internal_transition == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_discrete_transition_destroy: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_FREE(internal_transition);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_discrete_transition_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_discrete_transition_destroy: Successfully destroyed "
                "transition context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_discrete_transition_evaluate
 *
 * @param transition Parameter description.
 * @param progress Parameter description.
 * @param out_is_visible Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_discrete_transition_evaluate(cmp_discrete_transition_t *transition,
                                     float progress, int *out_is_visible) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_discrete_transition *internal_transition =
      (struct cmp_discrete_transition *)transition;

  if (internal_transition == NULL || out_is_visible == NULL ||
      progress < 0.0f || progress > 1.0f) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_discrete_transition_evaluate: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (progress < 0.5f) {
    *out_is_visible = 0; /* Old state / invisible */
  } else {
    *out_is_visible = 1; /* New state / visible */
  }

  cmp_log_debug("cmp_discrete_transition_evaluate: Evaluated step %.2f to "
                "visibility=%d\n",
                progress, *out_is_visible);
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
