/* clang-format off */
#include "cmp_css_fonts.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_prop_font_basic_group_init(cmp_prop_font_basic_group_t *group,
                                   const char **family, int weight,
                                   cmp_font_style_t style,
                                   cmp_font_stretch_t stretch,
                                   float stretch_percent, const char *size) {
  size_t count = 0;
  size_t i;
  if (!group) {
    return -1;
  }

  if (family) {
    while (family[count]) {
      count++;
    }
    group->font_family = (char **)malloc((count + 1) * sizeof(char *));
    if (!group->font_family) {
      return -1;
    }
    for (i = 0; i < count; i++) {
      size_t len = strlen(family[i]);
      group->font_family[i] = (char *)malloc(len + 1);
      if (!group->font_family[i]) {
        size_t j;
        for (j = 0; j < i; j++) {
          free(group->font_family[j]);
        }
        free(group->font_family);
        group->font_family = NULL;
        return -1;
      }
      memcpy(group->font_family[i], family[i], len + 1);
    }
    group->font_family[count] = NULL;
  } else {
    group->font_family = NULL;
  }

  group->font_weight = weight;
  group->font_style = style;
  group->font_stretch = stretch;
  group->font_stretch_percent = stretch_percent;

  if (size) {
    size_t len = strlen(size);
    group->font_size = (char *)malloc(len + 1);
    if (!group->font_size) {
      (void)cmp_prop_font_basic_group_free(group);
      return -1;
    }
    memcpy(group->font_size, size, len + 1);
  } else {
    group->font_size = NULL;
  }
  return 0;
}

int cmp_prop_font_basic_group_free(cmp_prop_font_basic_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->font_family) {
    size_t i = 0;
    while (group->font_family[i]) {
      free(group->font_family[i]);
      i++;
    }
    free(group->font_family);
    group->font_family = NULL;
  }
  if (group->font_size) {
    free(group->font_size);
    group->font_size = NULL;
  }
  return 0;
}

int cmp_prop_font_size_adjust_init(cmp_prop_font_size_adjust_t *prop,
                                   const char *size_adjust) {
  if (!prop) {
    return -1;
  }
  if (size_adjust) {
    size_t len = strlen(size_adjust);
    prop->size_adjust = (char *)malloc(len + 1);
    if (!prop->size_adjust) {
      return -1;
    }
    memcpy(prop->size_adjust, size_adjust, len + 1);
  } else {
    prop->size_adjust = NULL;
  }
  return 0;
}

int cmp_prop_font_size_adjust_free(cmp_prop_font_size_adjust_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->size_adjust) {
    free(prop->size_adjust);
    prop->size_adjust = NULL;
  }
  return 0;
}

int cmp_prop_font_variant_group_init(cmp_prop_font_variant_group_t *group,
                                     cmp_font_variant_caps_t caps, int numeric,
                                     int east_asian, int ligatures) {
  if (!group) {
    return -1;
  }
  group->caps = caps;
  group->numeric = numeric;
  group->east_asian = east_asian;
  group->ligatures = ligatures;
  return 0;
}

int cmp_prop_font_variant_advanced_init(cmp_prop_font_variant_advanced_t *adv,
                                        const char *alternates,
                                        cmp_font_variant_position_t position) {
  if (!adv) {
    return -1;
  }
  if (alternates) {
    size_t len = strlen(alternates);
    adv->alternates = (char *)malloc(len + 1);
    if (!adv->alternates) {
      return -1;
    }
    memcpy(adv->alternates, alternates, len + 1);
  } else {
    adv->alternates = NULL;
  }
  adv->position = position;
  return 0;
}

int cmp_prop_font_variant_advanced_free(cmp_prop_font_variant_advanced_t *adv) {
  if (!adv) {
    return -1;
  }
  if (adv->alternates) {
    free(adv->alternates);
    adv->alternates = NULL;
  }
  return 0;
}

