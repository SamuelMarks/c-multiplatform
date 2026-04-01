/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

typedef struct cmp_symbol_node {
  char *name;
  int weight;
  cmp_symbol_scale_t scale;

  cmp_symbol_rendering_mode_t rendering_mode;
  uint32_t *colors;
  size_t color_count;

  float variable_fill;

  cmp_symbol_animation_t active_animation;
  int is_looping;
} cmp_symbol_node_t;

TEST test_sf_symbols(void) {
  cmp_symbols_t *ctx = NULL;
  void *handle = NULL;
  cmp_symbol_node_t *node;
  uint32_t palette[3] = {0xFF0000FF, 0x00FF00FF, 0x0000FFFF};

  ASSERT_EQ(CMP_SUCCESS, cmp_symbols_create(&ctx));

  ASSERT_EQ(CMP_SUCCESS, cmp_symbols_request(ctx, "star.fill", 600,
                                             CMP_SYMBOL_SCALE_LARGE, &handle));
  node = (cmp_symbol_node_t *)handle;
  ASSERT_STR_EQ("star.fill", node->name);
  ASSERT_EQ(600, node->weight);
  ASSERT_EQ(CMP_SYMBOL_SCALE_LARGE, node->scale);

  /* Style / Rendering mode test */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_symbols_apply_style(ctx, handle, CMP_SYMBOL_RENDERING_PALETTE,
                                    palette, 3));
  ASSERT_EQ(CMP_SYMBOL_RENDERING_PALETTE, node->rendering_mode);
  ASSERT_EQ(3, node->color_count);
  ASSERT_EQ(0xFF0000FF, node->colors[0]);
  ASSERT_EQ(0x00FF00FF, node->colors[1]);
  ASSERT_EQ(0x0000FFFF, node->colors[2]);

  /* Variable color */
  ASSERT_EQ(CMP_SUCCESS, cmp_symbols_set_variable_value(ctx, handle, 0.45f));
  ASSERT_EQ(0.45f, node->variable_fill);

  /* Bounds logic clamp */
  ASSERT_EQ(CMP_SUCCESS, cmp_symbols_set_variable_value(ctx, handle, 2.5f));
  ASSERT_EQ(1.0f, node->variable_fill);

  /* Animations */
  ASSERT_EQ(CMP_SUCCESS, cmp_symbols_trigger_animation(
                             ctx, handle, CMP_SYMBOL_ANIM_BOUNCE, 1));
  ASSERT_EQ(CMP_SYMBOL_ANIM_BOUNCE, node->active_animation);
  ASSERT_EQ(1, node->is_looping);

  if (node->colors)
    CMP_FREE(node->colors);
  CMP_FREE(node->name);
  CMP_FREE(node);

  ASSERT_EQ(CMP_SUCCESS, cmp_symbols_destroy(ctx));
  PASS();
}

TEST test_custom_svg_templates(void) {
  cmp_symbols_t *ctx = NULL;
  void *handle = NULL;
  cmp_symbol_node_t *node;

  ASSERT_EQ(CMP_SUCCESS, cmp_symbols_create(&ctx));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_symbols_load_custom_template(ctx, "res/custom.svg", &handle));

  node = (cmp_symbol_node_t *)handle;
  ASSERT_STR_EQ("res/custom.svg", node->name);
  ASSERT_EQ(400, node->weight);

  CMP_FREE(node->name);
  CMP_FREE(node);
  ASSERT_EQ(CMP_SUCCESS, cmp_symbols_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_symbols_t *ctx = NULL;
  void *handle = NULL;
  cmp_symbol_node_t *node = NULL;
  uint32_t colors[1] = {0xFFFFFFFF};

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_symbols_create(NULL));
  cmp_symbols_create(&ctx);

  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_symbols_request(NULL, "star", 400, CMP_SYMBOL_SCALE_MEDIUM, &handle));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_symbols_request(ctx, NULL, 400, CMP_SYMBOL_SCALE_MEDIUM, &handle));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_symbols_request(ctx, "star", 400, CMP_SYMBOL_SCALE_MEDIUM, NULL));

  cmp_symbols_request(ctx, "star", 400, CMP_SYMBOL_SCALE_MEDIUM, &handle);
  node = (cmp_symbol_node_t *)handle;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_symbols_apply_style(
                NULL, handle, CMP_SYMBOL_RENDERING_MONOCHROME, colors, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_symbols_apply_style(ctx, NULL, CMP_SYMBOL_RENDERING_MONOCHROME,
                                    colors, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_symbols_apply_style(ctx, handle,
                                    CMP_SYMBOL_RENDERING_MONOCHROME, NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_symbols_set_variable_value(NULL, handle, 0.5f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_symbols_set_variable_value(ctx, NULL, 0.5f));

  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_symbols_trigger_animation(NULL, handle, CMP_SYMBOL_ANIM_BOUNCE, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_symbols_trigger_animation(
                                       ctx, NULL, CMP_SYMBOL_ANIM_BOUNCE, 0));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_symbols_load_custom_template(NULL, "path", &handle));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_symbols_load_custom_template(ctx, NULL, &handle));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_symbols_load_custom_template(ctx, "path", NULL));

  CMP_FREE(node->name);
  CMP_FREE(node);
  cmp_symbols_destroy(ctx);
  PASS();
}

SUITE(symbols_suite) {
  RUN_TEST(test_sf_symbols);
  RUN_TEST(test_custom_svg_templates);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(symbols_suite);
  GREATEST_MAIN_END();
}
