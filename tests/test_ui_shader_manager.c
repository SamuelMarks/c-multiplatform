/* clang-format off */
#include <stdio.h>
#include "../include/ui_shader_manager.h"
#include "../include/ui_error.h"
/* clang-format on */

extern int g_mock_shader_fail;
extern int g_mock_program_fail;
extern int g_malloc_fail_countdown;

int main(void) {
  struct ui_shader_manager *manager = NULL;
  ui_error_t rc;
  unsigned int program_id1;
  unsigned int program_id2;
  int failed = 0;

  /* Null checks */
  failed |= (ui_shader_manager_create(NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_shader_manager_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT);

  g_malloc_fail_countdown = 0;
  failed |= (ui_shader_manager_create(&manager) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  rc = ui_shader_manager_create(&manager);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create shader manager.\n");
    return 1;
  }

  failed |=
      (ui_shader_manager_get_program(NULL, "test", "v", "f", &program_id1) !=
       UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_shader_manager_get_program(manager, NULL, "v", "f", &program_id1) !=
       UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_shader_manager_get_program(manager, "test", NULL, "f",
                                           &program_id1) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_shader_manager_get_program(manager, "test", "v", NULL,
                                           &program_id1) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_shader_manager_get_program(manager, "test", "v", "f", NULL) !=
             UI_ERROR_INVALID_ARGUMENT);

  /* Compile failures */
  /* We know ID counter starts at 1, so vertex is 1, frag is 2 for the first
   * program. */
  g_mock_shader_fail = 1; /* fail vertex */
  failed |= (ui_shader_manager_get_program(manager, "test_v_fail", "v", "f",
                                           &program_id1) != UI_ERROR_UNKNOWN);

  g_mock_shader_fail = 3; /* fail fragment (since vertex got ID 2) */
  failed |= (ui_shader_manager_get_program(manager, "test_f_fail", "v", "f",
                                           &program_id1) != UI_ERROR_UNKNOWN);
  g_mock_shader_fail = 0;

  /* Link failure */
  g_mock_program_fail = 1;
  failed |= (ui_shader_manager_get_program(manager, "test_p_fail", "v", "f",
                                           &program_id1) != UI_ERROR_UNKNOWN);
  g_mock_program_fail = 0;

  /* Cache entry OOM */
  g_malloc_fail_countdown = 0;
  failed |=
      (ui_shader_manager_get_program(manager, "test_oom", "v", "f",
                                     &program_id1) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  rc = ui_shader_manager_get_program(manager, "test_shader", "vertex",
                                     "fragment", &program_id1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to get/compile program.\n");
    return 1;
  }

  rc = ui_shader_manager_get_program(manager, "test_shader2", "vertex2",
                                     "fragment2", &program_id2);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to get/compile program2.\n");
    return 1;
  }

  /* Getting the first shader should traverse past test_shader2 in the linked
   * list */
  rc = ui_shader_manager_get_program(manager, "test_shader", "vertex",
                                     "fragment", &program_id2);
  if (rc != UI_ERROR_NONE || program_id1 != program_id2) {
    printf("Failed cache check.\n");
    return 1;
  }

  /* Null checks for uniforms */
  float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  failed |= (ui_shader_manager_set_uniform_matrix(
                 NULL, program_id1, "u", matrix) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_shader_manager_set_uniform_matrix(
           manager, program_id1, NULL, matrix) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_shader_manager_set_uniform_matrix(
                 manager, program_id1, "u", NULL) != UI_ERROR_INVALID_ARGUMENT);

  failed |=
      (ui_shader_manager_set_uniform_color(NULL, program_id1, "u", 1, 1, 1,
                                           1) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_shader_manager_set_uniform_color(manager, program_id1, NULL, 1, 1, 1,
                                           1) != UI_ERROR_INVALID_ARGUMENT);

  failed |= (ui_shader_manager_set_uniform_float(NULL, program_id1, "u", 1) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_shader_manager_set_uniform_float(
                 manager, program_id1, NULL, 1) != UI_ERROR_INVALID_ARGUMENT);

  /* Test uniform injection */
  {
    rc = ui_shader_manager_set_uniform_matrix(manager, program_id1, "u_matrix",
                                              matrix);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to set uniform matrix.\n");
      return 1;
    }
    rc = ui_shader_manager_set_uniform_color(manager, program_id1, "u_color",
                                             1.0f, 0.0f, 0.0f, 1.0f);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to set uniform color.\n");
      return 1;
    }
    rc = ui_shader_manager_set_uniform_float(manager, program_id1, "u_time",
                                             1.5f);
    if (rc != UI_ERROR_NONE) {
      printf("Failed to set uniform float.\n");
      return 1;
    }
  }

  /* Uniform failing location lookup */
  {
    float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    (void)ui_shader_manager_set_uniform_matrix(manager, program_id1,
                                               "invalid_u", matrix);
    (void)ui_shader_manager_set_uniform_color(manager, program_id1, "invalid_u",
                                              1.0f, 0.0f, 0.0f, 1.0f);
    (void)ui_shader_manager_set_uniform_float(manager, program_id1, "invalid_u",
                                              1.5f);
  }

  rc = ui_shader_manager_destroy(manager);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to destroy shader manager.\n");
    return 1;
  }

  printf("ui_shader_manager tests finished.\n");
  return failed;
}
