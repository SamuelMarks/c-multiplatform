/**
 * @file sampler_preferences.c
 * @brief Implementation of user preferences storage for Compose Material
 * Catalog.
 */

/* clang-format off */
#include "sampler/sampler_preferences.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct sampler_preferences {
  char *file_path;
  char favorite_route[256];
  int has_favorite_route;
  struct sampler_theme theme;
  int has_theme;
};

static sampler_error_t sampler_preferences_write_file_internal(
    const struct sampler_preferences *prefs) {
  FILE *fp;
  char theme_buf[256];
  size_t written;
  sampler_error_t rc;

  if (prefs == NULL || prefs->file_path == NULL) {
    return SAMPLER_SUCCESS;
  }

#if defined(_MSC_VER)
  if (fopen_s(&fp, prefs->file_path, "w") != 0 || fp == NULL) {
    return SAMPLER_ERROR_STORAGE_FAILURE;
  }
#else
  fp = fopen(prefs->file_path, "w");
  if (fp == NULL) {
    return SAMPLER_ERROR_STORAGE_FAILURE;
  }
#endif

  if (prefs->has_favorite_route) {
    fprintf(fp, "favorite_route=%s\x0a", prefs->favorite_route);
  }

  if (prefs->has_theme) {
    rc = sampler_theme_serialize(&prefs->theme, theme_buf, sizeof(theme_buf),
                                 &written);
    if (rc != SAMPLER_SUCCESS) {
      fclose(fp);
      return rc;
    }
    fprintf(fp, "theme=%s\x0a", theme_buf);
  }

  fclose(fp);
  return SAMPLER_SUCCESS;
}

