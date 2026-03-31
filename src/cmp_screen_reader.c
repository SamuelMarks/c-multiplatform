/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

int cmp_screen_reader_create(cmp_a11y_tree_t *tree,
                             cmp_screen_reader_t **out_reader) {
  struct cmp_screen_reader *reader;

  if (!tree || !out_reader)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_screen_reader), (void **)&reader) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  reader->tree = tree;
  reader->mappings = NULL;
  reader->count = 0;
  reader->capacity = 0;

  *out_reader = (cmp_screen_reader_t *)reader;
  return CMP_SUCCESS;
}

int cmp_screen_reader_destroy(cmp_screen_reader_t *reader) {
  struct cmp_screen_reader *r = (struct cmp_screen_reader *)reader;

  if (!r)
    return CMP_ERROR_INVALID_ARG;

  if (r->mappings) {
    CMP_FREE(r->mappings);
  }

  CMP_FREE(r);
  return CMP_SUCCESS;
}

int cmp_screen_reader_announce(cmp_screen_reader_t *reader,
                               const char *message) {
  if (!reader || !message)
    return CMP_ERROR_INVALID_ARG;

  /* In a real implementation, this would trigger OS-specific TTS APIs */
  /* For now, we simulate success */
  return CMP_SUCCESS;
}

int cmp_screen_reader_map_node(cmp_screen_reader_t *reader, int node_id,
                               void *native_node) {
  struct cmp_screen_reader *r = (struct cmp_screen_reader *)reader;
  cmp_screen_reader_mapping_t *new_mappings;
  size_t new_capacity;
  size_t i;

  if (!r)
    return CMP_ERROR_INVALID_ARG;

  /* Check if it already exists */
  for (i = 0; i < r->count; ++i) {
    if (r->mappings[i].node_id == node_id) {
      r->mappings[i].native_node = native_node;
      return CMP_SUCCESS;
    }
  }

  if (r->count >= r->capacity) {
    new_capacity = r->capacity == 0 ? 16 : r->capacity * 2;
    if (CMP_MALLOC(new_capacity * sizeof(cmp_screen_reader_mapping_t),
                   (void **)&new_mappings) != CMP_SUCCESS)
      return CMP_ERROR_OOM;

    if (r->mappings) {
      memcpy(new_mappings, r->mappings,
             r->count * sizeof(cmp_screen_reader_mapping_t));
      CMP_FREE(r->mappings);
    }
    r->mappings = new_mappings;
    r->capacity = new_capacity;
  }

  r->mappings[r->count].node_id = node_id;
  r->mappings[r->count].native_node = native_node;
  r->count++;

  return CMP_SUCCESS;
}
