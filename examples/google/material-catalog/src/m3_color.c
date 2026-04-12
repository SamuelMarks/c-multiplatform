#include "m3_color.h"

/* Helper to safely retrieve tones from palette. If exact tone isn't mapped,
 * fallback to nearest or interpolate if needed. But cmp_m3_tonal_palette_t
 * defines tone0, tone10, tone20, tone30, tone40, tone50, tone60, tone70,
 * tone80, tone90, tone95, tone99, tone100.
 */
static cmp_color_t get_tone(const cmp_m3_tonal_palette_t *p, int tone) {
  if (tone <= 0)
    return p->tone0;
  if (tone <= 10)
    return p->tone10;
  if (tone <= 20)
    return p->tone20;
  if (tone <= 30)
    return p->tone30;
  if (tone <= 40)
    return p->tone40;
  if (tone <= 50)
    return p->tone50;
  if (tone <= 60)
    return p->tone60;
  if (tone <= 70)
    return p->tone70;
  if (tone <= 80)
    return p->tone80;
  if (tone <= 90)
    return p->tone90;
  if (tone <= 95)
    return p->tone95;
  if (tone <= 99)
    return p->tone99;
  return p->tone100;
}

int m3_color_generate_roles(cmp_color_t seed, int is_dark,
                            m3_color_roles_t *out_roles) {
  cmp_m3_palettes_t palettes;
  int res;

  if (!out_roles)
    return 1;

  res = cmp_m3_palettes_generate(seed, &palettes);
  if (res != 0)
    return res;

  if (is_dark) {
    /* Primary */
    out_roles->primary = get_tone(&palettes.primary, 80);
    out_roles->on_primary = get_tone(&palettes.primary, 20);
    out_roles->primary_container = get_tone(&palettes.primary, 30);
    out_roles->on_primary_container = get_tone(&palettes.primary, 90);
    out_roles->inverse_primary = get_tone(&palettes.primary, 40);

    /* Secondary */
    out_roles->secondary = get_tone(&palettes.secondary, 80);
    out_roles->on_secondary = get_tone(&palettes.secondary, 20);
    out_roles->secondary_container = get_tone(&palettes.secondary, 30);
    out_roles->on_secondary_container = get_tone(&palettes.secondary, 90);

    /* Tertiary */
    out_roles->tertiary = get_tone(&palettes.tertiary, 80);
    out_roles->on_tertiary = get_tone(&palettes.tertiary, 20);
    out_roles->tertiary_container = get_tone(&palettes.tertiary, 30);
    out_roles->on_tertiary_container = get_tone(&palettes.tertiary, 90);

    /* Error */
    out_roles->error = get_tone(&palettes.error, 80);
    out_roles->on_error = get_tone(&palettes.error, 20);
    out_roles->error_container = get_tone(&palettes.error, 30);
    out_roles->on_error_container = get_tone(&palettes.error, 90);

    /* Surfaces */
    out_roles->surface =
        get_tone(&palettes.neutral, 10); /* approximate tone 6 */
    out_roles->surface_dim =
        get_tone(&palettes.neutral, 10); /* approximate tone 6 */
    out_roles->surface_bright =
        get_tone(&palettes.neutral, 20); /* approximate tone 24 */
    out_roles->surface_container_lowest =
        get_tone(&palettes.neutral, 0); /* approx tone 4 */
    out_roles->surface_container_low = get_tone(&palettes.neutral, 10);
    out_roles->surface_container =
        get_tone(&palettes.neutral, 10); /* approx tone 12 */
    out_roles->surface_container_high =
        get_tone(&palettes.neutral, 20); /* approx tone 17 */
    out_roles->surface_container_highest =
        get_tone(&palettes.neutral, 20); /* approx tone 22 */

    out_roles->on_surface = get_tone(&palettes.neutral, 90);
    out_roles->on_surface_variant = get_tone(&palettes.neutral_variant, 80);
    out_roles->inverse_surface = get_tone(&palettes.neutral, 90);
    out_roles->inverse_on_surface = get_tone(&palettes.neutral, 20);

    /* Outline */
    out_roles->outline = get_tone(&palettes.neutral_variant, 60);
    out_roles->outline_variant = get_tone(&palettes.neutral_variant, 30);

  } else {
    /* Primary */
    out_roles->primary = get_tone(&palettes.primary, 40);
    out_roles->on_primary = get_tone(&palettes.primary, 100);
    out_roles->primary_container = get_tone(&palettes.primary, 90);
    out_roles->on_primary_container = get_tone(&palettes.primary, 10);
    out_roles->inverse_primary = get_tone(&palettes.primary, 80);

    /* Secondary */
    out_roles->secondary = get_tone(&palettes.secondary, 40);
    out_roles->on_secondary = get_tone(&palettes.secondary, 100);
    out_roles->secondary_container = get_tone(&palettes.secondary, 90);
    out_roles->on_secondary_container = get_tone(&palettes.secondary, 10);

    /* Tertiary */
    out_roles->tertiary = get_tone(&palettes.tertiary, 40);
    out_roles->on_tertiary = get_tone(&palettes.tertiary, 100);
    out_roles->tertiary_container = get_tone(&palettes.tertiary, 90);
    out_roles->on_tertiary_container = get_tone(&palettes.tertiary, 10);

    /* Error */
    out_roles->error = get_tone(&palettes.error, 40);
    out_roles->on_error = get_tone(&palettes.error, 100);
    out_roles->error_container = get_tone(&palettes.error, 90);
    out_roles->on_error_container = get_tone(&palettes.error, 10);

    /* Surfaces */
    out_roles->surface = get_tone(&palettes.neutral, 99);        /* tone 98 */
    out_roles->surface_dim = get_tone(&palettes.neutral, 90);    /* tone 87 */
    out_roles->surface_bright = get_tone(&palettes.neutral, 99); /* tone 98 */
    out_roles->surface_container_lowest = get_tone(&palettes.neutral, 100);
    out_roles->surface_container_low =
        get_tone(&palettes.neutral, 95); /* tone 96 */
    out_roles->surface_container =
        get_tone(&palettes.neutral, 95); /* tone 94 */
    out_roles->surface_container_high =
        get_tone(&palettes.neutral, 90); /* tone 92 */
    out_roles->surface_container_highest = get_tone(&palettes.neutral, 90);

    out_roles->on_surface = get_tone(&palettes.neutral, 10);
    out_roles->on_surface_variant = get_tone(&palettes.neutral_variant, 30);
    out_roles->inverse_surface = get_tone(&palettes.neutral, 20);
    out_roles->inverse_on_surface = get_tone(&palettes.neutral, 95);

    /* Outline */
    out_roles->outline = get_tone(&palettes.neutral_variant, 50);
    out_roles->outline_variant = get_tone(&palettes.neutral_variant, 80);
  }

  return 0;
}

