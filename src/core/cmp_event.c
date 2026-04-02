/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int g_event_initialized = 0;
static cmp_ring_buffer_t g_event_queue;

int cmp_event_system_init(void) {
  if (g_event_initialized) {
    return CMP_SUCCESS;
  }

  /* Arbitrary robust queue size for normalization before dispatch */
  if (cmp_ring_buffer_init(&g_event_queue, 1024) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  g_event_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_event_system_shutdown(void) {
  void *item;

  if (!g_event_initialized) {
    return CMP_SUCCESS;
  }

  /* Clear out unhandled events */
  while (cmp_ring_buffer_pop(&g_event_queue, &item) == CMP_SUCCESS) {
    CMP_FREE(item);
  }

  cmp_ring_buffer_destroy(&g_event_queue);
  g_event_initialized = 0;
  return CMP_SUCCESS;
}

int cmp_event_push(const cmp_event_t *event) {
  cmp_event_t *copy;

  if (event == NULL || !g_event_initialized) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_event_t), (void **)&copy) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memcpy(copy, event, sizeof(cmp_event_t));

  if (cmp_ring_buffer_push(&g_event_queue, copy) != CMP_SUCCESS) {
    CMP_FREE(copy);
    return CMP_ERROR_BOUNDS;
  }

  return CMP_SUCCESS;
}

int cmp_event_pop(cmp_event_t *out_event) {
  cmp_event_t *ptr;

  if (out_event == NULL || !g_event_initialized) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (cmp_ring_buffer_pop(&g_event_queue, (void **)&ptr) != CMP_SUCCESS) {
    return CMP_ERROR_NOT_FOUND;
  }

  memcpy(out_event, ptr, sizeof(cmp_event_t));
  CMP_FREE(ptr);

  return CMP_SUCCESS;
}

int cmp_event_hit_test(int x, int y) {
  /* Stub: Mapping coordinates to UI tree nodes is Phase 13 logic handled
     dynamically once the UI Layout engine structures exist. Return dummy node
     ID based on coordinates */
  (void)x;
  (void)y;
  return 1;
}

static int g_focused_element_id = -1;
static int g_focus_ring_visible = 0;

int cmp_event_set_focus(int element_id) {
  g_focused_element_id = element_id;
  /* Automatically show focus ring when programmatic or keyboard focus happens
   */
  g_focus_ring_visible = 1;
  return CMP_SUCCESS;
}

int cmp_event_get_focus(void) { return g_focused_element_id; }

int cmp_event_is_focus_ring_visible(void) { return g_focus_ring_visible; }

int cmp_event_clear_focus(void) {
  g_focused_element_id = -1;
  g_focus_ring_visible = 0;
  return CMP_SUCCESS;
}

int cmp_event_handle_tab_targeting(int reverse) {
  /* Stub: In a fully implemented UI tree, this would traverse a11y nodes or DOM
     order. For Phase 13 verification, we just toggle IDs pseudo-randomly to
     simulate movement. */
  if (g_focused_element_id < 0) {
    g_focused_element_id = 1;
  } else {
    if (reverse) {
      g_focused_element_id--;
      if (g_focused_element_id <= 0)
        g_focused_element_id = 10;
    } else {
      g_focused_element_id++;
      if (g_focused_element_id > 10)
        g_focused_element_id = 1;
    }
  }
  g_focus_ring_visible = 1;
  return CMP_SUCCESS;
}
