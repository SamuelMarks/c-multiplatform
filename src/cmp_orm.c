/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int g_orm_initialized = 0;

int cmp_orm_init(void) {
  if (g_orm_initialized) {
    return CMP_SUCCESS;
  }
  g_orm_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_orm_shutdown(void) {
  if (!g_orm_initialized) {
    return CMP_SUCCESS;
  }
  g_orm_initialized = 0;
  return CMP_SUCCESS;
}

int cmp_orm_connect(const char *virtual_path, c_orm_db_t **out_db) {
  cmp_string_t resolved_path;
  int err;

  if (virtual_path == NULL || out_db == NULL || !g_orm_initialized) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Automatically default plain filenames (e.g. "db.sqlite") to the executable
     directory (type 5) rather than relying strictly on the current working
     directory which can drift. */
  if (strncmp(virtual_path, "virt:/", 6) != 0 &&
      strchr(virtual_path, '/') == NULL && strchr(virtual_path, '\\') == NULL) {
    cmp_string_t exe_path;
    if (cmp_vfs_get_standard_path(5, &exe_path) == CMP_SUCCESS) {
      cmp_string_init(&resolved_path);
      cmp_string_append(&resolved_path, exe_path.data);
      cmp_string_append(&resolved_path, "/");
      cmp_string_append(&resolved_path, virtual_path);
      cmp_string_destroy(&exe_path);
    } else {
      if (cmp_vfs_resolve_path(virtual_path, &resolved_path) != CMP_SUCCESS) {
        return CMP_ERROR_INVALID_ARG;
      }
    }
  } else {
    if (cmp_vfs_resolve_path(virtual_path, &resolved_path) != CMP_SUCCESS) {
      return CMP_ERROR_INVALID_ARG;
    }
  }

#if defined(__EMSCRIPTEN__)
  /* Shim for WebAssembly: Append IndexedDB VFS parameters if requested or
     intercept connection to use browser-native shim */
  {
    /* If the URL doesn't already contain a ? we format it to force IndexedDB */
    if (strchr(resolved_path.data, '?') == NULL) {
      cmp_string_append(&resolved_path,
                        "?vfs=opfs"); /* Modern alternative to indexeddb */
    }
  }
#endif

  err = c_orm_sqlite_connect(resolved_path.data, out_db);

  cmp_string_destroy(&resolved_path);

  if (err != C_ORM_OK) {
    return CMP_ERROR_NOT_FOUND;
  }

  return CMP_SUCCESS;
}

int cmp_orm_disconnect(c_orm_db_t *db) {
  const c_orm_driver_vtable_t *vtable;

  if (db == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (c_orm_sqlite_get_vtable(&vtable) != C_ORM_OK) {
    return CMP_ERROR_NOT_FOUND;
  }

  if (vtable->disconnect(db) != C_ORM_OK) {
    return CMP_ERROR_NOT_FOUND;
  }

  return CMP_SUCCESS;
}

int cmp_orm_execute(c_orm_db_t *db, const char *sql) {
  if (db == NULL || sql == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (c_orm_execute_raw(db, sql) != C_ORM_OK) {
    return CMP_ERROR_NOT_FOUND;
  }

  return CMP_SUCCESS;
}

int cmp_orm_migrate(c_orm_db_t *db, const char *migrations_dir) {
  /* Stub: In a real implementation this would scan the directory
     for .sql files and execute them sequentially tracking version state */
  if (db == NULL || migrations_dir == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

int cmp_orm_set_encryption_key(c_orm_db_t *db, const char *key) {
  /* Stub: In a real implementation this would bridge to PRAGMA key
     when linked against SQLCipher */
  if (db == NULL || key == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

struct cmp_orm_observable {
  c_orm_db_t *db;
  char *query;
  cmp_ui_node_t *bound_node;
  char *bound_property;
};

int cmp_orm_observable_create(c_orm_db_t *db, const char *query,
                              cmp_orm_observable_t **out_obs) {
  cmp_orm_observable_t *obs;
  size_t len;

  if (db == NULL || query == NULL || out_obs == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_orm_observable_t), (void **)&obs) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  len = strlen(query);
  if (CMP_MALLOC(len + 1, (void **)&obs->query) != CMP_SUCCESS) {
    CMP_FREE(obs);
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  strcpy_s(obs->query, len + 1, query);
#else
  strcpy(obs->query, query);
#endif

  obs->db = db;
  obs->bound_node = NULL;
  obs->bound_property = NULL;

  *out_obs = obs;
  return CMP_SUCCESS;
}

int cmp_ui_node_bind(cmp_ui_node_t *node, cmp_orm_observable_t *obs,
                     const char *property_name) {
  size_t len;

  if (node == NULL || obs == NULL || property_name == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  obs->bound_node = node;

  if (obs->bound_property != NULL) {
    CMP_FREE(obs->bound_property);
  }

  len = strlen(property_name);
  if (CMP_MALLOC(len + 1, (void **)&obs->bound_property) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  strcpy_s(obs->bound_property, len + 1, property_name);
#else
  strcpy(obs->bound_property, property_name);
#endif

  return CMP_SUCCESS;
}

int cmp_orm_observable_destroy(cmp_orm_observable_t *obs) {
  if (obs == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (obs->query != NULL) {
    CMP_FREE(obs->query);
  }

  if (obs->bound_property != NULL) {
    CMP_FREE(obs->bound_property);
  }

  CMP_FREE(obs);
  return CMP_SUCCESS;
}