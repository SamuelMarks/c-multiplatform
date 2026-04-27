/* clang-format off */
#include "cmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_shader_cache_entry {
  char *key;
  cmp_shader_t *shader;
  struct cmp_shader_cache_entry *next;
};

struct cmp_shader_cache {
  struct cmp_shader_cache_entry *head;
};

/**
 * @brief Create a shader cache.
 *
 * @param out_cache Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_cache_create(cmp_shader_cache_t **out_cache) {
  int rc;
  cmp_shader_cache_t *cache;

  rc = CMP_SUCCESS;

  if (out_cache == NULL) {
    LOG_DEBUG("Invalid argument: out_cache is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_shader_cache_t), (void **)&cache);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  cache->head = NULL;
  *out_cache = cache;

  return CMP_SUCCESS;
}

/**
 * @brief Destroy a shader cache.
 *
 * @param cache Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_cache_destroy(cmp_shader_cache_t *cache) {
  int rc;
  struct cmp_shader_cache_entry *curr;
  struct cmp_shader_cache_entry *next;

  rc = CMP_SUCCESS;

  if (cache == NULL) {
    LOG_DEBUG("Invalid argument: cache is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  curr = cache->head;
  while (curr != NULL) {
    next = curr->next;
    if (curr->key != NULL) {
      rc = CMP_FREE(curr->key);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Free failed\n");
      }
    }
    if (curr->shader != NULL) {
      rc = cmp_shader_destroy(curr->shader);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_shader_destroy failed\n");
      }
      rc = CMP_FREE(curr->shader);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Free failed\n");
      }
    }
    rc = CMP_FREE(curr);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    curr = next;
  }

  rc = CMP_FREE(cache);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Store a shader in the cache.
 *
 * @param cache Parameter description.
 * @param key Parameter description.
 * @param shader Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_cache_store(cmp_shader_cache_t *cache, const char *key,
                           cmp_shader_t *shader) {
  int rc;
  struct cmp_shader_cache_entry *entry;
  size_t key_len;

  rc = CMP_SUCCESS;

  if (cache == NULL || key == NULL || shader == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_shader_cache_entry), (void **)&entry);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  key_len = strlen(key) + 1;
  rc = CMP_MALLOC(key_len, (void **)&entry->key);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    if (CMP_FREE(entry) != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  if (strcpy_s(entry->key, key_len, key) != 0) {
    LOG_DEBUG("strcpy_s failed\n");
    if (CMP_FREE(entry->key) != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    if (CMP_FREE(entry) != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(entry->key, key);
#endif

  rc = CMP_MALLOC(sizeof(cmp_shader_t), (void **)&entry->shader);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    if (CMP_FREE(entry->key) != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    if (CMP_FREE(entry) != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return CMP_ERROR_OOM;
  }

  /* Shallow copy, assuming the cache takes ownership of the shader */
  memcpy(entry->shader, shader, sizeof(cmp_shader_t));

  entry->next = cache->head;
  cache->head = entry;

  return CMP_SUCCESS;
}

