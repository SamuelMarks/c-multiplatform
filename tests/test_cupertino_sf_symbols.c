/**
 * @file test_cupertino_sf_symbols.c
 * @brief Unit tests for Apple SF Symbols Vector Engine and Optical Weight
 * Matching.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_sf_symbols.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_sf_symbols_suite);

TEST test_symbols_invalid_arguments(void) {
  struct cupertino_symbol_descriptor desc;
  struct cupertino_symbol *sym = NULL;
  struct cupertino_symbol_layer layer;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float w, h;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_symbol_create(NULL, &desc, &sym);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_create(dummy_engine, NULL, &sym);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range mode / weight / size */
  desc.rendering_mode = (enum cupertino_symbol_rendering_mode) - 1;
  rc = cupertino_symbol_create(dummy_engine, &desc, &sym);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.rendering_mode = (enum cupertino_symbol_rendering_mode)99;
  rc = cupertino_symbol_create(dummy_engine, &desc, &sym);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.rendering_mode = CUPERTINO_SYMBOL_RENDERING_MONOCHROME;
  desc.weight = (enum cupertino_symbol_weight) - 1;
  rc = cupertino_symbol_create(dummy_engine, &desc, &sym);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.weight = (enum cupertino_symbol_weight)99;
  rc = cupertino_symbol_create(dummy_engine, &desc, &sym);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.weight = CUPERTINO_SYMBOL_WEIGHT_REGULAR;
  desc.size = (enum cupertino_symbol_size) - 1;
  rc = cupertino_symbol_create(dummy_engine, &desc, &sym);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  desc.size = (enum cupertino_symbol_size)99;
  rc = cupertino_symbol_create(dummy_engine, &desc, &sym);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_symbol_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create valid symbol for method invalid checks */
  desc.size = CUPERTINO_SYMBOL_SIZE_MEDIUM;
  rc = cupertino_symbol_create(dummy_engine, &desc, &sym);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sym != NULL);

  /* Setters invalid */
  rc = cupertino_symbol_set_name(NULL, "gear");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_set_name(sym, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_symbol_set_rendering_mode(
      NULL, CUPERTINO_SYMBOL_RENDERING_HIERARCHICAL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_set_rendering_mode(
      sym, (enum cupertino_symbol_rendering_mode) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_set_rendering_mode(
      sym, (enum cupertino_symbol_rendering_mode)4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_symbol_set_weight(NULL, CUPERTINO_SYMBOL_WEIGHT_BOLD);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_set_weight(sym, (enum cupertino_symbol_weight) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_set_weight(sym, (enum cupertino_symbol_weight)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_symbol_set_size(NULL, CUPERTINO_SYMBOL_SIZE_LARGE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_set_size(sym, (enum cupertino_symbol_size) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_set_size(sym, (enum cupertino_symbol_size)3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_symbol_set_palette(NULL, 0, 0, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Getters invalid */
  rc = cupertino_symbol_get_layer(NULL, 0, &layer);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_get_layer(sym, -1, &layer);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_get_layer(sym, 10, &layer);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_get_layer(sym, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_symbol_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_get_dimensions(sym, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_symbol_get_dimensions(sym, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_symbol_destroy(sym);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_symbols_rendering_modes_and_weights(void) {
  struct cupertino_symbol_descriptor desc;
  struct cupertino_symbol *sym = NULL;
  struct cupertino_symbol_layer l0, l1, l2;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float w, h;
  float ultralight_stroke;
  float black_stroke;
  ui_error_t rc;

  /* Creation with desc.name == NULL and colors == 0 to hit defaults */
  memset(&desc, 0, sizeof(desc));
  desc.rendering_mode = CUPERTINO_SYMBOL_RENDERING_MONOCHROME;
  desc.weight = CUPERTINO_SYMBOL_WEIGHT_REGULAR;
  desc.size = CUPERTINO_SYMBOL_SIZE_MEDIUM;

  rc = cupertino_symbol_create(dummy_engine, &desc, &sym);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sym != NULL);
  ASSERT_STR_EQ("star", sym->name);

  rc = cupertino_symbol_destroy(sym);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Now create with specific name and pink color */
  memset(&desc, 0, sizeof(desc));
  desc.name = "heart";
  desc.rendering_mode = CUPERTINO_SYMBOL_RENDERING_HIERARCHICAL;
  desc.weight = CUPERTINO_SYMBOL_WEIGHT_REGULAR;
  desc.size = CUPERTINO_SYMBOL_SIZE_MEDIUM;
  desc.primary_color = UI_COLOR_ARGB(255, 255, 45, 85); /* Pink */
  desc.secondary_color = UI_COLOR_ARGB(255, 100, 100, 100);
  desc.tertiary_color = UI_COLOR_ARGB(255, 50, 50, 50);

  rc = cupertino_symbol_create(dummy_engine, &desc, &sym);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sym != NULL);

  rc = cupertino_symbol_get_dimensions(sym, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(18.0f, w);
  ASSERT_EQ(18.0f, h);

  /* Hierarchical layer opacities */
  rc = cupertino_symbol_get_layer(sym, 0, &l0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.0f, l0.opacity);

  rc = cupertino_symbol_get_layer(sym, 1, &l1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.50f, l1.opacity);

  rc = cupertino_symbol_get_layer(sym, 2, &l2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.25f, l2.opacity);

  /* Switch rendering mode to Palette */
  rc = cupertino_symbol_set_rendering_mode(sym,
                                           CUPERTINO_SYMBOL_RENDERING_PALETTE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_symbol_set_palette(sym, UI_COLOR_ARGB(255, 255, 0, 0),
                                    UI_COLOR_ARGB(255, 0, 255, 0),
                                    UI_COLOR_ARGB(255, 0, 0, 255));
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_symbol_get_layer(sym, 0, &l0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_COLOR_ARGB(255, 255, 0, 0), l0.color);

  rc = cupertino_symbol_get_layer(sym, 1, &l1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_COLOR_ARGB(255, 0, 255, 0), l1.color);

  rc = cupertino_symbol_get_layer(sym, 2, &l2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_COLOR_ARGB(255, 0, 0, 255), l2.color);

  /* Switch to Multicolor */
  rc = cupertino_symbol_set_rendering_mode(
      sym, CUPERTINO_SYMBOL_RENDERING_MULTICOLOR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_symbol_get_layer(sym, 0, &l0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_COLOR_ARGB(255, 0, 122, 255), l0.color);

  /* Switch to Monochrome */
  rc = cupertino_symbol_set_rendering_mode(
      sym, CUPERTINO_SYMBOL_RENDERING_MONOCHROME);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern void test_cupertino_symbol_evaluate_layers(struct cupertino_symbol *
                                                      s);
    sym->rendering_mode = (enum cupertino_symbol_rendering_mode)999;
    test_cupertino_symbol_evaluate_layers(sym);
    ASSERT_EQ(1.0f, sym->layers[0].opacity);
  }
#endif

  /* Weight matching test: UltraLight vs Black */
  rc = cupertino_symbol_set_weight(sym, CUPERTINO_SYMBOL_WEIGHT_ULTRALIGHT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_symbol_get_layer(sym, 0, &l0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ultralight_stroke = l0.stroke_width;

  rc = cupertino_symbol_set_weight(sym, CUPERTINO_SYMBOL_WEIGHT_BLACK);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_symbol_get_layer(sym, 0, &l0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  black_stroke = l0.stroke_width;

  ASSERT(black_stroke > ultralight_stroke * 3.0f);

  /* Size changes */
  rc = cupertino_symbol_set_size(sym, CUPERTINO_SYMBOL_SIZE_LARGE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_symbol_get_dimensions(sym, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(24.0f, w);

  rc = cupertino_symbol_set_size(sym, CUPERTINO_SYMBOL_SIZE_SMALL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_symbol_get_dimensions(sym, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(14.0f, w);

  /* Name change */
  rc = cupertino_symbol_set_name(sym, "star.fill");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("star.fill", sym->name);

  rc = cupertino_symbol_destroy(sym);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_symbols_oom_mock(void) {
  struct cupertino_symbol_descriptor desc;
  struct cupertino_symbol *sym = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_symbol_create(dummy_engine, &desc, &sym);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, sym);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_symbol_create(NULL, &desc, &sym);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_sf_symbols_suite) {
  RUN_TEST(test_symbols_invalid_arguments);
  RUN_TEST(test_symbols_rendering_modes_and_weights);
  RUN_TEST(test_symbols_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_sf_symbols_suite);
  GREATEST_MAIN_END();
}
