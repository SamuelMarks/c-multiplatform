/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"
/* clang-format on */

typedef struct cmp_screen_reader_mapping {
  int node_id;
  void *native_node;
} cmp_screen_reader_mapping_t;

struct cmp_screen_reader {
  cmp_a11y_tree_t *tree;
  cmp_screen_reader_mapping_t *mappings;
  size_t count;
  size_t capacity;
};

/**
 * @brief Create a screen reader integration context.
 *
 * @param tree The a11y tree context.
 * @param out_reader Pointer to receive the created reader instance.
 * @return 0 on success, or an error code on failure.
 */
int cmp_screen_reader_create(cmp_a11y_tree_t *tree,
                             cmp_screen_reader_t **out_reader) {
  int rc = CMP_SUCCESS;
  struct cmp_screen_reader *reader;

  rc = CMP_SUCCESS;

  if (tree == NULL || out_reader == NULL) {
    LOG_DEBUG("Invalid argument: tree or out_reader is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_screen_reader), (void **)&reader);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  reader->tree = tree;
  reader->mappings = NULL;
  reader->count = 0;
  reader->capacity = 0;

  *out_reader = (cmp_screen_reader_t *)reader;
  return rc;
}

/**
 * @brief Destroy a screen reader integration context.
 *
 * @param reader The screen reader instance.
 * @return 0 on success, or an error code on failure.
 */
int cmp_screen_reader_destroy(cmp_screen_reader_t *reader) {
  int rc = CMP_SUCCESS;
  struct cmp_screen_reader *r;

  rc = CMP_SUCCESS;
  r = (struct cmp_screen_reader *)reader;

  if (r == NULL) {
    LOG_DEBUG("Invalid argument: reader is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (r->mappings != NULL) {
    rc = CMP_FREE(r->mappings);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free mappings failed\n");
      return rc;
    }
  }

  rc = CMP_FREE(r);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free reader failed\n");
    return rc;
  }

  return rc;
}

/**
 * @brief Announce a message to the screen reader.
 *
 * @param reader The screen reader instance.
 * @param message The message to announce.
 * @return 0 on success, or an error code on failure.
 */
int cmp_screen_reader_announce(cmp_screen_reader_t *reader,
                               const char *message) {
  int rc = CMP_SUCCESS;
  if (reader == NULL || message == NULL) {
    LOG_DEBUG("Invalid argument: reader or message is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* In a real implementation, this would trigger OS-specific TTS APIs */
  /* For now, we simulate success */
  return rc;
}

/**
 * @brief Map a specific a11y tree node to a native screen reader node.
 *
 * @param reader The screen reader instance.
 * @param node_id The ID of the a11y node.
 * @param native_node The native node instance.
 * @return 0 on success, or an error code on failure.
 */
int cmp_screen_reader_map_node(cmp_screen_reader_t *reader, int node_id,
                               void *native_node) {
  int rc = CMP_SUCCESS;
  struct cmp_screen_reader *r;
  cmp_screen_reader_mapping_t *new_mappings;
  size_t new_capacity;
  size_t i;

  rc = CMP_SUCCESS;
  r = (struct cmp_screen_reader *)reader;

  if (r == NULL) {
    LOG_DEBUG("Invalid argument: reader is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Check if it already exists */
  for (i = 0; i < r->count; ++i) {
    if (r->mappings[i].node_id == node_id) {
      r->mappings[i].native_node = native_node;
      return rc;
    }
  }

  if (r->count >= r->capacity) {
    new_capacity = r->capacity == 0 ? 16 : r->capacity * 2;
    rc = CMP_MALLOC(new_capacity * sizeof(cmp_screen_reader_mapping_t),
                    (void **)&new_mappings);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("OOM mapping buffer\n");
      return CMP_ERROR_OOM;
    }

    if (r->mappings != NULL) {
      memcpy(new_mappings, r->mappings,
             r->count * sizeof(cmp_screen_reader_mapping_t));
      rc = CMP_FREE(r->mappings);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Free failed during resize\n");
      }
    }
    r->mappings = new_mappings;
    r->capacity = new_capacity;
  }

  r->mappings[r->count].node_id = node_id;
  r->mappings[r->count].native_node = native_node;
  r->count++;

  return rc;
}
