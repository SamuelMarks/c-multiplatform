/* clang-format off */
#include "cmp_css_typography.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_prop_text_align_group_init(cmp_prop_text_align_group_t *group,
                                   cmp_text_align_t align,
                                   cmp_text_align_t align_last) {
  if (!group) {
    return -1;
  }
  group->text_align = align;
  group->text_align_last = align_last;
  return 0;
}

int cmp_prop_text_justify_init(cmp_prop_text_justify_t *prop,
                               cmp_text_justify_t justify) {
  if (!prop) {
    return -1;
  }
  prop->justify = justify;
  return 0;
}

int cmp_prop_text_indent_init(cmp_prop_text_indent_t *prop,
                              const char *indent) {
  if (!prop) {
    return -1;
  }
  if (indent) {
    size_t len = strlen(indent);
    prop->indent = (char *)malloc(len + 1);
    if (!prop->indent) {
      return -1;
    }
    memcpy(prop->indent, indent, len + 1);
  } else {
    prop->indent = NULL;
  }
  return 0;
}

int cmp_prop_text_indent_free(cmp_prop_text_indent_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->indent) {
    free(prop->indent);
    prop->indent = NULL;
  }
  return 0;
}

int cmp_prop_spacing_init(cmp_prop_spacing_t *prop, const char *letter_spacing,
                          const char *word_spacing) {
  if (!prop) {
    return -1;
  }
  if (letter_spacing) {
    size_t len = strlen(letter_spacing);
    prop->letter_spacing = (char *)malloc(len + 1);
    if (!prop->letter_spacing) {
      return -1;
    }
    memcpy(prop->letter_spacing, letter_spacing, len + 1);
  } else {
    prop->letter_spacing = NULL;
  }

  if (word_spacing) {
    size_t len = strlen(word_spacing);
    prop->word_spacing = (char *)malloc(len + 1);
    if (!prop->word_spacing) {
      if (prop->letter_spacing) {
        free(prop->letter_spacing);
        prop->letter_spacing = NULL;
      }
      return -1;
    }
    memcpy(prop->word_spacing, word_spacing, len + 1);
  } else {
    prop->word_spacing = NULL;
  }
  return 0;
}

int cmp_prop_spacing_free(cmp_prop_spacing_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->letter_spacing) {
    free(prop->letter_spacing);
    prop->letter_spacing = NULL;
  }
  if (prop->word_spacing) {
    free(prop->word_spacing);
    prop->word_spacing = NULL;
  }
  return 0;
}

int cmp_prop_line_height_init(cmp_prop_line_height_t *prop,
                              const char *line_height) {
  if (!prop) {
    return -1;
  }
  if (line_height) {
    size_t len = strlen(line_height);
    prop->line_height = (char *)malloc(len + 1);
    if (!prop->line_height) {
      return -1;
    }
    memcpy(prop->line_height, line_height, len + 1);
  } else {
    prop->line_height = NULL;
  }
  return 0;
}

int cmp_prop_line_height_free(cmp_prop_line_height_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->line_height) {
    free(prop->line_height);
    prop->line_height = NULL;
  }
  return 0;
}

int cmp_prop_tab_size_init(cmp_prop_tab_size_t *prop, const char *tab_size) {
  if (!prop) {
    return -1;
  }
  if (tab_size) {
    size_t len = strlen(tab_size);
    prop->tab_size = (char *)malloc(len + 1);
    if (!prop->tab_size) {
      return -1;
    }
    memcpy(prop->tab_size, tab_size, len + 1);
  } else {
    prop->tab_size = NULL;
  }
  return 0;
}

int cmp_prop_tab_size_free(cmp_prop_tab_size_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->tab_size) {
    free(prop->tab_size);
    prop->tab_size = NULL;
  }
  return 0;
}

int cmp_prop_white_space_init(cmp_prop_white_space_t *prop,
                              cmp_white_space_t white_space) {
  if (!prop) {
    return -1;
  }
  prop->white_space = white_space;
  return 0;
}

int cmp_prop_breaking_group_init(cmp_prop_breaking_group_t *group,
                                 cmp_word_break_t word_break,
                                 cmp_line_break_t line_break,
                                 cmp_overflow_wrap_t overflow_wrap,
                                 cmp_overflow_wrap_t word_wrap) {
  if (!group) {
    return -1;
  }
  group->word_break = word_break;
  group->line_break = line_break;
  group->overflow_wrap = overflow_wrap;
  group->word_wrap = word_wrap;
  return 0;
}

int cmp_prop_hyphens_init(cmp_prop_hyphens_t *prop, cmp_hyphens_t hyphens) {
  if (!prop) {
    return -1;
  }
  prop->hyphens = hyphens;
  return 0;
}

int cmp_prop_line_clamp_init(cmp_prop_line_clamp_t *prop,
                             const char *line_clamp) {
  if (!prop) {
    return -1;
  }
  if (line_clamp) {
    size_t len = strlen(line_clamp);
    prop->line_clamp = (char *)malloc(len + 1);
    if (!prop->line_clamp) {
      return -1;
    }
    memcpy(prop->line_clamp, line_clamp, len + 1);
  } else {
    prop->line_clamp = NULL;
  }
  return 0;
}