static cmp_color_t tween_color(cmp_color_t c1, cmp_color_t c2, float t) {
  cmp_color_t r;
  r.space = c1.space;
  r.r = c1.r + (c2.r - c1.r) * t;
  r.g = c1.g + (c2.g - c1.g) * t;
  r.b = c1.b + (c2.b - c1.b) * t;
  r.a = c1.a + (c2.a - c1.a) * t;
  return r;
}

int m3_color_tween_roles(const m3_color_roles_t *start,
                         const m3_color_roles_t *end, float t,
                         m3_color_roles_t *out_roles) {
  if (!start || !end || !out_roles)
    return 1;

  out_roles->primary = tween_color(start->primary, end->primary, t);
  out_roles->on_primary = tween_color(start->on_primary, end->on_primary, t);
  out_roles->primary_container =
      tween_color(start->primary_container, end->primary_container, t);
  out_roles->on_primary_container =
      tween_color(start->on_primary_container, end->on_primary_container, t);
  out_roles->inverse_primary =
      tween_color(start->inverse_primary, end->inverse_primary, t);

  out_roles->secondary = tween_color(start->secondary, end->secondary, t);
  out_roles->on_secondary =
      tween_color(start->on_secondary, end->on_secondary, t);
  out_roles->secondary_container =
      tween_color(start->secondary_container, end->secondary_container, t);
  out_roles->on_secondary_container = tween_color(
      start->on_secondary_container, end->on_secondary_container, t);

  out_roles->tertiary = tween_color(start->tertiary, end->tertiary, t);
  out_roles->on_tertiary = tween_color(start->on_tertiary, end->on_tertiary, t);
  out_roles->tertiary_container =
      tween_color(start->tertiary_container, end->tertiary_container, t);
  out_roles->on_tertiary_container =
      tween_color(start->on_tertiary_container, end->on_tertiary_container, t);

  out_roles->error = tween_color(start->error, end->error, t);
  out_roles->on_error = tween_color(start->on_error, end->on_error, t);
  out_roles->error_container =
      tween_color(start->error_container, end->error_container, t);
  out_roles->on_error_container =
      tween_color(start->on_error_container, end->on_error_container, t);

  out_roles->surface = tween_color(start->surface, end->surface, t);
  out_roles->surface_dim = tween_color(start->surface_dim, end->surface_dim, t);
  out_roles->surface_bright =
      tween_color(start->surface_bright, end->surface_bright, t);
  out_roles->surface_container_lowest = tween_color(
      start->surface_container_lowest, end->surface_container_lowest, t);
  out_roles->surface_container_low =
      tween_color(start->surface_container_low, end->surface_container_low, t);
  out_roles->surface_container =
      tween_color(start->surface_container, end->surface_container, t);
  out_roles->surface_container_high = tween_color(
      start->surface_container_high, end->surface_container_high, t);
  out_roles->surface_container_highest = tween_color(
      start->surface_container_highest, end->surface_container_highest, t);
  out_roles->on_surface = tween_color(start->on_surface, end->on_surface, t);
  out_roles->on_surface_variant =
      tween_color(start->on_surface_variant, end->on_surface_variant, t);
  out_roles->inverse_surface =
      tween_color(start->inverse_surface, end->inverse_surface, t);
  out_roles->inverse_on_surface =
      tween_color(start->inverse_on_surface, end->inverse_on_surface, t);

  out_roles->outline = tween_color(start->outline, end->outline, t);
  out_roles->outline_variant =
      tween_color(start->outline_variant, end->outline_variant, t);

  return 0;
}
