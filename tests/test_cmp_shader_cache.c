/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

TEST test_shader_cache_create_destroy(void) {
  cmp_shader_cache_t *cache = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_create(&cache));
  ASSERT_NEQ(NULL, cache);

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_destroy(cache));
  PASS();
}

TEST test_shader_cache_store_retrieve(void) {
  cmp_shader_cache_t *cache = NULL;
  cmp_shader_t shader1;
  cmp_shader_t shader2;
  cmp_shader_t *retrieved1 = NULL;
  cmp_shader_t *retrieved2 = NULL;
  cmp_shader_t *missing = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_create(&cache));

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_init_rounded_rect(&shader1));
  ASSERT_EQ(CMP_SUCCESS, cmp_shader_init_gradient(&shader2));

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_store(cache, "rounded", &shader1));
  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_store(cache, "gradient", &shader2));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_shader_cache_retrieve(cache, "rounded", &retrieved1));
  ASSERT_NEQ(NULL, retrieved1);
  ASSERT_EQ(1, *((int *)retrieved1->internal_handle));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_shader_cache_retrieve(cache, "gradient", &retrieved2));
  ASSERT_NEQ(NULL, retrieved2);
  ASSERT_EQ(2, *((int *)retrieved2->internal_handle));

  ASSERT_EQ(CMP_ERROR_NOT_FOUND,
            cmp_shader_cache_retrieve(cache, "missing", &missing));
  ASSERT_EQ(NULL, missing);

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_destroy(cache));
  PASS();
}

TEST test_shader_cache_save_load(void) {
  cmp_shader_cache_t *cache1 = NULL;
  cmp_shader_cache_t *cache2 = NULL;
  cmp_shader_t shader1;
  cmp_shader_t shader2;
  cmp_shader_t shader3;
  cmp_shader_t *retrieved1 = NULL;
  cmp_shader_t *retrieved2 = NULL;
  cmp_shader_t *retrieved3 = NULL;
  const char *test_file = "test_shader_cache.bin";

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_create(&cache1));

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_init_rounded_rect(&shader1));
  ASSERT_EQ(CMP_SUCCESS, cmp_shader_init_gradient(&shader2));
  ASSERT_EQ(CMP_SUCCESS, cmp_shader_init_sdf_text(&shader3));

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_store(cache1, "key1", &shader1));
  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_store(cache1, "key2", &shader2));
  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_store(cache1, "key3", &shader3));

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_save_to_disk(cache1, test_file));
  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_destroy(cache1));

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_create(&cache2));
  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_load_from_disk(cache2, test_file));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_shader_cache_retrieve(cache2, "key1", &retrieved1));
  ASSERT_NEQ(NULL, retrieved1);
  ASSERT_EQ(1, *((int *)retrieved1->internal_handle));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_shader_cache_retrieve(cache2, "key2", &retrieved2));
  ASSERT_NEQ(NULL, retrieved2);
  ASSERT_EQ(2, *((int *)retrieved2->internal_handle));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_shader_cache_retrieve(cache2, "key3", &retrieved3));
  ASSERT_NEQ(NULL, retrieved3);
  ASSERT_EQ(3, *((int *)retrieved3->internal_handle));

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_destroy(cache2));
  remove(test_file);
  PASS();
}

TEST test_shader_cache_edge_cases(void) {
  cmp_shader_cache_t *cache = NULL;
  cmp_shader_t shader;
  cmp_shader_t *retrieved_ptr = NULL;
  cmp_shader_t *retrieved = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_shader_cache_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_shader_cache_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_shader_cache_store(NULL, "key", &shader));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_shader_cache_retrieve(NULL, "key", &retrieved_ptr));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_shader_cache_save_to_disk(NULL, "file"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_shader_cache_load_from_disk(NULL, "file"));

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_create(&cache));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_shader_cache_store(cache, NULL, &shader));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_shader_cache_store(cache, "key", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_shader_cache_retrieve(cache, NULL, &retrieved));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_shader_cache_retrieve(cache, "key", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_shader_cache_save_to_disk(cache, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_shader_cache_load_from_disk(cache, NULL));

  /* Load from missing file */
  ASSERT_EQ(CMP_ERROR_IO, cmp_shader_cache_load_from_disk(
                              cache, "this_file_does_not_exist.bin"));

  ASSERT_EQ(CMP_SUCCESS, cmp_shader_cache_destroy(cache));
  PASS();
}

SUITE(cmp_shader_cache_suite) {
  RUN_TEST(test_shader_cache_create_destroy);
  RUN_TEST(test_shader_cache_store_retrieve);
  RUN_TEST(test_shader_cache_save_load);
  RUN_TEST(test_shader_cache_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_shader_cache_suite);
  GREATEST_MAIN_END();
}
