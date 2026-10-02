/**
 * @file cupertino_tokens.c
 * @brief Implementation of Apple Human Interface Guidelines design tokens.
 */

/* clang-format off */
#include "cupertino/cupertino_tokens.h"
#include <string.h>
/* clang-format on */

/**
 * @brief Resolves an Apple dynamic system color for light/dark mode and high
 * contrast.
 */
ui_error_t cupertino_get_system_color(enum cupertino_system_color color,
                                      int is_dark, int high_contrast,
                                      ui_color_t *out_color) {
  if (!out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (high_contrast) {
    switch (color) {
    case CUPERTINO_COLOR_BLUE:
      *out_color = UI_COLOR_ARGB(255, 0x00, 0x40, 0xDD);
      break;
    case CUPERTINO_COLOR_GREEN:
      *out_color = UI_COLOR_ARGB(255, 0x24, 0x8A, 0x3D);
      break;
    case CUPERTINO_COLOR_INDIGO:
      *out_color = UI_COLOR_ARGB(255, 0x36, 0x34, 0xA3);
      break;
    case CUPERTINO_COLOR_ORANGE:
      *out_color = UI_COLOR_ARGB(255, 0xC9, 0x34, 0x00);
      break;
    case CUPERTINO_COLOR_PINK:
      *out_color = UI_COLOR_ARGB(255, 0xD3, 0x0F, 0x45);
      break;
    case CUPERTINO_COLOR_PURPLE:
      *out_color = UI_COLOR_ARGB(255, 0x89, 0x44, 0xAB);
      break;
    case CUPERTINO_COLOR_RED:
      *out_color = UI_COLOR_ARGB(255, 0xD7, 0x00, 0x15);
      break;
    case CUPERTINO_COLOR_TEAL:
      *out_color = UI_COLOR_ARGB(255, 0x00, 0x71, 0xA4);
      break;
    case CUPERTINO_COLOR_YELLOW:
      *out_color = UI_COLOR_ARGB(255, 0xB2, 0x50, 0x00);
      break;
    case CUPERTINO_COLOR_MINT:
      *out_color = UI_COLOR_ARGB(255, 0x0C, 0x81, 0x7B);
      break;
    case CUPERTINO_COLOR_CYAN:
      *out_color = UI_COLOR_ARGB(255, 0x00, 0x71, 0xA4);
      break;
    default:
      return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
  }

  if (is_dark) {
    switch (color) {
    case CUPERTINO_COLOR_BLUE:
      *out_color = UI_COLOR_ARGB(255, 0x0A, 0x84, 0xFF);
      break;
    case CUPERTINO_COLOR_GREEN:
      *out_color = UI_COLOR_ARGB(255, 0x30, 0xD1, 0x58);
      break;
    case CUPERTINO_COLOR_INDIGO:
      *out_color = UI_COLOR_ARGB(255, 0x5E, 0x5C, 0xE6);
      break;
    case CUPERTINO_COLOR_ORANGE:
      *out_color = UI_COLOR_ARGB(255, 0xFF, 0x9F, 0x0A);
      break;
    case CUPERTINO_COLOR_PINK:
      *out_color = UI_COLOR_ARGB(255, 0xFF, 0x37, 0x5F);
      break;
    case CUPERTINO_COLOR_PURPLE:
      *out_color = UI_COLOR_ARGB(255, 0xBF, 0x5A, 0xF2);
      break;
    case CUPERTINO_COLOR_RED:
      *out_color = UI_COLOR_ARGB(255, 0xFF, 0x45, 0x3A);
      break;
    case CUPERTINO_COLOR_TEAL:
      *out_color = UI_COLOR_ARGB(255, 0x64, 0xD2, 0xFF);
      break;
    case CUPERTINO_COLOR_YELLOW:
      *out_color = UI_COLOR_ARGB(255, 0xFF, 0xD6, 0x0A);
      break;
    case CUPERTINO_COLOR_MINT:
      *out_color = UI_COLOR_ARGB(255, 0x63, 0xE6, 0xE2);
      break;
    case CUPERTINO_COLOR_CYAN:
      *out_color = UI_COLOR_ARGB(255, 0x64, 0xD2, 0xFF);
      break;
    default:
      return UI_ERROR_INVALID_ARGUMENT;
    }
  } else {
    switch (color) {
    case CUPERTINO_COLOR_BLUE:
      *out_color = UI_COLOR_ARGB(255, 0x00, 0x7A, 0xFF);
      break;
    case CUPERTINO_COLOR_GREEN:
      *out_color = UI_COLOR_ARGB(255, 0x34, 0xC7, 0x59);
      break;
    case CUPERTINO_COLOR_INDIGO:
      *out_color = UI_COLOR_ARGB(255, 0x58, 0x56, 0xD6);
      break;
    case CUPERTINO_COLOR_ORANGE:
      *out_color = UI_COLOR_ARGB(255, 0xFF, 0x95, 0x00);
      break;
    case CUPERTINO_COLOR_PINK:
      *out_color = UI_COLOR_ARGB(255, 0xFF, 0x2D, 0x55);
      break;
    case CUPERTINO_COLOR_PURPLE:
      *out_color = UI_COLOR_ARGB(255, 0xAF, 0x52, 0xDE);
      break;
    case CUPERTINO_COLOR_RED:
      *out_color = UI_COLOR_ARGB(255, 0xFF, 0x3B, 0x30);
      break;
    case CUPERTINO_COLOR_TEAL:
      *out_color = UI_COLOR_ARGB(255, 0x59, 0xAD, 0xC4);
      break;
    case CUPERTINO_COLOR_YELLOW:
      *out_color = UI_COLOR_ARGB(255, 0xFF, 0xCC, 0x00);
      break;
    case CUPERTINO_COLOR_MINT:
      *out_color = UI_COLOR_ARGB(255, 0x00, 0xC7, 0xBE);
      break;
    case CUPERTINO_COLOR_CYAN:
      *out_color = UI_COLOR_ARGB(255, 0x32, 0xAD, 0xE6);
      break;
    default:
      return UI_ERROR_INVALID_ARGUMENT;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Resolves a tiered neutral system gray.
 */
ui_error_t cupertino_get_system_gray(enum cupertino_gray_level gray_level,
                                     int is_dark, ui_color_t *out_color) {
  if (!out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_dark) {
    switch (gray_level) {
    case CUPERTINO_GRAY_1:
      *out_color = UI_COLOR_ARGB(255, 0x8E, 0x8E, 0x93);
      break;
    case CUPERTINO_GRAY_2:
      *out_color = UI_COLOR_ARGB(255, 0x63, 0x63, 0x66);
      break;
    case CUPERTINO_GRAY_3:
      *out_color = UI_COLOR_ARGB(255, 0x48, 0x48, 0x4A);
      break;
    case CUPERTINO_GRAY_4:
      *out_color = UI_COLOR_ARGB(255, 0x3A, 0x3A, 0x3C);
      break;
    case CUPERTINO_GRAY_5:
      *out_color = UI_COLOR_ARGB(255, 0x2C, 0x2C, 0x2E);
      break;
    case CUPERTINO_GRAY_6:
      *out_color = UI_COLOR_ARGB(255, 0x1C, 0x1C, 0x1E);
      break;
    default:
      return UI_ERROR_INVALID_ARGUMENT;
    }
  } else {
    switch (gray_level) {
    case CUPERTINO_GRAY_1:
      *out_color = UI_COLOR_ARGB(255, 0x8E, 0x8E, 0x93);
      break;
    case CUPERTINO_GRAY_2:
      *out_color = UI_COLOR_ARGB(255, 0xAE, 0xAE, 0xB2);
      break;
    case CUPERTINO_GRAY_3:
      *out_color = UI_COLOR_ARGB(255, 0xC7, 0xC7, 0xCC);
      break;
    case CUPERTINO_GRAY_4:
      *out_color = UI_COLOR_ARGB(255, 0xD1, 0xD1, 0xD6);
      break;
    case CUPERTINO_GRAY_5:
      *out_color = UI_COLOR_ARGB(255, 0xE5, 0xE5, 0xEA);
      break;
    case CUPERTINO_GRAY_6:
      *out_color = UI_COLOR_ARGB(255, 0xF2, 0xF2, 0xF7);
      break;
    default:
      return UI_ERROR_INVALID_ARGUMENT;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Resolves a semantic surface background color.
 */
ui_error_t cupertino_get_surface_color(enum cupertino_surface_level surface,
                                       int is_dark, ui_color_t *out_color) {
  if (!out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_dark) {
    switch (surface) {
    case CUPERTINO_SURFACE_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0x00, 0x00, 0x00);
      break;
    case CUPERTINO_SURFACE_SECONDARY_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0x1C, 0x1C, 0x1E);
      break;
    case CUPERTINO_SURFACE_TERTIARY_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0x2C, 0x2C, 0x2E);
      break;
    case CUPERTINO_SURFACE_GROUPED_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0x00, 0x00, 0x00);
      break;
    case CUPERTINO_SURFACE_SECONDARY_GROUPED_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0x1C, 0x1C, 0x1E);
      break;
    case CUPERTINO_SURFACE_TERTIARY_GROUPED_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0x2C, 0x2C, 0x2E);
      break;
    default:
      return UI_ERROR_INVALID_ARGUMENT;
    }
  } else {
    switch (surface) {
    case CUPERTINO_SURFACE_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0xFF, 0xFF, 0xFF);
      break;
    case CUPERTINO_SURFACE_SECONDARY_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0xF2, 0xF2, 0xF7);
      break;
    case CUPERTINO_SURFACE_TERTIARY_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0xFF, 0xFF, 0xFF);
      break;
    case CUPERTINO_SURFACE_GROUPED_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0xF2, 0xF2, 0xF7);
      break;
    case CUPERTINO_SURFACE_SECONDARY_GROUPED_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0xFF, 0xFF, 0xFF);
      break;
    case CUPERTINO_SURFACE_TERTIARY_GROUPED_BACKGROUND:
      *out_color = UI_COLOR_ARGB(255, 0xF2, 0xF2, 0xF7);
      break;
    default:
      return UI_ERROR_INVALID_ARGUMENT;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Resolves a semantic text label foreground color with opacity.
 */
