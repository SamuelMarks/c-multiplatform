/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_dropdowns.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

CMP_API int cmp_f2_dropdown_create(cmp_ui_node_t **out_node,
                                   cmp_f2_dropdown_selection_mode_t mode) {
  cmp_f2_dropdown_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node); /* Base container */
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_dropdown_t *)malloc(sizeof(cmp_f2_dropdown_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->variant = CMP_F2_TEXT_INPUT_VARIANT_OUTLINE;
  data->selection_mode = mode;
  data->is_open = 0;
  data->is_disabled = 0;

  data->trigger_node = NULL;
  data->listbox_node = NULL;
  data->chevron_node = NULL;

  /* Mock children */
  res = cmp_ui_text_create(&data->trigger_node, "Select...", -1);
  if (res == CMP_SUCCESS) {
    cmp_ui_node_add_child(*out_node, data->trigger_node);
  }

  res = cmp_ui_text_create(&data->chevron_node, "V", -1);
  if (res == CMP_SUCCESS) {
    cmp_ui_node_add_child(*out_node, data->chevron_node);
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_dropdown_set_variant(cmp_ui_node_t *node,
                                        cmp_f2_text_input_variant_t variant) {
  cmp_f2_dropdown_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_dropdown_t *)node->properties;
  data->variant = variant;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_dropdown_set_open(cmp_ui_node_t *node, int is_open) {
  cmp_f2_dropdown_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_dropdown_t *)node->properties;
  data->is_open = is_open ? 1 : 0;
  /* Rotates chevron natively */
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_dropdown_set_disabled(cmp_ui_node_t *node, int is_disabled) {
  cmp_f2_dropdown_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_dropdown_t *)node->properties;
  data->is_disabled = is_disabled ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_combobox_create(cmp_ui_node_t **out_node, int is_freeform) {
  cmp_f2_combobox_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_combobox_t *)malloc(sizeof(cmp_f2_combobox_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_freeform = is_freeform ? 1 : 0;
  data->is_open = 0;
  data->is_disabled = 0;

  data->input_node = NULL;
  data->listbox_node = NULL;
  data->chevron_node = NULL;

  res = cmp_ui_text_input_create(&data->input_node);
  if (res == CMP_SUCCESS) {
    cmp_ui_node_add_child(*out_node, data->input_node);
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_combobox_set_open(cmp_ui_node_t *node, int is_open) {
  cmp_f2_combobox_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_combobox_t *)node->properties;
  data->is_open = is_open ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_combobox_set_disabled(cmp_ui_node_t *node, int is_disabled) {
  cmp_f2_combobox_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_combobox_t *)node->properties;
  data->is_disabled = is_disabled ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_listbox_create(cmp_ui_node_t **out_node) {
  int res;
  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res == CMP_SUCCESS) {
    (*out_node)->layout->direction = CMP_FLEX_COLUMN;
  }
  return res;
}

CMP_API int cmp_f2_listbox_option_create(cmp_ui_node_t **out_node,
                                         const char *label) {
  cmp_f2_listbox_option_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_listbox_option_t *)malloc(sizeof(cmp_f2_listbox_option_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_selected = 0;
  data->is_disabled = 0;
  data->checkmark_node = NULL;
  data->content_node = NULL;

  if (label) {
    res = cmp_ui_text_create(&data->content_node, label, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, data->content_node);
    }
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_listbox_option_set_selected(cmp_ui_node_t *node,
                                               int is_selected) {
  cmp_f2_listbox_option_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_listbox_option_t *)node->properties;
  data->is_selected = is_selected ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_listbox_option_set_disabled(cmp_ui_node_t *node,
                                               int is_disabled) {
  cmp_f2_listbox_option_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_listbox_option_t *)node->properties;
  data->is_disabled = is_disabled ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_listbox_group_create(cmp_ui_node_t **out_node,
                                        const char *header_label) {
  int res;
  cmp_ui_node_t *header_node = NULL;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_f2_listbox_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  if (header_label) {
    res = cmp_ui_text_create(&header_node, header_label, -1);
    if (res == CMP_SUCCESS) {
      cmp_ui_node_add_child(*out_node, header_node);
    }
  }

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_color_picker_create(cmp_ui_node_t **out_node,
                                       uint32_t initial_color) {
  cmp_f2_color_picker_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_color_picker_t *)malloc(sizeof(cmp_f2_color_picker_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->current_color = initial_color;
  data->hue_slider_node = NULL;
  data->sv_area_node = NULL;
  data->alpha_slider_node = NULL;
  data->hex_input_node = NULL;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_color_picker_set_color(cmp_ui_node_t *node, uint32_t color) {
  cmp_f2_color_picker_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_color_picker_t *)node->properties;
  data->current_color = color;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_calendar_create(cmp_ui_node_t **out_node) {
  cmp_f2_calendar_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_calendar_t *)malloc(sizeof(cmp_f2_calendar_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->view_mode = CMP_F2_CALENDAR_VIEW_MONTH;
  data->selected_year = 0;
  data->selected_month = 0;
  data->selected_day = 0;

  data->header_node = NULL;
  data->grid_node = NULL;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_calendar_set_view(cmp_ui_node_t *node,
                                     cmp_f2_calendar_view_t view) {
  cmp_f2_calendar_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_calendar_t *)node->properties;
  data->view_mode = view;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_calendar_set_date(cmp_ui_node_t *node, int year, int month,
                                     int day) {
  cmp_f2_calendar_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_calendar_t *)node->properties;
  data->selected_year = year;
  data->selected_month = month;
  data->selected_day = day;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_date_picker_create(cmp_ui_node_t **out_node) {
  cmp_f2_date_picker_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_date_picker_t *)malloc(sizeof(cmp_f2_date_picker_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_open = 0;
  data->is_disabled = 0;
  data->input_node = NULL;
  data->calendar_flyout_node = NULL;
  data->icon_node = NULL;

  res = cmp_ui_text_input_create(&data->input_node);
  if (res == CMP_SUCCESS) {
    cmp_ui_node_add_child(*out_node, data->input_node);
  }

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_date_picker_set_open(cmp_ui_node_t *node, int is_open) {
  cmp_f2_date_picker_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_date_picker_t *)node->properties;
  data->is_open = is_open ? 1 : 0;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_time_picker_create(cmp_ui_node_t **out_node) {
  cmp_f2_time_picker_t *data;
  int res;

  if (!out_node)
    return CMP_ERROR_INVALID_ARG;

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS)
    return res;

  data = (cmp_f2_time_picker_t *)malloc(sizeof(cmp_f2_time_picker_t));
  if (!data) {
    cmp_ui_node_destroy(*out_node);
    *out_node = NULL;
    return CMP_ERROR_OOM;
  }

  data->is_open = 0;
  data->selected_hour = 12;
  data->selected_minute = 0;
  data->is_pm = 0;

  data->trigger_node = NULL;
  data->listbox_flyout_node = NULL;

  (*out_node)->properties = (void *)data;
  return CMP_SUCCESS;
}

CMP_API int cmp_f2_time_picker_set_time(cmp_ui_node_t *node, int hour,
                                        int minute, int is_pm) {
  cmp_f2_time_picker_t *data;
  if (!node || !node->properties)
    return CMP_ERROR_INVALID_ARG;
  data = (cmp_f2_time_picker_t *)node->properties;
  data->selected_hour = hour;
  data->selected_minute = minute;
  data->is_pm = is_pm ? 1 : 0;
  return CMP_SUCCESS;
}
