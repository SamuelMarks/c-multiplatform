/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Evaluate a media query.
 *
 * @param query The media query to evaluate.
 * @param env The environment to evaluate against.
 * @param out_matches Pointer to store the result.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_media_query_evaluate(const cmp_media_query_t *query,
                             const cmp_media_query_env_t *env,
                             int *out_matches) {
  float current_aspect;

  if (query == NULL || env == NULL || out_matches == NULL) {
    LOG_DEBUG("Error in cmp_media_query_evaluate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_matches = 1;

  if (query->min_width > 0 && env->viewport_width < query->min_width) {
    *out_matches = 0;
  }
  if (query->max_width > 0 && env->viewport_width > query->max_width) {
    *out_matches = 0;
  }
  if (query->min_height > 0 && env->viewport_height < query->min_height) {
    *out_matches = 0;
  }
  if (query->max_height > 0 && env->viewport_height > query->max_height) {
    *out_matches = 0;
  }

  if (env->viewport_height > 0) {
    current_aspect = env->viewport_width / env->viewport_height;
    if (query->min_aspect_ratio > 0 &&
        current_aspect < query->min_aspect_ratio) {
      *out_matches = 0;
    }
    if (query->max_aspect_ratio > 0 &&
        current_aspect > query->max_aspect_ratio) {
      *out_matches = 0;
    }
  }

  if (query->min_resolution > 0 &&
      env->resolution_dpi < query->min_resolution) {
    *out_matches = 0;
  }
  if (query->max_resolution > 0 &&
      env->resolution_dpi > query->max_resolution) {
    *out_matches = 0;
  }

  if (query->orientation != -1 && query->orientation != env->is_landscape) {
    *out_matches = 0;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Evaluate pointer-related media features.
 *
 * @param query The media query to evaluate.
 * @param env The environment to evaluate against.
 * @param out_matches Pointer to store the result.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_pointer_media_evaluate(const cmp_media_query_t *query,
                               const cmp_media_query_env_t *env,
                               int *out_matches) {
  if (query == NULL || env == NULL || out_matches == NULL) {
    LOG_DEBUG("Error in cmp_pointer_media_evaluate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_matches = 1;

  if (query->hover != -1 && query->hover != env->has_hover) {
    *out_matches = 0;
  }
  if (query->pointer != -1 && query->pointer != env->is_pointer_coarse) {
    *out_matches = 0;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Evaluate update frequency media feature.
 *
 * @param query The media query to evaluate.
 * @param env The environment to evaluate against.
 * @param out_matches Pointer to store the result.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_update_media_evaluate(const cmp_media_query_t *query,
                              const cmp_media_query_env_t *env,
                              int *out_matches) {
  if (query == NULL || env == NULL || out_matches == NULL) {
    LOG_DEBUG("Error in cmp_update_media_evaluate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_matches = 1;
  if (query->update != -1 && query->update != env->update_frequency) {
    *out_matches = 0;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Evaluate light-level media feature.
 *
 * @param query The media query to evaluate.
 * @param env The environment to evaluate against.
 * @param out_matches Pointer to store the result.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_light_level_evaluate(const cmp_media_query_t *query,
                             const cmp_media_query_env_t *env,
                             int *out_matches) {
  if (query == NULL || env == NULL || out_matches == NULL) {
    LOG_DEBUG("Error in cmp_light_level_evaluate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_matches = 1;
  if (query->light_level != -1 && query->light_level != env->light_level) {
    *out_matches = 0;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Create a container query context.
 *
 * @param out_ctx Pointer to store the created context.
 * @param type The container type.
 * @param name The container name.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_container_ctx_create(cmp_container_ctx_t **out_ctx,
                             cmp_container_type_t type, const char *name) {
  int rc;
  cmp_container_ctx_t *ctx;

  rc = CMP_SUCCESS;

  if (out_ctx == NULL) {
    LOG_DEBUG("Error in cmp_container_ctx_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_container_ctx_t), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_container_ctx_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  memset(ctx, 0, sizeof(cmp_container_ctx_t));
  ctx->type = type;
  ctx->name = name;
  ctx->inline_size = 0.0f;
  ctx->block_size = 0.0f;

  *out_ctx = ctx;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a container query context.
 *
 * @param ctx The context to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_container_ctx_destroy(cmp_container_ctx_t *ctx) {
  int rc;

  rc = CMP_SUCCESS;

  if (ctx == NULL) {
    LOG_DEBUG("Error in cmp_container_ctx_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_container_ctx_destroy: CMP_FREE failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Evaluate a container query.
 *
 * @param query The container query to evaluate.
 * @param ctx The context to evaluate against.
 * @param out_matches Pointer to store the result.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_container_query_evaluate(const cmp_container_query_t *query,
                                 const cmp_container_ctx_t *ctx,
                                 int *out_matches) {
  if (query == NULL || ctx == NULL || out_matches == NULL) {
    LOG_DEBUG("Error in cmp_container_query_evaluate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_matches = 1;

  if (query->name != NULL && ctx->name != NULL &&
      strcmp(query->name, ctx->name) != 0) {
    *out_matches = 0;
    return CMP_SUCCESS;
  }

  if (query->min_width > 0 && ctx->inline_size < query->min_width) {
    *out_matches = 0;
  }
  if (query->max_width > 0 && ctx->inline_size > query->max_width) {
    *out_matches = 0;
  }
  if (query->min_height > 0 && ctx->block_size < query->min_height) {
    *out_matches = 0;
  }
  if (query->max_height > 0 && ctx->block_size > query->max_height) {
    *out_matches = 0;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Evaluate a style query.
 *
 * @param query The style query to evaluate.
 * @param container_styles Array of container styles.
 * @param num_styles Number of container styles.
 * @param out_matches Pointer to store the result.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_style_query_evaluate(const cmp_style_query_t *query,
                             const cmp_style_query_t *container_styles,
                             int num_styles, int *out_matches) {
  int i;

  if (query == NULL || out_matches == NULL) {
    LOG_DEBUG("Error in cmp_style_query_evaluate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_matches = 0;

  if (container_styles == NULL || num_styles <= 0) {
    return CMP_SUCCESS;
  }

  for (i = 0; i < num_styles; i++) {
    if (query->property_name != NULL &&
        container_styles[i].property_name != NULL &&
        strcmp(query->property_name, container_styles[i].property_name) == 0) {

      if (query->property_value != NULL &&
          container_styles[i].property_value != NULL &&
          strcmp(query->property_value, container_styles[i].property_value) ==
              0) {
        *out_matches = 1;
        break;
      } else if (query->property_value == NULL) {
        *out_matches = 1;
        break;
      }
    }
  }

  return CMP_SUCCESS;
}

/**
 * @brief Evaluate content visibility.
 *
 * @param visibility The visibility parameter.
 * @param viewport The viewport rect.
 * @param node_rect The node rect.
 * @param out_is_visible Pointer to store the result.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_content_visibility_evaluate(cmp_content_visibility_t visibility,
                                    const cmp_rect_t *viewport,
                                    const cmp_rect_t *node_rect,
                                    int *out_is_visible) {
  int x_overlap;
  int y_overlap;

  if (viewport == NULL || node_rect == NULL || out_is_visible == NULL) {
    LOG_DEBUG("Error in cmp_content_visibility_evaluate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (visibility == CMP_CONTENT_VISIBILITY_VISIBLE) {
    *out_is_visible = 1;
  } else if (visibility == CMP_CONTENT_VISIBILITY_HIDDEN) {
    *out_is_visible = 0;
  } else { /* AUTO */
    /* Simple AABB intersection check */
    x_overlap = (node_rect->x < viewport->x + viewport->width) &&
                (node_rect->x + node_rect->width > viewport->x);
    y_overlap = (node_rect->y < viewport->y + viewport->height) &&
                (node_rect->y + node_rect->height > viewport->y);
    *out_is_visible = x_overlap && y_overlap;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Evaluate containment parameters.
 *
 * @param contain The contain mode.
 * @param out_isolates_layout Pointer to store whether layout is isolated.
 * @param out_isolates_paint Pointer to store whether paint is isolated.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_contain_evaluate(cmp_contain_t contain, int *out_isolates_layout,
                         int *out_isolates_paint) {
  if (out_isolates_layout == NULL || out_isolates_paint == NULL) {
    LOG_DEBUG("Error in cmp_contain_evaluate: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  *out_isolates_layout = 0;
  *out_isolates_paint = 0;

  if (contain == CMP_CONTAIN_NONE) {
    return CMP_SUCCESS;
  }

  if (contain & CMP_CONTAIN_STRICT) {
    *out_isolates_layout = 1;
    *out_isolates_paint = 1;
    return CMP_SUCCESS;
  }
  if (contain & CMP_CONTAIN_CONTENT) {
    *out_isolates_layout = 1;
    *out_isolates_paint = 1;
    return CMP_SUCCESS;
  }

  if (contain & CMP_CONTAIN_LAYOUT) {
    *out_isolates_layout = 1;
  }
  if (contain & CMP_CONTAIN_PAINT) {
    *out_isolates_paint = 1;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Create a resize observer.
 *
 * @param out_observer Pointer to store the created observer.
 * @param on_resize Callback when a resize occurs.
 * @param user_data User data for the callback.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_resize_observer_create(cmp_resize_observer_t **out_observer,
                               void (*on_resize)(struct cmp_resize_observer *,
                                                 cmp_layout_node_t *, float,
                                                 float),
                               void *user_data) {
  int rc;
  cmp_resize_observer_t *obs;

  rc = CMP_SUCCESS;

  if (out_observer == NULL || on_resize == NULL) {
    LOG_DEBUG("Error in cmp_resize_observer_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_resize_observer_t), (void **)&obs);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_resize_observer_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  memset(obs, 0, sizeof(cmp_resize_observer_t));
  obs->user_data = user_data;
  obs->on_resize = on_resize;

  *out_observer = obs;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a resize observer.
 *
 * @param observer The observer to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_resize_observer_destroy(cmp_resize_observer_t *observer) {
  int rc;

  rc = CMP_SUCCESS;

  if (observer == NULL) {
    LOG_DEBUG("Error in cmp_resize_observer_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(observer);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_resize_observer_destroy: CMP_FREE failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Notify the observer of a resize.
 *
 * @param observer The observer to notify.
 * @param node The node that was resized.
 * @param new_width The new width.
 * @param new_height The new height.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_resize_observer_notify(cmp_resize_observer_t *observer,
                               cmp_layout_node_t *node, float new_width,
                               float new_height) {
  if (observer == NULL || node == NULL) {
    LOG_DEBUG("Error in cmp_resize_observer_notify: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (observer->on_resize != NULL) {
    observer->on_resize(observer, node, new_width, new_height);
  }
  return CMP_SUCCESS;
}
