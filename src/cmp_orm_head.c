/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cfs/cfs.h>
/* clang-format on */

static int g_orm_initialized = 0;

/**
 * @brief Initialize ORM subsystem.
 *
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_orm_init(void) {
  if (g_orm_initialized) {
    return CMP_SUCCESS;
  }
  g_orm_initialized = 1;
  return CMP_SUCCESS;
}

/**
 * @brief Shutdown ORM subsystem.
 *
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_orm_shutdown(void) {
  if (!g_orm_initialized) {
    return CMP_SUCCESS;
  }
  g_orm_initialized = 0;
  return CMP_SUCCESS;
}

/**
 * @brief Open a database connection mapped via the VFS.
 *
 * @param virtual_path Path to the database file.
 * @param out_db Pointer to store the database handle.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_orm_connect(const char *virtual_path, c_orm_db_t **out_db) {
  cmp_string_t resolved_path;
  int err;
  int rc;
  cmp_string_t exe_path;

  rc = CMP_SUCCESS;

  if (virtual_path == NULL || out_db == NULL || !g_orm_initialized) {
    LOG_DEBUG("Error in cmp_orm_connect: Invalid argument or uninitialized\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Automatically default plain filenames (e.g. "db.sqlite") to the executable
     directory (type 5) rather than relying strictly on the current working
     directory which can drift. */
  if (strncmp(virtual_path, "virt:/", 6) != 0 &&
      strchr(virtual_path, '/') == NULL && strchr(virtual_path, '\\') == NULL) {
    if (cmp_vfs_get_standard_path(5, &exe_path) == CMP_SUCCESS) {
      cmp_string_init(&resolved_path);
      cmp_string_append(&resolved_path, exe_path.data);
      cmp_string_append(&resolved_path, "/");
      cmp_string_append(&resolved_path, virtual_path);
      cmp_string_destroy(&exe_path);
    } else {
      if (cmp_vfs_resolve_path(virtual_path, &resolved_path) != CMP_SUCCESS) {
        LOG_DEBUG("Error in cmp_orm_connect: cmp_vfs_resolve_path failed\n");
        return CMP_ERROR_INVALID_ARG;
      }
    }
  } else {
    if (cmp_vfs_resolve_path(virtual_path, &resolved_path) != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_orm_connect: cmp_vfs_resolve_path failed\n");
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
    LOG_DEBUG("Error in cmp_orm_connect: c_orm_sqlite_connect failed\n");
    return CMP_ERROR_NOT_FOUND;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Close a database connection.
 *
 * @param db Database handle to close.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_orm_disconnect(c_orm_db_t *db) {
  const c_orm_driver_vtable_t *vtable;

  if (db == NULL) {
    LOG_DEBUG("Error in cmp_orm_disconnect: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (c_orm_sqlite_get_vtable(&vtable) != C_ORM_OK) {
    LOG_DEBUG("Error in cmp_orm_disconnect: c_orm_sqlite_get_vtable failed\n");
    return CMP_ERROR_NOT_FOUND;
  }

  if (vtable->disconnect(db) != C_ORM_OK) {
    LOG_DEBUG("Error in cmp_orm_disconnect: vtable->disconnect failed\n");
    return CMP_ERROR_NOT_FOUND;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Execute a raw SQL statement that modifies state.
 *
 * @param db Database connection.
 * @param sql Raw SQL statement.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_orm_execute(c_orm_db_t *db, const char *sql) {
  if (db == NULL || sql == NULL) {
    LOG_DEBUG("Error in cmp_orm_execute: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (c_orm_execute_raw(db, sql) != C_ORM_OK) {
    LOG_DEBUG("Error in cmp_orm_execute: c_orm_execute_raw failed\n");
    return CMP_ERROR_NOT_FOUND;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Run pending database migrations automatically.
 *
 * @param db Database handle.
 * @param migrations_dir Virtual path to the directory containing .sql migration
 * scripts.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_orm_migrate(c_orm_db_t *db, const char *migrations_dir) {
  cmp_string_t resolved_path;
  cfs_path cfs_dir_path;
  cfs_directory_iterator *it;
  const cfs_directory_entry *entry;
  cfs_error_code ec;

  it = NULL;
  entry = NULL;

  if (db == NULL || migrations_dir == NULL) {
    LOG_DEBUG("Error in cmp_orm_migrate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (cmp_vfs_resolve_path(migrations_dir, &resolved_path) != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_orm_migrate: cmp_vfs_resolve_path failed\n");
    return CMP_ERROR_NOT_FOUND;
  }

  cfs_path_init(&cfs_dir_path);
  cfs_path_assign(&cfs_dir_path, (const cfs_char_t *)resolved_path.data);

  if (cfs_dir_itr_init(&cfs_dir_path, &it, &ec) != 0 || !it) {
    LOG_DEBUG("Error in cmp_orm_migrate: cfs_dir_itr_init failed\n");
    cfs_path_clear(&cfs_dir_path);
    cmp_string_destroy(&resolved_path);
    return CMP_SUCCESS;
  }

  /* Note: A production implementation would sort by version number.
     This implementation iterates in filesystem order. */
  while (cfs_dir_itr_next(it, &entry, &ec) == 0 && entry) {
    {
      char aname[1024];
      size_t len;
      size_t i;
      int rc;
#if defined(CFS_UNICODE)
      const wchar_t *wname;

      wname = (const wchar_t *)entry->path.str;
      i = 0;
      while (wname[i] && i < 1023) {
        aname[i] = (char)wname[i];
        i++;
      }
      aname[i] = '\0';
#else
      const char *cname;

      cname = (const char *)entry->path.str;
#if defined(_MSC_VER)
      rc = strcpy_s(aname, sizeof(aname), cname);
      if (rc != 0) {
        LOG_DEBUG("Error in cmp_orm_migrate: strcpy_s failed\n");
        continue;
      }
#else
      strncpy(aname, cname, sizeof(aname) - 1);
      aname[sizeof(aname) - 1] = '\0';
#endif
#endif
      len = strlen(aname);
      if (len > 4 && strcmp(aname + len - 4, ".sql") == 0) {

        size_t file_len;

        {
          FILE *f;
#if defined(_MSC_VER)
          rc = fopen_s(&f, aname, "rb");
          if (rc != 0) {
            f = NULL;
          }
#else
          f = fopen(aname, "rb");
#endif
          if (f) {
            char *sql_str;
            size_t bytes_read;
            fseek(f, 0, SEEK_END);
            file_len = ftell(f);
            fseek(f, 0, SEEK_SET);

            if (CMP_MALLOC(file_len + 1, (void **)&sql_str) == CMP_SUCCESS) {
              bytes_read = fread(sql_str, 1, file_len, f);
              sql_str[bytes_read] = '\0';
              c_orm_execute_raw(db, sql_str);
              rc = CMP_FREE(sql_str);
              if (rc != CMP_SUCCESS) {
                LOG_DEBUG("Error in cmp_orm_migrate: CMP_FREE failed\n");
              }
            }
            fclose(f);
          }
        }
      }
    }
  }
  cfs_dir_itr_close(it);
  cfs_path_clear(&cfs_dir_path);
  cmp_string_destroy(&resolved_path);
  return CMP_SUCCESS;
}

/**
 * @brief Configure the encryption key for the database connection.
 *
 * @param db Database handle.
 * @param key Encryption key string.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_orm_set_encryption_key(c_orm_db_t *db, const char *key) {
  char query[256];
#if defined(_MSC_VER)
  int rc;
#endif

  if (db == NULL || key == NULL) {
    LOG_DEBUG("Error in cmp_orm_set_encryption_key: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(_MSC_VER)
  rc = sprintf_s(query, sizeof(query), "PRAGMA key = '%s';", key);
  if (rc < 0) {
    LOG_DEBUG("Error in cmp_orm_set_encryption_key: sprintf_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  snprintf(query, sizeof(query), "PRAGMA key = '%s';", key);
#endif
  if (c_orm_execute_raw(db, query) != C_ORM_OK) {
    LOG_DEBUG(
        "Error in cmp_orm_set_encryption_key: c_orm_execute_raw failed\n");
    return CMP_ERROR_GENERAL;
  }
  return CMP_SUCCESS;
}

struct cmp_orm_observable {
  c_orm_db_t *db;
  char *query;
  cmp_ui_node_t *bound_node;
  char *bound_property;
};

/**
 * @brief Create an observable data stream from an SQL query.
 *
 * @param db Database handle.
 * @param query SELECT query string.
 * @param out_obs Pointer to receive the observable instance.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_orm_observable_create(c_orm_db_t *db, const char *query,
                              cmp_orm_observable_t **out_obs) {
  cmp_orm_observable_t *obs;
  size_t len;
  int rc;

  if (db == NULL || query == NULL || out_obs == NULL) {
    LOG_DEBUG("Error in cmp_orm_observable_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_orm_observable_t), (void **)&obs);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_orm_observable_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  len = strlen(query);
  rc = CMP_MALLOC(len + 1, (void **)&obs->query);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_orm_observable_create: CMP_MALLOC failed for query "
              "(OOM)\n");
    rc = CMP_FREE(obs);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_orm_observable_create: CMP_FREE failed during "
                "cleanup\n");
    }
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  rc = strcpy_s(obs->query, len + 1, query);
  if (rc != 0) {
    LOG_DEBUG("Error in cmp_orm_observable_create: strcpy_s failed\n");
    rc = CMP_FREE(obs->query);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_orm_observable_create: CMP_FREE failed during "
                "cleanup\n");
    }
    rc = CMP_FREE(obs);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_orm_observable_create: CMP_FREE failed during "
                "cleanup\n");
    }
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(obs->query, query);
#endif

  obs->db = db;
  obs->bound_node = NULL;
  obs->bound_property = NULL;

  *out_obs = obs;
  return CMP_SUCCESS;
}

/**
 * @brief Bind an observable database query to a UI node's property.
 *
 * @param node UI node to bind to.
 * @param obs Observable database query.
 * @param property_name Name of the node property to update (e.g. "text").
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_ui_node_bind(cmp_ui_node_t *node, cmp_orm_observable_t *obs,
                     const char *property_name) {
  size_t len;
  int rc;

  if (node == NULL || obs == NULL || property_name == NULL) {
    LOG_DEBUG("Error in cmp_ui_node_bind: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  obs->bound_node = node;

  if (obs->bound_property != NULL) {
    rc = CMP_FREE(obs->bound_property);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_ui_node_bind: CMP_FREE failed for bound_property\n");
    }
    obs->bound_property = NULL;
  }

  len = strlen(property_name);
  rc = CMP_MALLOC(len + 1, (void **)&obs->bound_property);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_ui_node_bind: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  rc = strcpy_s(obs->bound_property, len + 1, property_name);
  if (rc != 0) {
    LOG_DEBUG("Error in cmp_ui_node_bind: strcpy_s failed\n");
    rc = CMP_FREE(obs->bound_property);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_ui_node_bind: CMP_FREE failed during cleanup\n");
    }
    obs->bound_property = NULL;
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(obs->bound_property, property_name);
#endif

  return CMP_SUCCESS;
}

/**
 * @brief Destroy an observable database query instance.
 *
 * @param obs The observable query to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_orm_observable_destroy(cmp_orm_observable_t *obs) {
  int rc;

  if (obs == NULL) {
    LOG_DEBUG("Error in cmp_orm_observable_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (obs->query != NULL) {
    rc = CMP_FREE(obs->query);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_orm_observable_destroy: CMP_FREE failed for query\n");
    }
  }

  if (obs->bound_property != NULL) {
    rc = CMP_FREE(obs->bound_property);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_orm_observable_destroy: CMP_FREE failed for "
                "bound_property\n");
    }
  }

  rc = CMP_FREE(obs);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_orm_observable_destroy: CMP_FREE failed for obs\n");
    return rc;
  }
  return CMP_SUCCESS;
}
