/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include "themes/cmp_f2_dropdowns.h"
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @brief cmp_f2_dropdown_create
 *
 * @param out_node Parameter description.
 * @param mode Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_dropdown_create(cmp_ui_node_t **out_node,
                                   cmp_f2_dropdown_selection_mode_t mode) {
  int rc = 0;
  cmp_f2_dropdown_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_dropdown_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node); /* Base container */
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_dropdown_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_dropdown_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_dropdown_create: OOM\n");
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
    res = cmp_ui_node_add_child(*out_node, data->trigger_node);
    if (res != CMP_SUCCESS)
      LOG_DEBUG("cmp_f2_dropdown_create: cmp_ui_node_add_child failed\n");
  } else {
    LOG_DEBUG("cmp_f2_dropdown_create: cmp_ui_text_create failed\n");
  }

  res = cmp_ui_text_create(&data->chevron_node, "V", -1);
  if (res == CMP_SUCCESS) {
    res = cmp_ui_node_add_child(*out_node, data->chevron_node);
    if (res != CMP_SUCCESS)
      LOG_DEBUG(
          "cmp_f2_dropdown_create: cmp_ui_node_add_child (chevron) failed\n");
  } else {
    LOG_DEBUG("cmp_f2_dropdown_create: cmp_ui_text_create (chevron) failed\n");
  }

  (*out_node)->properties = (void *)data;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_dropdown_set_variant
 *
 * @param node Parameter description.
 * @param variant Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_dropdown_set_variant(cmp_ui_node_t *node,
                                        cmp_f2_text_input_variant_t variant) {
  int rc = 0;
  cmp_f2_dropdown_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_dropdown_set_variant: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_dropdown_t *)node->properties;
  data->variant = variant;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_dropdown_set_open
 *
 * @param node Parameter description.
 * @param is_open Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_dropdown_set_open(cmp_ui_node_t *node, int is_open) {
  int rc = 0;
  cmp_f2_dropdown_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_dropdown_set_open: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_dropdown_t *)node->properties;
  data->is_open = is_open ? 1 : 0;
  /* Rotates chevron natively */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_dropdown_set_disabled
 *
 * @param node Parameter description.
 * @param is_disabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_dropdown_set_disabled(cmp_ui_node_t *node, int is_disabled) {
  int rc = 0;
  cmp_f2_dropdown_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_dropdown_set_disabled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_dropdown_t *)node->properties;
  data->is_disabled = is_disabled ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_combobox_create
 *
 * @param out_node Parameter description.
 * @param is_freeform Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_combobox_create(cmp_ui_node_t **out_node, int is_freeform) {
  int rc = 0;
  cmp_f2_combobox_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_combobox_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_combobox_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_combobox_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_combobox_create: OOM\n");
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
    res = cmp_ui_node_add_child(*out_node, data->input_node);
    if (res != CMP_SUCCESS)
      LOG_DEBUG("cmp_f2_combobox_create: cmp_ui_node_add_child failed\n");
  } else {
    LOG_DEBUG("cmp_f2_combobox_create: cmp_ui_text_input_create failed\n");
  }

  (*out_node)->properties = (void *)data;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_combobox_set_open
 *
 * @param node Parameter description.
 * @param is_open Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_combobox_set_open(cmp_ui_node_t *node, int is_open) {
  int rc = 0;
  cmp_f2_combobox_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_combobox_set_open: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_combobox_t *)node->properties;
  data->is_open = is_open ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_combobox_set_disabled
 *
 * @param node Parameter description.
 * @param is_disabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_combobox_set_disabled(cmp_ui_node_t *node, int is_disabled) {
  int rc = 0;
  cmp_f2_combobox_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_combobox_set_disabled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_combobox_t *)node->properties;
  data->is_disabled = is_disabled ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_listbox_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_listbox_create(cmp_ui_node_t **out_node) {
  int rc = 0;
  int res;
  if (!out_node) {
    LOG_DEBUG("cmp_f2_listbox_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res == CMP_SUCCESS) {
    (*out_node)->layout->direction = CMP_FLEX_COLUMN;
  } else {
    LOG_DEBUG("cmp_f2_listbox_create: cmp_ui_box_create failed\n");
  }
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  return res;
}

/**
 * @brief cmp_f2_listbox_option_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_listbox_option_create(cmp_ui_node_t **out_node,
                                         const char *label) {
  int rc = 0;
  cmp_f2_listbox_option_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_listbox_option_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_listbox_option_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_listbox_option_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_listbox_option_create: OOM\n");
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
      res = cmp_ui_node_add_child(*out_node, data->content_node);
      if (res != CMP_SUCCESS)
        LOG_DEBUG(
            "cmp_f2_listbox_option_create: cmp_ui_node_add_child failed\n");
    } else {
      LOG_DEBUG("cmp_f2_listbox_option_create: cmp_ui_text_create failed\n");
    }
  }

  (*out_node)->properties = (void *)data;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_listbox_option_set_selected
 *
 * @param node Parameter description.
 * @param is_selected Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_listbox_option_set_selected(cmp_ui_node_t *node,
                                               int is_selected) {
  int rc = 0;
  cmp_f2_listbox_option_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_listbox_option_set_selected: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_listbox_option_t *)node->properties;
  data->is_selected = is_selected ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_listbox_option_set_disabled
 *
 * @param node Parameter description.
 * @param is_disabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_listbox_option_set_disabled(cmp_ui_node_t *node,
                                               int is_disabled) {
  int rc = 0;
  cmp_f2_listbox_option_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_listbox_option_set_disabled: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_listbox_option_t *)node->properties;
  data->is_disabled = is_disabled ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_listbox_group_create
 *
 * @param out_node Parameter description.
 * @param header_label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_listbox_group_create(cmp_ui_node_t **out_node,
                                        const char *header_label) {
  int rc = 0;
  int res;
  cmp_ui_node_t *header_node = NULL;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_listbox_group_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_f2_listbox_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_listbox_group_create: cmp_f2_listbox_create failed\n");
    return res;
  }

  if (header_label) {
    res = cmp_ui_text_create(&header_node, header_label, -1);
    if (res == CMP_SUCCESS) {
      res = cmp_ui_node_add_child(*out_node, header_node);
      if (res != CMP_SUCCESS)
        LOG_DEBUG(
            "cmp_f2_listbox_group_create: cmp_ui_node_add_child failed\n");
    } else {
      LOG_DEBUG("cmp_f2_listbox_group_create: cmp_ui_text_create failed\n");
    }
  }

  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_color_picker_create
 *
 * @param out_node Parameter description.
 * @param initial_color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_color_picker_create(cmp_ui_node_t **out_node,
                                       uint32_t initial_color) {
  int rc = 0;
  cmp_f2_color_picker_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_color_picker_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_color_picker_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_color_picker_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_color_picker_create: OOM\n");
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
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_color_picker_set_color
 *
 * @param node Parameter description.
 * @param color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_color_picker_set_color(cmp_ui_node_t *node, uint32_t color) {
  int rc = 0;
  cmp_f2_color_picker_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_color_picker_set_color: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_color_picker_t *)node->properties;
  data->current_color = color;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_calendar_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_calendar_create(cmp_ui_node_t **out_node) {
  int rc = 0;
  cmp_f2_calendar_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_calendar_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_calendar_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_calendar_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_calendar_create: OOM\n");
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
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_calendar_set_view
 *
 * @param node Parameter description.
 * @param view Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_calendar_set_view(cmp_ui_node_t *node,
                                     cmp_f2_calendar_view_t view) {
  int rc = 0;
  cmp_f2_calendar_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_calendar_set_view: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_calendar_t *)node->properties;
  data->view_mode = view;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_calendar_set_date
 *
 * @param node Parameter description.
 * @param year Parameter description.
 * @param month Parameter description.
 * @param day Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_calendar_set_date(cmp_ui_node_t *node, int year, int month,
                                     int day) {
  int rc = 0;
  cmp_f2_calendar_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_calendar_set_date: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_calendar_t *)node->properties;
  data->selected_year = year;
  data->selected_month = month;
  data->selected_day = day;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_date_picker_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_date_picker_create(cmp_ui_node_t **out_node) {
  int rc = 0;
  cmp_f2_date_picker_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_date_picker_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_date_picker_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_date_picker_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_date_picker_create: OOM\n");
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
    res = cmp_ui_node_add_child(*out_node, data->input_node);
    if (res != CMP_SUCCESS)
      LOG_DEBUG("cmp_f2_date_picker_create: cmp_ui_node_add_child failed\n");
  } else {
    LOG_DEBUG("cmp_f2_date_picker_create: cmp_ui_text_input_create failed\n");
  }

  (*out_node)->properties = (void *)data;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_date_picker_set_open
 *
 * @param node Parameter description.
 * @param is_open Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_date_picker_set_open(cmp_ui_node_t *node, int is_open) {
  int rc = 0;
  cmp_f2_date_picker_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_date_picker_set_open: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_date_picker_t *)node->properties;
  data->is_open = is_open ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_time_picker_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_time_picker_create(cmp_ui_node_t **out_node) {
  int rc = 0;
  cmp_f2_time_picker_t *data;
  int res;

  if (!out_node) {
    LOG_DEBUG("cmp_f2_time_picker_create: out_node is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_time_picker_create: cmp_ui_box_create failed\n");
    return res;
  }

  res = CMP_MALLOC(sizeof(cmp_f2_time_picker_t), (void **)&data);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_f2_time_picker_create: OOM\n");
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
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_f2_time_picker_set_time
 *
 * @param node Parameter description.
 * @param hour Parameter description.
 * @param minute Parameter description.
 * @param is_pm Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_time_picker_set_time(cmp_ui_node_t *node, int hour,
                                        int minute, int is_pm) {
  int rc = 0;
  cmp_f2_time_picker_t *data;
  if (!node || !node->properties) {
    LOG_DEBUG("cmp_f2_time_picker_set_time: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  data = (cmp_f2_time_picker_t *)node->properties;
  data->selected_hour = hour;
  data->selected_minute = minute;
  data->is_pm = is_pm ? 1 : 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}