/**
 * @brief Retrieve a shader from the cache.
 *
 * @param cache Parameter description.
 * @param key Parameter description.
 * @param out_shader Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_cache_retrieve(cmp_shader_cache_t *cache, const char *key,
                              cmp_shader_t **out_shader) {
  struct cmp_shader_cache_entry *curr;

  if (cache == NULL || key == NULL || out_shader == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  curr = cache->head;
  while (curr != NULL) {
    if (strcmp(curr->key, key) == 0) {
      *out_shader = curr->shader;
      return CMP_SUCCESS;
    }
    curr = curr->next;
  }

  return CMP_ERROR_NOT_FOUND;
}

/**
 * @brief Save the shader cache to disk.
 *
 * @param cache Parameter description.
 * @param filepath Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_cache_save_to_disk(cmp_shader_cache_t *cache,
                                  const char *filepath) {
  FILE *f;
  struct cmp_shader_cache_entry *curr;

  if (cache == NULL || filepath == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_MSC_VER)
  if (fopen_s(&f, filepath, "wb") != 0 || f == NULL) {
    LOG_DEBUG("Failed to open file for writing\n");
    return CMP_ERROR_IO;
  }
#else
  f = fopen(filepath, "wb");
  if (f == NULL) {
    LOG_DEBUG("Failed to open file for writing\n");
    return CMP_ERROR_IO;
  }
#endif

  curr = cache->head;
  while (curr != NULL) {
    uint32_t key_len = (uint32_t)strlen(curr->key);
    uint32_t shader_type = 0;

    if (curr->shader != NULL && curr->shader->internal_handle != NULL) {
      shader_type = (uint32_t)(*((int *)curr->shader->internal_handle));
    }

    if (fwrite(&key_len, sizeof(uint32_t), 1, f) != 1) {
      LOG_DEBUG("Failed to write key_len\n");
      fclose(f);
      return CMP_ERROR_IO;
    }
    if (fwrite(curr->key, 1, key_len, f) != key_len) {
      LOG_DEBUG("Failed to write key\n");
      fclose(f);
      return CMP_ERROR_IO;
    }
    if (fwrite(&shader_type, sizeof(uint32_t), 1, f) != 1) {
      LOG_DEBUG("Failed to write shader_type\n");
      fclose(f);
      return CMP_ERROR_IO;
    }
    curr = curr->next;
  }

  fclose(f);
  return CMP_SUCCESS;
}

/**
 * @brief Load the shader cache from disk.
 *
 * @param cache Parameter description.
 * @param filepath Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shader_cache_load_from_disk(cmp_shader_cache_t *cache,
                                    const char *filepath) {
  int rc;
  FILE *f;

  rc = CMP_SUCCESS;

  if (cache == NULL || filepath == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_MSC_VER)
  if (fopen_s(&f, filepath, "rb") != 0 || f == NULL) {
    LOG_DEBUG("Failed to open file for reading\n");
    return CMP_ERROR_IO;
  }
#else
  f = fopen(filepath, "rb");
  if (f == NULL) {
    LOG_DEBUG("Failed to open file for reading\n");
    return CMP_ERROR_IO;
  }
#endif

  while (!feof(f)) {
    uint32_t key_len = 0;
    uint32_t shader_type = 0;
    char *key_buf;
    cmp_shader_t shader;

    if (fread(&key_len, sizeof(uint32_t), 1, f) != 1) {
      break; /* End of file or read error */
    }

    rc = CMP_MALLOC(key_len + 1, (void **)&key_buf);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("OOM\n");
      fclose(f);
      return CMP_ERROR_OOM;
    }

    if (fread(key_buf, 1, key_len, f) != key_len) {
      LOG_DEBUG("Failed to read key\n");
      if (CMP_FREE(key_buf) != CMP_SUCCESS) {
        LOG_DEBUG("Free failed\n");
      }
      fclose(f);
      return CMP_ERROR_IO;
    }
    key_buf[key_len] = '\0';

    if (fread(&shader_type, sizeof(uint32_t), 1, f) != 1) {
      LOG_DEBUG("Failed to read shader_type\n");
      if (CMP_FREE(key_buf) != CMP_SUCCESS) {
        LOG_DEBUG("Free failed\n");
      }
      fclose(f);
      return CMP_ERROR_IO;
    }

    memset(&shader, 0, sizeof(cmp_shader_t));
    if (shader_type == 1) {
      rc = cmp_shader_init_rounded_rect(&shader);
    } else if (shader_type == 2) {
      rc = cmp_shader_init_gradient(&shader);
    } else if (shader_type == 3) {
      rc = cmp_shader_init_sdf_text(&shader);
    } else {
      rc = CMP_SUCCESS;
    }

    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Failed to initialize shader\n");
      if (CMP_FREE(key_buf) != CMP_SUCCESS) {
        LOG_DEBUG("Free failed\n");
      }
      fclose(f);
      return rc;
    }

    rc = cmp_shader_cache_store(cache, key_buf, &shader);
    if (CMP_FREE(key_buf) != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }

    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Failed to store shader\n");
      fclose(f);
      return rc;
    }
  }

  fclose(f);
  return CMP_SUCCESS;
}
