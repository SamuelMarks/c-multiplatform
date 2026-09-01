/* clang-format off */
#include "../include/ui_camera_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int frame_received = 0;

static ui_error_t test_frame_callback(struct ui_camera_base *camera,
                                      const void *frame_data, size_t size,
                                      int width, int height, void *user_data) {
  (void)camera;
  (void)frame_data;
  (void)size;
  (void)width;
  (void)height;
  if (user_data != (void *)0x9999) {
    return UI_ERROR_UNKNOWN;
  }
  frame_received = 1;
  return UI_ERROR_NONE;
}

static int test_camera_lifecycle(void) {
  struct ui_camera_base *camera = NULL;
  struct ui_component *comp;
  ui_error_t rc;
  enum ui_camera_state state;

  rc = ui_camera_base_create(&camera);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_camera_base_get_component(camera, &comp);
  if (rc != UI_ERROR_NONE || !comp || !comp->shadow_root)
    return 1;

  rc = ui_camera_base_get_state(camera, &state);
  if (rc != UI_ERROR_NONE || state != UI_CAMERA_STATE_UNINITIALIZED)
    return 1;

  /* Test denied permission flow */
  rc = ui_camera_base_request_permission(camera);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_camera_base_get_state(camera, &state);
  if (rc != UI_ERROR_NONE || state != UI_CAMERA_STATE_REQUESTING_PERMISSION)
    return 1;

  rc = ui_camera_base_mock_permission_response(camera, 0); /* Deny */
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_camera_base_get_state(camera, &state);
  if (rc != UI_ERROR_NONE || state != UI_CAMERA_STATE_PERMISSION_DENIED)
    return 1;

  /* Cannot start stream if denied */
  rc = ui_camera_base_start_stream(camera);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Retry and grant permission */
  rc = ui_camera_base_request_permission(camera);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_camera_base_mock_permission_response(camera, 1); /* Grant */
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_camera_base_get_state(camera, &state);
  if (rc != UI_ERROR_NONE || state != UI_CAMERA_STATE_READY)
    return 1;

  /* Register callback and start stream */
  rc = ui_camera_base_set_frame_callback(camera, test_frame_callback,
                                         (void *)0x9999);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_camera_base_start_stream(camera);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_camera_base_get_state(camera, &state);
  if (rc != UI_ERROR_NONE || state != UI_CAMERA_STATE_STREAMING)
    return 1;

  rc = ui_camera_base_mock_frame(camera);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Stop stream while destroying */
  rc = ui_camera_base_destroy(camera);
  if (rc != UI_ERROR_NONE)
    return 1;

  return 0;
}

static int test_camera_null_args_and_coverage(void) {
  struct ui_camera_base *camera = NULL;
  struct ui_component *comp;
  enum ui_camera_state state;
  int failed = 0;
  int i;

  /* Test create null arg */
  if (ui_camera_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1;

  /* Test allocation failures */
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    if (ui_camera_base_create(&camera) == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_camera_base_destroy(camera);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  /* Create a valid camera for further testing */
  if (ui_camera_base_create(&camera) != UI_ERROR_NONE)
    failed = 1;

  /* Test get_component */
  if (ui_camera_base_get_component(NULL, &comp) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1;
  if (ui_camera_base_get_component(camera, NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1;

  /* Test request_permission */
  if (ui_camera_base_request_permission(NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1;
  {
    ui_error_t rc_cleanup = ui_camera_base_request_permission(camera);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_camera_base_mock_permission_response(camera, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (ui_camera_base_request_permission(camera) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1; /* wrong state */

  /* Test start_stream */
  if (ui_camera_base_start_stream(NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1;
  {
    ui_error_t rc_cleanup = ui_camera_base_start_stream(camera);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (ui_camera_base_start_stream(camera) != UI_ERROR_NONE)
    failed = 1; /* Already streaming */
  {
    ui_error_t rc_cleanup = ui_camera_base_stop_stream(camera);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Test stop_stream */
  if (ui_camera_base_stop_stream(NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1;

  /* Test get_state */
  if (ui_camera_base_get_state(NULL, &state) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1;
  if (ui_camera_base_get_state(camera, NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1;

  /* Test set_frame_callback */
  if (ui_camera_base_set_frame_callback(NULL, test_frame_callback, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    failed = 1;

  /* Test mock_frame */
  if (ui_camera_base_mock_frame(NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1;
  if (ui_camera_base_mock_frame(camera) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1; /* not streaming */
  {
    ui_error_t rc_cleanup = ui_camera_base_start_stream(camera);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (ui_camera_base_mock_frame(camera) != UI_ERROR_NONE)
    failed = 1; /* no callback set */

  /* Test mock_permission_response */
  if (ui_camera_base_mock_permission_response(NULL, 1) !=
      UI_ERROR_INVALID_ARGUMENT)
    failed = 1;
  if (ui_camera_base_mock_permission_response(camera, 1) !=
      UI_ERROR_INVALID_ARGUMENT)
    failed = 1; /* not requesting */

  /* Test destroy */
  if (ui_camera_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    failed = 1;
  {
    ui_error_t rc_cleanup = ui_camera_base_destroy(camera);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return failed;
}

static void test_camera_coverage(void) {
  struct ui_camera_base *cam = NULL;

  {
    ui_error_t rc_cleanup = ui_camera_base_create(&cam);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_camera_base_start_stream(cam);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_camera_base_stop_stream(cam);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_camera_base_start_stream(cam);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    struct ui_camera_base_internal {
      struct ui_component *component;
      struct ui_dom_node *root_node;
      struct ui_dom_node *video_node;
      enum ui_camera_state state;
      void *frame_callback;
      void *frame_user_data;
    } *ci = (struct ui_camera_base_internal *)cam;

    struct ui_component *saved = ci->component;
    ci->component = NULL;
    {
      ui_error_t rc_cleanup = ui_camera_base_destroy(cam);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_component_destroy(saved);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  {
    ui_error_t rc_cleanup = ui_camera_base_create(&cam);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_camera_base_request_permission(cam);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_camera_base_mock_permission_response(cam, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_camera_base_set_frame_callback(cam, test_frame_callback, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_camera_base_start_stream(cam);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_camera_base_mock_frame(cam);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_camera_base_destroy(cam);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}
int main(void) {
  test_camera_coverage();
  int failed = 0;
  printf("Running ui_camera_base tests...\n");

  failed |= test_camera_lifecycle();
  failed |= test_camera_null_args_and_coverage();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
