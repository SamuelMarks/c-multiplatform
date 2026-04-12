#include "cmp.h"
#include "greatest.h"

TEST test_gpu_allocator(void) {
  cmp_gpu_t *gpu = NULL;
  cmp_gpu_allocator_t *alloc = NULL;
  void *mem1 = NULL, *mem2 = NULL;
  size_t offset1 = 0, offset2 = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_create(CMP_BACKEND_CPU_SOFTWARE, &gpu));
  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_allocator_create(gpu, 1024, &alloc));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_gpu_allocator_alloc(alloc, 256, 16, &mem1, &offset1));
  ASSERT_NEQ(NULL, mem1);
  ASSERT_EQ(0, offset1);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_gpu_allocator_alloc(alloc, 512, 16, &mem2, &offset2));
  ASSERT_NEQ(NULL, mem2);
  ASSERT_EQ(256, offset2);

  /* Over allocate */
  ASSERT_EQ(CMP_ERROR_OOM,
            cmp_gpu_allocator_alloc(alloc, 512, 16, &mem1, &offset1));

  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_allocator_destroy(alloc));
  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_destroy(gpu));
  PASS();
}

TEST test_dynamic_atlas(void) {
  cmp_gpu_t *gpu = NULL;
  cmp_atlas_t *atlas = NULL;
  int x, y;
  char mock_pixels[64 * 64 * 4];

  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_create(CMP_BACKEND_CPU_SOFTWARE, &gpu));
  ASSERT_EQ(CMP_SUCCESS, cmp_atlas_create(gpu, 2048, 2048, &atlas));

  ASSERT_EQ(CMP_SUCCESS, cmp_atlas_insert(atlas, 64, 64, mock_pixels, &x, &y));
  ASSERT_EQ(0, x);
  ASSERT_EQ(0, y);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_atlas_insert(atlas, 128, 128, mock_pixels, &x, &y));
  ASSERT_EQ(64, x);
  ASSERT_EQ(0, y);

  ASSERT_EQ(CMP_SUCCESS, cmp_atlas_evict(atlas));

  ASSERT_EQ(CMP_SUCCESS, cmp_atlas_insert(atlas, 64, 64, mock_pixels, &x, &y));
  ASSERT_EQ(0, x);
  ASSERT_EQ(0, y);

  ASSERT_EQ(CMP_SUCCESS, cmp_atlas_destroy(atlas));
  ASSERT_EQ(CMP_SUCCESS, cmp_gpu_destroy(gpu));
  PASS();
}

TEST test_texture_compression(void) {
  void *rgba = NULL;
  int w, h;
  char mock_data[1024];

  ASSERT_EQ(CMP_SUCCESS,
            cmp_tex_compression_decode_astc(mock_data, 1024, &rgba, &w, &h));
  ASSERT_NEQ(NULL, rgba);
  ASSERT_EQ(64, w);
  ASSERT_EQ(64, h);
  CMP_FREE(rgba);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_tex_compression_decode_bc7(mock_data, 1024, &rgba, &w, &h));
  ASSERT_NEQ(NULL, rgba);
  ASSERT_EQ(64, w);
  ASSERT_EQ(64, h);
  CMP_FREE(rgba);

  PASS();
}

SUITE(cmp_gpu_allocator_suite) {
  RUN_TEST(test_gpu_allocator);
  RUN_TEST(test_dynamic_atlas);
  RUN_TEST(test_texture_compression);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_gpu_allocator_suite);
  GREATEST_MAIN_END();
}
