/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_search_controller {
  cmp_search_bar_state_t state;
  char *text;
  char **scopes;
  size_t scope_count;
  char **suggestions;
  size_t suggestion_count;
};

/**
 * @brief cmp_search_controller_create
 *
 * @param out_controller Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_search_controller_create(cmp_search_controller_t **out_controller) {
  int rc = CMP_SUCCESS;
  struct cmp_search_controller *ctx;
  if (!out_controller)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_search_controller), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->state = CMP_SEARCH_BAR_INACTIVE;
  ctx->text = NULL;
  ctx->scopes = NULL;
  ctx->scope_count = 0;
  ctx->suggestions = NULL;
  ctx->suggestion_count = 0;

  *out_controller = (cmp_search_controller_t *)ctx;

  return rc;
}

/**
 * @brief cmp_search_controller_destroy
 *
 * @param controller Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_search_controller_destroy(cmp_search_controller_t *controller) {
  int rc = CMP_SUCCESS;
  struct cmp_search_controller *ctx =
      (struct cmp_search_controller *)controller;
  size_t i;
  if (!ctx)
    return rc;

  if (ctx->text)
    CMP_FREE(ctx->text);

  if (ctx->scopes) {
    for (i = 0; i < ctx->scope_count; ++i) {
      CMP_FREE(ctx->scopes[i]);
    }
    CMP_FREE(ctx->scopes);
  }

  if (ctx->suggestions) {
    for (i = 0; i < ctx->suggestion_count; ++i) {
      CMP_FREE(ctx->suggestions[i]);
    }
    CMP_FREE(ctx->suggestions);
  }

  CMP_FREE(ctx);

  return rc;
}

/**
 * @brief cmp_search_controller_set_text
 *
 * @param controller Parameter description.
 * @param text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_search_controller_set_text(cmp_search_controller_t *controller,
                                   const char *text) {
  int rc = CMP_SUCCESS;
  struct cmp_search_controller *ctx =
      (struct cmp_search_controller *)controller;
  size_t len;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->text) {
    CMP_FREE(ctx->text);
    ctx->text = NULL;
  }

  if (text) {
    len = strlen(text);
    if (CMP_MALLOC(len + 1, (void **)&ctx->text) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
#if defined(_MSC_VER)
    strcpy_s(ctx->text, len + 1, text);
#else
    strcpy(ctx->text, text);
#endif
  }

  return rc;
}

/**
 * @brief cmp_search_controller_add_scope
 *
 * @param controller Parameter description.
 * @param scope_title Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_search_controller_add_scope(cmp_search_controller_t *controller,
                                    const char *scope_title) {
  int rc = CMP_SUCCESS;
  struct cmp_search_controller *ctx =
      (struct cmp_search_controller *)controller;
  char **new_scopes;
  size_t len;

  if (!ctx || !scope_title)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC((ctx->scope_count + 1) * sizeof(char *),
                 (void **)&new_scopes) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  if (ctx->scopes) {
    memcpy(new_scopes, ctx->scopes, ctx->scope_count * sizeof(char *));
    CMP_FREE(ctx->scopes);
  }
  ctx->scopes = new_scopes;

  len = strlen(scope_title);
  if (CMP_MALLOC(len + 1, (void **)&ctx->scopes[ctx->scope_count]) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(ctx->scopes[ctx->scope_count], len + 1, scope_title);
#else
  strcpy(ctx->scopes[ctx->scope_count], scope_title);
#endif

  ctx->scope_count++;

  return rc;
}

/**
 * @brief cmp_search_controller_set_active
 *
 * @param controller Parameter description.
 * @param active Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_search_controller_set_active(cmp_search_controller_t *controller,
                                     int active) {
  int rc = CMP_SUCCESS;
  struct cmp_search_controller *ctx =
      (struct cmp_search_controller *)controller;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->state = active ? CMP_SEARCH_BAR_ACTIVE : CMP_SEARCH_BAR_INACTIVE;

  return rc;
}

/**
 * @brief cmp_search_controller_resolve_metrics
 *
 * @param controller Parameter description.
 * @param available_width Parameter description.
 * @param out_show_clear Parameter description.
 * @param out_show_cancel Parameter description.
 * @param out_placeholder_offset Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_search_controller_resolve_metrics(cmp_search_controller_t *controller,
                                          float available_width,
                                          int *out_show_clear,
                                          int *out_show_cancel,
                                          float *out_placeholder_offset) {
  int rc = CMP_SUCCESS;
  struct cmp_search_controller *ctx =
      (struct cmp_search_controller *)controller;
  float content_w;
  if (!ctx || !out_show_clear || !out_show_cancel || !out_placeholder_offset)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->state == CMP_SEARCH_BAR_INACTIVE) {
    /* Centered, no clear, no cancel */
    *out_show_clear = 0;
    *out_show_cancel = 0;

    content_w = ctx->text ? (float)strlen(ctx->text) * 8.0f
                          : 80.0f; /* approximate placeholder text width */
    *out_placeholder_offset = (available_width / 2.0f) - (content_w / 2.0f);
    if (*out_placeholder_offset < 16.0f)
      *out_placeholder_offset = 16.0f; /* margin */
  } else {
    /* Active: Leading aligned, show clear if text exists, show cancel button */
    *out_show_clear = (ctx->text && strlen(ctx->text) > 0) ? 1 : 0;
    *out_show_cancel = 1;
    *out_placeholder_offset = 16.0f; /* Standard leading margin */
  }

  return rc;
}

/**
 * @brief cmp_search_controller_set_suggestions
 *
 * @param controller Parameter description.
 * @param suggestions Parameter description.
 * @param count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_search_controller_set_suggestions(cmp_search_controller_t *controller,
                                          const char **suggestions,
                                          size_t count) {
  int rc = CMP_SUCCESS;
  struct cmp_search_controller *ctx =
      (struct cmp_search_controller *)controller;
  char **new_sug;
  size_t i, len;

  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->suggestions) {
    for (i = 0; i < ctx->suggestion_count; ++i) {
      CMP_FREE(ctx->suggestions[i]);
    }
    CMP_FREE(ctx->suggestions);
    ctx->suggestions = NULL;
    ctx->suggestion_count = 0;
  }

  if (count > 0 && suggestions) {
    if (CMP_MALLOC(count * sizeof(char *), (void **)&new_sug) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
    for (i = 0; i < count; ++i) {
      len = strlen(suggestions[i]);
      if (CMP_MALLOC(len + 1, (void **)&new_sug[i]) != CMP_SUCCESS)
        return CMP_ERROR_OOM;
#if defined(_MSC_VER)
      strcpy_s(new_sug[i], len + 1, suggestions[i]);
#else
      strcpy(new_sug[i], suggestions[i]);
#endif
    }
    ctx->suggestions = new_sug;
    ctx->suggestion_count = count;
  }

  return rc;
}
