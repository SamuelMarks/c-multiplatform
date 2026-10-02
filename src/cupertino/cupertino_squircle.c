/**
 * @file cupertino_squircle.c
 * @brief Cupertino Squircle geometry and continuous corner curvature
 * implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_squircle.h"
#include "ui_internal_mem.h"
#include <math.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_squircle_mock_cmd_fail_countdown = -1;
int g_cupertino_squircle_mock_denom_zero = 0;

/**
 * @brief Checks if a path command should simulate failure.
 * @return UI_ERROR_OUT_OF_MEMORY if simulated failure, UI_ERROR_NONE otherwise.
 */
static ui_error_t mock_path_check_fail(void) {
  if (g_cupertino_squircle_mock_cmd_fail_countdown == 0) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  if (g_cupertino_squircle_mock_cmd_fail_countdown > 0) {
    g_cupertino_squircle_mock_cmd_fail_countdown--;
  }
  return UI_ERROR_NONE;
}
#endif

/**
 * @brief Ensures sufficient capacity in the vector path command buffer.
 *
 * @param path Pointer to the vector path.
 * @return UI_ERROR_NONE on success, or UI_ERROR_OUT_OF_MEMORY /
 * UI_ERROR_INVALID_ARGUMENT.
 */
static ui_error_t ensure_path_capacity(struct ui_path *path) {
  int new_capacity;
  struct ui_path_cmd *new_cmds;

  if (!path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (path->cmd_count < path->cmd_capacity) {
    return UI_ERROR_NONE;
  }

  new_capacity = (path->cmd_capacity > 0) ? (path->cmd_capacity * 2) : 16;
  new_cmds = (struct ui_path_cmd *)C_MULTIPLATFORM_REALLOC(
      path->cmds, (size_t)new_capacity * sizeof(struct ui_path_cmd));
  if (!new_cmds) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  path->cmds = new_cmds;
  path->cmd_capacity = new_capacity;
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
/**
 * @brief Test hook for testing capacity allocation logic directly.
 * @param path Pointer to the vector path.
 * @return Return value from ensure_path_capacity.
 */
ui_error_t test_hook_ensure_path_capacity(struct ui_path *path) {
  return ensure_path_capacity(path);
}
#endif

/**
 * @brief Calculates a 2D Cartesian point on a Lamé superellipse at a given
 * angle.
 */
ui_error_t cupertino_squircle_point_at(float a, float b, float n, float theta,
                                       float *out_x, float *out_y) {
  float cos_th;
  float sin_th;
  float b_cos;
  float a_sin;
  float denom_term;
  float denom;
  float r;

  if (a <= 0.0f || b <= 0.0f || n <= 0.0f || !out_x || !out_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cos_th = (float)cos(theta);
  sin_th = (float)sin(theta);

  b_cos = (float)fabs(b * cos_th);
  a_sin = (float)fabs(a * sin_th);

  denom_term = (float)(pow(b_cos, n) + pow(a_sin, n));
  if (denom_term <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  denom = (float)pow(denom_term, 1.0f / n);
#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_squircle_mock_denom_zero) {
    denom = 0.0f;
  }
#endif
  if (denom <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  r = (a * b) / denom;
  *out_x = r * cos_th;
  *out_y = r * sin_th;

  return UI_ERROR_NONE;
}

/**
 * @brief Computes dynamic inner border radius for concentric continuous
 * borders.
 */
ui_error_t cupertino_squircle_inset(float outer_radius, float stroke_width,
                                    float *out_inner_radius) {
  if (outer_radius < 0.0f || stroke_width < 0.0f || !out_inner_radius) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (outer_radius > stroke_width) {
    *out_inner_radius = outer_radius - stroke_width;
  } else {
    *out_inner_radius = 0.0f;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Resolves hairline border stroke width based on display density scale.
 */
ui_error_t cupertino_hairline_width(float scale, float *out_width) {
  if (scale <= 0.0f || !out_width) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (scale >= 2.9f) {
    *out_width = 1.0f / 3.0f;
  } else if (scale >= 1.9f) {
    *out_width = 0.5f;
  } else {
    *out_width = 1.0f;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Snaps a layout coordinate to physical device pixel boundaries.
 */
ui_error_t cupertino_snap_to_device_pixel(float pt, float scale,
                                          float stroke_width,
                                          float *out_snapped) {
  int stroke_px;
  float snapped;

  if (scale <= 0.0f || stroke_width < 0.0f || !out_snapped) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  stroke_px = (int)(stroke_width * scale + 0.5f);

  if ((stroke_px % 2) != 0) {
    /* Odd stroke pixel count: center on half-pixel */
    snapped = ((float)((int)floor(pt * scale)) + 0.5f) / scale;
  } else {
    /* Even stroke pixel count: align to integer pixel */
    snapped = (float)((int)floor(pt * scale + 0.5f)) / scale;
  }

  *out_snapped = snapped;
  return UI_ERROR_NONE;
}

/**
 * @brief Initializes a vector path structure with pre-allocated command
 * capacity.
 */
ui_error_t cupertino_path_init(struct ui_path *path, int initial_capacity) {
  if (!path || initial_capacity <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  path->cmds = (struct ui_path_cmd *)C_MULTIPLATFORM_MALLOC(
      (size_t)initial_capacity * sizeof(struct ui_path_cmd));
  if (!path->cmds) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(path->cmds, 0, (size_t)initial_capacity * sizeof(struct ui_path_cmd));
  path->cmd_count = 0;
  path->cmd_capacity = initial_capacity;

  return UI_ERROR_NONE;
}

/**
 * @brief Frees resources allocated for a vector path structure.
 */
ui_error_t cupertino_path_destroy(struct ui_path *path) {
  if (!path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (path->cmds) {
    C_MULTIPLATFORM_FREE(path->cmds);
    path->cmds = NULL;
  }

  path->cmd_count = 0;
  path->cmd_capacity = 0;

  return UI_ERROR_NONE;
}

/**
 * @brief Appends a move-to command to a vector path.
 */
ui_error_t cupertino_path_move_to(struct ui_path *path, float x, float y) {
  ui_error_t rc;
  struct ui_path_cmd *cmd;

  if (!path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#ifdef UI_TEST_MOCK_ALLOC
  rc = mock_path_check_fail();
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
#endif

  rc = ensure_path_capacity(path);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  cmd = &path->cmds[path->cmd_count++];
  cmd->type = UI_PATH_CMD_MOVE_TO;
  cmd->x1 = x;
  cmd->y1 = y;
  cmd->x2 = 0.0f;
  cmd->y2 = 0.0f;
  cmd->x3 = 0.0f;
  cmd->y3 = 0.0f;

  return UI_ERROR_NONE;
}

/**
 * @brief Appends a line-to command to a vector path.
 */
ui_error_t cupertino_path_line_to(struct ui_path *path, float x, float y) {
  ui_error_t rc;
  struct ui_path_cmd *cmd;

  if (!path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#ifdef UI_TEST_MOCK_ALLOC
  rc = mock_path_check_fail();
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
#endif

  rc = ensure_path_capacity(path);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  cmd = &path->cmds[path->cmd_count++];
  cmd->type = UI_PATH_CMD_LINE_TO;
  cmd->x1 = x;
  cmd->y1 = y;
  cmd->x2 = 0.0f;
  cmd->y2 = 0.0f;
  cmd->x3 = 0.0f;
  cmd->y3 = 0.0f;

  return UI_ERROR_NONE;
}

/**
 * @brief Appends a cubic Bézier curve command to a vector path.
 */
ui_error_t cupertino_path_bezier_to(struct ui_path *path, float x1, float y1,
                                    float x2, float y2, float x3, float y3) {
  ui_error_t rc;
  struct ui_path_cmd *cmd;

  if (!path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#ifdef UI_TEST_MOCK_ALLOC
  rc = mock_path_check_fail();
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
#endif

  rc = ensure_path_capacity(path);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  cmd = &path->cmds[path->cmd_count++];
  cmd->type = UI_PATH_CMD_BEZIER_TO;
  cmd->x1 = x1;
  cmd->y1 = y1;
  cmd->x2 = x2;
  cmd->y2 = y2;
  cmd->x3 = x3;
  cmd->y3 = y3;

  return UI_ERROR_NONE;
}

/**
 * @brief Appends a close-path command to a vector path.
 */
ui_error_t cupertino_path_close(struct ui_path *path) {
  ui_error_t rc;
  struct ui_path_cmd *cmd;

  if (!path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#ifdef UI_TEST_MOCK_ALLOC
  rc = mock_path_check_fail();
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
#endif

  rc = ensure_path_capacity(path);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  cmd = &path->cmds[path->cmd_count++];
  cmd->type = UI_PATH_CMD_CLOSE;
  cmd->x1 = 0.0f;
  cmd->y1 = 0.0f;
  cmd->x2 = 0.0f;
  cmd->y2 = 0.0f;
  cmd->x3 = 0.0f;
  cmd->y3 = 0.0f;

  return UI_ERROR_NONE;
}

/**
 * @brief Generates a closed G2 curvature-continuous squircle path.
 */
ui_error_t cupertino_squircle_generate_path(float x, float y, float width,
                                            float height, float corner_radius,
                                            float smoothness,
                                            struct ui_path *out_path) {
  float max_r;
  float p;
  float L;
  float c;
  ui_error_t rc;

  if (width <= 0.0f || height <= 0.0f || corner_radius < 0.0f || !out_path) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  max_r = (width < height ? width : height) * 0.5f;
  if (corner_radius > max_r) {
    corner_radius = max_r;
  }

  if (corner_radius <= 0.001f) {
    /* Sharp rectangle */
    rc = cupertino_path_move_to(out_path, x, y);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_line_to(out_path, x + width, y);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_line_to(out_path, x + width, y + height);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_line_to(out_path, x, y + height);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_close(out_path);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    return UI_ERROR_NONE;
  }

  p = smoothness;
  if (p < 0.0f) {
    p = 0.0f;
  } else if (p > 1.0f) {
    p = 1.0f;
  }

  /* Extended corner curve transition length */
  L = corner_radius * (1.0f + p * 0.528665f);
  if (L > max_r) {
    L = max_r;
  }

  c = corner_radius * 0.55228475f;

  if (p <= 0.01f) {
    /* Standard circular rounded rectangle */
    rc = cupertino_path_move_to(out_path, x + corner_radius, y);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    /* Top edge to Top-Right corner */
    rc = cupertino_path_line_to(out_path, x + width - corner_radius, y);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_bezier_to(out_path, x + width - corner_radius + c, y,
                                  x + width, y + corner_radius - c, x + width,
                                  y + corner_radius);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    /* Right edge to Bottom-Right corner */
    rc =
        cupertino_path_line_to(out_path, x + width, y + height - corner_radius);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_bezier_to(out_path, x + width,
                                  y + height - corner_radius + c,
                                  x + width - corner_radius + c, y + height,
                                  x + width - corner_radius, y + height);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    /* Bottom edge to Bottom-Left corner */
    rc = cupertino_path_line_to(out_path, x + corner_radius, y + height);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_bezier_to(out_path, x + corner_radius - c, y + height,
                                  x, y + height - corner_radius + c, x,
                                  y + height - corner_radius);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    /* Left edge to Top-Left corner */
    rc = cupertino_path_line_to(out_path, x, y + corner_radius);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_bezier_to(out_path, x, y + corner_radius - c,
                                  x + corner_radius - c, y, x + corner_radius,
                                  y);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    /* Continuous G2 corner curvature approximation */
    float p_ratio;
    float ctrl_lead;
    float ctrl_trail;

    p_ratio = p;
    ctrl_lead = L * (0.33f + 0.12f * p_ratio);
    ctrl_trail = corner_radius * (0.45f + 0.10f * p_ratio);

    rc = cupertino_path_move_to(out_path, x + L, y);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    /* Top edge to Top-Right continuous corner */
    rc = cupertino_path_line_to(out_path, x + width - L, y);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_bezier_to(out_path, x + width - L + ctrl_lead, y,
                                  x + width - ctrl_trail, y + ctrl_trail * 0.4f,
                                  x + width - ctrl_trail, y + ctrl_trail);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_bezier_to(out_path, x + width - ctrl_trail * 0.4f,
                                  y + ctrl_trail, x + width, y + L - ctrl_lead,
                                  x + width, y + L);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    /* Right edge to Bottom-Right continuous corner */
    rc = cupertino_path_line_to(out_path, x + width, y + height - L);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_bezier_to(
        out_path, x + width, y + height - L + ctrl_lead,
        x + width - ctrl_trail * 0.4f, y + height - ctrl_trail,
        x + width - ctrl_trail, y + height - ctrl_trail);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_bezier_to(
        out_path, x + width - ctrl_trail, y + height - ctrl_trail * 0.4f,
        x + width - L + ctrl_lead, y + height, x + width - L, y + height);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    /* Bottom edge to Bottom-Left continuous corner */
    rc = cupertino_path_line_to(out_path, x + L, y + height);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc =
        cupertino_path_bezier_to(out_path, x + L - ctrl_lead, y + height,
                                 x + ctrl_trail, y + height - ctrl_trail * 0.4f,
                                 x + ctrl_trail, y + height - ctrl_trail);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_bezier_to(
        out_path, x + ctrl_trail * 0.4f, y + height - ctrl_trail, x,
        y + height - L + ctrl_lead, x, y + height - L);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    /* Left edge to Top-Left continuous corner */
    rc = cupertino_path_line_to(out_path, x, y + L);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_bezier_to(out_path, x, y + L - ctrl_lead,
                                  x + ctrl_trail * 0.4f, y + ctrl_trail,
                                  x + ctrl_trail, y + ctrl_trail);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_path_bezier_to(out_path, x + ctrl_trail,
                                  y + ctrl_trail * 0.4f, x + L - ctrl_lead, y,
                                  x + L, y);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  rc = cupertino_path_close(out_path);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}
