#ifdef _MSC_VER
#pragma warning(disable : 4702)
#endif

/* clang-format off */
#include "ui_input_mask.h"

#include <stdlib.h>
#include "ui_internal_mem.h"
#include <string.h>
#include <ctype.h>
/* clang-format on */

#define MAX_MASK_LEN 256

struct ui_input_mask {
  struct ui_input_base *input;
  char pattern[MAX_MASK_LEN];
  char raw_value[MAX_MASK_LEN];
  char formatted_value[MAX_MASK_LEN];
  int is_processing; /* guard against recursive updates */
};

static ui_error_t safe_strcpy(char *dst, size_t sz, const char *src) {
#if defined(_MSC_VER)
  strcpy_s(dst, sz, src);
#else
  strncpy(dst, src, sz - 1);
  dst[sz - 1] = '\0';
#endif
  return UI_ERROR_NONE;
}

static ui_error_t on_input_change(struct ui_input_base *input, const char *text,
                                  void *user_data) {
  struct ui_input_mask *mask = (struct ui_input_mask *)user_data;
  (void)input;
  if (mask->is_processing) {
    return UI_ERROR_NONE;
  }
  return ui_input_mask_process_text(mask, text);
}

ui_error_t ui_input_mask_create(struct ui_input_mask **out_mask) {
  struct ui_input_mask *mask;

  if (!out_mask) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  mask = (struct ui_input_mask *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_input_mask));
  if (!mask) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  mask->input = NULL;
  mask->pattern[0] = '\0';
  mask->raw_value[0] = '\0';
  mask->formatted_value[0] = '\0';
  mask->is_processing = 0;

  *out_mask = mask;
  return UI_ERROR_NONE;
}

ui_error_t ui_input_mask_destroy(struct ui_input_mask *mask) {
  if (!mask) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (mask->input) {
    {
      ui_error_t _ign_rc = ui_input_base_set_on_change(mask->input, NULL, NULL);
      (void)_ign_rc;
    }
  }
  C_MULTIPLATFORM_FREE(mask);
  return UI_ERROR_NONE;
}

ui_error_t ui_input_mask_bind(struct ui_input_mask *mask,
                              struct ui_input_base *input) {

  if (!mask || !input) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  mask->input = input;
  {
    ui_error_t set_rc =
        ui_input_base_set_on_change(input, on_input_change, mask);
    (void)set_rc;
  }

  /* initial formatting */
  {
    const char *tmp_text;
    ui_error_t get_rc = ui_input_base_get_text(input, &tmp_text);
    (void)get_rc;
    return ui_input_mask_process_text(mask, tmp_text);
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_input_mask_set_pattern(struct ui_input_mask *mask,
                                     const char *pattern) {
  if (!mask || !pattern) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  {
    ui_error_t cp_rc = safe_strcpy(mask->pattern, MAX_MASK_LEN, pattern);
    (void)cp_rc;
  }

  if (mask->input) {
    ui_error_t proc_rc = ui_input_mask_process_text(mask, mask->raw_value);
    (void)proc_rc;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_input_mask_get_raw_value(struct ui_input_mask *mask,
                                       const char **out_raw) {
  if (!mask || !out_raw) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_raw = mask->raw_value;
  return UI_ERROR_NONE;
}

ui_error_t ui_input_mask_process_text(struct ui_input_mask *mask,
                                      const char *text) {
  size_t p_idx = 0;
  size_t t_idx = 0;
  size_t r_idx = 0;
  size_t f_idx = 0;

  if (!mask || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  mask->is_processing = 1;

  while (mask->pattern[p_idx] != '\0' && text[t_idx] != '\0' &&
         f_idx < MAX_MASK_LEN - 1) {

    char p_char = mask->pattern[p_idx];
    char t_char = text[t_idx];

    if (p_char == '9') {
      if (isdigit(t_char)) {
        mask->raw_value[r_idx++] = t_char;
        mask->formatted_value[f_idx++] = t_char;
        p_idx++;
        t_idx++;
      } else {
        t_idx++; /* skip invalid input char */
      }
    } else if (p_char == 'a' || p_char == 'A') {
      if (isalpha(t_char)) {
        mask->raw_value[r_idx++] = t_char;
        mask->formatted_value[f_idx++] = t_char;
        p_idx++;
        t_idx++;
      } else {
        t_idx++;
      }
    } else if (p_char == '*') {
      if (isalnum(t_char)) {
        mask->raw_value[r_idx++] = t_char;
        mask->formatted_value[f_idx++] = t_char;
        p_idx++;
        t_idx++;
      } else {
        t_idx++;
      }
    } else {
      /* literal match */
      mask->formatted_value[f_idx++] = p_char;
      if (t_char == p_char) {
        t_idx++; /* consume matching literal from input */
      }
      p_idx++;
    }
  }

  mask->raw_value[r_idx] = '\0';
  mask->formatted_value[f_idx] = '\0';

  if (mask->input) {
    ui_error_t rc;
    rc = ui_input_base_set_text(mask->input, mask->formatted_value);
    (void)rc;
  }

  mask->is_processing = 0;
  return UI_ERROR_NONE;
}
