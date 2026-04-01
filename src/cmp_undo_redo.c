/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_undo_redo {
  char *stack[10];
  int count;
  int position;
};

int cmp_undo_redo_create(cmp_undo_redo_t **out_stack) {
  struct cmp_undo_redo *stack;

  if (!out_stack)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_undo_redo), (void **)&stack) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(stack, 0, sizeof(struct cmp_undo_redo));

  *out_stack = (cmp_undo_redo_t *)stack;
  return CMP_SUCCESS;
}

int cmp_undo_redo_destroy(cmp_undo_redo_t *stack) {
  struct cmp_undo_redo *internal_stack = (struct cmp_undo_redo *)stack;
  int i;
  if (!internal_stack)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < internal_stack->count; ++i) {
    if (internal_stack->stack[i]) {
      CMP_FREE(internal_stack->stack[i]);
    }
  }

  CMP_FREE(internal_stack);
  return CMP_SUCCESS;
}

int cmp_undo_redo_push(cmp_undo_redo_t *stack, const char *state) {
  struct cmp_undo_redo *internal_stack = (struct cmp_undo_redo *)stack;
  size_t len;
  char *new_state = NULL;

  if (!internal_stack || !state)
    return CMP_ERROR_INVALID_ARG;

  if (internal_stack->count >= 10) {
    /* Simple drop oldest for stub */
    if (internal_stack->stack[0]) {
      CMP_FREE(internal_stack->stack[0]);
    }
    memmove(&internal_stack->stack[0], &internal_stack->stack[1],
            9 * sizeof(char *));
    internal_stack->count--;
  }

  len = strlen(state);
  if (CMP_MALLOC(len + 1, (void **)&new_state) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  strcpy(new_state, state);
  internal_stack->stack[internal_stack->count++] = new_state;
  internal_stack->position = internal_stack->count;

  return CMP_SUCCESS;
}

int cmp_undo_redo_undo(cmp_undo_redo_t *stack, char *out_buffer,
                       size_t out_capacity) {
  struct cmp_undo_redo *internal_stack = (struct cmp_undo_redo *)stack;

  if (!internal_stack || !out_buffer || out_capacity == 0)
    return CMP_ERROR_INVALID_ARG;

  if (internal_stack->position <= 0) {
    return CMP_ERROR_INVALID_STATE; /* Nothing to undo */
  }

  internal_stack->position--;

  if (internal_stack->position > 0) {
    strncpy(out_buffer, internal_stack->stack[internal_stack->position - 1],
            out_capacity - 1);
    out_buffer[out_capacity - 1] = '\0';
  } else {
    out_buffer[0] = '\0';
  }

  return CMP_SUCCESS;
}
