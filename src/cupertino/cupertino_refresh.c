/**
 * @file cupertino_refresh.c
 * @brief Cupertino Pull-To-Refresh Control implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_refresh.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_refresh_mock_create_fail = 0;
int g_cupertino_refresh_mock_destroy_fail = 0;
int g_cupertino_refresh_mock_set_cb_fail = 0;
int g_cupertino_refresh_mock_complete_fail = 0;

static ui_error_t
mock_ptr_base_create(struct ui_pull_to_refresh_base **out_base) {
  if (g_cupertino_refresh_mock_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_pull_to_refresh_base_create(out_base);
}
#undef ui_pull_to_refresh_base_create
/** @cond */
#define ui_pull_to_refresh_base_create mock_ptr_base_create
/** @endcond */

static ui_error_t mock_ptr_base_destroy(struct ui_pull_to_refresh_base *base) {
  if (g_cupertino_refresh_mock_destroy_fail) {
    (ui_pull_to_refresh_base_destroy)(base);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_pull_to_refresh_base_destroy)(base);
}
#undef ui_pull_to_refresh_base_destroy
/** @cond */
#define ui_pull_to_refresh_base_destroy mock_ptr_base_destroy
/** @endcond */

static ui_error_t
mock_ptr_base_set_on_refresh(struct ui_pull_to_refresh_base *base,
                             ui_pull_to_refresh_on_refresh_t cb,
                             void *user_data) {
  if (g_cupertino_refresh_mock_set_cb_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_pull_to_refresh_base_set_on_refresh(base, cb, user_data);
}
#undef ui_pull_to_refresh_base_set_on_refresh
/** @cond */
#define ui_pull_to_refresh_base_set_on_refresh mock_ptr_base_set_on_refresh
/** @endcond */

static ui_error_t mock_ptr_base_complete(struct ui_pull_to_refresh_base *base) {
  if (g_cupertino_refresh_mock_complete_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_pull_to_refresh_base_complete(base);
}
#undef ui_pull_to_refresh_base_complete
/** @cond */
#define ui_pull_to_refresh_base_complete mock_ptr_base_complete
/** @endcond */
#endif

ui_error_t
cupertino_refresh_create(struct ui_engine *engine,
                         const struct cupertino_refresh_descriptor *desc,
                         struct cupertino_refresh **out_refresh) {
  struct cupertino_refresh *refresh;
  ui_error_t rc;

  if (!engine || !desc || !out_refresh) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  refresh = (struct cupertino_refresh *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_refresh));
  if (!refresh) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(refresh, 0, sizeof(*refresh));
  refresh->trigger_distance = (desc->trigger_distance > 0.0f)
                                  ? desc->trigger_distance
                                  : CUPERTINO_REFRESH_DEFAULT_TRIGGER_DISTANCE;
  refresh->resting_distance = (desc->resting_distance > 0.0f)
                                  ? desc->resting_distance
                                  : CUPERTINO_REFRESH_DEFAULT_RESTING_DISTANCE;
  refresh->state = UI_PULL_TO_REFRESH_RESTING;

  rc = ui_pull_to_refresh_base_create(&refresh->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(refresh);
    return rc;
  }

  if (desc->on_refresh) {
    rc = ui_pull_to_refresh_base_set_on_refresh(refresh->base, desc->on_refresh,
                                                desc->user_data);
    if (rc != UI_ERROR_NONE) {
      ui_error_t destroy_rc;
      destroy_rc = ui_pull_to_refresh_base_destroy(refresh->base);
      if (destroy_rc != UI_ERROR_NONE) {
        /* Keep original error */
      }
      C_MULTIPLATFORM_FREE(refresh);
      return rc;
    }
  }

  *out_refresh = refresh;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_refresh_destroy(struct cupertino_refresh *refresh) {
  ui_error_t rc;

  if (!refresh) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (refresh->base) {
    rc = ui_pull_to_refresh_base_destroy(refresh->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    refresh->base = NULL;
  }

  C_MULTIPLATFORM_FREE(refresh);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_refresh_handle_pull(struct cupertino_refresh *refresh,
                                         float pull_distance) {
  float p;

  if (!refresh) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (refresh->state == UI_PULL_TO_REFRESH_REFRESHING) {
    return UI_ERROR_NONE;
  }

  if (pull_distance < 0.0f) {
    pull_distance = 0.0f;
  }

  refresh->current_pull_distance = pull_distance;
  p = pull_distance / refresh->trigger_distance;
  if (p > 1.0f) {
    p = 1.0f;
  }
  refresh->progress = p;

  refresh->active_spoke = (int)(p * (float)CUPERTINO_REFRESH_SPOKE_COUNT);
  if (refresh->active_spoke >= CUPERTINO_REFRESH_SPOKE_COUNT) {
    refresh->active_spoke = CUPERTINO_REFRESH_SPOKE_COUNT - 1;
  }

  if (pull_distance > 0.0f) {
    refresh->state = UI_PULL_TO_REFRESH_PULLING;
  } else {
    refresh->state = UI_PULL_TO_REFRESH_RESTING;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_refresh_release(struct cupertino_refresh *refresh) {
  if (!refresh) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (refresh->current_pull_distance >= refresh->trigger_distance) {
    refresh->state = UI_PULL_TO_REFRESH_REFRESHING;
    refresh->current_pull_distance = refresh->resting_distance;
  } else {
    refresh->state = UI_PULL_TO_REFRESH_RESTING;
    refresh->current_pull_distance = 0.0f;
    refresh->progress = 0.0f;
    refresh->active_spoke = 0;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_refresh_complete(struct cupertino_refresh *refresh) {
  ui_error_t rc;

  if (!refresh) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (refresh->base) {
    rc = ui_pull_to_refresh_base_complete(refresh->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  refresh->state = UI_PULL_TO_REFRESH_RESTING;
  refresh->current_pull_distance = 0.0f;
  refresh->progress = 0.0f;
  refresh->active_spoke = 0;

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_refresh_get_progress(const struct cupertino_refresh *refresh,
                               float *out_progress) {
  if (!refresh || !out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_progress = refresh->progress;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_refresh_get_active_spoke(const struct cupertino_refresh *refresh,
                                   int *out_spoke) {
  if (!refresh || !out_spoke) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_spoke = refresh->active_spoke;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_refresh_get_state(const struct cupertino_refresh *refresh,
                            enum ui_pull_to_refresh_state *out_state) {
  if (!refresh || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_state = refresh->state;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_refresh_get_base(struct cupertino_refresh *refresh,
                           struct ui_pull_to_refresh_base **out_base) {
  if (!refresh || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = refresh->base;
  return UI_ERROR_NONE;
}