static sampler_error_t
sampler_preferences_read_file_internal(struct sampler_preferences *prefs) {
  FILE *fp;
  char line[512];

  if (prefs == NULL || prefs->file_path == NULL) {
    return SAMPLER_SUCCESS;
  }

#if defined(_MSC_VER)
  if (fopen_s(&fp, prefs->file_path, "r") != 0 || fp == NULL) {
    return SAMPLER_SUCCESS; /* file doesn't exist yet, not fatal */
  }
#else
  fp = fopen(prefs->file_path, "r");
  if (fp == NULL) {
    return SAMPLER_SUCCESS;
  }
#endif

  while (fgets(line, (int)sizeof(line), fp) != NULL) {
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\x0d' || line[len - 1] == '\x0a')) {
      line[len - 1] = '\0';
      len--;
    }

    if (strncmp(line, "favorite_route=", 15) == 0) {
#if defined(_MSC_VER)
      strncpy_s(prefs->favorite_route, sizeof(prefs->favorite_route), line + 15,
                _TRUNCATE);
#else
      strncpy(prefs->favorite_route, line + 15,
              sizeof(prefs->favorite_route) - 1);
      prefs->favorite_route[sizeof(prefs->favorite_route) - 1] = '\0';
#endif
      prefs->has_favorite_route = 1;
    } else if (strncmp(line, "theme=", 6) == 0) {
      sampler_error_t rc = sampler_theme_deserialize(line + 6, &prefs->theme);
      if (rc == SAMPLER_SUCCESS) {
        prefs->has_theme = 1;
      }
    }
  }

  fclose(fp);
  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_preferences_create_in_memory(struct sampler_preferences **out_prefs) {
  struct sampler_preferences *prefs;
  sampler_error_t rc;

  if (out_prefs == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  prefs = (struct sampler_preferences *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct sampler_preferences));
  if (prefs == NULL) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  memset(prefs, 0, sizeof(*prefs));
  rc = sampler_theme_init_default(&prefs->theme);
  if (rc != SAMPLER_SUCCESS) {
    C_MULTIPLATFORM_FREE(prefs);
    return rc;
  }

  *out_prefs = prefs;
  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_preferences_create_file(const char *file_path,
                                struct sampler_preferences **out_prefs) {
  struct sampler_preferences *prefs;
  sampler_error_t rc;
  size_t path_len;

  if (file_path == NULL || out_prefs == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  rc = sampler_preferences_create_in_memory(&prefs);
  if (rc != SAMPLER_SUCCESS) {
    return rc;
  }

  path_len = strlen(file_path);
  prefs->file_path = (char *)C_MULTIPLATFORM_MALLOC(path_len + 1);
  if (prefs->file_path == NULL) {
    sampler_preferences_destroy(&prefs);
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  strcpy_s(prefs->file_path, path_len + 1, file_path);
#else
  strcpy(prefs->file_path, file_path);
#endif

  rc = sampler_preferences_read_file_internal(prefs);
  if (rc != SAMPLER_SUCCESS) {
    sampler_preferences_destroy(&prefs);
    return rc;
  }

  *out_prefs = prefs;
  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_preferences_destroy(struct sampler_preferences **prefs) {
  if (prefs == NULL || *prefs == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  if ((*prefs)->file_path != NULL) {
    C_MULTIPLATFORM_FREE((*prefs)->file_path);
    (*prefs)->file_path = NULL;
  }

  C_MULTIPLATFORM_FREE(*prefs);
  *prefs = NULL;
  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_preferences_save_favorite_route(struct sampler_preferences *prefs,
                                        const char *route) {
  sampler_error_t rc;

  if (prefs == NULL || route == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

#if defined(_MSC_VER)
  strncpy_s(prefs->favorite_route, sizeof(prefs->favorite_route), route,
            _TRUNCATE);
#else
  strncpy(prefs->favorite_route, route, sizeof(prefs->favorite_route) - 1);
  prefs->favorite_route[sizeof(prefs->favorite_route) - 1] = '\0';
#endif
  prefs->has_favorite_route = 1;

  rc = sampler_preferences_write_file_internal(prefs);
  if (rc != SAMPLER_SUCCESS) {
    return rc;
  }

  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_preferences_get_favorite_route(const struct sampler_preferences *prefs,
                                       char *out_route, size_t max_len,
                                       int *out_has_route) {
  if (prefs == NULL || out_route == NULL || out_has_route == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }
  if (max_len == 0) {
    return SAMPLER_ERROR_OUT_OF_BOUNDS;
  }

  if (prefs->has_favorite_route) {
#if defined(_MSC_VER)
    strncpy_s(out_route, max_len, prefs->favorite_route, _TRUNCATE);
#else
    strncpy(out_route, prefs->favorite_route, max_len - 1);
    out_route[max_len - 1] = '\0';
#endif
    *out_has_route = 1;
  } else {
    out_route[0] = '\0';
    *out_has_route = 0;
  }

  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_preferences_clear_favorite_route(struct sampler_preferences *prefs) {
  if (prefs == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  prefs->favorite_route[0] = '\0';
  prefs->has_favorite_route = 0;

  return sampler_preferences_write_file_internal(prefs);
}

sampler_error_t
sampler_preferences_save_theme(struct sampler_preferences *prefs,
                               const struct sampler_theme *theme) {
  sampler_error_t rc;

  if (prefs == NULL || theme == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  rc = sampler_theme_copy(theme, &prefs->theme);
  if (rc != SAMPLER_SUCCESS) {
    return rc;
  }
  prefs->has_theme = 1;

  rc = sampler_preferences_write_file_internal(prefs);
  if (rc != SAMPLER_SUCCESS) {
    return rc;
  }

  return SAMPLER_SUCCESS;
}

sampler_error_t
sampler_preferences_get_theme(const struct sampler_preferences *prefs,
                              struct sampler_theme *out_theme,
                              int *out_has_theme) {
  sampler_error_t rc;

  if (prefs == NULL || out_theme == NULL || out_has_theme == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  if (prefs->has_theme) {
    rc = sampler_theme_copy(&prefs->theme, out_theme);
    if (rc != SAMPLER_SUCCESS) {
      return rc;
    }
    *out_has_theme = 1;
  } else {
    rc = sampler_theme_init_default(out_theme);
    if (rc != SAMPLER_SUCCESS) {
      return rc;
    }
    *out_has_theme = 0;
  }

  return SAMPLER_SUCCESS;
}
