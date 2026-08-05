/* clang-format off */
#include <stdio.h>
#include "../include/ui_video_shaders.h"
#include "../include/ui_shader_manager.h"
#include "../include/ui_error.h"
/* clang-format on */

int main(void) {
  struct ui_shader_manager *manager = NULL;
  ui_error_t rc;
  unsigned int program_id1;
  unsigned int program_id2;
  int failed = 0;

  rc = ui_shader_manager_create(&manager);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_video_shaders_get_yuv_program(manager, &program_id1);
  failed |= (rc != UI_ERROR_NONE);

  /* Getting the same shader should hit the cache and return the exact same
   * program ID */
  rc = ui_video_shaders_get_yuv_program(manager, &program_id2);
  failed |= (rc != UI_ERROR_NONE || program_id1 != program_id2);

  /* Invalid arguments */
  rc = ui_video_shaders_get_yuv_program(NULL, &program_id1);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_video_shaders_get_yuv_program(manager, NULL);
  failed |= (rc != UI_ERROR_INVALID_ARGUMENT);

  rc = ui_shader_manager_destroy(manager);
  failed |= (rc != UI_ERROR_NONE);

  if (!failed) {
    printf("test_ui_video_shaders passed.\n");
  }
  return failed;
}
