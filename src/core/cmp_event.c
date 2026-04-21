/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int g_event_initialized = 0;
static cmp_ring_buffer_t g_event_queue;

int cmp_event_system_init(void) {
  int rc;
  if (g_event_initialized) {
    return CMP_SUCCESS;
  }

  /* Robust queue size for normalization before dispatch.
   * Increased to 131072 to comfortably absorb thousands of
   * WM_MOUSEMOVE and WM_SIZE events emitted during slow Windows
   * modal resize drag loops where PeekMessage is temporarily blocked. */
  rc = cmp_ring_buffer_init(&g_event_queue, 131072);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_event_system_init cmp_ring_buffer_init: %s\n", cmp_strerror(rc));
    return CMP_ERROR_OOM;
  }

  g_event_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_event_system_shutdown(void) {
  int rc;
  void *item;

  if (!g_event_initialized) {
    return CMP_SUCCESS;
  }

  /* Clear out unhandled events */
  while (1) {
    rc = cmp_ring_buffer_pop(&g_event_queue, &item);
    if (rc != CMP_SUCCESS) break;
    CMP_FREE(item);
  }

  cmp_ring_buffer_destroy(&g_event_queue);
  g_event_initialized = 0;
  return CMP_SUCCESS;
}

int cmp_event_push(const cmp_event_t *event) {
  int rc;
  cmp_event_t *copy;

  if (event == NULL || !g_event_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_event_push: %s\n", cmp_strerror(rc));
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_event_t), (void **)&copy);
  if (rc != CMP_SUCCESS) {
    if (rc == CMP_SUCCESS) rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_event_push CMP_MALLOC: %s\n", cmp_strerror(rc));
    return rc;
  }

  memcpy(copy, event, sizeof(cmp_event_t));

  rc = cmp_ring_buffer_push(&g_event_queue, copy);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(copy);
    LOG_DEBUG("cmp_event_push cmp_ring_buffer_push: %s\n", cmp_strerror(rc));
    return rc;
  }

  return CMP_SUCCESS;
}

int cmp_event_pop(cmp_event_t *out_event) {
  int rc;
  cmp_event_t *ptr;

  if (out_event == NULL || !g_event_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_event_pop: %s\n", cmp_strerror(rc));
    return rc;
  }

  rc = cmp_ring_buffer_pop(&g_event_queue, (void **)&ptr);
  if (rc != CMP_SUCCESS) {
    return rc; /* Usually NOT_FOUND, no need to log excessively here on idle */
  }

  memcpy(out_event, ptr, sizeof(cmp_event_t));
  CMP_FREE(ptr);

  return CMP_SUCCESS;
}

int cmp_event_hit_test(int x, int y) {
  int rc;
  /* Mapping coordinates to UI tree nodes (Full implementation deferred to Phase 25) */
  (void)x;
  (void)y;
  rc = 1;
  return rc;
}

static int g_focused_element_id = -1;
static int g_focus_ring_visible = 0;

int cmp_event_set_focus(int element_id) {
  int rc;
  g_focused_element_id = element_id;
  /* Automatically show focus ring when programmatic or keyboard focus happens
   */
  g_focus_ring_visible = 1;
  rc = CMP_SUCCESS;
  return rc;
}

int cmp_event_get_focus(void) { 
  int rc;
  rc = g_focused_element_id;
  return rc; 
}

int cmp_event_is_focus_ring_visible(void) { 
  int rc;
  rc = g_focus_ring_visible;
  return rc; 
}

int cmp_event_clear_focus(void) {
  int rc;
  g_focused_element_id = -1;
  g_focus_ring_visible = 0;
  rc = CMP_SUCCESS;
  return rc;
}

int cmp_event_handle_tab_targeting(int reverse) {
  int rc;
  if (g_focused_element_id < 0) {
    g_focused_element_id = 1;
  } else {
    if (reverse) {
      g_focused_element_id = (g_focused_element_id > 1) ? g_focused_element_id - 1 : 100;
    } else {
      g_focused_element_id = (g_focused_element_id < 100) ? g_focused_element_id + 1 : 1;
    }
  }
  g_focus_ring_visible = 1; /* Keyboard-initiated focus shows the ring */
  rc = CMP_SUCCESS;
  return rc;
}