int cmp_prop_font_settings_group_init(cmp_prop_font_settings_group_t *group,
                                      const char *feature,
                                      const char *variation) {
  if (!group) {
    return -1;
  }
  if (feature) {
    size_t len = strlen(feature);
    group->feature_settings = (char *)malloc(len + 1);
    if (!group->feature_settings) {
      return -1;
    }
    memcpy(group->feature_settings, feature, len + 1);
  } else {
    group->feature_settings = NULL;
  }

  if (variation) {
    size_t len = strlen(variation);
    group->variation_settings = (char *)malloc(len + 1);
    if (!group->variation_settings) {
      if (group->feature_settings) {
        free(group->feature_settings);
        group->feature_settings = NULL;
      }
      return -1;
    }
    memcpy(group->variation_settings, variation, len + 1);
  } else {
    group->variation_settings = NULL;
  }
  return 0;
}

int cmp_prop_font_settings_group_free(cmp_prop_font_settings_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->feature_settings) {
    free(group->feature_settings);
    group->feature_settings = NULL;
  }
  if (group->variation_settings) {
    free(group->variation_settings);
    group->variation_settings = NULL;
  }
  return 0;
}

int cmp_prop_font_synthesis_group_init(cmp_prop_font_synthesis_group_t *group,
                                       int synthesis) {
  if (!group) {
    return -1;
  }
  group->synthesis = synthesis;
  return 0;
}

int cmp_prop_font_kerning_init(cmp_prop_font_kerning_t *prop,
                               cmp_font_kerning_t kerning) {
  if (!prop) {
    return -1;
  }
  prop->kerning = kerning;
  return 0;
}

int cmp_prop_font_language_override_init(
    cmp_prop_font_language_override_t *prop, const char *override) {
  if (!prop) {
    return -1;
  }
  if (override) {
    size_t len = strlen(override);
    prop->language_override = (char *)malloc(len + 1);
    if (!prop->language_override) {
      return -1;
    }
    memcpy(prop->language_override, override, len + 1);
  } else {
    prop->language_override = NULL;
  }
  return 0;
}

int cmp_prop_font_language_override_free(
    cmp_prop_font_language_override_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->language_override) {
    free(prop->language_override);
    prop->language_override = NULL;
  }
  return 0;
}

int cmp_prop_font_palette_init(cmp_prop_font_palette_t *prop,
                               const char *palette) {
  if (!prop) {
    return -1;
  }
  if (palette) {
    size_t len = strlen(palette);
    prop->palette = (char *)malloc(len + 1);
    if (!prop->palette) {
      return -1;
    }
    memcpy(prop->palette, palette, len + 1);
  } else {
    prop->palette = NULL;
  }
  return 0;
}

int cmp_prop_font_palette_free(cmp_prop_font_palette_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->palette) {
    free(prop->palette);
    prop->palette = NULL;
  }
  return 0;
}

int cmp_at_rule_font_face_init(cmp_at_rule_font_face_t *rule,
                               const char *family, const char **src,
                               const char *weight, const char *style,
                               const char *display, const char *range) {
  size_t count = 0;
  size_t i;
  if (!rule) {
    return -1;
  }

  memset(rule, 0, sizeof(*rule));

  if (family) {
    size_t len = strlen(family);
    rule->font_family = (char *)malloc(len + 1);
    if (!rule->font_family)
      goto error;
    memcpy(rule->font_family, family, len + 1);
  }

  if (src) {
    while (src[count]) {
      count++;
    }
    rule->src = (char **)malloc((count + 1) * sizeof(char *));
    if (!rule->src)
      goto error;
    for (i = 0; i < count; i++) {
      size_t len = strlen(src[i]);
      rule->src[i] = (char *)malloc(len + 1);
      if (!rule->src[i])
        goto error;
      memcpy(rule->src[i], src[i], len + 1);
    }
    rule->src[count] = NULL;
  }

  if (weight) {
    size_t len = strlen(weight);
    rule->font_weight = (char *)malloc(len + 1);
    if (!rule->font_weight)
      goto error;
    memcpy(rule->font_weight, weight, len + 1);
  }

  if (style) {
    size_t len = strlen(style);
    rule->font_style = (char *)malloc(len + 1);
    if (!rule->font_style)
      goto error;
    memcpy(rule->font_style, style, len + 1);
  }

  if (display) {
    size_t len = strlen(display);
    rule->font_display = (char *)malloc(len + 1);
    if (!rule->font_display)
      goto error;
    memcpy(rule->font_display, display, len + 1);
  }

  if (range) {
    size_t len = strlen(range);
    rule->unicode_range = (char *)malloc(len + 1);
    if (!rule->unicode_range)
      goto error;
    memcpy(rule->unicode_range, range, len + 1);
  }

  return 0;

error:
  cmp_at_rule_font_face_free(rule);
  return -1;
}

