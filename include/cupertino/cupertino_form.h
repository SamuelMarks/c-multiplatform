/**
 * @file cupertino_form.h
 * @brief Cupertino Form Section, Form Row, and Text Form Field Row components
 * conforming to Apple Human Interface Guidelines (HIG).
 */

#ifndef CUPERTINO_CUPERTINO_FORM_H
#define CUPERTINO_CUPERTINO_FORM_H

/* clang-format off */
#include "ui_card_base.h"
#include "ui_color_space.h"
#include "ui_component.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_form_control.h"
#include "ui_form_field_base.h"
#include "ui_input_base.h"
#include "ui_reactor.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum cupertino_form_section_style
 * @brief Visual presentation mode for Cupertino Form Section.
 */
enum cupertino_form_section_style {
  CUPERTINO_FORM_SECTION_PLAIN = 0,     /**< Edge-to-edge section style. */
  CUPERTINO_FORM_SECTION_INSET_GROUPED, /**< Inset grouped squircle card with
                                           16pt margin. */
  CUPERTINO_FORM_SECTION_STYLE_COUNT
};

/**
 * @struct cupertino_form_section
 * @brief iOS Settings-style grouped form container wrapping ui_card_base.
 */
struct cupertino_form_section {
  struct ui_card_base *base;               /**< Wrapped CDK card component. */
  enum cupertino_form_section_style style; /**< Plain or Inset Grouped style. */
  char header_text[128];       /**< Uppercase section header label. */
  char footer_text[256];       /**< Footnote explanatory description. */
  ui_color_t background_color; /**< Card background color. */
  ui_color_t header_color;     /**< Secondary label header color. */
  ui_color_t footer_color;     /**< Footnote muted footer color. */
  float corner_radius;         /**< Squircle corner radius (10pt for inset). */
  float margin; /**< Side margin (16pt for inset, 0 for plain). */
};

/**
 * @struct cupertino_form_row
 * @brief iOS form row layout wrapper wrapping ui_form_field_base.
 */
struct cupertino_form_row {
  struct ui_form_field_base *field; /**< Wrapped CDK form field base. */
  char prefix_label[64];            /**< Leading row label text. */
  char helper_text[128];            /**< Subtitle/footnote explanatory text. */
  char error_text[128];             /**< Red inline error description. */
  ui_color_t prefix_color;          /**< Primary text color (Label). */
  ui_color_t helper_color; /**< Muted secondary color (SecondaryLabel). */
  ui_color_t error_color;  /**< Validation error color (SystemRed). */
  int is_disabled;         /**< Disabled flag (0 = enabled, 1 = disabled). */
};

/**
 * @struct cupertino_text_form_field_row
 * @brief High-level form control wrapping ui_input_base and ui_form_field_base.
 */
struct cupertino_text_form_field_row {
  struct cupertino_form_row *row; /**< Parent row container. */
  struct ui_input_base *input;    /**< Internal input control. */
  struct ui_control_value_accessor
      cva; /**< Control Value Accessor interface. */
  struct ui_form_control *form_control; /**< Bound form control instance. */
  struct ui_reactor *reactor;           /**< Bound reactor for updates. */
  char text_value[256];                 /**< Current text content buffer. */
  char placeholder[128];                /**< Placeholder prompt string. */
};

/* --- Cupertino Form Section Functions --- */

/**
 * @brief Creates a new Cupertino Form Section component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param style Section style (Plain or Inset Grouped).
 * @param out_section Pointer to receive newly created section.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_form_section_create(
    struct ui_engine *engine, enum cupertino_form_section_style style,
    struct cupertino_form_section **out_section);

/**
 * @brief Destroys a Cupertino Form Section and releases allocated resources.
 *
 * @param section Pointer to section to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_form_section_destroy(struct cupertino_form_section *section);

/**
 * @brief Sets section header text.
 *
 * @param section Pointer to section.
 * @param header Header string (displayed in uppercase footnote style).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_form_section_set_header(
    struct cupertino_form_section *section, const char *header);

/**
 * @brief Sets section footer text.
 *
 * @param section Pointer to section.
 * @param footer Footer string (explanatory footnote).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_form_section_set_footer(
    struct cupertino_form_section *section, const char *footer);

/**
 * @brief Sets child content component within section.
 *
 * @param section Pointer to section.
 * @param content Pointer to UI component to mount inside section.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_form_section_set_content(
    struct cupertino_form_section *section, struct ui_component *content);

/**
 * @brief Retrieves underlying ui_card_base component handle.
 *
 * @param section Pointer to section.
 * @param out_base Pointer to receive ui_card_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_form_section_get_base(
    struct cupertino_form_section *section, struct ui_card_base **out_base);

/* --- Cupertino Form Row Functions --- */

