/**
 * @file cupertino_form.c
 * @brief Implementation of Cupertino Form Section, Form Row, and Text Form
 * Field Row conforming to Apple Human Interface Guidelines (HIG).
 */

/* clang-format off */
#include "cupertino/cupertino_form.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_form_mock_card_destroy_fail = 0;
int g_cupertino_form_mock_card_set_title_fail = 0;
int g_cupertino_form_mock_card_set_subtitle_fail = 0;
int g_cupertino_form_mock_field_destroy_fail = 0;
int g_cupertino_form_mock_field_set_label_fail = 0;
int g_cupertino_form_mock_field_set_hint_fail = 0;
int g_cupertino_form_mock_field_set_error_fail = 0;
int g_cupertino_form_mock_field_bind_control_fail = 0;
int g_cupertino_form_mock_field_set_has_value_fail = 0;
int g_cupertino_form_mock_input_get_comp_fail = 0;
int g_cupertino_form_mock_row_set_child_fail = 0;
int g_cupertino_form_mock_input_set_text_fail = 0;
int g_cupertino_form_mock_input_set_placeholder_fail = 0;
int g_cupertino_form_mock_input_destroy_fail = 0;
int g_cupertino_form_mock_row_destroy_fail = 0;

static ui_error_t mock_card_base_destroy(struct ui_card_base *card) {
  if (g_cupertino_form_mock_card_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_card_base_destroy)(card);
}
#undef ui_card_base_destroy
/** @cond */
#define ui_card_base_destroy mock_card_base_destroy
/** @endcond */

