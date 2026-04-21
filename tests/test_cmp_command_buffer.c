/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_command_buffer_creation(void) {
  cmp_gpu_t *gpu = NULL;
  cmp_command_buffer_t *cb = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_create(CMP_BACKEND_CPU_SOFTWARE, &gpu));

  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_create(gpu, 0, &cb));
  ASSERT_NEQ(NULL, cb);

  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_destroy(cb));
  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_destroy(gpu));
  PASS();
}

TEST test_command_buffer_recording(void) {
  cmp_gpu_t *gpu = NULL;
  cmp_command_buffer_t *cb = NULL;
  cmp_draw_call_t draw_call;

  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_create(CMP_BACKEND_CPU_SOFTWARE, &gpu));
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_create(gpu, 1, &cb));

  memset(&draw_call, 0, sizeof(draw_call));
  draw_call.shader_id = 42;
  draw_call.vertex_count = 100;

  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_begin(cb));
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_draw(cb, &draw_call));
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_end(cb));

  ASSERT_EQ(CMP_ERROR_INVALID_STATE, cmp_command_buffer_draw(cb, &draw_call));

  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_destroy(cb));
  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_destroy(gpu));
  PASS();
}

TEST test_command_buffer_execution(void) {
  cmp_gpu_t *gpu = NULL;
  cmp_command_buffer_t *primary = NULL;
  cmp_command_buffer_t *secondary = NULL;
  cmp_draw_call_t draw_call;

  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_create(CMP_BACKEND_CPU_SOFTWARE, &gpu));
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_create(gpu, 0, &primary));
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_create(gpu, 1, &secondary));

  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_begin(secondary));
  memset(&draw_call, 0, sizeof(draw_call));
  draw_call.shader_id = 42;
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_draw(secondary, &draw_call));
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_end(secondary));

  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_begin(primary));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_command_buffer_execute_commands(primary, &secondary, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_end(primary));

  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_destroy(secondary));
  ASSERT_EQ(CMP_SUCCESS, cmp_command_buffer_destroy(primary));
  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_destroy(gpu));
  PASS();
}

SUITE(cmp_command_buffer_suite) {
  RUN_TEST(test_command_buffer_creation);
  RUN_TEST(test_command_buffer_recording);
  RUN_TEST(test_command_buffer_execution);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_command_buffer_suite);
  GREATEST_MAIN_END();
}
