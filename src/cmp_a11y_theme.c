/* clang-format off */
#include "cmp.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
/* clang-format on */

int cmp_a11y_theme_init(void) {
  /* Initialize any underlying resources or OS hooks if needed */
  return 0;
}

int cmp_a11y_theme_cleanup(void) {
  /* Clean up */
  return 0;
}

int cmp_a11y_detect_high_contrast(void) {
#if defined(_WIN32)
  HIGHCONTRASTA hc;
  hc.cbSize = sizeof(HIGHCONTRASTA);
  if (SystemParametersInfoA(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRASTA), &hc,
                            0)) {
    if (hc.dwFlags & HCF_HIGHCONTRASTON) {
      return 1;
    }
  }
#else
  /* Future platform support goes here. */
#endif
  return 0;
}

/* Helper to map standard colors based on color blindness types */
static void apply_color_blindness(cmp_color_blind_type_t type,
                                  cmp_color_t *out_accent,
                                  cmp_color_t *out_error) {
  /* Default palettes for regular vision */
  /* Blue-ish primary */
  out_accent->r = 0.0f;
  out_accent->g = 0.47f;
  out_accent->b = 1.0f;
  out_accent->a = 1.0f;

  /* Red error */
  out_error->r = 1.0f;
  out_error->g = 0.23f;
  out_error->b = 0.18f;
  out_error->a = 1.0f;

  /* Shift colors to remain distinguishable and safe for the specified defect */
  switch (type) {
  case CMP_COLOR_BLIND_PROTANOPIA:
  case CMP_COLOR_BLIND_DEUTERANOPIA:
    /* Red-Green blindness:
       Shift accents to deeper Blues/Yellows so they remain distinct.
       Shift error red to a distinct Orange/Magenta. */
    out_accent->r = 0.0f;
    out_accent->g = 0.33f;
    out_accent->b = 0.88f; /* High contrast blue */
    out_error->r = 0.85f;
    out_error->g = 0.37f;
    out_error->b = 0.06f; /* Orange-shift for error */
    break;
  case CMP_COLOR_BLIND_TRITANOPIA:
    /* Blue-Yellow blindness:
       Shift blue accent to red/teal. */
    out_accent->r = 0.85f;
    out_accent->g = 0.11f;
    out_accent->b = 0.38f;
    out_error->r = 0.8f;
    out_error->g = 0.0f;
    out_error->b = 0.0f;
    break;
  case CMP_COLOR_BLIND_NONE:
  default:
    break; /* Use defaults */
  }
}

int cmp_a11y_build_theme(cmp_color_blind_type_t type,
                         cmp_a11y_theme_t *out_theme) {
  int is_hc;

  if (!out_theme) {
    return -1;
  }

  is_hc = cmp_a11y_detect_high_contrast();
  out_theme->is_high_contrast = is_hc;

  if (is_hc) {
    /* Extreme contrast: pure black/white */
    out_theme->background.r = 0.0f;
    out_theme->background.g = 0.0f;
    out_theme->background.b = 0.0f;
    out_theme->background.a = 1.0f;

    out_theme->foreground.r = 1.0f;
    out_theme->foreground.g = 1.0f;
    out_theme->foreground.b = 1.0f;
    out_theme->foreground.a = 1.0f;

    /* High contrast mode requires bright identifiable primary colors */
    out_theme->primary_accent.r = 1.0f;
    out_theme->primary_accent.g = 1.0f;
    out_theme->primary_accent.b = 0.0f; /* High Contrast Yellow */
    out_theme->primary_accent.a = 1.0f;

    out_theme->error_text.r = 1.0f;
    out_theme->error_text.g = 0.0f;
    out_theme->error_text.b = 0.0f; /* Pure Red */
    out_theme->error_text.a = 1.0f;
  } else {
    /* Standard theme */
    out_theme->background.r = 0.1f;
    out_theme->background.g = 0.1f;
    out_theme->background.b = 0.12f;
    out_theme->background.a = 1.0f;

    out_theme->foreground.r = 0.9f;
    out_theme->foreground.g = 0.9f;
    out_theme->foreground.b = 0.9f;
    out_theme->foreground.a = 1.0f;

    apply_color_blindness(type, &out_theme->primary_accent,
                          &out_theme->error_text);
  }

  return 0;
}
