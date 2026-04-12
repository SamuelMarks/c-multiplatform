#include "m3_typography.h"

/* Predefined metrics based on M3 spec */
static const m3_type_metrics_t base_metrics[M3_TYPE_COUNT] = {
    /* Display */
    {57.0f, 64.0f, 0.0f, 400}, /* DISPLAY_LARGE */
    {45.0f, 52.0f, 0.0f, 400}, /* DISPLAY_MEDIUM */
    {36.0f, 44.0f, 0.0f, 400}, /* DISPLAY_SMALL */
    /* Headline */
    {32.0f, 40.0f, 0.0f, 400}, /* HEADLINE_LARGE */
    {28.0f, 36.0f, 0.0f, 400}, /* HEADLINE_MEDIUM */
    {24.0f, 32.0f, 0.0f, 400}, /* HEADLINE_SMALL */
    /* Title */
    {22.0f, 28.0f, 0.0f, 400},   /* TITLE_LARGE */
    {16.0f, 24.0f, 0.015f, 500}, /* TITLE_MEDIUM */
    {14.0f, 20.0f, 0.01f, 500},  /* TITLE_SMALL */
    /* Label */
    {14.0f, 20.0f, 0.01f, 500}, /* LABEL_LARGE */
    {12.0f, 16.0f, 0.05f, 500}, /* LABEL_MEDIUM */
    {11.0f, 16.0f, 0.05f, 500}, /* LABEL_SMALL */
    /* Body */
    {16.0f, 24.0f, 0.05f, 400},  /* BODY_LARGE */
    {14.0f, 20.0f, 0.025f, 400}, /* BODY_MEDIUM */
    {12.0f, 16.0f, 0.04f, 400}   /* BODY_SMALL */
};

/* Forward declaration for cmp_font_load and destroy which we assume exists in
 * cmp.h */
extern int cmp_font_load(const char *virtual_path, float default_size,
                         cmp_font_t **out_font);
extern int cmp_font_destroy(cmp_font_t *font);

int m3_typography_init(m3_typography_t *typo, const char *vfs_path_regular,
                       const char *vfs_path_medium, const char *vfs_path_bold) {
  int res;

  if (!typo)
    return 1;

  typo->accessibility_scale = 1.0f;
  typo->regular_font = NULL;
  typo->medium_font = NULL;
  typo->bold_font = NULL;

  if (vfs_path_regular) {
    res = cmp_font_load(vfs_path_regular, 14.0f, &typo->regular_font);
    if (res != 0)
      return res;
  }
  if (vfs_path_medium) {
    res = cmp_font_load(vfs_path_medium, 14.0f, &typo->medium_font);
    if (res != 0)
      return res;
  }
  if (vfs_path_bold) {
    res = cmp_font_load(vfs_path_bold, 14.0f, &typo->bold_font);
    if (res != 0)
      return res;
  }

  return 0;
}

int m3_typography_cleanup(m3_typography_t *typo) {
  if (!typo)
    return 1;

  if (typo->regular_font) {
    cmp_font_destroy(typo->regular_font);
    typo->regular_font = NULL;
  }
  if (typo->medium_font) {
    cmp_font_destroy(typo->medium_font);
    typo->medium_font = NULL;
  }
  if (typo->bold_font) {
    cmp_font_destroy(typo->bold_font);
    typo->bold_font = NULL;
  }

  return 0;
}

int m3_typography_get_metrics(const m3_typography_t *typo,
                              m3_type_scale_t scale,
                              m3_type_metrics_t *out_metrics) {
  if (!typo || !out_metrics)
    return 1;
  if (scale < 0 || scale >= M3_TYPE_COUNT)
    return 1;

  *out_metrics = base_metrics[scale];

  /* Apply accessibility multiplier strictly to the font size and line height */
  out_metrics->size_sp *= typo->accessibility_scale;
  out_metrics->line_height_dp *= typo->accessibility_scale;

  return 0;
}

void m3_typography_set_accessibility_scale(m3_typography_t *typo, float scale) {
  if (!typo)
    return;

  if (scale < 1.0f)
    scale = 1.0f;
  if (scale > 2.0f)
    scale = 2.0f;

  typo->accessibility_scale = scale;
}
