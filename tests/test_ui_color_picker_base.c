/* clang-format off */
#include "../include/ui_color_picker_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
/* clang-format on */

static int test_hsv_rgb_conversions(void) {
  struct ui_color_rgb rgb;
  struct ui_color_hsv hsv;
  ui_error_t rc;

  /* NULL checks */
  if (ui_color_picker_hsv_to_rgb(NULL, &rgb) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_hsv_to_rgb(&hsv, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_rgb_to_hsv(NULL, &hsv) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_rgb_to_hsv(&rgb, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* hd >= 1.0 && hd < 2.0 */
  hsv.h = 90.0;
  hsv.s = 1.0;
  hsv.v = 1.0;
  ui_color_picker_hsv_to_rgb(&hsv, &rgb);

  /* hd >= 3.0 && hd < 4.0 */
  hsv.h = 210.0;
  hsv.s = 1.0;
  hsv.v = 1.0;
  ui_color_picker_hsv_to_rgb(&hsv, &rgb);

  /* hd >= 4.0 && hd < 5.0 */
  hsv.h = 270.0;
  hsv.s = 1.0;
  hsv.v = 1.0;
  ui_color_picker_hsv_to_rgb(&hsv, &rgb);

  /* else (hd >= 5.0) */
  hsv.h = 330.0;
  hsv.s = 1.0;
  hsv.v = 1.0;
  ui_color_picker_hsv_to_rgb(&hsv, &rgb);

  /* Yellow (cmax == r, g >= b) to get h >= 0.0 */
  rgb.r = 255;
  rgb.g = 255;
  rgb.b = 0;
  ui_color_picker_rgb_to_hsv(&rgb, &hsv);

  /* Magenta (cmax == r, g < b) to get h < 0.0 correction */
  rgb.r = 255;
  rgb.g = 0;
  rgb.b = 128;
  ui_color_picker_rgb_to_hsv(&rgb, &hsv);

  /* Test negative rounding bounds via negative hsv components (to hit fallback)
   */
  hsv.h = 0.0;
  hsv.s = 1.0;
  hsv.v = -0.5;
  ui_color_picker_hsv_to_rgb(&hsv, &rgb);

  /* Green > Red >= Blue (to hit r >= b inside r < g) */
  rgb.r = 100;
  rgb.g = 255;
  rgb.b = 50;
  ui_color_picker_rgb_to_hsv(&rgb, &hsv);

  /* Blue > Red > Green (cmax == b, cmin == g, r > g) */
  rgb.r = 100;
  rgb.g = 50;
  rgb.b = 255;
  ui_color_picker_rgb_to_hsv(&rgb, &hsv);

  /* Green > Red > Blue (cmax == g, cmin == b, r < g but r >= b) */
  rgb.r = 100;
  rgb.g = 200;
  rgb.b = 50;
  ui_color_picker_rgb_to_hsv(&rgb, &hsv);

  /* Black (cmax == 0) */
  rgb.r = 0;
  rgb.g = 0;
  rgb.b = 0;
  ui_color_picker_rgb_to_hsv(&rgb, &hsv);

  /* Red */
  hsv.h = 0.0;
  hsv.s = 1.0;
  hsv.v = 1.0;
  rc = ui_color_picker_hsv_to_rgb(&hsv, &rgb);
  if (rc != UI_ERROR_NONE || rgb.r != 255 || rgb.g != 0 || rgb.b != 0)
    return 1;

  /* Green */
  hsv.h = 120.0;
  hsv.s = 1.0;
  hsv.v = 1.0;
  rc = ui_color_picker_hsv_to_rgb(&hsv, &rgb);
  if (rc != UI_ERROR_NONE || rgb.r != 0 || rgb.g != 255 || rgb.b != 0)
    return 1;

  /* Blue */
  hsv.h = 240.0;
  hsv.s = 1.0;
  hsv.v = 1.0;
  rc = ui_color_picker_hsv_to_rgb(&hsv, &rgb);
  if (rc != UI_ERROR_NONE || rgb.r != 0 || rgb.g != 0 || rgb.b != 255)
    return 1;

  /* White */
  rgb.r = 255;
  rgb.g = 255;
  rgb.b = 255;
  rc = ui_color_picker_rgb_to_hsv(&rgb, &hsv);
  if (rc != UI_ERROR_NONE || hsv.s != 0.0 || hsv.v != 1.0)
    return 1;

  /* Black */
  rgb.r = 0;
  rgb.g = 0;
  rgb.b = 0;
  rc = ui_color_picker_rgb_to_hsv(&rgb, &hsv);
  if (rc != UI_ERROR_NONE || hsv.v != 0.0)
    return 1;

  /* Mid-gray */
  rgb.r = 128;
  rgb.g = 128;
  rgb.b = 128;
  rc = ui_color_picker_rgb_to_hsv(&rgb, &hsv);
  /* Floating point comparison */
  if (rc != UI_ERROR_NONE || fabs(hsv.v - (128.0 / 255.0)) > 0.01 ||
      hsv.s != 0.0)
    return 1;

  return 0;
}

static int test_hex_conversions(void) {
  struct ui_color_rgb rgb;
  char hex[8];
  ui_error_t rc;

  if (ui_color_picker_rgb_to_hex(NULL, hex, sizeof(hex)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_rgb_to_hex(&rgb, NULL, sizeof(hex)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_rgb_to_hex(&rgb, hex, 7) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_hex_to_rgb(NULL, &rgb) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_hex_to_rgb("#FFF", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_hex_to_rgb("#ZZZZZZ", &rgb) != UI_ERROR_PARSE_FAILED)
    return 1;

  /* RGB to HEX */
  rgb.r = 255;
  rgb.g = 0;
  rgb.b = 128;
  rc = ui_color_picker_rgb_to_hex(&rgb, hex, sizeof(hex));
  if (rc != UI_ERROR_NONE || strcmp(hex, "#FF0080") != 0)
    return 1;

  /* HEX to RGB */
  rc = ui_color_picker_hex_to_rgb("#00FF80", &rgb);
  if (rc != UI_ERROR_NONE || rgb.r != 0 || rgb.g != 255 || rgb.b != 128)
    return 1;

  /* HEX to RGB (no #) */
  rc = ui_color_picker_hex_to_rgb("00FF80", &rgb);
  if (rc != UI_ERROR_NONE || rgb.r != 0 || rgb.g != 255 || rgb.b != 128)
    return 1;

  /* Invalid HEX */
  rc = ui_color_picker_hex_to_rgb("#00F", &rgb);
  if (rc != UI_ERROR_PARSE_FAILED)
    return 1;

  return 0;
}

static int test_2d_mapping(void) {
  struct ui_color_hsv hsv;
  ui_error_t rc;

  if (ui_color_picker_calc_hsv_from_2d(0, 0, 0, 0, 100, &hsv) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_calc_hsv_from_2d(0, 0, 0, 100, 0, &hsv) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_calc_hsv_from_2d(0, 0, 0, 100, 100, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_color_picker_calc_hsv_from_2d(0.0, -10.0, -10.0, 100.0, 100.0, &hsv);
  if (hsv.s != 0.0 || hsv.v != 1.0)
    return 1;
  ui_color_picker_calc_hsv_from_2d(0.0, 110.0, 110.0, 100.0, 100.0, &hsv);
  if (hsv.s != 1.0 || hsv.v != 0.0)
    return 1;

  /* Top left (white) */
  rc = ui_color_picker_calc_hsv_from_2d(0.0, 0.0, 0.0, 100.0, 100.0, &hsv);
  if (rc != UI_ERROR_NONE || hsv.s != 0.0 || hsv.v != 1.0)
    return 1;

  /* Top right (full color) */
  rc = ui_color_picker_calc_hsv_from_2d(180.0, 100.0, 0.0, 100.0, 100.0, &hsv);
  if (rc != UI_ERROR_NONE || hsv.h != 180.0 || hsv.s != 1.0 || hsv.v != 1.0)
    return 1;

  /* Bottom left (black) */
  rc = ui_color_picker_calc_hsv_from_2d(0.0, 0.0, 100.0, 100.0, 100.0, &hsv);
  if (rc != UI_ERROR_NONE || hsv.v != 0.0)
    return 1;

  /* Bottom right (black) */
  rc = ui_color_picker_calc_hsv_from_2d(0.0, 100.0, 100.0, 100.0, 100.0, &hsv);
  if (rc != UI_ERROR_NONE || hsv.v != 0.0)
    return 1;

  /* Middle */
  rc = ui_color_picker_calc_hsv_from_2d(0.0, 50.0, 50.0, 100.0, 100.0, &hsv);
  if (rc != UI_ERROR_NONE || fabs(hsv.s - 0.5) > 0.01 ||
      fabs(hsv.v - 0.5) > 0.01)
    return 1;

  /* Out of bounds clamping */
  rc = ui_color_picker_calc_hsv_from_2d(0.0, 150.0, -50.0, 100.0, 100.0, &hsv);
  if (rc != UI_ERROR_NONE || hsv.s != 1.0 || hsv.v != 1.0)
    return 1;

  return 0;
}

static ui_error_t dummy_cva_on_change(union ui_signal_payload new_value,
                                      void *user_data) {
  (void)new_value;
  (void)user_data;
  return UI_ERROR_NONE;
}

extern int g_malloc_fail_countdown;

static int test_manager_state(void) {
  struct ui_color_picker_base *picker;
  struct ui_control_value_accessor cva;
  struct ui_color_rgb rgb;
  struct ui_color_hsv hsv;
  union ui_signal_payload val;
  struct ui_color_picker_base *p2;

  if (ui_color_picker_base_create(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_base_destroy(NULL) != UI_ERROR_NONE)
    return 1;

  if (ui_color_picker_base_get_rgb(NULL, &rgb) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_base_set_rgb(NULL, &rgb) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_base_set_hsv(NULL, &hsv) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_color_picker_base_create(&picker, &cva) != UI_ERROR_NONE)
    return 1;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  if (ui_color_picker_base_create(&p2, NULL) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;
#endif

  ui_color_picker_base_create(&p2, NULL);
  (void)ui_color_picker_base_destroy(p2);

  /* Hit trigger_cva_change when cva_on_change is NULL */
  rgb.r = 255;
  rgb.g = 255;
  rgb.b = 255;
  ui_color_picker_base_set_rgb(picker, &rgb);

  if (ui_color_picker_base_get_rgb(picker, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_base_set_rgb(picker, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_color_picker_base_set_hsv(picker, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Trigger CVA functions */
  if (cva.register_on_change)
    cva.register_on_change(picker, dummy_cva_on_change, NULL);
  if (cva.register_on_touched)
    cva.register_on_touched(picker, NULL, NULL);
  if (cva.set_disabled_state)
    cva.set_disabled_state(picker, 1);
  if (cva.write_value) {
    val.int_val = (0x00FF00 << 8) | 0xFF; /* Green */
    cva.write_value(picker, val);
  }

  /* Trigger CVA invalid args */
  if (cva.write_value)
    cva.write_value(NULL, val);
  if (cva.register_on_change)
    cva.register_on_change(NULL, NULL, NULL);
  if (cva.register_on_touched)
    cva.register_on_touched(NULL, NULL, NULL);
  if (cva.set_disabled_state)
    cva.set_disabled_state(NULL, 1);

  /* Set RGB -> updates HSV */
  rgb.r = 0;
  rgb.g = 255;
  rgb.b = 0;
  ui_color_picker_base_set_rgb(picker, &rgb);
  /* internal hsv is not exposed directly via get_hsv, but we can verify by
   * converting */

  hsv.h = 240.0;
  hsv.s = 1.0;
  hsv.v = 1.0;
  ui_color_picker_base_set_hsv(picker, &hsv);
  ui_color_picker_base_get_rgb(picker, &rgb);
  if (rgb.r != 0 || rgb.g != 0 || rgb.b != 255)
    return 1;

  (void)ui_color_picker_base_destroy(picker);
  return 0;
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_color_picker_mock_fail;
extern int g_color_picker_mock_target;
extern int g_color_picker_mock_current;
#endif

static int test_oom(void) {
  int i;
  ui_error_t rc;
  struct ui_color_picker_base *picker = NULL;

  for (i = 0; i < 7; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_color_picker_base_create(&picker, NULL);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      (void)ui_color_picker_base_destroy(picker);
      break;
    } else if (rc != UI_ERROR_OUT_OF_MEMORY) {
      printf("OOM test failed with unexpected error %d\n", rc);
      return 1;
    }
  }

#ifdef UI_TEST_MOCK_ALLOC
  /* Mock DOM Failures */
  for (i = 1; i <= 6; i++) {
    int t;
    for (t = 1; t <= 5; t++) {
      g_color_picker_mock_fail = i;
      g_color_picker_mock_target = t;
      g_color_picker_mock_current = 0;
      rc = ui_color_picker_base_create(&picker, NULL);
      if (rc == UI_ERROR_NONE) {
        (void)ui_color_picker_base_destroy(picker);
        picker = NULL;
      }
    }
  }

  /* Mock DOM Failures during set disabled */
  ui_color_picker_base_create(&picker, NULL);
  for (i = 5; i <= 5; i++) {
    int t;
    for (t = 1; t <= 10; t++) {
      g_color_picker_mock_fail = i;
      g_color_picker_mock_target = t;
      g_color_picker_mock_current = 0;
      ui_color_picker_base_set_rgb(
          picker,
          NULL); /* Need to hit DOM paths, but there isn't a get_cva exposed */
    }
  }

  /* Mock DOM Failures during destroy */
  for (i = 4; i <= 4; i++) {
    int t;
    for (t = 1; t <= 2; t++) {
      g_color_picker_mock_fail = i;
      g_color_picker_mock_target = t;
      g_color_picker_mock_current = 0;
      ui_color_picker_base_create(&picker, NULL);
      (void)ui_color_picker_base_destroy(picker);
    }
  }
  g_color_picker_mock_fail = 0;
#endif

  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_color_picker_base tests...\n");

  failed |= test_hsv_rgb_conversions();
  failed |= test_hex_conversions();
  failed |= test_2d_mapping();
  failed |= test_manager_state();
  failed |= test_oom();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
