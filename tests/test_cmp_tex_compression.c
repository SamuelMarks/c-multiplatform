/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_tex_compression_create_destroy(void) {
  cmp_tex_compression_t *comp = NULL;
  uint8_t mock_data[64] = {0};

  ASSERT_EQ(CMP_SUCCESS,
            cmp_tex_compression_create(CMP_TEX_COMPRESSION_ASTC, 128, 128,
                                       mock_data, sizeof(mock_data), &comp));
  ASSERT_NEQ(NULL, comp);

  ASSERT_EQ(CMP_SUCCESS, cmp_tex_compression_destroy(comp));
  PASS();
}

TEST test_tex_compression_mount(void) {
  cmp_tex_compression_t *comp = NULL;
  cmp_texture_t tex = {0};
  uint8_t mock_data[64] = {0};

  ASSERT_EQ(CMP_SUCCESS,
            cmp_tex_compression_create(CMP_TEX_COMPRESSION_BC7, 256, 256,
                                       mock_data, sizeof(mock_data), &comp));

  tex.internal_handle = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_tex_compression_mount(comp, &tex));

  ASSERT_EQ(256, tex.width);
  ASSERT_EQ(256, tex.height);
  ASSERT_EQ((int)CMP_TEX_COMPRESSION_BC7, tex.format);
  ASSERT_NEQ(NULL, tex.internal_handle); /* Internal mock should be populated */

  ASSERT_EQ(CMP_SUCCESS, cmp_tex_compression_destroy(comp));
  PASS();
}

TEST test_tex_compression_edge_cases(void) {
  cmp_tex_compression_t *comp = NULL;
  cmp_texture_t tex;
  uint8_t mock_data[64] = {0};

  /* Null parameter cases */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tex_compression_create(CMP_TEX_COMPRESSION_ASTC, 100, 100,
                                       mock_data, 64, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tex_compression_create(CMP_TEX_COMPRESSION_ASTC, 100, 100, NULL,
                                       64, &comp));

  /* Zero dimensions or size */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tex_compression_create(CMP_TEX_COMPRESSION_ASTC, 0, 100,
                                       mock_data, 64, &comp));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tex_compression_create(CMP_TEX_COMPRESSION_ASTC, 100, 0,
                                       mock_data, 64, &comp));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tex_compression_create(CMP_TEX_COMPRESSION_ASTC, 100, 100,
                                       mock_data, 0, &comp));

  /* Destroy null */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tex_compression_destroy(NULL));

  /* Mount null */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_tex_compression_create(CMP_TEX_COMPRESSION_ETC2, 64, 64,
                                       mock_data, 64, &comp));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tex_compression_mount(NULL, &tex));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_tex_compression_mount(comp, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_tex_compression_destroy(comp));

  /* Mount NONE type */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_tex_compression_create(CMP_TEX_COMPRESSION_NONE, 32, 32,
                                       mock_data, 64, &comp));
  tex.internal_handle =
      (void *)(size_t)0x1234; /* Prevents auto mock assignment */
  ASSERT_EQ(CMP_SUCCESS, cmp_tex_compression_mount(comp, &tex));
  ASSERT_EQ(0, tex.format);
  ASSERT_EQ(32, tex.width);
  ASSERT_EQ(32, tex.height);
  ASSERT_EQ(CMP_SUCCESS, cmp_tex_compression_destroy(comp));

  PASS();
}

SUITE(cmp_tex_compression_suite) {
  RUN_TEST(test_tex_compression_create_destroy);
  RUN_TEST(test_tex_compression_mount);
  RUN_TEST(test_tex_compression_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_tex_compression_suite);
  GREATEST_MAIN_END();
}
