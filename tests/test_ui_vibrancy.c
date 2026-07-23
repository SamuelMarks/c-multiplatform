/* clang-format off */
#include <assert.h>
#include <math.h>

#include "ui_vibrancy.h"
#include "ui_error.h"
/* clang-format on */

static int test_vibrancy_init(void) {
  struct ui_vibrancy_params params;
  enum ui_error err;
  int failed = 0;

  err = ui_vibrancy_params_init(NULL, UI_VIBRANCY_MATERIAL_ACRYLIC);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_vibrancy_params_init(&params, UI_VIBRANCY_MATERIAL_ACRYLIC);
  failed |= (err != UI_ERROR_NONE);
  failed |= (params.material != UI_VIBRANCY_MATERIAL_ACRYLIC);
  failed |= (params.blur_radius != 30.0f);

  err = ui_vibrancy_params_init(&params, UI_VIBRANCY_MATERIAL_MICA);
  failed |= (err != UI_ERROR_NONE);
  failed |= (params.material != UI_VIBRANCY_MATERIAL_MICA);
  failed |= (params.blur_radius != 0.0f);

  err = ui_vibrancy_params_init(&params, UI_VIBRANCY_MATERIAL_CUPERTINO_LIGHT);
  failed |= (err != UI_ERROR_NONE);
  failed |= (params.material != UI_VIBRANCY_MATERIAL_CUPERTINO_LIGHT);
  failed |= (params.blur_radius != 20.0f);

  err = ui_vibrancy_params_init(&params, UI_VIBRANCY_MATERIAL_CUPERTINO_DARK);
  failed |= (err != UI_ERROR_NONE);
  failed |= (params.material != UI_VIBRANCY_MATERIAL_CUPERTINO_DARK);
  failed |= (params.blur_radius != 20.0f);

  err = ui_vibrancy_params_init(&params, UI_VIBRANCY_MATERIAL_NONE);
  failed |= (err != UI_ERROR_NONE);
  failed |= (params.material != UI_VIBRANCY_MATERIAL_NONE);
  failed |= (params.blur_radius != 0.0f);

  /* Test default case */
  err = ui_vibrancy_params_init(&params, (enum ui_vibrancy_material)999);
  failed |= (err != UI_ERROR_NONE);
  failed |= (params.material != (enum ui_vibrancy_material)999);
  failed |= (params.blur_radius != 0.0f);

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_vibrancy_init();
  return failed;
}
