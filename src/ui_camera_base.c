/**
 * @file ui_camera_base.c
 * @brief Implementation of the camera base component.
 */

/* clang-format off */
#include "ui_camera_base.h"
#include "ui_internal_mem.h"
#include <string.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
void EMSCRIPTEN_KEEPALIVE ui_camera_base_mock_permission_response_js(struct ui_camera_base *camera, int granted);
#endif
/* clang-format on */

/**
 * @struct ui_camera_base
 * @struct ui_camera_base
 * @brief Internal representation of a camera base component.
 */
struct ui_camera_base {
  struct ui_component *component;          /**< component */
  enum ui_camera_state state;              /**< state */
  ui_camera_frame_callback frame_callback; /**< frame_callback */
  void *frame_user_data;                   /**< frame_user_data */
};

ui_error_t ui_camera_base_create(struct ui_camera_base **out_camera) {
  struct ui_camera_base *camera;
  struct ui_dom_node *root_node = NULL;
  ui_error_t rc;

  if (!out_camera) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  camera = (struct ui_camera_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_camera_base));
  if (!camera) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(camera, 0, sizeof(struct ui_camera_base));

  camera->state = UI_CAMERA_STATE_UNINITIALIZED;

  rc = ui_component_create(&camera->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(camera);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    (void)ui_component_destroy(camera->component);
    C_MULTIPLATFORM_FREE(camera);
    return rc;
  }

  /* Represent the camera unstyled boundary as a video tag internally */
  rc = ui_dom_node_set_tag_name(root_node, "video");
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(root_node);
    (void)ui_component_destroy(camera->component);
    C_MULTIPLATFORM_FREE(camera);
    return rc;
  }
  rc = ui_dom_node_set_attribute(root_node, "autoplay", "true");
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(root_node);
    (void)ui_component_destroy(camera->component);
    C_MULTIPLATFORM_FREE(camera);
    return rc;
  }
  camera->component->shadow_root = root_node;

  *out_camera = camera;
  return UI_ERROR_NONE;
}

ui_error_t ui_camera_base_destroy(struct ui_camera_base *camera) {
  if (!camera) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (camera->state == UI_CAMERA_STATE_STREAMING) {
    (void)ui_camera_base_stop_stream(camera);
  }
  if (camera->component) {
    (void)ui_component_destroy(camera->component);
  }
  C_MULTIPLATFORM_FREE(camera);
  return UI_ERROR_NONE;
}

ui_error_t ui_camera_base_get_component(struct ui_camera_base *camera,
                                        struct ui_component **out_component) {
  if (!camera || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = camera->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_camera_base_request_permission(struct ui_camera_base *camera) {
  if (!camera) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (camera->state != UI_CAMERA_STATE_UNINITIALIZED &&
      camera->state != UI_CAMERA_STATE_PERMISSION_DENIED) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  camera->state = UI_CAMERA_STATE_REQUESTING_PERMISSION;
  /* In real implementation, trigger AVFoundation/V4L2 async prompt here */
  return UI_ERROR_NONE;
}

ui_error_t ui_camera_base_start_stream(struct ui_camera_base *camera) {
  if (!camera) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (camera->state != UI_CAMERA_STATE_READY &&
      camera->state != UI_CAMERA_STATE_STREAMING) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  camera->state = UI_CAMERA_STATE_STREAMING;
  return UI_ERROR_NONE;
}

ui_error_t ui_camera_base_stop_stream(struct ui_camera_base *camera) {
  if (!camera) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (camera->state == UI_CAMERA_STATE_STREAMING) {
    camera->state = UI_CAMERA_STATE_READY;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_camera_base_get_state(struct ui_camera_base *camera,
                                    enum ui_camera_state *out_state) {
  if (!camera || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_state = camera->state;
  return UI_ERROR_NONE;
}

ui_error_t ui_camera_base_set_frame_callback(struct ui_camera_base *camera,
                                             ui_camera_frame_callback callback,
                                             void *user_data) {
  if (!camera) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  camera->frame_callback = callback;
  camera->frame_user_data = user_data;
  return UI_ERROR_NONE;
}

ui_error_t ui_camera_base_mock_frame(struct ui_camera_base *camera) {
  static const char mock_data[4] = {0, 0, 0, 0};
  if (!camera) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (camera->state != UI_CAMERA_STATE_STREAMING) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (camera->frame_callback) {
    ui_error_t rc = camera->frame_callback(camera, mock_data, sizeof(mock_data),
                                           1, 1, camera->frame_user_data);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return UI_ERROR_NONE;
}

ui_error_t
ui_camera_base_mock_permission_response(struct ui_camera_base *camera,
                                        int granted) {
  if (!camera) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (camera->state != UI_CAMERA_STATE_REQUESTING_PERMISSION) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (granted) {
    camera->state = UI_CAMERA_STATE_READY;
  } else {
    camera->state = UI_CAMERA_STATE_PERMISSION_DENIED;
  }
  return UI_ERROR_NONE;
}