int cmp_at_rule_font_face_free(cmp_at_rule_font_face_t *rule) {
  if (!rule) {
    return -1;
  }
  if (rule->font_family) {
    free(rule->font_family);
    rule->font_family = NULL;
  }
  if (rule->src) {
    size_t i = 0;
    while (rule->src[i]) {
      free(rule->src[i]);
      i++;
    }
    free(rule->src);
    rule->src = NULL;
  }
  if (rule->font_weight) {
    free(rule->font_weight);
    rule->font_weight = NULL;
  }
  if (rule->font_style) {
    free(rule->font_style);
    rule->font_style = NULL;
  }
  if (rule->font_display) {
    free(rule->font_display);
    rule->font_display = NULL;
  }
  if (rule->unicode_range) {
    free(rule->unicode_range);
    rule->unicode_range = NULL;
  }
  return 0;
}

int cmp_at_rule_font_advanced_init(cmp_at_rule_font_advanced_t *rule,
                                   const char *family, const char *blob) {
  if (!rule) {
    return -1;
  }
  memset(rule, 0, sizeof(*rule));

  if (family) {
    size_t len = strlen(family);
    rule->font_family = (char *)malloc(len + 1);
    if (!rule->font_family) {
      return -1;
    }
    memcpy(rule->font_family, family, len + 1);
  }

  if (blob) {
    size_t len = strlen(blob);
    rule->values_blob = (char *)malloc(len + 1);
    if (!rule->values_blob) {
      cmp_at_rule_font_advanced_free(rule);
      return -1;
    }
    memcpy(rule->values_blob, blob, len + 1);
  }
  return 0;
}

int cmp_at_rule_font_advanced_free(cmp_at_rule_font_advanced_t *rule) {
  if (!rule) {
    return -1;
  }
  if (rule->font_family) {
    free(rule->font_family);
    rule->font_family = NULL;
  }
  if (rule->values_blob) {
    free(rule->values_blob);
    rule->values_blob = NULL;
  }
  return 0;
}

int cmp_prop_initial_letter_init(cmp_prop_initial_letter_t *prop,
                                 const char *letter) {
  if (!prop) {
    return -1;
  }
  if (letter) {
    size_t len = strlen(letter);
    prop->initial_letter = (char *)malloc(len + 1);
    if (!prop->initial_letter) {
      return -1;
    }
    memcpy(prop->initial_letter, letter, len + 1);
  } else {
    prop->initial_letter = NULL;
  }
  return 0;
}

int cmp_prop_initial_letter_free(cmp_prop_initial_letter_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->initial_letter) {
    free(prop->initial_letter);
    prop->initial_letter = NULL;
  }
  return 0;
}

int cmp_prop_initial_letter_align_init(cmp_prop_initial_letter_align_t *prop,
                                       cmp_initial_letter_align_t align) {
  if (!prop) {
    return -1;
  }
  prop->align = align;
  return 0;
}

int cmp_prop_font_smoothing_init(cmp_prop_font_smoothing_t *prop,
                                 cmp_font_smoothing_t smoothing) {
  if (!prop) {
    return -1;
  }
  prop->smoothing = smoothing;
  return 0;
}
