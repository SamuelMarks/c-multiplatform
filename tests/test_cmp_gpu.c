/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */
TEST test_gpu_create(void) {
  cmp_gpu_t *gpu = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_create(CMP_BACKEND_CPU_SOFTWARE, &gpu));
  ASSERT_NEQ(NULL, gpu);
  ASSERT_EQ(CMP_BACKEND_CPU_SOFTWARE, gpu->backend);
  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_destroy(gpu));
  PASS();
}
TEST test_gpu_begin_end_frame(void) {
  cmp_gpu_t *gpu = NULL;
  int *state = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_create(CMP_BACKEND_CPU_SOFTWARE, &gpu));

  /* Validate initialization state */
  state = (int *)gpu->context;
  ASSERT_NEQ(NULL, state);
  ASSERT_EQ(0, *state);

  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_begin_frame(gpu));
  ASSERT_EQ(1, *state); /* Validates vtable mapping logic executed */

  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_end_frame(gpu));
  ASSERT_EQ(2, *state); /* Validates vtable mapping logic executed */

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_gpu_begin_frame(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_gpu_end_frame(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_destroy(gpu));
  PASS();
}
TEST test_vbo(void) {
  cmp_vbo_t *vbo = NULL;
  float data[3] = {1.0f, 2.0f, 3.0f};
  ASSERT_EQ(CMP_SUCCESS, cmp_vbo_create(&vbo));
  ASSERT_EQ(CMP_SUCCESS, cmp_vbo_append(vbo, data, 3));
  ASSERT_EQ(3, vbo->count);
  ASSERT_EQ(1.0f, vbo->data[0]);
  ASSERT_EQ(CMP_SUCCESS, cmp_vbo_destroy(vbo));
  PASS();
}
TEST test_ubo(void) {
  cmp_ubo_t *ubo = NULL;
  float data = 42.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_ubo_create(sizeof(float), &ubo));
  ASSERT_EQ(sizeof(float), ubo->size);
  ASSERT_EQ(CMP_SUCCESS, cmp_ubo_update(ubo, &data, sizeof(float)));
  ASSERT_EQ(42.0f, *(float *)ubo->data);
  ASSERT_EQ(CMP_SUCCESS, cmp_ubo_destroy(ubo));
  PASS();
}
TEST test_draw_call_optimizer(void) {
  cmp_draw_call_optimizer_t *opt = NULL;
  cmp_draw_call_t call1 = {1, 1, 0, 0, 10, 0, {0, 0, 0, 0}};
  cmp_draw_call_t call2 = {1, 1, 0, 10, 20, 0, {0, 0, 0, 0}};
  cmp_draw_call_t call3 = {2, 1, 0, 30, 5, 0, {0, 0, 0, 0}};
  cmp_draw_call_t call4 = {2, 1, 0, 35, 10, 1, {0, 0, 100, 100}};
  cmp_draw_call_t call5 = {2, 1, 0, 45, 15, 1, {0, 0, 100, 100}};
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_create(&opt));
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_add(opt, &call1));
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_add(opt, &call2));
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_add(opt, &call3));
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_add(opt, &call4));
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_add(opt, &call5));
  ASSERT_EQ(5, opt->count);
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_optimize(opt));
  ASSERT_EQ(3, opt->count);
  ASSERT_EQ(30, opt->calls[0].vertex_count);
  ASSERT_EQ(5, opt->calls[1].vertex_count);
  ASSERT_EQ(25, opt->calls[2].vertex_count);
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_destroy(opt));
  PASS();
}

TEST test_frustum_culling(void) {
  cmp_rect_f_t vp = {0, 0, 1920, 1080};
  cmp_rect_f_t inside = {100, 100, 200, 200};
  cmp_rect_f_t outside = {-500, -500, 100, 100};
  cmp_rect_f_t partial = {-50, -50, 100, 100};
  int visible = 0;
  ASSERT_EQ(CMP_SUCCESS, cmp_frustum_culling_test(&inside, &vp, &visible));
  ASSERT_EQ(1, visible);
  ASSERT_EQ(CMP_SUCCESS, cmp_frustum_culling_test(&outside, &vp, &visible));
  ASSERT_EQ(0, visible);
  ASSERT_EQ(CMP_SUCCESS, cmp_frustum_culling_test(&partial, &vp, &visible));
  ASSERT_EQ(1, visible);
  PASS();
}

SUITE(cmp_gpu_suite) {
  RUN_TEST(test_gpu_create);
  RUN_TEST(test_gpu_begin_end_frame);
  RUN_TEST(test_vbo);
  RUN_TEST(test_ubo);
  RUN_TEST(test_draw_call_optimizer);
  RUN_TEST(test_frustum_culling);
}
GREATEST_MAIN_DEFS();
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_gpu_suite);
  GREATEST_MAIN_END();
}
