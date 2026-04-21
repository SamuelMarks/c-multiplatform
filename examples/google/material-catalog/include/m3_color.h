/* clang-format off */
#ifndef M3_COLOR_H
#define M3_COLOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cmp.h>
#include <themes/cmp_material3_color.h>

/**
 * @brief Semantic Color Roles for Material 3.
 */
typedef struct m3_color_roles_t {
    /* Primary */
    cmp_color_t primary;
    cmp_color_t on_primary;
    cmp_color_t primary_container;
    cmp_color_t on_primary_container;
    cmp_color_t inverse_primary;

    /* Secondary */
    cmp_color_t secondary;
    cmp_color_t on_secondary;
    cmp_color_t secondary_container;
    cmp_color_t on_secondary_container;

    /* Tertiary */
    cmp_color_t tertiary;
    cmp_color_t on_tertiary;
    cmp_color_t tertiary_container;
    cmp_color_t on_tertiary_container;

    /* Error */
    cmp_color_t error;
    cmp_color_t on_error;
    cmp_color_t error_container;
    cmp_color_t on_error_container;

    /* Surfaces */
    cmp_color_t surface;
    cmp_color_t surface_dim;
    cmp_color_t surface_bright;
    cmp_color_t surface_container_lowest;
    cmp_color_t surface_container_low;
    cmp_color_t surface_container;
    cmp_color_t surface_container_high;
    cmp_color_t surface_container_highest;
    cmp_color_t on_surface;
    cmp_color_t on_surface_variant;
    cmp_color_t inverse_surface;
    cmp_color_t inverse_on_surface;

    /* Outline */
    cmp_color_t outline;
    cmp_color_t outline_variant;
} m3_color_roles_t;

/**
 * @brief Generate the 30+ M3 Semantic Roles from a seed color.
 * @param seed The seed color to generate from.
 * @param is_dark Whether to generate dark mode roles (1) or light mode (0).
 * @param out_roles Output pointer for generated roles.
 * @return 0 on success, non-zero on error.
 */
int m3_color_generate_roles(cmp_color_t seed, int is_dark, m3_color_roles_t *out_roles);

/**
 * @brief Tween (interpolate) between two sets of color roles (e.g. for light to dark mode transition).
 * @param start The initial color roles.
 * @param end The target color roles.
 * @param t Interpolation factor (0.0 to 1.0).
 * @param out_roles Output for the interpolated color roles.
 * @return 0 on success, non-zero on error.
 */
int m3_color_tween_roles(const m3_color_roles_t *start, const m3_color_roles_t *end, float t, m3_color_roles_t *out_roles);

struct catalog_state;
int material_catalog_update_sys_colors_hex(struct catalog_state *state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_COLOR_H */
/* clang-format on */