ui_error_t cupertino_get_label_color(enum cupertino_label_level label,
                                     int is_dark, ui_color_t *out_color) {
  ui_uint8 r, g, b, a;

  if (!out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_dark) {
    r = 255;
    g = 255;
    b = 255;
  } else {
    r = 0;
    g = 0;
    b = 0;
  }

  switch (label) {
  case CUPERTINO_LABEL_PRIMARY:
    a = 255; /* 100% */
    break;
  case CUPERTINO_LABEL_SECONDARY:
    a = 153; /* 60% */
    break;
  case CUPERTINO_LABEL_TERTIARY:
    a = 77; /* 30% */
    break;
  case CUPERTINO_LABEL_QUATERNARY:
    a = 46; /* 18% */
    break;
  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_color = UI_COLOR_ARGB(a, r, g, b);
  return UI_ERROR_NONE;
}

/**
 * @brief Resolves separator, opaque separator, or link colors.
 */
ui_error_t cupertino_resolve_named_color(const char *token_name, int is_dark,
                                         ui_color_t *out_color) {
  if (!token_name || !out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(token_name, "--apple-separator") == 0) {
    if (is_dark) {
      *out_color = UI_COLOR_ARGB(153, 84, 84, 88);
    } else {
      *out_color = UI_COLOR_ARGB(74, 60, 60, 67);
    }
    return UI_ERROR_NONE;
  }

  if (strcmp(token_name, "--apple-opaque-separator") == 0) {
    if (is_dark) {
      *out_color = UI_COLOR_ARGB(255, 0x38, 0x38, 0x3A);
    } else {
      *out_color = UI_COLOR_ARGB(255, 0xC6, 0xC6, 0xC8);
    }
    return UI_ERROR_NONE;
  }

  if (strcmp(token_name, "--apple-link") == 0) {
    if (is_dark) {
      *out_color = UI_COLOR_ARGB(255, 0x09, 0x84, 0xFF);
    } else {
      *out_color = UI_COLOR_ARGB(255, 0x00, 0x7A, 0xFF);
    }
    return UI_ERROR_NONE;
  }

  return UI_ERROR_NOT_FOUND;
}