int cmp_prop_line_clamp_free(cmp_prop_line_clamp_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->line_clamp) {
    free(prop->line_clamp);
    prop->line_clamp = NULL;
  }
  return 0;
}

int cmp_prop_hanging_punctuation_init(
    cmp_prop_hanging_punctuation_t *prop,
    cmp_hanging_punctuation_t hanging_punctuation) {
  if (!prop) {
    return -1;
  }
  prop->hanging_punctuation = hanging_punctuation;
  return 0;
}

int cmp_prop_text_decoration_group_init(cmp_prop_text_decoration_group_t *group,
                                        int line, const cmp_prop_color_t *color,
                                        cmp_text_decoration_style_t style,
                                        const char *thickness) {
  if (!group) {
    return -1;
  }
  group->text_decoration_line = line;
  if (color) {
    group->text_decoration_color = *color;
  } else {
    group->text_decoration_color.type = CMP_PROP_COLOR_CURRENTCOLOR;
  }
  group->text_decoration_style = style;
  if (thickness) {
    size_t len = strlen(thickness);
    group->text_decoration_thickness = (char *)malloc(len + 1);
    if (!group->text_decoration_thickness) {
      return -1;
    }
    memcpy(group->text_decoration_thickness, thickness, len + 1);
  } else {
    group->text_decoration_thickness = NULL;
  }
  return 0;
}

int cmp_prop_text_decoration_group_free(
    cmp_prop_text_decoration_group_t *group) {
  if (!group) {
    return -1;
  }
  (void)cmp_prop_color_free(&group->text_decoration_color);
  if (group->text_decoration_thickness) {
    free(group->text_decoration_thickness);
    group->text_decoration_thickness = NULL;
  }
  return 0;
}

int cmp_prop_text_underline_group_init(
    cmp_prop_text_underline_group_t *group,
    cmp_text_underline_position_t position, const char *offset,
    cmp_text_decoration_skip_ink_t skip_ink) {
  if (!group) {
    return -1;
  }
  group->text_underline_position = position;
  if (offset) {
    size_t len = strlen(offset);
    group->text_underline_offset = (char *)malloc(len + 1);
    if (!group->text_underline_offset) {
      return -1;
    }
    memcpy(group->text_underline_offset, offset, len + 1);
  } else {
    group->text_underline_offset = NULL;
  }
  group->text_decoration_skip_ink = skip_ink;
  return 0;
}

int cmp_prop_text_underline_group_free(cmp_prop_text_underline_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->text_underline_offset) {
    free(group->text_underline_offset);
    group->text_underline_offset = NULL;
  }
  return 0;
}

int cmp_prop_text_shadow_init(cmp_prop_text_shadow_t *prop,
                              const char *text_shadow) {
  if (!prop) {
    return -1;
  }
  if (text_shadow) {
    size_t len = strlen(text_shadow);
    prop->text_shadow = (char *)malloc(len + 1);
    if (!prop->text_shadow) {
      return -1;
    }
    memcpy(prop->text_shadow, text_shadow, len + 1);
  } else {
    prop->text_shadow = NULL;
  }
  return 0;
}

int cmp_prop_text_shadow_free(cmp_prop_text_shadow_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->text_shadow) {
    free(prop->text_shadow);
    prop->text_shadow = NULL;
  }
  return 0;
}

int cmp_prop_text_transform_init(cmp_prop_text_transform_t *prop,
                                 cmp_text_transform_t transform) {
  if (!prop) {
    return -1;
  }
  prop->transform = transform;
  return 0;
}

int cmp_prop_text_emphasis_group_init(cmp_prop_text_emphasis_group_t *group,
                                      const cmp_prop_color_t *color,
                                      cmp_text_emphasis_style_t style,
                                      const char *string_val,
                                      cmp_text_emphasis_position_t position) {
  if (!group) {
    return -1;
  }
  if (color) {
    group->text_emphasis_color = *color;
  } else {
    group->text_emphasis_color.type = CMP_PROP_COLOR_CURRENTCOLOR;
  }
  group->text_emphasis_style = style;
  if (string_val) {
    size_t len = strlen(string_val);
    group->text_emphasis_string = (char *)malloc(len + 1);
    if (!group->text_emphasis_string) {
      return -1;
    }
    memcpy(group->text_emphasis_string, string_val, len + 1);
  } else {
    group->text_emphasis_string = NULL;
  }
  group->text_emphasis_position = position;
  return 0;
}

int cmp_prop_text_emphasis_group_free(cmp_prop_text_emphasis_group_t *group) {
  if (!group) {
    return -1;
  }
  (void)cmp_prop_color_free(&group->text_emphasis_color);
  if (group->text_emphasis_string) {
    free(group->text_emphasis_string);
    group->text_emphasis_string = NULL;
  }
  return 0;
}
