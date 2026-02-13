#include "cmpc/cmp_path.h"

#define CMP_PATH_DEFAULT_CAPACITY 16u

#ifdef CMP_TESTING
static CMPBool g_cmp_path_test_force_reserve = CMP_FALSE;

int CMP_CALL cmp_path_test_set_force_reserve(CMPBool enable) {
  g_cmp_path_test_force_reserve = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}
#endif

static cmp_usize cmp_path_max_value(void) { return (cmp_usize) ~(cmp_usize)0; }

static int cmp_path_add_overflow(cmp_usize a, cmp_usize b, cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = cmp_path_max_value();
  if (a > max_value - b) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a + b;
  return CMP_OK;
}

static int cmp_path_mul_overflow(cmp_usize a, cmp_usize b, cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = cmp_path_max_value();
  if (a != 0 && b > max_value / a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return CMP_OK;
}

static int cmp_path_validate_initialized(const CMPPath *path) {
  if (path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (path->commands == NULL) {
    return CMP_ERR_STATE;
  }
  if (path->count > path->capacity) {
    return CMP_ERR_STATE;
  }
  return CMP_OK;
}

static int cmp_path_has_current(const CMPPath *path, CMPBool *out_has_current) {
  int rc;

  if (out_has_current == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_path_validate_initialized(path);
  if (rc != CMP_OK) {
    return rc;
  }

  if (path->count == 0) {
    *out_has_current = CMP_FALSE;
    return CMP_OK;
  }

  *out_has_current = (path->commands[path->count - 1].type != CMP_PATH_CMD_CLOSE)
                         ? CMP_TRUE
                         : CMP_FALSE;
  return CMP_OK;
}

static int cmp_path_reserve(CMPPath *path, cmp_usize additional) {
  cmp_usize required;
  cmp_usize new_capacity;
  cmp_usize bytes;
  cmp_usize max_value;
  void *mem;
  int rc;

  rc = cmp_path_validate_initialized(path);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_path_add_overflow(path->count, additional, &required);
  if (rc != CMP_OK) {
    return rc;
  }

#ifdef CMP_TESTING
  if (g_cmp_path_test_force_reserve != CMP_TRUE) {
#endif
    if (required <= path->capacity) {
      return CMP_OK;
    }
#ifdef CMP_TESTING
  }
#endif

  if (path->capacity == 0) {
    return CMP_ERR_STATE;
  }

  max_value = cmp_path_max_value();
  if (path->capacity > max_value / 2) {
    if (path->capacity < required) {
      return CMP_ERR_OVERFLOW;
    }
    new_capacity = path->capacity;
  } else {
    new_capacity = path->capacity * 2;
  }

  if (new_capacity < required) {
    new_capacity = required;
  }

  rc = cmp_path_mul_overflow(new_capacity, (cmp_usize)sizeof(CMPPathCmd), &bytes);
  if (rc != CMP_OK) {
    return rc;
  }

  rc =
      path->allocator.realloc(path->allocator.ctx, path->commands, bytes, &mem);
  if (rc != CMP_OK) {
    return rc;
  }

  path->commands = (CMPPathCmd *)mem;
  path->capacity = new_capacity;
  return CMP_OK;
}

static int cmp_path_append(CMPPath *path, const CMPPathCmd *cmd) {
  int rc;

  if (cmd == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_path_reserve(path, 1);
  if (rc != CMP_OK) {
    return rc;
  }

  path->commands[path->count] = *cmd;
  path->count += 1;
  return CMP_OK;
}

int CMP_CALL cmp_path_init(CMPPath *path, const CMPAllocator *allocator,
                         cmp_usize initial_capacity) {
  CMPAllocator chosen;
  cmp_usize bytes;
  void *mem;
  int rc;

  if (path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (path->commands != NULL || path->capacity != 0 || path->count != 0) {
    return CMP_ERR_STATE;
  }

  if (allocator == NULL) {
    rc = cmp_get_default_allocator(&chosen);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    chosen = *allocator;
  }

  if (chosen.alloc == NULL || chosen.realloc == NULL || chosen.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (initial_capacity == 0) {
    initial_capacity = CMP_PATH_DEFAULT_CAPACITY;
  }

  rc = cmp_path_mul_overflow(initial_capacity, (cmp_usize)sizeof(CMPPathCmd),
                            &bytes);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = chosen.alloc(chosen.ctx, bytes, &mem);
  if (rc != CMP_OK) {
    return rc;
  }

  path->allocator = chosen;
  path->commands = (CMPPathCmd *)mem;
  path->count = 0;
  path->capacity = initial_capacity;
  return CMP_OK;
}

int CMP_CALL cmp_path_reset(CMPPath *path) {
  int rc;

  rc = cmp_path_validate_initialized(path);
  if (rc != CMP_OK) {
    return rc;
  }

  path->count = 0;
  return CMP_OK;
}

int CMP_CALL cmp_path_shutdown(CMPPath *path) {
  int rc;

  rc = cmp_path_validate_initialized(path);
  if (rc != CMP_OK) {
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

  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

int CMP_CALL cmp_path_move_to(CMPPath *path, CMPScalar x, CMPScalar y) {
  CMPPathCmd cmd;
  int rc;

  rc = cmp_path_validate_initialized(path);
  if (rc != CMP_OK) {
    return rc;
  }

  cmd.type = CMP_PATH_CMD_MOVE_TO;
  cmd.data.move_to.x = x;
  cmd.data.move_to.y = y;
  return cmp_path_append(path, &cmd);
}

int CMP_CALL cmp_path_line_to(CMPPath *path, CMPScalar x, CMPScalar y) {
  CMPPathCmd cmd;
  CMPBool has_current;
  int rc;

  rc = cmp_path_has_current(path, &has_current);
  if (rc != CMP_OK) {
    return rc;
  }
  if (!has_current) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_PATH_CMD_LINE_TO;
  cmd.data.line_to.x = x;
  cmd.data.line_to.y = y;
  return cmp_path_append(path, &cmd);
}

int CMP_CALL cmp_path_quad_to(CMPPath *path, CMPScalar cx, CMPScalar cy, CMPScalar x,
                            CMPScalar y) {
  CMPPathCmd cmd;
  CMPBool has_current;
  int rc;

  rc = cmp_path_has_current(path, &has_current);
  if (rc != CMP_OK) {
    return rc;
  }
  if (!has_current) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_PATH_CMD_QUAD_TO;
  cmd.data.quad_to.cx = cx;
  cmd.data.quad_to.cy = cy;
  cmd.data.quad_to.x = x;
  cmd.data.quad_to.y = y;
  return cmp_path_append(path, &cmd);
}

int CMP_CALL cmp_path_cubic_to(CMPPath *path, CMPScalar cx1, CMPScalar cy1,
                             CMPScalar cx2, CMPScalar cy2, CMPScalar x,
                             CMPScalar y) {
  CMPPathCmd cmd;
  CMPBool has_current;
  int rc;

  rc = cmp_path_has_current(path, &has_current);
  if (rc != CMP_OK) {
    return rc;
  }
  if (!has_current) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_PATH_CMD_CUBIC_TO;
  cmd.data.cubic_to.cx1 = cx1;
  cmd.data.cubic_to.cy1 = cy1;
  cmd.data.cubic_to.cx2 = cx2;
  cmd.data.cubic_to.cy2 = cy2;
  cmd.data.cubic_to.x = x;
  cmd.data.cubic_to.y = y;
  return cmp_path_append(path, &cmd);
}

int CMP_CALL cmp_path_close(CMPPath *path) {
  CMPPathCmd cmd;
  CMPBool has_current;
  int rc;

  rc = cmp_path_has_current(path, &has_current);
  if (rc != CMP_OK) {
    return rc;
  }
  if (!has_current) {
    return CMP_ERR_STATE;
  }

  cmd.type = CMP_PATH_CMD_CLOSE;
  return cmp_path_append(path, &cmd);
}

#ifdef CMP_TESTING
int CMP_CALL cmp_path_test_add_overflow(cmp_usize a, cmp_usize b,
                                      cmp_usize *out_value) {
  return cmp_path_add_overflow(a, b, out_value);
}

int CMP_CALL cmp_path_test_mul_overflow(cmp_usize a, cmp_usize b,
                                      cmp_usize *out_value) {
  return cmp_path_mul_overflow(a, b, out_value);
}

int CMP_CALL cmp_path_test_has_current(const CMPPath *path,
                                     CMPBool *out_has_current) {
  return cmp_path_has_current(path, out_has_current);
}

int CMP_CALL cmp_path_test_reserve(CMPPath *path, cmp_usize additional) {
  return cmp_path_reserve(path, additional);
}

int CMP_CALL cmp_path_test_append(CMPPath *path, const CMPPathCmd *cmd) {
  return cmp_path_append(path, cmd);
}
#endif
