/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_nav_bar {
  int prefers_large_titles;
  cmp_nav_bar_appearance_t appearance;
};

int cmp_nav_bar_create(cmp_nav_bar_t **out_nav_bar) {
  struct cmp_nav_bar *ctx;
  if (!out_nav_bar)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_nav_bar), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->prefers_large_titles = 1; /* HIG default */
  ctx->appearance = CMP_NAV_BAR_APPEARANCE_STANDARD;

  *out_nav_bar = (cmp_nav_bar_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_nav_bar_destroy(cmp_nav_bar_t *nav_bar) {
  if (nav_bar)
    CMP_FREE(nav_bar);
  return CMP_SUCCESS;
}

int cmp_nav_bar_set_prefers_large_titles(cmp_nav_bar_t *nav_bar,
                                         int prefers_large_titles) {
  struct cmp_nav_bar *ctx = (struct cmp_nav_bar *)nav_bar;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->prefers_large_titles = prefers_large_titles;
  return CMP_SUCCESS;
}

int cmp_nav_bar_calculate_scroll_collapse(cmp_nav_bar_t *nav_bar,
                                          float scroll_y,
                                          float *out_title_scale,
                                          float *out_title_y_offset) {
  struct cmp_nav_bar *ctx = (struct cmp_nav_bar *)nav_bar;
  float progress;
  if (!ctx || !out_title_scale || !out_title_y_offset)
    return CMP_ERROR_INVALID_ARG;

  if (!ctx->prefers_large_titles) {
    *out_title_scale = 1.0f;
    *out_title_y_offset = 0.0f;
    return CMP_SUCCESS;
  }

  /* HIG large title collapse occurs over ~50pts of scrolling */
  if (scroll_y <= 0.0f) {
    *out_title_scale = 1.0f;
    *out_title_y_offset = 0.0f; /* At rest */
  } else if (scroll_y >= 50.0f) {
    *out_title_scale = 0.6f;      /* Approximate inline size ratio */
    *out_title_y_offset = -40.0f; /* Moved up into the nav bar center */
  } else {
    progress = scroll_y / 50.0f;
    *out_title_scale = 1.0f - (0.4f * progress);
    *out_title_y_offset = -40.0f * progress;
  }

  return CMP_SUCCESS;
}

int cmp_nav_bar_set_appearance(cmp_nav_bar_t *nav_bar,
                               cmp_nav_bar_appearance_t appearance) {
  struct cmp_nav_bar *ctx = (struct cmp_nav_bar *)nav_bar;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->appearance = appearance;
  return CMP_SUCCESS;
}

int cmp_nav_bar_resolve_back_button_label(cmp_nav_bar_t *nav_bar,
                                          const char *previous_title,
                                          float available_width,
                                          char *out_label, size_t label_cap) {
  size_t len;
  (void)nav_bar;
  if (!out_label || label_cap == 0)
    return CMP_ERROR_INVALID_ARG;

  if (!previous_title) {
#if defined(_MSC_VER)
    strcpy_s(out_label, label_cap, "Back");
#else
    strcpy(out_label, "Back");
#endif
    return CMP_SUCCESS;
  }

  len = strlen(previous_title);

  /* Naive approximation: ~10px per character for standard body weight font.
     If it doesn't fit, truncate to "Back". */
  if ((float)len * 10.0f > available_width) {
#if defined(_MSC_VER)
    strcpy_s(out_label, label_cap, "Back");
#else
    strcpy(out_label, "Back");
#endif
  } else {
#if defined(_MSC_VER)
    strcpy_s(out_label, label_cap, previous_title);
#else
    strcpy(out_label, previous_title);
#endif
  }

  return CMP_SUCCESS;
}

int cmp_nav_bar_get_chevron_metrics(cmp_nav_bar_t *nav_bar, float *out_weight,
                                    float *out_leading_padding) {
  if (!nav_bar || !out_weight || !out_leading_padding)
    return CMP_ERROR_INVALID_ARG;

  /* HIG: Semibold/Heavy weight for back chevron, 8pt leading padding minimum */
  *out_weight = 600.0f;
  *out_leading_padding = 8.0f;

  return CMP_SUCCESS;
}