/**
 * @brief Creates a new Cupertino Form Row layout component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_row Pointer to receive newly created row.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_form_row_create(
    struct ui_engine *engine, struct cupertino_form_row **out_row);

/**
 * @brief Destroys a Cupertino Form Row and releases allocated resources.
 *
 * @param row Pointer to row to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_form_row_destroy(struct cupertino_form_row *row);

/**
 * @brief Sets prefix label text on the leading side of row.
 *
 * @param row Pointer to row.
 * @param label Prefix label string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_form_row_set_prefix_label(
    struct cupertino_form_row *row, const char *label);

/**
 * @brief Sets helper footnote text displayed beneath row.
 *
 * @param row Pointer to row.
 * @param helper Helper text string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_form_row_set_helper_text(
    struct cupertino_form_row *row, const char *helper);

/**
 * @brief Sets validation error message text displayed beneath row in red.
 *
 * @param row Pointer to row.
 * @param error_text Error message string, or NULL to clear error state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_form_row_set_error_text(
    struct cupertino_form_row *row, const char *error_text);

/**
 * @brief Mounts an input child component inside the form row.
 *
 * @param row Pointer to row.
 * @param child Pointer to child component (e.g. switch, slider, input).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_form_row_set_child(
    struct cupertino_form_row *row, struct ui_component *child);

/**
 * @brief Sets disabled state of form row.
 *
 * @param row Pointer to row.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_form_row_set_disabled(struct cupertino_form_row *row, int disabled);

/**
 * @brief Retrieves underlying ui_form_field_base handle.
 *
 * @param row Pointer to row.
 * @param out_field Pointer to receive ui_form_field_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_form_row_get_field(
    struct cupertino_form_row *row, struct ui_form_field_base **out_field);

/* --- Cupertino Text Form Field Row Functions --- */

/**
 * @brief Creates a high-level Cupertino Text Form Field Row component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_field_row Pointer to receive newly created field row.
 * @param out_cva Optional pointer to receive Control Value Accessor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_form_field_row_create(
    struct ui_engine *engine,
    struct cupertino_text_form_field_row **out_field_row,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Cupertino Text Form Field Row and releases all resources.
 *
 * @param field_row Pointer to field row to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_form_field_row_destroy(
    struct cupertino_text_form_field_row *field_row);

/**
 * @brief Sets text value of text form field row.
 *
 * @param field_row Pointer to field row.
 * @param text New text string value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_form_field_row_set_text(
    struct cupertino_text_form_field_row *field_row, const char *text);

/**
 * @brief Gets current text value of text form field row.
 *
 * @param field_row Pointer to field row.
 * @param out_text Pointer to receive current text string pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_form_field_row_get_text(
    const struct cupertino_text_form_field_row *field_row,
    const char **out_text);

/**
 * @brief Sets placeholder prompt text.
 *
 * @param field_row Pointer to field row.
 * @param placeholder Placeholder string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_form_field_row_set_placeholder(
    struct cupertino_text_form_field_row *field_row, const char *placeholder);

/**
 * @brief Binds form control to text form field row for reactive validation.
 *
 * @param field_row Pointer to field row.
 * @param form_control Form control instance to bind.
 * @param reactor Reactor instance for dispatching updates.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_form_field_row_bind_form_control(
    struct cupertino_text_form_field_row *field_row,
    struct ui_form_control *form_control, struct ui_reactor *reactor);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_FORM_H */
