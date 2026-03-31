/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_shader_cache_entry {
  char *key;
  cmp_shader_t *shader;
  struct cmp_shader_cache_entry *next;
};

struct cmp_shader_cache {
  struct cmp_shader_cache_entry *head;
};

int cmp_shader_cache_create(cmp_shader_cache_t **out_cache) {
  cmp_shader_cache_t *cache;
  if (!out_cache)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(cmp_shader_cache_t), (void **)&cache) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  cache->head = NULL;
  *out_cache = cache;
  return CMP_SUCCESS;
}

int cmp_shader_cache_destroy(cmp_shader_cache_t *cache) {
  struct cmp_shader_cache_entry *curr, *next;
  if (!cache)
    return CMP_ERROR_INVALID_ARG;

  curr = cache->head;
  while (curr) {
    next = curr->next;
    if (curr->key)
      CMP_FREE(curr->key);
    if (curr->shader) {
      cmp_shader_destroy(curr->shader);
      CMP_FREE(curr->shader);
    }
    CMP_FREE(curr);
    curr = next;
  }
  CMP_FREE(cache);
  return CMP_SUCCESS;
}

int cmp_shader_cache_store(cmp_shader_cache_t *cache, const char *key,
                           cmp_shader_t *shader) {
  struct cmp_shader_cache_entry *entry;
  size_t key_len;
  if (!cache || !key || !shader)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_shader_cache_entry), (void **)&entry) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  key_len = strlen(key) + 1;
  if (CMP_MALLOC(key_len, (void **)&entry->key) != CMP_SUCCESS) {
    CMP_FREE(entry);
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(entry->key, key_len, key);
#else
  strcpy(entry->key, key);
#endif

  if (CMP_MALLOC(sizeof(cmp_shader_t), (void **)&entry->shader) !=
      CMP_SUCCESS) {
    CMP_FREE(entry->key);
    CMP_FREE(entry);
    return CMP_ERROR_OOM;
  }
  /* Shallow copy, assuming the cache takes ownership of the shader */
  memcpy(entry->shader, shader, sizeof(cmp_shader_t));

  entry->next = cache->head;
  cache->head = entry;

  return CMP_SUCCESS;
}

int cmp_shader_cache_retrieve(cmp_shader_cache_t *cache, const char *key,
                              cmp_shader_t **out_shader) {
  struct cmp_shader_cache_entry *curr;
  if (!cache || !key || !out_shader)
    return CMP_ERROR_INVALID_ARG;

  curr = cache->head;
  while (curr) {
    if (strcmp(curr->key, key) == 0) {
      *out_shader = curr->shader;
      return CMP_SUCCESS;
    }
    curr = curr->next;
  }
  return CMP_ERROR_NOT_FOUND;
}

int cmp_shader_cache_save_to_disk(cmp_shader_cache_t *cache,
                                  const char *filepath) {
  FILE *f;
  struct cmp_shader_cache_entry *curr;
  if (!cache || !filepath)
    return CMP_ERROR_INVALID_ARG;

#if defined(_MSC_VER)
  if (fopen_s(&f, filepath, "wb") != 0 || !f)
    return CMP_ERROR_IO;
#else
  f = fopen(filepath, "wb");
  if (!f)
    return CMP_ERROR_IO;
#endif

  curr = cache->head;
  while (curr) {
    uint32_t key_len = (uint32_t)strlen(curr->key);
    uint32_t shader_type = 0;

    if (curr->shader && curr->shader->internal_handle) {
      shader_type = (uint32_t)(*((int *)curr->shader->internal_handle));
    }

    if (fwrite(&key_len, sizeof(uint32_t), 1, f) != 1) {
      fclose(f);
      return CMP_ERROR_IO;
    }
    if (fwrite(curr->key, 1, key_len, f) != key_len) {
      fclose(f);
      return CMP_ERROR_IO;
    }
    if (fwrite(&shader_type, sizeof(uint32_t), 1, f) != 1) {
      fclose(f);
      return CMP_ERROR_IO;
    }
    curr = curr->next;
  }

  fclose(f);
  return CMP_SUCCESS;
}

int cmp_shader_cache_load_from_disk(cmp_shader_cache_t *cache,
                                    const char *filepath) {
  FILE *f;
  if (!cache || !filepath)
    return CMP_ERROR_INVALID_ARG;

#if defined(_MSC_VER)
  if (fopen_s(&f, filepath, "rb") != 0 || !f)
    return CMP_ERROR_IO;
#else
  f = fopen(filepath, "rb");
  if (!f)
    return CMP_ERROR_IO;
#endif

  while (!feof(f)) {
    uint32_t key_len = 0;
    uint32_t shader_type = 0;
    char *key_buf;
    cmp_shader_t shader;
    int err;

    if (fread(&key_len, sizeof(uint32_t), 1, f) != 1) {
      break; /* End of file or read error */
    }

    if (CMP_MALLOC(key_len + 1, (void **)&key_buf) != CMP_SUCCESS) {
      fclose(f);
      return CMP_ERROR_OOM;
    }

    if (fread(key_buf, 1, key_len, f) != key_len) {
      CMP_FREE(key_buf);
      fclose(f);
      return CMP_ERROR_IO;
    }
    key_buf[key_len] = '\0';

    if (fread(&shader_type, sizeof(uint32_t), 1, f) != 1) {
      CMP_FREE(key_buf);
      fclose(f);
      return CMP_ERROR_IO;
    }

    memset(&shader, 0, sizeof(cmp_shader_t));
    if (shader_type == 1) {
      cmp_shader_init_rounded_rect(&shader);
    } else if (shader_type == 2) {
      cmp_shader_init_gradient(&shader);
    } else if (shader_type == 3) {
      cmp_shader_init_sdf_text(&shader);
    }

    err = cmp_shader_cache_store(cache, key_buf, &shader);
    CMP_FREE(key_buf);

    if (err != CMP_SUCCESS) {
      fclose(f);
      return err;
    }
  }

  fclose(f);
  return CMP_SUCCESS;
}
