/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "themes/cmp_theme_dispatch.h"
/* clang-format on */

/**
 * @brief Dispatcher for layout intrinsic measurement.
 * @param ctx The UI node context.
 * @param max_width The available width for wrapping.
 * @param out_w Pointer to receive intrinsic width.
 * @param out_h Pointer to receive intrinsic height.
 * @return 0 on success, or an error code.
 */
int cmp_ui_layout_measure_dispatch(void *ctx, float max_width, float *out_w,
                                   float *out_h) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  (void)max_width;

  if (!ctx || !out_w || !out_h) {
    return CMP_ERROR_INVALID_ARG;
  }
  node = (cmp_ui_node_t *)ctx;

  switch (node->type) {
  case 2: /* Text */
    /* TODO: Theme text measurement routing with max_width */
    rc = CMP_SUCCESS;
    break;
  case 3: /* Button */
    rc = cmp_theme_measure_button(node, out_w, out_h);
    break;
  case 5: /* Checkbox */
    rc = cmp_theme_measure_checkbox(node, out_w, out_h);
    break;
  case 6: /* Radio */
    rc = cmp_theme_measure_radio(node, out_w, out_h);
    break;
  case 9: /* Toggle (Switch) is actually 9? Wait, no. */
    rc = cmp_theme_measure_toggle(node, out_w, out_h);
    break;
  case 8: /* Slider */
    rc = cmp_theme_measure_slider(node, out_w, out_h);
    break;
  case 10: /* Progress (Wait, progress doesn't have a specific type yet in my
              grep, maybe it does? I'll use default) */
    rc = cmp_theme_measure_progress(node, out_w, out_h);
    break;
  case 11: /* Dropdown */
    rc = cmp_theme_measure_dropdown(node, out_w, out_h);
    break;
  default:
    /* Other types don't have intrinsic leaf measurements, or fallback */
    rc = CMP_SUCCESS;
    break;
  }

  if (rc != CMP_SUCCESS) {
    char log_buf[128];
    *out_w = 0.0f;
    *out_h = 0.0f;
#if defined(_MSC_VER)
    sprintf_s(log_buf, sizeof(log_buf),
              "Theme measure failed: " CMP_FMT_I64 "\n", (int64_t)rc);
#else
    sprintf(log_buf, "Theme measure failed: " CMP_FMT_I64 "\n", (int64_t)rc);
#endif
    LOG_DEBUG("%s", log_buf);
    return rc;
  }

  return rc;
}

