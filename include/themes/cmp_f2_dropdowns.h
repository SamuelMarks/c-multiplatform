#ifndef CMP_F2_DROPDOWNS_H
#define CMP_F2_DROPDOWNS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_theme.h"
#include "themes/cmp_f2_text_inputs.h"
/* clang-format on */

/* 4.1 Select / Dropdown */
typedef enum cmp_f2_dropdown_selection_mode {
  CMP_F2_DROPDOWN_SELECTION_SINGLE = 0,
  CMP_F2_DROPDOWN_SELECTION_MULTIPLE
} cmp_f2_dropdown_selection_mode_t;

typedef struct cmp_f2_dropdown_s {
  cmp_f2_text_input_variant_t variant;
  cmp_f2_dropdown_selection_mode_t selection_mode;
  int is_open;
  int is_disabled;

  cmp_ui_node_t *trigger_node;
  cmp_ui_node_t *listbox_node; /* Container for options */
  cmp_ui_node_t *chevron_node;
} cmp_f2_dropdown_t;

/**
 * @brief Initialize a Fluent 2 dropdown_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_dropdown_create(cmp_ui_node_t **out_node,
                           cmp_f2_dropdown_selection_mode_t mode);
CMP_API int cmp_f2_dropdown_set_variant(cmp_ui_node_t *node,
                                cmp_f2_text_input_variant_t variant);
CMP_API int cmp_f2_dropdown_set_open(cmp_ui_node_t *node, int is_open);
CMP_API int cmp_f2_dropdown_set_disabled(cmp_ui_node_t *node, int is_disabled);

/* 4.2 ComboBox */
typedef struct cmp_f2_combobox_s {
  int is_freeform;
  int is_open;
  int is_disabled;

  cmp_ui_node_t *input_node; /* Editable text input */
  cmp_ui_node_t *listbox_node;
  cmp_ui_node_t *chevron_node;
} cmp_f2_combobox_t;

/**
 * @brief Initialize a Fluent 2 combobox_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_combobox_create(cmp_ui_node_t **out_node, int is_freeform);
CMP_API int cmp_f2_combobox_set_open(cmp_ui_node_t *node, int is_open);
CMP_API int cmp_f2_combobox_set_disabled(cmp_ui_node_t *node, int is_disabled);

/* 4.3 Listbox & Options */
typedef struct cmp_f2_listbox_option_s {
  int is_selected;
  int is_disabled;
  cmp_ui_node_t *checkmark_node; /* Visible if selected */
  cmp_ui_node_t *content_node;
} cmp_f2_listbox_option_t;

/**
 * @brief Initialize a Fluent 2 listbox_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_listbox_create(cmp_ui_node_t **out_node);
/**
 * @brief Initialize a Fluent 2 listbox_option_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_listbox_option_create(cmp_ui_node_t **out_node, const char *label);
CMP_API int cmp_f2_listbox_option_set_selected(cmp_ui_node_t *node, int is_selected);
CMP_API int cmp_f2_listbox_option_set_disabled(cmp_ui_node_t *node, int is_disabled);

/**
 * @brief Initialize a Fluent 2 listbox_group_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_listbox_group_create(cmp_ui_node_t **out_node,
                                const char *header_label);

/* 4.4 ColorPicker */
typedef struct cmp_f2_color_picker_s {
  uint32_t current_color; /* 0xAARRGGBB */

  cmp_ui_node_t *hue_slider_node;
  cmp_ui_node_t *sv_area_node;
  cmp_ui_node_t *alpha_slider_node;
  cmp_ui_node_t *hex_input_node;
} cmp_f2_color_picker_t;

/**
 * @brief Initialize a Fluent 2 color_picker_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_color_picker_create(cmp_ui_node_t **out_node,
                               uint32_t initial_color);
CMP_API int cmp_f2_color_picker_set_color(cmp_ui_node_t *node, uint32_t color);

/* 4.5 Calendar & DatePicker */
typedef enum cmp_f2_calendar_view {
  CMP_F2_CALENDAR_VIEW_MONTH = 0,
  CMP_F2_CALENDAR_VIEW_YEAR,
  CMP_F2_CALENDAR_VIEW_DECADE
} cmp_f2_calendar_view_t;

typedef struct cmp_f2_calendar_s {
  cmp_f2_calendar_view_t view_mode;
  int selected_year;
  int selected_month;
  int selected_day;

  cmp_ui_node_t *header_node; /* Month/Year title and arrows */
  cmp_ui_node_t *grid_node;   /* Days/Months/Years grid */
} cmp_f2_calendar_t;

typedef struct cmp_f2_date_picker_s {
  int is_open;
  int is_disabled;

  cmp_ui_node_t *input_node;
  cmp_ui_node_t *calendar_flyout_node;
  cmp_ui_node_t *icon_node;
} cmp_f2_date_picker_t;

/**
 * @brief Initialize a Fluent 2 calendar_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_calendar_create(cmp_ui_node_t **out_node);
CMP_API int cmp_f2_calendar_set_view(cmp_ui_node_t *node, cmp_f2_calendar_view_t view);
CMP_API int cmp_f2_calendar_set_date(cmp_ui_node_t *node, int year, int month, int day);

/**
 * @brief Initialize a Fluent 2 date_picker_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_date_picker_create(cmp_ui_node_t **out_node);
CMP_API int cmp_f2_date_picker_set_open(cmp_ui_node_t *node, int is_open);

/* 4.6 TimePicker */
typedef struct cmp_f2_time_picker_s {
  int is_open;
  int selected_hour;
  int selected_minute;
  int is_pm;

  cmp_ui_node_t *trigger_node; /* Often a dropdown or spin button */
  cmp_ui_node_t *listbox_flyout_node;
} cmp_f2_time_picker_t;

/**
 * @brief Initialize a Fluent 2 time_picker_create component.
 * @param
 * out_node Pointer to receive the allocated node.
 * @return 0 on success, or
 * an error code.
 */
CMP_API int cmp_f2_time_picker_create(cmp_ui_node_t **out_node);
CMP_API int cmp_f2_time_picker_set_time(cmp_ui_node_t *node, int hour, int minute,
                                int is_pm);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_F2_DROPDOWNS_H */
