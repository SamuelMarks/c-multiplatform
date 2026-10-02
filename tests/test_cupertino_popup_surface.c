/**
 * @file test_cupertino_popup_surface.c
 * @brief Unit tests for Cupertino Popup Surface (CupertinoPopupSurface).
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_popup_surface.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_cupertino_popup_surface_mock_surface_create_fail;
extern int g_cupertino_popup_surface_mock_set_elevation_fail;
extern int g_cupertino_popup_surface_mock_destroy_fail;
#endif

SUITE(cupertino_popup_surface_suite);

TEST test_popup_surface_invalid_arguments(void) {
  struct cupertino_popup_surface_descriptor desc;
  struct cupertino_popup_surface *surface = NULL;
  struct ui_surface_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum ui_vibrancy_material material;
  enum ui_elevation_level elevation;
  float val1, val2, val3, val4;
  int flag;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Creation invalid */
  rc = cupertino_popup_surface_create(NULL, &desc, &surface);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_create(dummy_engine, NULL, &surface);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range preset */
  desc.preset = (enum cupertino_popup_surface_preset) - 1;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.preset = (enum cupertino_popup_surface_preset)4;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.preset = (enum cupertino_popup_surface_preset)99;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destruction invalid */
  rc = cupertino_popup_surface_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Setters / getters invalid */
  rc = cupertino_popup_surface_set_corner_radius(NULL, 10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_corner_radius(NULL, &val1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_popup_surface_set_dark_mode(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_dark_mode(NULL, &flag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_popup_surface_set_reduce_transparency(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_reduce_transparency(NULL, &flag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_popup_surface_get_vibrancy_material(NULL, &material);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_popup_surface_get_elevation(NULL, &elevation);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_set_elevation(NULL, UI_ELEVATION_LEVEL_1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_popup_surface_get_shadow_params(NULL, &val1, &val2, &val3,
                                                 &val4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_popup_surface_set_dimensions(NULL, 100.0f, 100.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_dimensions(NULL, &val1, &val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_popup_surface_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid surface pointer, but NULL out parameters */
  desc.preset = CUPERTINO_POPUP_SURFACE_PRESET_ALERT;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(surface != NULL);

  rc = cupertino_popup_surface_get_corner_radius(surface, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_dark_mode(surface, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_reduce_transparency(surface, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_vibrancy_material(surface, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_elevation(surface, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set elevation out of bounds */
  rc = cupertino_popup_surface_set_elevation(surface,
                                             (enum ui_elevation_level) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_set_elevation(surface,
                                             (enum ui_elevation_level)6);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Shadow params individual NULL tests */
  rc = cupertino_popup_surface_get_shadow_params(surface, NULL, &val2, &val3,
                                                 &val4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_shadow_params(surface, &val1, NULL, &val3,
                                                 &val4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_shadow_params(surface, &val1, &val2, NULL,
                                                 &val4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_shadow_params(surface, &val1, &val2, &val3,
                                                 NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set dimensions invalid */
  rc = cupertino_popup_surface_set_dimensions(surface, 100.0f, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Get dimensions individual NULL tests */
  rc = cupertino_popup_surface_get_dimensions(surface, NULL, &val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_popup_surface_get_dimensions(surface, &val1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Get base NULL test */
  rc = cupertino_popup_surface_get_base(surface, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_popup_surface_destroy(surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_popup_surface_presets_and_properties(void) {
  struct cupertino_popup_surface_descriptor desc;
  struct cupertino_popup_surface *surface = NULL;
  struct ui_surface_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum ui_vibrancy_material material;
  enum ui_elevation_level elevation;
  float r, w, h;
  float blur, spread, opacity, offset_y;
  int is_dark, reduce;
  ui_error_t rc;

  /* Alert preset */
  memset(&desc, 0, sizeof(desc));
  desc.preset = CUPERTINO_POPUP_SURFACE_PRESET_ALERT;
  desc.width = 270.0f;
  desc.height = 160.0f;

  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(surface != NULL);

  rc = cupertino_popup_surface_get_base(surface, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = cupertino_popup_surface_get_corner_radius(surface, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(CUPERTINO_POPUP_SURFACE_RADIUS_ALERT, r, "%f");

  rc = cupertino_popup_surface_get_elevation(surface, &elevation);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_ELEVATION_LEVEL_4, elevation);

  rc = cupertino_popup_surface_get_dimensions(surface, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(270.0f, w, "%f");
  ASSERT_EQ_FMT(160.0f, h, "%f");

  rc = cupertino_popup_surface_get_shadow_params(surface, &blur, &spread,
                                                 &opacity, &offset_y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(blur > 0.0f);
  ASSERT(opacity > 0.0f);

  rc = cupertino_popup_surface_destroy(surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Action sheet preset */
  desc.preset = CUPERTINO_POPUP_SURFACE_PRESET_ACTION_SHEET;
  desc.is_dark = 1;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_popup_surface_get_corner_radius(surface, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(CUPERTINO_POPUP_SURFACE_RADIUS_ACTION_SHEET, r, "%f");

  rc = cupertino_popup_surface_get_dark_mode(surface, &is_dark);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_dark);

  rc = cupertino_popup_surface_get_vibrancy_material(surface, &material);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_VIBRANCY_MATERIAL_CUPERTINO_DARK, material);

  rc = cupertino_popup_surface_destroy(surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Popover preset */
  desc.preset = CUPERTINO_POPUP_SURFACE_PRESET_POPOVER;
  desc.is_dark = 0;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_popup_surface_get_corner_radius(surface, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(CUPERTINO_POPUP_SURFACE_RADIUS_POPOVER, r, "%f");

  rc = cupertino_popup_surface_get_vibrancy_material(surface, &material);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_VIBRANCY_MATERIAL_CUPERTINO_LIGHT, material);

  rc = cupertino_popup_surface_destroy(surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Custom preset with valid custom radius */
  desc.preset = CUPERTINO_POPUP_SURFACE_PRESET_CUSTOM;
  desc.custom_corner_radius = 20.0f;
  desc.width = 300.0f;
  desc.height = 200.0f;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_popup_surface_get_corner_radius(surface, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(20.0f, r, "%f");

  rc = cupertino_popup_surface_destroy(surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Custom preset with negative custom radius to hit fallback */
  desc.preset = CUPERTINO_POPUP_SURFACE_PRESET_CUSTOM;
  desc.custom_corner_radius = -5.0f;
  desc.width = -1.0f;
  desc.height = -1.0f;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_popup_surface_get_corner_radius(surface, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(CUPERTINO_POPUP_SURFACE_RADIUS_ALERT, r, "%f");

  rc = cupertino_popup_surface_get_dimensions(surface, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(280.0f, w, "%f");
  ASSERT_EQ_FMT(180.0f, h, "%f");

  /* Test setting corner radius */
  rc = cupertino_popup_surface_set_corner_radius(surface, 22.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_get_corner_radius(surface, &r);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(22.5f, r, "%f");
  rc = cupertino_popup_surface_set_corner_radius(surface, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Test dark mode toggle */
  rc = cupertino_popup_surface_set_dark_mode(surface, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_get_dark_mode(surface, &is_dark);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_dark);
  rc = cupertino_popup_surface_get_vibrancy_material(surface, &material);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_VIBRANCY_MATERIAL_CUPERTINO_DARK, material);

  /* Test reduce transparency */
  rc = cupertino_popup_surface_set_reduce_transparency(surface, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_get_reduce_transparency(surface, &reduce);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, reduce);

  /* Set dark mode with reduce transparency */
  rc = cupertino_popup_surface_set_dark_mode(surface, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_set_dark_mode(surface, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set reduce transparency 0 */
  rc = cupertino_popup_surface_set_reduce_transparency(surface, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_set_dark_mode(surface, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test elevation levels 0 through 5 */
  rc = cupertino_popup_surface_set_elevation(surface, UI_ELEVATION_LEVEL_0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_get_shadow_params(surface, &blur, &spread,
                                                 &opacity, &offset_y);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(0.0f, blur, "%f");

  rc = cupertino_popup_surface_set_elevation(surface, UI_ELEVATION_LEVEL_1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_set_elevation(surface, UI_ELEVATION_LEVEL_2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_set_elevation(surface, UI_ELEVATION_LEVEL_3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_set_elevation(surface, UI_ELEVATION_LEVEL_4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_set_elevation(surface, UI_ELEVATION_LEVEL_5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_set_elevation(surface,
                                             (enum ui_elevation_level)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Test dimensions */
  rc = cupertino_popup_surface_set_dimensions(surface, 400.0f, 300.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_get_dimensions(surface, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(400.0f, w, "%f");
  ASSERT_EQ_FMT(300.0f, h, "%f");
  rc = cupertino_popup_surface_set_dimensions(surface, -10.0f, 300.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy with surface->base set */
  rc = cupertino_popup_surface_destroy(surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test destroy with surface->base == NULL */
  desc.preset = CUPERTINO_POPUP_SURFACE_PRESET_ALERT;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Free base component directly so surface->base can be set to NULL */
  ui_component_destroy(&surface->base->base);
  surface->base = NULL;
  /* Now set elevation with NULL base to test branch */
  rc = cupertino_popup_surface_set_elevation(surface, UI_ELEVATION_LEVEL_2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_popup_surface_destroy(surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_popup_surface_mock_failures(void) {
  struct cupertino_popup_surface_descriptor desc;
  struct cupertino_popup_surface *surface = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.preset = CUPERTINO_POPUP_SURFACE_PRESET_ALERT;

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail ui_surface_base_create */
  g_cupertino_popup_surface_mock_surface_create_fail = 1;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, surface);
  g_cupertino_popup_surface_mock_surface_create_fail = 0;

  /* Fail ui_surface_base_set_elevation during create, with normal destroy */
  g_cupertino_popup_surface_mock_set_elevation_fail = 1;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, surface);
  g_cupertino_popup_surface_mock_set_elevation_fail = 0;

  /* Fail ui_surface_base_set_elevation during create AND fail destroy cleanup
   */
  g_cupertino_popup_surface_mock_set_elevation_fail = 1;
  g_cupertino_popup_surface_mock_destroy_fail = 1;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, surface);
  g_cupertino_popup_surface_mock_set_elevation_fail = 0;
  g_cupertino_popup_surface_mock_destroy_fail = 0;

  /* Create surface and test fail set_elevation */
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(surface != NULL);

  g_cupertino_popup_surface_mock_set_elevation_fail = 1;
  rc = cupertino_popup_surface_set_elevation(surface, UI_ELEVATION_LEVEL_1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_popup_surface_mock_set_elevation_fail = 0;

  /* Test fail destroy */
  g_cupertino_popup_surface_mock_destroy_fail = 1;
  rc = cupertino_popup_surface_destroy(surface);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_popup_surface_mock_destroy_fail = 0;

  rc = cupertino_popup_surface_destroy(surface);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  (void)dummy_engine;
  (void)surface;
  (void)rc;
#endif

  PASS();
}

TEST test_popup_surface_oom_simulation(void) {
  struct cupertino_popup_surface_descriptor desc;
  struct cupertino_popup_surface *surface = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.preset = CUPERTINO_POPUP_SURFACE_PRESET_ALERT;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_popup_surface_create(dummy_engine, &desc, &surface);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, surface);
  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_popup_surface_create(NULL, &desc, &surface);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_popup_surface_suite) {
  RUN_TEST(test_popup_surface_invalid_arguments);
  RUN_TEST(test_popup_surface_presets_and_properties);
  RUN_TEST(test_popup_surface_mock_failures);
  RUN_TEST(test_popup_surface_oom_simulation);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_popup_surface_suite);
  GREATEST_MAIN_END();
}
