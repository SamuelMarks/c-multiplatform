/* clang-format off */
#ifndef M3_TYPOGRAPHY_H
#define M3_TYPOGRAPHY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cmp.h>

/**
 * @brief Material 3 Typography Scales.
 */
typedef enum m3_type_scale_t {
    M3_TYPE_DISPLAY_LARGE = 0,
    M3_TYPE_DISPLAY_MEDIUM,
    M3_TYPE_DISPLAY_SMALL,
    M3_TYPE_HEADLINE_LARGE,
    M3_TYPE_HEADLINE_MEDIUM,
    M3_TYPE_HEADLINE_SMALL,
    M3_TYPE_TITLE_LARGE,
    M3_TYPE_TITLE_MEDIUM,
    M3_TYPE_TITLE_SMALL,
    M3_TYPE_LABEL_LARGE,
    M3_TYPE_LABEL_MEDIUM,
    M3_TYPE_LABEL_SMALL,
    M3_TYPE_BODY_LARGE,
    M3_TYPE_BODY_MEDIUM,
    M3_TYPE_BODY_SMALL,
    M3_TYPE_COUNT
} m3_type_scale_t;

/**
 * @brief Typography metrics for a specific scale.
 */
typedef struct m3_type_metrics_t {
    float size_sp;
    float line_height_dp;
    float letter_spacing_em;
    int weight; /* 400 = regular, 500 = medium, 700 = bold */
} m3_type_metrics_t;

/**
 * @brief Global Typography system state for the M3 Catalog.
 */
typedef struct m3_typography_t {
    cmp_font_t *regular_font;
    cmp_font_t *medium_font;
    cmp_font_t *bold_font;
    float accessibility_scale; /* 1.0 = normal, up to 2.0x */
} m3_typography_t;

/**
 * @brief Initializes the M3 Typography system, loading fonts via c-fs (VFS).
 * @param typo Pointer to the typography state to initialize.
 * @param vfs_path_regular VFS path to regular font file.
 * @param vfs_path_medium VFS path to medium font file.
 * @param vfs_path_bold VFS path to bold font file.
 * @return 0 on success, non-zero on error.
 */
int m3_typography_init(m3_typography_t *typo, const char *vfs_path_regular, const char *vfs_path_medium, const char *vfs_path_bold);

/**
 * @brief Cleans up the typography system.
 * @param typo Pointer to the typography state.
 * @return 0 on success, non-zero on error.
 */
int m3_typography_cleanup(m3_typography_t *typo);

/**
 * @brief Gets the exact metrics for a specific M3 scale, applying the accessibility multiplier.
 * @param typo Pointer to the typography state.
 * @param scale The M3 type scale.
 * @param out_metrics Pointer to store the resulting metrics.
 * @return 0 on success, non-zero on error.
 */
int m3_typography_get_metrics(const m3_typography_t *typo, m3_type_scale_t scale, m3_type_metrics_t *out_metrics);

/**
 * @brief Sets the accessibility scaling factor for dynamic type scaling.
 * @param typo Pointer to the typography state.
 * @param scale Multiplier (e.g., 1.5). Clamped between 1.0 and 2.0.
 */
void m3_typography_set_accessibility_scale(m3_typography_t *typo, float scale);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_TYPOGRAPHY_H */
/* clang-format on */