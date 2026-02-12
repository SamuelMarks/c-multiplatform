#include "m3/m3_path.h"

#define M3_PATH_DEFAULT_CAPACITY 16u

static m3_usize m3_path_max_value(void) { return (m3_usize) ~(m3_usize)0; }

static int m3_path_add_overflow(m3_usize a, m3_usize b, m3_usize *out_value) {
  m3_usize max_value;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_value = m3_path_max_value();
  if (a > max_value - b) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a + b;
  return M3_OK;
}

static int m3_path_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value) {
  m3_usize max_value;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_value = m3_path_max_value();
  if (a != 0 && b > max_value / a) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return M3_OK;
}

static int m3_path_validate_initialized(const M3Path *path) {
  if (path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (path->commands == NULL) {
    return M3_ERR_STATE;
  }
  if (path->count > path->capacity) {
    return M3_ERR_STATE;
  }
  return M3_OK;
}

static int m3_path_has_current(const M3Path *path, M3Bool *out_has_current) {
  int rc;

  if (out_has_current == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_path_validate_initialized(path);
  if (rc != M3_OK) {
    return rc;
  }

  if (path->count == 0) {
    *out_has_current = M3_FALSE;
    return M3_OK;
  }

  *out_has_current = (path->commands[path->count - 1].type != M3_PATH_CMD_CLOSE)
                         ? M3_TRUE
                         : M3_FALSE;
  return M3_OK;
}

static int m3_path_reserve(M3Path *path, m3_usize additional) {
  m3_usize required;
  m3_usize new_capacity;
  m3_usize bytes;
  m3_usize max_value;
  void *mem;
  int rc;

  rc = m3_path_validate_initialized(path);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_path_add_overflow(path->count, additional, &required);
  if (rc != M3_OK) {
    return rc;
  }

  if (required <= path->capacity) {
    return M3_OK;
  }

  if (path->capacity == 0) {
    return M3_ERR_STATE;
  }

  max_value = m3_path_max_value();
  if (path->capacity > max_value / 2) {
    if (path->capacity < required) {
      return M3_ERR_OVERFLOW;
    }
    new_capacity = path->capacity;
  } else {
    new_capacity = path->capacity * 2;
  }

  if (new_capacity < required) {
    new_capacity = required;
  }

  rc = m3_path_mul_overflow(new_capacity, (m3_usize)sizeof(M3PathCmd), &bytes);
  if (rc != M3_OK) {
    return rc;
  }

  rc =
      path->allocator.realloc(path->allocator.ctx, path->commands, bytes, &mem);
  if (rc != M3_OK) {
    return rc;
  }

  path->commands = (M3PathCmd *)mem;
  path->capacity = new_capacity;
  return M3_OK;
}

static int m3_path_append(M3Path *path, const M3PathCmd *cmd) {
  int rc;

  if (cmd == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_path_reserve(path, 1);
  if (rc != M3_OK) {
    return rc;
  }

  path->commands[path->count] = *cmd;
  path->count += 1;
  return M3_OK;
}

int M3_CALL m3_path_init(M3Path *path, const M3Allocator *allocator,
                         m3_usize initial_capacity) {
  M3Allocator chosen;
  m3_usize bytes;
  void *mem;
  int rc;

  if (path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (path->commands != NULL || path->capacity != 0 || path->count != 0) {
    return M3_ERR_STATE;
  }

  if (allocator == NULL) {
    rc = m3_get_default_allocator(&chosen);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    chosen = *allocator;
  }

  if (chosen.alloc == NULL || chosen.realloc == NULL || chosen.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (initial_capacity == 0) {
    initial_capacity = M3_PATH_DEFAULT_CAPACITY;
  }

  rc = m3_path_mul_overflow(initial_capacity, (m3_usize)sizeof(M3PathCmd),
                            &bytes);
  if (rc != M3_OK) {
    return rc;
  }

  rc = chosen.alloc(chosen.ctx, bytes, &mem);
  if (rc != M3_OK) {
    return rc;
  }

  path->allocator = chosen;
  path->commands = (M3PathCmd *)mem;
  path->count = 0;
  path->capacity = initial_capacity;
  return M3_OK;
}

int M3_CALL m3_path_reset(M3Path *path) {
  int rc;

  rc = m3_path_validate_initialized(path);
  if (rc != M3_OK) {
    return rc;
  }

  path->count = 0;
  return M3_OK;
}

int M3_CALL m3_path_shutdown(M3Path *path) {
  int rc;

  rc = m3_path_validate_initialized(path);
  if (rc != M3_OK) {
    return rc;
  }

  rc = path->allocator.free(path->allocator.ctx, path->commands);

  path->commands = NULL;
  path->count = 0;
  path->capacity = 0;
  path->allocator.ctx = NULL;
  path->allocator.alloc = NULL;
  path->allocator.realloc = NULL;
  path->allocator.free = NULL;

  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

int M3_CALL m3_path_move_to(M3Path *path, M3Scalar x, M3Scalar y) {
  M3PathCmd cmd;
  int rc;

  rc = m3_path_validate_initialized(path);
  if (rc != M3_OK) {
    return rc;
  }

  cmd.type = M3_PATH_CMD_MOVE_TO;
  cmd.data.move_to.x = x;
  cmd.data.move_to.y = y;
  return m3_path_append(path, &cmd);
}

int M3_CALL m3_path_line_to(M3Path *path, M3Scalar x, M3Scalar y) {
  M3PathCmd cmd;
  M3Bool has_current;
  int rc;

  rc = m3_path_has_current(path, &has_current);
  if (rc != M3_OK) {
    return rc;
  }
  if (!has_current) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_PATH_CMD_LINE_TO;
  cmd.data.line_to.x = x;
  cmd.data.line_to.y = y;
  return m3_path_append(path, &cmd);
}

int M3_CALL m3_path_quad_to(M3Path *path, M3Scalar cx, M3Scalar cy, M3Scalar x,
                            M3Scalar y) {
  M3PathCmd cmd;
  M3Bool has_current;
  int rc;

  rc = m3_path_has_current(path, &has_current);
  if (rc != M3_OK) {
    return rc;
  }
  if (!has_current) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_PATH_CMD_QUAD_TO;
  cmd.data.quad_to.cx = cx;
  cmd.data.quad_to.cy = cy;
  cmd.data.quad_to.x = x;
  cmd.data.quad_to.y = y;
  return m3_path_append(path, &cmd);
}

int M3_CALL m3_path_cubic_to(M3Path *path, M3Scalar cx1, M3Scalar cy1,
                             M3Scalar cx2, M3Scalar cy2, M3Scalar x,
                             M3Scalar y) {
  M3PathCmd cmd;
  M3Bool has_current;
  int rc;

  rc = m3_path_has_current(path, &has_current);
  if (rc != M3_OK) {
    return rc;
  }
  if (!has_current) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_PATH_CMD_CUBIC_TO;
  cmd.data.cubic_to.cx1 = cx1;
  cmd.data.cubic_to.cy1 = cy1;
  cmd.data.cubic_to.cx2 = cx2;
  cmd.data.cubic_to.cy2 = cy2;
  cmd.data.cubic_to.x = x;
  cmd.data.cubic_to.y = y;
  return m3_path_append(path, &cmd);
}

int M3_CALL m3_path_close(M3Path *path) {
  M3PathCmd cmd;
  M3Bool has_current;
  int rc;

  rc = m3_path_has_current(path, &has_current);
  if (rc != M3_OK) {
    return rc;
  }
  if (!has_current) {
    return M3_ERR_STATE;
  }

  cmd.type = M3_PATH_CMD_CLOSE;
  return m3_path_append(path, &cmd);
}

#ifdef M3_TESTING
int M3_CALL m3_path_test_add_overflow(m3_usize a, m3_usize b,
                                      m3_usize *out_value) {
  return m3_path_add_overflow(a, b, out_value);
}

int M3_CALL m3_path_test_mul_overflow(m3_usize a, m3_usize b,
                                      m3_usize *out_value) {
  return m3_path_mul_overflow(a, b, out_value);
}

int M3_CALL m3_path_test_has_current(const M3Path *path,
                                     M3Bool *out_has_current) {
  return m3_path_has_current(path, out_has_current);
}
#endif
