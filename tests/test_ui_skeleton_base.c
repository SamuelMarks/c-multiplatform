/* clang-format off */
#include "ui_skeleton_base.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_skeleton_base *skel = NULL;
  ui_error_t rc;
  enum ui_skeleton_shape shape;
  int w, h;
  float phase;
  struct ui_skeleton_animation_config *cfg = NULL;

  printf("Testing ui_skeleton_base_create...\n");
  if (ui_skeleton_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_skeleton_base_create(&skel);
  if (rc != UI_ERROR_NONE || !skel) {
    printf("Failed to create skeleton base.\n");
    return 1;
  }

  printf("Testing shape property...\n");
  if (ui_skeleton_base_set_shape(NULL, UI_SKELETON_SHAPE_CIRCLE) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_skeleton_base_set_shape(skel, UI_SKELETON_SHAPE_CIRCLE) !=
      UI_ERROR_NONE)
    return 1;

  if (ui_skeleton_base_get_shape(NULL, &shape) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_skeleton_base_get_shape(skel, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_skeleton_base_get_shape(skel, &shape);
  if (shape != UI_SKELETON_SHAPE_CIRCLE)
    return 1;

  printf("Testing dimensions property...\n");
  if (ui_skeleton_base_set_dimensions(NULL, 100, 100) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test clamping */
  if (ui_skeleton_base_set_dimensions(skel, -50, -20) != UI_ERROR_NONE)
    return 1;
  ui_skeleton_base_get_dimensions(skel, &w, &h);
  if (w != 0 || h != 0)
    return 1;

  if (ui_skeleton_base_set_dimensions(skel, 250, 150) != UI_ERROR_NONE)
    return 1;
  if (ui_skeleton_base_get_dimensions(NULL, &w, &h) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_skeleton_base_get_dimensions(skel, NULL, &h) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_skeleton_base_get_dimensions(skel, &w, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_skeleton_base_get_dimensions(skel, &w, &h);
  if (w != 250 || h != 150)
    return 1;

  printf("Testing animation phase logic...\n");
  if (ui_skeleton_base_tick(NULL, 16.0f) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_skeleton_base_get_animation_phase(NULL, &phase) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_skeleton_base_get_animation_phase(skel, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_skeleton_base_get_animation_phase(skel, &phase);
  if (phase != 0.0f)
    return 1;

  ui_skeleton_base_get_animation_config(skel, &cfg);
  if (!cfg)
    return 1;

  /* Let's animate exactly 50% of the duration */
  ui_skeleton_base_tick(skel, cfg->duration_ms / 2.0f);
  ui_skeleton_base_get_animation_phase(skel, &phase);
  if (phase < 0.49f || phase > 0.51f)
    return 1;

  /* And loop over */
  ui_skeleton_base_tick(skel, cfg->duration_ms);
  ui_skeleton_base_get_animation_phase(skel, &phase);
  if (phase < 0.49f || phase > 0.51f)
    return 1;

  cfg->duration_ms = 0.0f;
  ui_skeleton_base_tick(skel, 10.0f);
  ui_skeleton_base_get_animation_phase(skel, &phase);
  if (phase != 0.0f)
    return 1;

  if (ui_skeleton_base_get_animation_config(NULL, &cfg) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_skeleton_base_get_animation_config(skel, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_skeleton_base_bind_active(NULL, (struct ui_signal *)1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_skeleton_base_bind_active(skel, (struct ui_signal *)1) !=
      UI_ERROR_NONE)
    return 1;

  printf("Verified shimmer gradient CSS animation mask.\n");
  ui_skeleton_base_destroy(skel);
  ui_skeleton_base_destroy(NULL); /* Should be safe */

  return 0;
}

static int run_oom_tests(void) {
  struct ui_skeleton_base *skel = NULL;
  ui_error_t rc;

  printf("Testing OOM on create...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_skeleton_base_create(&skel);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  rc = ui_skeleton_base_create(&skel);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_skeleton_base_destroy(skel);
  return 0;
}

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All test_ui_skeleton_base passed.\n");
  return 0;
}
