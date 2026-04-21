#include "cmp.h"
#include "greatest.h"
#include <string.h>

TEST test_pso_caching(void) {
  cmp_pso_cache_t *cache = NULL;
  cmp_pipeline_state_t state1;
  cmp_pipeline_state_t state2;
  cmp_pso_t *pso1 = NULL;
  cmp_pso_t *pso2 = NULL;
  cmp_pso_t *pso3 = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_pso_cache_create(&cache));

  memset(&state1, 0, sizeof(state1));
  state1.blend_mode = 1;
  state1.depth_test_enable = 0;

  memset(&state2, 0, sizeof(state2));
  state2.blend_mode = 2;
  state2.depth_test_enable = 1;

  /* Miss -> Create */
  ASSERT_EQ(CMP_SUCCESS, cmp_pso_cache_get_or_create(cache, &state1, &pso1));
  ASSERT_NEQ(NULL, pso1);

  /* Miss -> Create (different state) */
  ASSERT_EQ(CMP_SUCCESS, cmp_pso_cache_get_or_create(cache, &state2, &pso2));
  ASSERT_NEQ(NULL, pso2);
  ASSERT_NEQ(pso1, pso2);

  /* Hit -> Retrieve cached */
  ASSERT_EQ(CMP_SUCCESS, cmp_pso_cache_get_or_create(cache, &state1, &pso3));
  ASSERT_EQ(pso1, pso3); /* Exactly the same pointer */

  ASSERT_EQ(CMP_SUCCESS, cmp_pso_cache_destroy(cache));
  PASS();
}

TEST test_shader_compilation_mock(void) {
  cmp_shader_t *shader_spirv = NULL;
  cmp_shader_t *shader_msl = NULL;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_shader_compile_spirv("fake_vulkan", 11, &shader_spirv));
  ASSERT_NEQ(NULL, shader_spirv);
  ASSERT_NEQ(NULL, shader_spirv->internal_handle);

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_compile_msl("fake_metal", 10, &shader_msl));
  ASSERT_NEQ(NULL, shader_msl);

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_destroy(shader_spirv));
  ASSERT_EQ(CMP_SUCCESS, cmp_shader_destroy(shader_msl));
  PASS();
}

SUITE(cmp_pso_cache_suite) {
  RUN_TEST(test_pso_caching);
  RUN_TEST(test_shader_compilation_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_pso_cache_suite);
  GREATEST_MAIN_END();
}
