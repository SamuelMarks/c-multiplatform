/* clang-format off */
#include "m3_memory.h"
#include <string.h>
#include <stdlib.h>

#define M3_STATE_ARENA_SIZE (1024 * 1024 * 4) /* 4MB */
#define M3_UI_ARENA_SIZE    (1024 * 1024 * 8) /* 8MB */
#define M3_STRING_POOL_INITIAL_CAPACITY 1024
/* clang-format on */

static void free_fallback_list(m3_fallback_node_t **head) {
  m3_fallback_node_t *current = *head;
  m3_fallback_node_t *next;
  while (current) {
    next = current->next;
    if (current->ptr) {
      free(current->ptr);
    }
    free(current);
    current = next;
  }
  *head = NULL;
}

static int add_fallback_alloc(m3_fallback_node_t **head, void *ptr) {
  m3_fallback_node_t *node =
      (m3_fallback_node_t *)malloc(sizeof(m3_fallback_node_t));
  if (!node) {
    return 1;
  }
  node->ptr = ptr;
  node->next = *head;
  *head = node;
  return 0;
}

int m3_memory_init(m3_memory_state_t *state) {
  void *strings_ptr = NULL;
  size_t strings_size;

  if (!state) {
    return 1;
  }

  state->state_fallback_allocs = NULL;
  state->ui_fallback_allocs = NULL;
  state->string_pool.fallback_allocs = NULL;

  if (cmp_arena_init(&state->m3_state_arena, M3_STATE_ARENA_SIZE) != 0) {
    return 1;
  }

  if (cmp_arena_init(&state->m3_ui_arena, M3_UI_ARENA_SIZE) != 0) {
    cmp_arena_free(&state->m3_state_arena);
    return 1;
  }

  if (cmp_arena_init(&state->string_pool.arena, M3_STATE_ARENA_SIZE) != 0) {
    cmp_arena_free(&state->m3_ui_arena);
    cmp_arena_free(&state->m3_state_arena);
    return 1;
  }

  strings_size = M3_STRING_POOL_INITIAL_CAPACITY * sizeof(char *);
  if (cmp_arena_alloc(&state->string_pool.arena, strings_size, &strings_ptr) !=
      0) {
    cmp_arena_free(&state->string_pool.arena);
    cmp_arena_free(&state->m3_ui_arena);
    cmp_arena_free(&state->m3_state_arena);
    return 1;
  }

  state->string_pool.strings = (char **)strings_ptr;
  state->string_pool.count = 0;
  state->string_pool.capacity = M3_STRING_POOL_INITIAL_CAPACITY;

  return 0;
}

int m3_memory_cleanup(m3_memory_state_t *state) {
  if (!state) {
    return 1;
  }

  free_fallback_list(&state->state_fallback_allocs);
  free_fallback_list(&state->ui_fallback_allocs);
  free_fallback_list(&state->string_pool.fallback_allocs);

  cmp_arena_free(&state->string_pool.arena);
  cmp_arena_free(&state->m3_ui_arena);
  cmp_arena_free(&state->m3_state_arena);

  return 0;
}

int m3_state_alloc(m3_memory_state_t *state, size_t size, void **out_ptr) {
  if (!state || !out_ptr) {
    return 1;
  }

  if (cmp_arena_alloc(&state->m3_state_arena, size, out_ptr) != 0) {
    *out_ptr = malloc(size);
    if (!*out_ptr) {
      return 1;
    }
    if (add_fallback_alloc(&state->state_fallback_allocs, *out_ptr) != 0) {
      free(*out_ptr);
      *out_ptr = NULL;
      return 1;
    }
  }

  return 0;
}

int m3_ui_alloc(m3_memory_state_t *state, size_t size, void **out_ptr) {
  if (!state || !out_ptr) {
    return 1;
  }

  if (cmp_arena_alloc(&state->m3_ui_arena, size, out_ptr) != 0) {
    *out_ptr = malloc(size);
    if (!*out_ptr) {
      return 1;
    }
    if (add_fallback_alloc(&state->ui_fallback_allocs, *out_ptr) != 0) {
      free(*out_ptr);
      *out_ptr = NULL;
      return 1;
    }
  }

  return 0;
}

int m3_ui_arena_clear(m3_memory_state_t *state) {
  if (!state) {
    return 1;
  }

  free_fallback_list(&state->ui_fallback_allocs);

  cmp_arena_free(&state->m3_ui_arena);
  if (cmp_arena_init(&state->m3_ui_arena, M3_UI_ARENA_SIZE) != 0) {
    return 1;
  }

  return 0;
}

int m3_string_intern(m3_memory_state_t *state, const char *str,
                     const char **out_str) {
  size_t i;
  size_t len;
  void *new_str_ptr = NULL;
  char *new_str;

  if (!state || !str || !out_str) {
    return 1;
  }

  for (i = 0; i < state->string_pool.count; ++i) {
    if (strcmp(state->string_pool.strings[i], str) == 0) {
      *out_str = state->string_pool.strings[i];
      return 0;
    }
  }

  if (state->string_pool.count >= state->string_pool.capacity) {
    size_t new_capacity = state->string_pool.capacity * 2;
    void *new_strings_ptr = NULL;
    char **new_strings;

    if (cmp_arena_alloc(&state->string_pool.arena,
                        new_capacity * sizeof(char *), &new_strings_ptr) != 0) {
      new_strings = (char **)malloc(new_capacity * sizeof(char *));
      if (!new_strings) {
        return 1;
      }
      if (add_fallback_alloc(&state->string_pool.fallback_allocs,
                             new_strings) != 0) {
        free(new_strings);
        return 1;
      }
    } else {
      new_strings = (char **)new_strings_ptr;
    }

    memcpy(new_strings, state->string_pool.strings,
           state->string_pool.count * sizeof(char *));
    state->string_pool.strings = new_strings;
    state->string_pool.capacity = new_capacity;
  }

  len = strlen(str) + 1;
  if (cmp_arena_alloc(&state->string_pool.arena, len, &new_str_ptr) != 0) {
    new_str = (char *)malloc(len);
    if (!new_str) {
      return 1;
    }
    if (add_fallback_alloc(&state->string_pool.fallback_allocs, new_str) != 0) {
      free(new_str);
      return 1;
    }
  } else {
    new_str = (char *)new_str_ptr;
  }

#if defined(_MSC_VER)
  strcpy_s(new_str, len, str);
#else
  strcpy(new_str, str);
#endif

  state->string_pool.strings[state->string_pool.count++] = new_str;
  *out_str = new_str;

  return 0;
}