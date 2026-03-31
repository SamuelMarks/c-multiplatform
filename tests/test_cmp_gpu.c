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
  cmp_draw_call_t call1 = {1, 1, 0, 0, 10};
  cmp_draw_call_t call2 = {1, 1, 0, 10, 20};
  cmp_draw_call_t call3 = {2, 1, 0, 30, 5};
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_create(&opt));
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_add(opt, &call1));
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_add(opt, &call2));
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_add(opt, &call3));
  ASSERT_EQ(3, opt->count);
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_optimize(opt));
  ASSERT_EQ(2, opt->count);
  ASSERT_EQ(30, opt->calls[0].vertex_count);
  ASSERT_EQ(5, opt->calls[1].vertex_count);
  ASSERT_EQ(CMP_SUCCESS, cmp_draw_call_optimizer_destroy(opt));
  PASS();
}
SUITE(cmp_gpu_suite) {
  RUN_TEST(test_gpu_create);
  RUN_TEST(test_vbo);
  RUN_TEST(test_ubo);
  RUN_TEST(test_draw_call_optimizer);
}
GREATEST_MAIN_DEFS();
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_gpu_suite);
  GREATEST_MAIN_END();
}