static ui_error_t mock_card_base_set_title(struct ui_card_base *card,
                                           const char *title) {
  if (g_cupertino_form_mock_card_set_title_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_card_base_set_title(card, title);
}
#undef ui_card_base_set_title
/** @cond */
#define ui_card_base_set_title mock_card_base_set_title
/** @endcond */

static ui_error_t mock_card_base_set_subtitle(struct ui_card_base *card,
                                              const char *subtitle) {
  if (g_cupertino_form_mock_card_set_subtitle_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_card_base_set_subtitle(card, subtitle);
}
#undef ui_card_base_set_subtitle
/** @cond */
#define ui_card_base_set_subtitle mock_card_base_set_subtitle
/** @endcond */

static ui_error_t mock_form_field_base_destroy(struct ui_form_field_base *f) {
  if (g_cupertino_form_mock_field_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_form_field_base_destroy)(f);
}
#undef ui_form_field_base_destroy
/** @cond */
#define ui_form_field_base_destroy mock_form_field_base_destroy
/** @endcond */

static ui_error_t mock_form_field_base_set_label(struct ui_form_field_base *f,
                                                 const char *label) {
  if (g_cupertino_form_mock_field_set_label_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_form_field_base_set_label(f, label);
}
#undef ui_form_field_base_set_label
/** @cond */
#define ui_form_field_base_set_label mock_form_field_base_set_label
/** @endcond */

static ui_error_t mock_form_field_base_set_hint(struct ui_form_field_base *f,
                                                const char *hint) {
  if (g_cupertino_form_mock_field_set_hint_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_form_field_base_set_hint(f, hint);
}
#undef ui_form_field_base_set_hint
/** @cond */
#define ui_form_field_base_set_hint mock_form_field_base_set_hint
/** @endcond */

static ui_error_t mock_form_field_base_set_error(struct ui_form_field_base *f,
                                                 const char *error) {
  if (g_cupertino_form_mock_field_set_error_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_form_field_base_set_error(f, error);
}
#undef ui_form_field_base_set_error
/** @cond */
#define ui_form_field_base_set_error mock_form_field_base_set_error
/** @endcond */

static ui_error_t
mock_form_field_base_bind_form_control(struct ui_form_field_base *f,
                                       struct ui_form_control *fc,
                                       struct ui_reactor *r) {
  if (g_cupertino_form_mock_field_bind_control_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_form_field_base_bind_form_control(f, fc, r);
}
#undef ui_form_field_base_bind_form_control
/** @cond */
#define ui_form_field_base_bind_form_control                                   \
  mock_form_field_base_bind_form_control
/** @endcond */

static ui_error_t
mock_form_field_base_set_has_value(struct ui_form_field_base *f,
                                   int has_value) {
  if (g_cupertino_form_mock_field_set_has_value_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_form_field_base_set_has_value(f, has_value);
}
#undef ui_form_field_base_set_has_value
/** @cond */
#define ui_form_field_base_set_has_value mock_form_field_base_set_has_value
/** @endcond */

static ui_error_t
mock_input_base_get_component(struct ui_input_base *i,
                              struct ui_component **out_comp) {
  if (g_cupertino_form_mock_input_get_comp_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_input_base_get_component(i, out_comp);
}
#undef ui_input_base_get_component
/** @cond */
#define ui_input_base_get_component mock_input_base_get_component
/** @endcond */

static ui_error_t mock_input_base_set_text(struct ui_input_base *i,
                                           const char *text) {
  if (g_cupertino_form_mock_input_set_text_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_input_base_set_text(i, text);
}
#undef ui_input_base_set_text
/** @cond */
#define ui_input_base_set_text mock_input_base_set_text
/** @endcond */

static ui_error_t mock_input_base_set_placeholder(struct ui_input_base *i,
                                                  const char *placeholder) {
  if (g_cupertino_form_mock_input_set_placeholder_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_input_base_set_placeholder(i, placeholder);
}
#undef ui_input_base_set_placeholder
/** @cond */
#define ui_input_base_set_placeholder mock_input_base_set_placeholder
/** @endcond */

static ui_error_t mock_input_base_destroy(struct ui_input_base *i) {
  if (g_cupertino_form_mock_input_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_input_base_destroy)(i);
}
#undef ui_input_base_destroy
/** @cond */
#define ui_input_base_destroy mock_input_base_destroy
/** @endcond */

int g_cupertino_form_mock_input_create_fail = 0;
static ui_error_t mock_input_base_create(struct ui_input_base **out_input) {
  if (g_cupertino_form_mock_input_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return (ui_input_base_create)(out_input);
}
#undef ui_input_base_create
/** @cond */
#define ui_input_base_create mock_input_base_create
/** @endcond */

int g_cupertino_form_mock_field_set_control_fail = 0;
static ui_error_t
mock_form_field_base_set_control(struct ui_form_field_base *f,
                                 struct ui_component *control) {
  if (g_cupertino_form_mock_field_set_control_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return (ui_form_field_base_set_control)(f, control);
}
#undef ui_form_field_base_set_control
/** @cond */
#define ui_form_field_base_set_control mock_form_field_base_set_control
/** @endcond */

#endif /* UI_TEST_MOCK_ALLOC */

static void copy_string(char *dst, size_t sz, const char *src) {
#if defined(_MSC_VER)
  strcpy_s(dst, sz, src);
#else
  strncpy(dst, src, sz - 1);
  dst[sz - 1] = '\0';
#endif
}

/* --- CVA Helper Callbacks for Cupertino Text Form Field Row --- */

static ui_error_t
cupertino_text_form_field_cva_write_value(void *component,
                                          union ui_signal_payload value) {
  struct cupertino_text_form_field_row *field_row;
  const char *str;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field_row = (struct cupertino_text_form_field_row *)component;
  str = (const char *)value.ptr_val;
  return cupertino_text_form_field_row_set_text(field_row, str ? str : "");
}

static ui_error_t
cupertino_text_form_field_cva_set_disabled_state(void *component,
                                                 ui_bool_t is_disabled) {
  struct cupertino_text_form_field_row *field_row;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field_row = (struct cupertino_text_form_field_row *)component;
  if (!field_row->row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return cupertino_form_row_set_disabled(field_row->row, is_disabled ? 1 : 0);
}

/* --- Cupertino Form Section Implementation --- */

ui_error_t
cupertino_form_section_create(struct ui_engine *engine,
                              enum cupertino_form_section_style style,
                              struct cupertino_form_section **out_section) {
  struct cupertino_form_section *section;
  ui_error_t rc;

  if (!engine || !out_section ||
      (unsigned)style >= (unsigned)CUPERTINO_FORM_SECTION_STYLE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  section = (struct cupertino_form_section *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_form_section));
  if (!section) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(section, 0, sizeof(struct cupertino_form_section));
  section->style = style;
  section->corner_radius =
      (style == CUPERTINO_FORM_SECTION_INSET_GROUPED) ? 10.0f : 0.0f;
  section->margin =
      (style == CUPERTINO_FORM_SECTION_INSET_GROUPED) ? 16.0f : 0.0f;
  section->background_color =
      UI_COLOR_ARGB(255, 0xFF, 0xFF, 0xFF); /* SystemBackground */
  section->header_color =
      UI_COLOR_ARGB(153, 0x3C, 0x3C, 0x43); /* SecondaryLabel */
  section->footer_color =
      UI_COLOR_ARGB(153, 0x3C, 0x3C, 0x43); /* SecondaryLabel */

  rc = ui_card_base_create(&section->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(section);
    return rc;
  }

  *out_section = section;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_form_section_destroy(struct cupertino_form_section *section) {
  ui_error_t rc;

  if (!section) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (section->base) {
    rc = ui_card_base_destroy(section->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    section->base = NULL;
  }

  C_MULTIPLATFORM_FREE(section);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_form_section_set_header(struct cupertino_form_section *section,
                                  const char *header) {
  ui_error_t rc;

  if (!section) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (header) {
    copy_string(section->header_text, sizeof(section->header_text), header);
  } else {
    section->header_text[0] = '\0';
  }

  if (section->base) {
    rc = ui_card_base_set_title(section->base, section->header_text);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_form_section_set_footer(struct cupertino_form_section *section,
                                  const char *footer) {
  ui_error_t rc;

  if (!section) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (footer) {
    copy_string(section->footer_text, sizeof(section->footer_text), footer);
  } else {
    section->footer_text[0] = '\0';
  }

  if (section->base) {
    rc = ui_card_base_set_subtitle(section->base, section->footer_text);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_form_section_set_content(struct cupertino_form_section *section,
                                   struct ui_component *content) {
  if (!section) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_card_base_set_content(section->base, content);
}

ui_error_t
cupertino_form_section_get_base(struct cupertino_form_section *section,
                                struct ui_card_base **out_base) {
  if (!section || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = section->base;
  return UI_ERROR_NONE;
}

/* --- Cupertino Form Row Implementation --- */

ui_error_t cupertino_form_row_create(struct ui_engine *engine,
                                     struct cupertino_form_row **out_row) {
  struct cupertino_form_row *row;
  ui_error_t rc;

  if (!engine || !out_row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  row = (struct cupertino_form_row *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_form_row));
  if (!row) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(row, 0, sizeof(struct cupertino_form_row));
  row->prefix_color = UI_COLOR_ARGB(255, 0x00, 0x00, 0x00); /* Label */
  row->helper_color = UI_COLOR_ARGB(153, 0x3C, 0x3C, 0x43); /* SecondaryLabel */
  row->error_color = UI_COLOR_ARGB(255, 0xFF, 0x3B, 0x30);  /* SystemRed */
  row->is_disabled = 0;

  rc = ui_form_field_base_create(&row->field);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(row);
    return rc;
  }

  *out_row = row;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_form_row_destroy(struct cupertino_form_row *row) {
  ui_error_t rc;

  if (!row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (row->field) {
    rc = ui_form_field_base_destroy(row->field);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    row->field = NULL;
  }

  C_MULTIPLATFORM_FREE(row);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_form_row_set_prefix_label(struct cupertino_form_row *row,
                                               const char *label) {
  ui_error_t rc;

  if (!row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (label) {
    copy_string(row->prefix_label, sizeof(row->prefix_label), label);
  } else {
    row->prefix_label[0] = '\0';
  }

  if (row->field) {
    rc = ui_form_field_base_set_label(row->field, row->prefix_label);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_form_row_set_helper_text(struct cupertino_form_row *row,
                                              const char *helper) {
  ui_error_t rc;

  if (!row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (helper) {
    copy_string(row->helper_text, sizeof(row->helper_text), helper);
  } else {
    row->helper_text[0] = '\0';
  }

  if (row->field) {
    rc = ui_form_field_base_set_hint(row->field, row->helper_text);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_form_row_set_error_text(struct cupertino_form_row *row,
                                             const char *error_text) {
  ui_error_t rc;

  if (!row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (error_text) {
    copy_string(row->error_text, sizeof(row->error_text), error_text);
  } else {
    row->error_text[0] = '\0';
  }

  if (row->field) {
    rc = ui_form_field_base_set_error(row->field,
                                      error_text ? row->error_text : NULL);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_form_row_set_child(struct cupertino_form_row *row,
                                        struct ui_component *child) {
  if (!row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_form_field_base_set_control(row->field, child);
}

ui_error_t cupertino_form_row_set_disabled(struct cupertino_form_row *row,
                                           int disabled) {
  if (!row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  row->is_disabled = disabled ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_form_row_get_field(struct cupertino_form_row *row,
                                        struct ui_form_field_base **out_field) {
  if (!row || !out_field) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_field = row->field;
  return UI_ERROR_NONE;
}

/* --- Cupertino Text Form Field Row Implementation --- */

ui_error_t cupertino_text_form_field_row_create(
    struct ui_engine *engine,
    struct cupertino_text_form_field_row **out_field_row,
    struct ui_control_value_accessor **out_cva) {
  struct cupertino_text_form_field_row *field_row;
  struct ui_component *input_comp = NULL;
  ui_error_t rc;

  if (!engine || !out_field_row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field_row = (struct cupertino_text_form_field_row *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_text_form_field_row));
  if (!field_row) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(field_row, 0, sizeof(struct cupertino_text_form_field_row));

  rc = cupertino_form_row_create(engine, &field_row->row);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(field_row);
    return rc;
  }

  rc = ui_input_base_create(&field_row->input);
  if (rc != UI_ERROR_NONE) {
    cupertino_form_row_destroy(field_row->row);
    C_MULTIPLATFORM_FREE(field_row);
    return rc;
  }

  rc = ui_input_base_get_component(field_row->input, &input_comp);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_input_base_destroy(field_row->input);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    destroy_rc = cupertino_form_row_destroy(field_row->row);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(field_row);
    return rc;
  }

  rc = cupertino_form_row_set_child(field_row->row, input_comp);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_input_base_destroy(field_row->input);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    destroy_rc = cupertino_form_row_destroy(field_row->row);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(field_row);
    return rc;
  }

  field_row->cva.write_value = cupertino_text_form_field_cva_write_value;
  field_row->cva.set_disabled_state =
      cupertino_text_form_field_cva_set_disabled_state;

  if (out_cva) {
    *out_cva = &field_row->cva;
  }

  *out_field_row = field_row;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_form_field_row_destroy(
    struct cupertino_text_form_field_row *field_row) {
  ui_error_t rc;

  if (!field_row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (field_row->input) {
    rc = ui_input_base_destroy(field_row->input);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    field_row->input = NULL;
  }

  if (field_row->row) {
    rc = cupertino_form_row_destroy(field_row->row);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    field_row->row = NULL;
  }

  C_MULTIPLATFORM_FREE(field_row);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_form_field_row_set_text(
    struct cupertino_text_form_field_row *field_row, const char *text) {
  ui_error_t rc;

  if (!field_row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (text) {
    copy_string(field_row->text_value, sizeof(field_row->text_value), text);
  } else {
    field_row->text_value[0] = '\0';
  }

  if (field_row->input) {
    rc = ui_input_base_set_text(field_row->input, field_row->text_value);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (field_row->row && field_row->row->field) {
    rc = ui_form_field_base_set_has_value(
        field_row->row->field, (field_row->text_value[0] != '\0') ? 1 : 0);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_form_field_row_get_text(
    const struct cupertino_text_form_field_row *field_row,
    const char **out_text) {
  if (!field_row || !out_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_text = field_row->text_value;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_form_field_row_set_placeholder(
    struct cupertino_text_form_field_row *field_row, const char *placeholder) {
  ui_error_t rc;

  if (!field_row) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (placeholder) {
    copy_string(field_row->placeholder, sizeof(field_row->placeholder),
                placeholder);
  } else {
    field_row->placeholder[0] = '\0';
  }

  if (field_row->input) {
    rc =
        ui_input_base_set_placeholder(field_row->input, field_row->placeholder);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_text_form_field_row_bind_form_control(
    struct cupertino_text_form_field_row *field_row,
    struct ui_form_control *form_control, struct ui_reactor *reactor) {
  ui_error_t rc;

  if (!field_row || !form_control || !reactor) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  field_row->form_control = form_control;
  field_row->reactor = reactor;

  if (field_row->row && field_row->row->field) {
    rc = ui_form_field_base_bind_form_control(field_row->row->field,
                                              form_control, reactor);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}
