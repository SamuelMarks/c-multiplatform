/**
 * @file cupertino_text_field.c
 * @brief Implementation of Cupertino Text Field & Text View components
 * conforming to Apple Human Interface Guidelines (HIG).
 */

/* clang-format off */
#include "cupertino/cupertino_text_field.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_text_field_mock_destroy_fail = 0;
int g_cupertino_text_field_mock_set_placeholder_fail = 0;
int g_cupertino_text_field_mock_get_text_fail = 0;
int g_cupertino_text_field_mock_get_text_null = 0;
int g_cupertino_text_field_mock_set_type_fail = 0;

static ui_error_t mock_input_base_destroy(struct ui_input_base *input) {
  if (g_cupertino_text_field_mock_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_input_base_destroy(input);
}
#undef ui_input_base_destroy
/** @cond */
#define ui_input_base_destroy mock_input_base_destroy
/** @endcond */

static ui_error_t mock_input_base_set_placeholder(struct ui_input_base *input,
                                                  const char *placeholder) {
  if (g_cupertino_text_field_mock_set_placeholder_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_input_base_set_placeholder(input, placeholder);
}
#undef ui_input_base_set_placeholder
/** @cond */
#define ui_input_base_set_placeholder mock_input_base_set_placeholder
/** @endcond */

static ui_error_t mock_input_base_get_text(const struct ui_input_base *input,
                                           const char **out_text) {
  if (g_cupertino_text_field_mock_get_text_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (g_cupertino_text_field_mock_get_text_null) {
    *out_text = NULL;
    return UI_ERROR_NONE;
  }
  return ui_input_base_get_text(input, out_text);
}
#undef ui_input_base_get_text
/** @cond */
#define ui_input_base_get_text mock_input_base_get_text
/** @endcond */

static ui_error_t mock_input_base_set_type(struct ui_input_base *input,
                                           const char *type) {
  if (g_cupertino_text_field_mock_set_type_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_input_base_set_type(input, type);
}
#undef ui_input_base_set_type
/** @cond */
#define ui_input_base_set_type mock_input_base_set_type
/** @endcond */
#endif

static void copy_string(char *dst, size_t sz, const char *src) {
#if defined(_MSC_VER)
  strcpy_s(dst, sz, src);
#else
  strncpy(dst, src, sz - 1);
  dst[sz - 1] = '\0';
#endif
}

/* --- CVA Helper Callbacks for Cupertino Text Field --- */

static ui_error_t
cupertino_text_field_cva_write_value(void *component,
                                     union ui_signal_payload value) {
  struct cupertino_text_field *field;
  const char *str;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field = (struct cupertino_text_field *)component;
  str = (const char *)value.ptr_val;
  return cupertino_text_field_set_text(field, str ? str : "");
}

static ui_error_t
cupertino_text_field_cva_set_disabled_state(void *component,
                                            ui_bool_t is_disabled) {
  struct cupertino_text_field *field;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field = (struct cupertino_text_field *)component;
  return cupertino_text_field_set_disabled(field, is_disabled ? 1 : 0);
}

/* --- Internal Construction Helper --- */

static ui_error_t
cupertino_text_field_init_internal(struct ui_engine *engine, int is_multiline,
                                   size_t min_lines, size_t max_lines,
                                   struct cupertino_text_field **out_field,
                                   struct ui_control_value_accessor **out_cva) {
  struct cupertino_text_field *field;
  ui_error_t rc;

  if (!engine || !out_field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field = (struct cupertino_text_field *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_text_field));
  if (!field) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(field, 0, sizeof(struct cupertino_text_field));
  field->is_multiline = is_multiline;
  field->min_lines = min_lines;
  field->max_lines = max_lines;
  field->clear_button_mode = CUPERTINO_OVERLAY_WHILE_EDITING;
  field->corner_radius = 8.0f;
  field->background_color =
      UI_COLOR_ARGB(255, 0xF2, 0xF2, 0xF7);                 /* SystemGray6 */
  field->text_color = UI_COLOR_ARGB(255, 0x00, 0x00, 0x00); /* Label */
  field->placeholder_color =
      UI_COLOR_ARGB(76, 0x3C, 0x3C, 0x43); /* TertiaryLabel */

  rc = ui_input_base_create(&field->input);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(field);
    return rc;
  }

  field->cva.write_value = cupertino_text_field_cva_write_value;
  field->cva.set_disabled_state = cupertino_text_field_cva_set_disabled_state;

  if (out_cva) {
    *out_cva = &field->cva;
  }

  *out_field = field;
  return UI_ERROR_NONE;
}

/* --- Public API --- */

ui_error_t
cupertino_text_field_create(struct ui_engine *engine,
                            struct cupertino_text_field **out_field,
                            struct ui_control_value_accessor **out_cva) {
  return cupertino_text_field_init_internal(engine, 0, 1, 1, out_field,
                                            out_cva);
}

ui_error_t
cupertino_text_view_create(struct ui_engine *engine, size_t min_lines,
                           size_t max_lines,
                           struct cupertino_text_field **out_view,
                           struct ui_control_value_accessor **out_cva) {
  return cupertino_text_field_init_internal(engine, 1, min_lines, max_lines,
                                            out_view, out_cva);
}

ui_error_t cupertino_text_field_destroy(struct cupertino_text_field *field) {
  ui_error_t rc;

  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (field->input) {
    rc = ui_input_base_destroy(field->input);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    field->input = NULL;
  }

  C_MULTIPLATFORM_FREE(field);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_field_set_text(struct cupertino_text_field *field,
                                         const char *text) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_input_base_set_text(field->input, text ? text : "");
}

ui_error_t
cupertino_text_field_get_text(const struct cupertino_text_field *field,
                              const char **out_text) {
  if (!field || !out_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_input_base_get_text(field->input, out_text);
}

ui_error_t
cupertino_text_field_set_placeholder(struct cupertino_text_field *field,
                                     const char *placeholder) {
  ui_error_t rc;

  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (placeholder) {
    copy_string(field->placeholder, sizeof(field->placeholder), placeholder);
  } else {
    field->placeholder[0] = '\0';
  }

  rc = ui_input_base_set_placeholder(field->input, field->placeholder);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_field_set_clear_button_mode(
    struct cupertino_text_field *field,
    enum cupertino_overlay_visibility_mode mode) {
  if (!field ||
      (unsigned)mode >= (unsigned)CUPERTINO_OVERLAY_VISIBILITY_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field->clear_button_mode = mode;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_field_is_clear_button_visible(
    const struct cupertino_text_field *field, int *out_visible) {
  const char *text = NULL;
  ui_error_t rc;

  if (!field || !out_visible) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_input_base_get_text(field->input, &text);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (!text || text[0] == '\0') {
    *out_visible = 0;
    return UI_ERROR_NONE;
  }

  switch (field->clear_button_mode) {
  case CUPERTINO_OVERLAY_NEVER:
    *out_visible = 0;
    break;
  case CUPERTINO_OVERLAY_WHILE_EDITING:
    *out_visible = field->is_editing ? 1 : 0;
    break;
  case CUPERTINO_OVERLAY_UNLESS_EDITING:
    *out_visible = field->is_editing ? 0 : 1;
    break;
  case CUPERTINO_OVERLAY_ALWAYS:
    *out_visible = 1;
    break;
  default:
    *out_visible = 0;
    break;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_text_field_trigger_clear(struct cupertino_text_field *field) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_input_base_set_text(field->input, "");
}

ui_error_t
cupertino_text_field_set_secure_text_entry(struct cupertino_text_field *field,
                                           int is_secure) {
  ui_error_t rc;

  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field->is_secure_text_entry = is_secure ? 1 : 0;
  rc = ui_input_base_set_type(field->input, is_secure ? "password" : "text");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_field_set_editing(struct cupertino_text_field *field,
                                            int is_editing) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field->is_editing = is_editing ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_field_set_prefix(struct cupertino_text_field *field,
                                           const char *prefix) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (prefix) {
    copy_string(field->prefix_text, sizeof(field->prefix_text), prefix);
  } else {
    field->prefix_text[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_field_set_suffix(struct cupertino_text_field *field,
                                           const char *suffix) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (suffix) {
    copy_string(field->suffix_text, sizeof(field->suffix_text), suffix);
  } else {
    field->suffix_text[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_field_set_disabled(struct cupertino_text_field *field,
                                             int disabled) {
  if (!field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_input_base_set_disabled(field->input, disabled ? 1 : 0);
}

ui_error_t cupertino_text_field_get_base(struct cupertino_text_field *field,
                                         struct ui_input_base **out_base) {
  if (!field || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = field->input;
  return UI_ERROR_NONE;
}