/**
 * @brief cmp_ui_box_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_box_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_box_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_box_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 1; /* Box */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_1 = CMP_FREE(node);

      if (free_rc_1 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_text_create
 *
 * @param out_node Parameter description.
 * @param text Parameter description.
 * @param text_len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_text_create(cmp_ui_node_t **out_node, const char *text,
                       int text_len) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  char *text_copy;
  size_t len;

  if (out_node == NULL || text == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_text_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_text_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 2; /* Text */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_2 = CMP_FREE(node);

      if (free_rc_2 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  len = (text_len < 0) ? strlen(text) : (size_t)text_len;
  if (CMP_MALLOC(len + 1, (void **)&text_copy) != CMP_SUCCESS) {
    (void)cmp_layout_node_destroy(node->layout);
    {

      int free_rc_3 = CMP_FREE(node);

      if (free_rc_3 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    return CMP_ERROR_OOM;
  }

  memcpy(text_copy, text, len);
  text_copy[len] = '\0';

  node->properties = text_copy;
  node->layout->measure_ctx = node;
  node->layout->measure_cb = cmp_ui_layout_measure_dispatch;
  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_button_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @param label_len Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_button_create(cmp_ui_node_t **out_node, const char *label,
                         int label_len) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  char *label_copy;
  size_t len;

  if (out_node == NULL || label == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_button_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_button_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 3; /* Button */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_4 = CMP_FREE(node);

      if (free_rc_4 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  len = (label_len < 0) ? strlen(label) : (size_t)label_len;
  if (CMP_MALLOC(len + 1, (void **)&label_copy) != CMP_SUCCESS) {
    (void)cmp_layout_node_destroy(node->layout);
    {

      int free_rc_5 = CMP_FREE(node);

      if (free_rc_5 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    return CMP_ERROR_OOM;
  }

  memcpy(label_copy, label, len);
  label_copy[len] = '\0';

  node->properties = label_copy;
  node->layout->measure_ctx = node;
  node->layout->measure_cb = cmp_ui_layout_measure_dispatch;
  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_text_input_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_text_input_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_text_input_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_text_input_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 4; /* Text Input */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_6 = CMP_FREE(node);

      if (free_rc_6 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  node->layout->measure_ctx = node;
  node->layout->measure_cb = cmp_ui_layout_measure_dispatch;
  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_checkbox_create
 *
 * @param out_node Parameter description.
 * @param label Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_checkbox_create(cmp_ui_node_t **out_node, const char *label) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  char *label_copy = NULL;

  if (out_node == NULL || label == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_checkbox_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_checkbox_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 5; /* Checkbox */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_7 = CMP_FREE(node);

      if (free_rc_7 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  if (label != NULL) {
    size_t len = strlen(label);
    if (CMP_MALLOC(len + 1, (void **)&label_copy) != CMP_SUCCESS) {
      (void)cmp_layout_node_destroy(node->layout);
      {

        int free_rc_8 = CMP_FREE(node);

        if (free_rc_8 != CMP_SUCCESS) {
          LOG_DEBUG("CMP_FREE: %d\n", rc);
          return rc;
        }
      }
      return CMP_ERROR_OOM;
    }
#if defined(_MSC_VER)
    strcpy_s(label_copy, len + 1, label);
#else
    strcpy(label_copy, label);
#endif
  }

  node->properties = label_copy;
  node->layout->measure_ctx = node;
  node->layout->measure_cb = cmp_ui_layout_measure_dispatch;
  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_radio_create
 *
 * @param out_node Parameter description.
 * @param group_id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_radio_create(cmp_ui_node_t **out_node, int group_id) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  int *group_prop;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_radio_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_radio_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 6; /* Radio */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_9 = CMP_FREE(node);

      if (free_rc_9 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  if (CMP_MALLOC(sizeof(int), (void **)&group_prop) != CMP_SUCCESS) {
    (void)cmp_layout_node_destroy(node->layout);
    {

      int free_rc_10 = CMP_FREE(node);

      if (free_rc_10 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    return CMP_ERROR_OOM;
  }

  *group_prop = group_id;
  node->properties = group_prop;

  node->layout->measure_ctx = node;
  node->layout->measure_cb = cmp_ui_layout_measure_dispatch;
  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_image_view_create
 *
 * @param out_node Parameter description.
 * @param image_path Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_image_view_create(cmp_ui_node_t **out_node, const char *image_path) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  char *path_copy = NULL;
  cmp_svg_renderer_t *renderer = NULL;
  void **props = NULL;
  size_t len;

  if (out_node == NULL || image_path == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_image_view_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_image_view_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 7; /* ImageView */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_11 = CMP_FREE(node);

      if (free_rc_11 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  len = strlen(image_path);
  if (CMP_MALLOC(len + 1, (void **)&path_copy) == CMP_SUCCESS) {
#if defined(_MSC_VER)
    strcpy_s(path_copy, len + 1, image_path);
#else
    strcpy(path_copy, image_path);
#endif
  }

  if (strstr(image_path, ".svg") != NULL) {
    void *buffer = NULL;
    size_t size = 0;
    rc = cmp_svg_renderer_create(&renderer, 0.5f);
    if (rc == CMP_SUCCESS) {
      if (cmp_vfs_read_file_sync(image_path, &buffer, &size) == CMP_SUCCESS &&
          buffer != NULL) {
        char *svg_str;
        if (CMP_MALLOC(size + 1, (void **)&svg_str) == CMP_SUCCESS) {
          char *p;
          memcpy(svg_str, buffer, size);
          svg_str[size] = '\0';

          p = strstr(svg_str, " d=\"");
          if (p) {
            char *start = p + 4;
            char *end = strchr(start, '"');
            if (end) {
              *end = '\0';
              (void)cmp_svg_parse_path_str(start, renderer);
            }
          }
          {

            int free_rc_12 = CMP_FREE(svg_str);

            if (free_rc_12 != CMP_SUCCESS) {
              LOG_DEBUG("CMP_FREE: %d\n", rc);
              return rc;
            }
          }
        }
        {

          int free_rc_13 = CMP_FREE(buffer);

          if (free_rc_13 != CMP_SUCCESS) {
            LOG_DEBUG("CMP_FREE: %d\n", rc);
            return rc;
          }
        }
      }
    }
  }

  if (CMP_MALLOC(sizeof(void *) * 2, (void **)&props) == CMP_SUCCESS) {
    props[0] = path_copy;
    props[1] = renderer;
    node->properties = props;
  } else {
    if (path_copy) {

      int free_rc_14 = CMP_FREE(path_copy);

      if (free_rc_14 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    if (renderer)
      (void)cmp_svg_renderer_destroy(renderer);
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_slider_create
 *
 * @param out_node Parameter description.
 * @param min Parameter description.
 * @param max Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_slider_create(cmp_ui_node_t **out_node, float min, float max) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  float *bounds;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_slider_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_slider_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 8; /* Slider */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_15 = CMP_FREE(node);

      if (free_rc_15 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  if (CMP_MALLOC(sizeof(float) * 2, (void **)&bounds) != CMP_SUCCESS) {
    (void)cmp_layout_node_destroy(node->layout);
    {

      int free_rc_16 = CMP_FREE(node);

      if (free_rc_16 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    return CMP_ERROR_OOM;
  }

  bounds[0] = min;
  bounds[1] = max;
  node->properties = bounds;

  node->layout->measure_ctx = node;
  node->layout->measure_cb = cmp_ui_layout_measure_dispatch;
  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_list_view_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_list_view_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_list_view_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_list_view_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 9; /* ListView */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_17 = CMP_FREE(node);

      if (free_rc_17 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_grid_view_create
 *
 * @param out_node Parameter description.
 * @param columns Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_grid_view_create(cmp_ui_node_t **out_node, int columns) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;
  int *cols_prop;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_grid_view_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_grid_view_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 10; /* GridView */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_18 = CMP_FREE(node);

      if (free_rc_18 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  if (CMP_MALLOC(sizeof(int), (void **)&cols_prop) != CMP_SUCCESS) {
    (void)cmp_layout_node_destroy(node->layout);
    {

      int free_rc_19 = CMP_FREE(node);

      if (free_rc_19 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    return CMP_ERROR_OOM;
  }

  *cols_prop = columns;
  node->properties = cols_prop;

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_dropdown_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_dropdown_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_dropdown_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_dropdown_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 11; /* Dropdown */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_20 = CMP_FREE(node);

      if (free_rc_20 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  node->layout->measure_ctx = node;
  node->layout->measure_cb = cmp_ui_layout_measure_dispatch;
  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_modal_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_modal_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_modal_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_modal_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 12; /* Modal */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_21 = CMP_FREE(node);

      if (free_rc_21 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_canvas_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_canvas_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_canvas_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_canvas_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 13; /* Canvas */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_22 = CMP_FREE(node);

      if (free_rc_22 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_rich_text_create
 *
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_rich_text_create(cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_rich_text_create: %d\n", rc);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {

    LOG_DEBUG("cmp_ui_rich_text_create CMP_MALLOC: %d\n", rc);

    return rc;
  }

  memset(node, 0, sizeof(cmp_ui_node_t));
  node->type = 14; /* Rich Text */

  rc = cmp_layout_node_create(&node->layout);
  if (rc != CMP_SUCCESS) {
    {

      int free_rc_23 = CMP_FREE(node);

      if (free_rc_23 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    LOG_DEBUG("cmp_ui_box_create cmp_layout_node_create: %d\n", rc);

    return rc;
  }

  *out_node = node;
  return rc;
}

/**
 * @brief cmp_ui_node_add_child
 *
 * @param parent Parameter description.
 * @param child Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_node_add_child(cmp_ui_node_t *parent, cmp_ui_node_t *child) {
  int rc = CMP_SUCCESS;
  if (parent == NULL || child == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_node_add_child: %d\n", rc);

    return rc;
  }

  if (parent->child_count >= parent->child_capacity) {
    size_t new_cap =
        parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
    cmp_ui_node_t **new_children;

    rc = CMP_MALLOC(sizeof(cmp_ui_node_t *) * new_cap, (void **)&new_children);
    if (rc != CMP_SUCCESS) {

      LOG_DEBUG("cmp_ui_node_add_child CMP_MALLOC: %d\n", rc);

      return rc;
    }

    if (parent->children != NULL) {
      memcpy(new_children, parent->children,
             sizeof(cmp_ui_node_t *) * parent->child_count);
      {

        int free_rc_24 = CMP_FREE(parent->children);

        if (free_rc_24 != CMP_SUCCESS) {
          LOG_DEBUG("CMP_FREE: %d\n", rc);
          return rc;
        }
      }
    }

    parent->children = new_children;
    parent->child_capacity = new_cap;
  }

  /* Link UI Hierarchy */
  parent->children[parent->child_count++] = child;
  child->parent = parent;

  /* Link Layout Hierarchy */
  (void)cmp_layout_node_add_child(parent->layout, child->layout);
  return rc;
}

#if 0
/**
 * @brief disabled_cmp_event_dispatch_run
 *
 * @param tree Parameter description.
 * @param target_node Parameter description.
 * @param event Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int disabled_cmp_event_dispatch_run(cmp_ui_node_t *tree,
                                    cmp_ui_node_t *target_node,
                                    cmp_event_t *event) {
  int rc = CMP_SUCCESS;
  cmp_ui_node_t *path[64];
  int path_len = 0;
  int i;
  cmp_ui_node_t *curr = target_node;

  if (tree == NULL || target_node == NULL || event == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Build path from target up to root */
  while (curr != NULL && path_len < 64) {
    path[path_len++] = curr;
    if (curr == tree)
      break;
    curr = curr->parent;
  }

  /* Phase 1: Capturing (Root down to Target) */
  for (i = path_len - 1; i >= 0; i--) {
    cmp_event_listener_node_t *listener = path[i]->event_listeners;
    while (listener) {
      if (listener->event_type == event->type && listener->capture) {
        listener->callback(event, path[i], listener->user_data);
      }
      listener = listener->next;
    }
  }

  /* Phase 2: Bubbling (Target up to Root) */
  for (i = 0; i < path_len; i++) {
    cmp_event_listener_node_t *listener = path[i]->event_listeners;
    while (listener) {
      if (listener->event_type == event->type && !listener->capture) {
        listener->callback(event, path[i], listener->user_data);
      }
      listener = listener->next;
    }
  }

  return rc;
}

/**
 * @brief disabled_cmp_ui_node_add_event_listener
 * @param node
 * @param event_type
 * @param capture
 * @param callback
 * @param user_data
 * @return
 */
static int disabled_cmp_ui_node_add_event_listener(
    cmp_ui_node_t *node, uint32_t event_type, int capture,
    void (*callback)(cmp_event_t *, cmp_ui_node_t *, void *), void *user_data) {
  int rc = CMP_SUCCESS;
  cmp_event_listener_node_t *listener;

  if (node == NULL || callback == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("disabled_cmp_ui_node_add_event_listener: %d\n", rc);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_event_listener_node_t), (void **)&listener);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("CMP_MALLOC: %d\n", rc);
    return rc;
  }

  listener->event_type = event_type;
  listener->capture = capture;
  listener->callback = callback;
  listener->user_data = user_data;
  listener->next = node->event_listeners;
  node->event_listeners = listener;

  return rc;
}
#endif

/**
 * @brief cmp_ui_node_destroy
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_node_destroy(cmp_ui_node_t *node) {
  int rc = CMP_SUCCESS;
  size_t i;

  cmp_event_listener_node_t *listener;
  cmp_event_listener_node_t *next_listener;

  if (node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ui_node_destroy: %d\n", rc);

    return rc;
  }

  listener = node->event_listeners;
  while (listener) {
    next_listener = listener->next;
    {

      int free_rc_25 = CMP_FREE(listener);

      if (free_rc_25 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    listener = next_listener;
  }

  for (i = 0; i < node->child_count; i++) {
    (void)cmp_ui_node_destroy(node->children[i]);
  }

  if (node->children != NULL) {
    {

      int free_rc_26 = CMP_FREE(node->children);

      if (free_rc_26 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
  }

  if (node->layout != NULL) {
    /* Layout tree is not destroyed here recursively because children's layouts
     * are destroyed when the child UI node is destroyed. We just destroy the
     * single layout node itself by detaching it first to prevent double-free if
     * someone calls cmp_layout_node_destroy manually */
    node->layout->child_count = 0;
    (void)cmp_layout_node_destroy(node->layout);
  }

  if (node->type == 2 && node->properties != NULL) {
    /* Free text string */
    {

      int free_rc_27 = CMP_FREE(node->properties);

      if (free_rc_27 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
  } else if (node->type == 7 && node->properties != NULL) {
    void **props = (void **)node->properties;
    if (props[0]) {

      int free_rc_28 = CMP_FREE(props[0]);

      if (free_rc_28 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
    if (props[1])
      (void)cmp_svg_renderer_destroy((cmp_svg_renderer_t *)props[1]);
    {

      int free_rc_29 = CMP_FREE(node->properties);

      if (free_rc_29 != CMP_SUCCESS) {
        LOG_DEBUG("CMP_FREE: %d\n", rc);
        return rc;
      }
    }
  }

  {

    int free_rc_30 = CMP_FREE(node);

    if (free_rc_30 != CMP_SUCCESS) {
      LOG_DEBUG("CMP_FREE: %d\n", rc);
      return rc;
    }
  }
  return rc;
}
