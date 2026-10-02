/**
 * @file test_cupertino_list_tile.c
 * @brief Unit tests for Cupertino List Tile component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_list_tile.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_list_tile_suite);

TEST test_list_tile_invalid_args(void) {
  struct cupertino_list_tile_descriptor desc;
  struct cupertino_list_tile *tile = NULL;
  struct ui_list_item_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_list_tile_accessory acc;
  const char *str = NULL;
  float val = 0.0f;
  int is_pressed = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Create invalid */
  rc = cupertino_list_tile_create(NULL, &desc, &tile);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_create(dummy_engine, NULL, &tile);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy invalid */
  rc = cupertino_list_tile_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Title / subtitle / info */
  rc = cupertino_list_tile_set_title(NULL, "Title");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_set_title((struct cupertino_list_tile *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_title(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_title((const struct cupertino_list_tile *)0x123,
                                     NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_list_tile_set_subtitle(NULL, "Sub");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_subtitle(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_subtitle(
      (const struct cupertino_list_tile *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_list_tile_set_additional_info(NULL, "Info");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_additional_info(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_additional_info(
      (const struct cupertino_list_tile *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Accessory */
  rc = cupertino_list_tile_set_accessory(
      NULL, CUPERTINO_LIST_TILE_ACCESSORY_DISCLOSURE_CHEVRON);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_set_accessory((struct cupertino_list_tile *)0x1234,
                                         (enum cupertino_list_tile_accessory) -
                                             1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_accessory(NULL, &acc);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_accessory(
      (const struct cupertino_list_tile *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Pressed / opacity / height */
  rc = cupertino_list_tile_set_pressed(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_is_pressed(NULL, &is_pressed);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_is_pressed((const struct cupertino_list_tile *)0x123,
                                      NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_highlight_opacity(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_highlight_opacity(
      (const struct cupertino_list_tile *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_height(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_height((const struct cupertino_list_tile *)0x123,
                                      NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Base */
  rc = cupertino_list_tile_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_get_base((struct cupertino_list_tile *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_list_tile_lifecycle_and_properties(void) {
  struct cupertino_list_tile_descriptor desc;
  struct cupertino_list_tile *tile = NULL;
  struct ui_list_item_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_list_tile_accessory acc;
  const char *str = NULL;
  float h = 0.0f;
  float op = 0.0f;
  int is_pressed = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_LIST_TILE_VALUE1;
  desc.accessory = CUPERTINO_LIST_TILE_ACCESSORY_DISCLOSURE_CHEVRON;
  desc.title = "Wi-Fi";
  desc.subtitle = NULL;
  desc.additional_info = "HomeNetwork";
  desc.leading_icon = "wifi";
  desc.is_rtl = 0;

  rc = cupertino_list_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tile != NULL);

  /* Base */
  rc = cupertino_list_tile_get_base(tile, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Check initial getters */
  rc = cupertino_list_tile_get_title(tile, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Wi-Fi", str);

  rc = cupertino_list_tile_get_subtitle(tile, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(str == NULL);

  rc = cupertino_list_tile_get_additional_info(tile, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("HomeNetwork", str);

  rc = cupertino_list_tile_get_accessory(tile, &acc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_LIST_TILE_ACCESSORY_DISCLOSURE_CHEVRON, acc);

  /* Height for Value 1 = 44pt */
  rc = cupertino_list_tile_get_height(tile, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(h > 43.9f && h < 44.1f);

  /* Mutate title, subtitle, info */
  rc = cupertino_list_tile_set_title(tile, "Bluetooth");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_get_title(tile, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Bluetooth", str);

  rc = cupertino_list_tile_set_subtitle(tile, "Connected to AirPods");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_get_subtitle(tile, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Connected to AirPods", str);

  rc = cupertino_list_tile_set_additional_info(tile, "On");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_get_additional_info(tile, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("On", str);

  /* Mutate accessory */
  rc = cupertino_list_tile_set_accessory(
      tile, CUPERTINO_LIST_TILE_ACCESSORY_CHECKMARK);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_get_accessory(tile, &acc);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_LIST_TILE_ACCESSORY_CHECKMARK, acc);

  /* Invalid accessory */
  rc = cupertino_list_tile_set_accessory(
      tile, (enum cupertino_list_tile_accessory)999);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Press highlight */
  rc = cupertino_list_tile_is_pressed(tile, &is_pressed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_pressed);
  rc = cupertino_list_tile_get_highlight_opacity(tile, &op);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(op < 0.001f);

  rc = cupertino_list_tile_set_pressed(tile, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_is_pressed(tile, &is_pressed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_pressed);
  rc = cupertino_list_tile_get_highlight_opacity(tile, &op);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(op > 0.149f && op < 0.151f);

  rc = cupertino_list_tile_set_pressed(tile, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_is_pressed(tile, &is_pressed);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_pressed);

  /* Subtitle style height = 54pt */
  tile->style = CUPERTINO_LIST_TILE_SUBTITLE;
  rc = cupertino_list_tile_get_height(tile, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(h > 53.9f && h < 54.1f);

  /* Set subtitle to NULL to clear it */
  rc = cupertino_list_tile_set_subtitle(tile, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_get_subtitle(tile, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(str == NULL);

  /* Set additional_info to NULL to clear it */
  rc = cupertino_list_tile_set_additional_info(tile, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_get_additional_info(tile, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(str == NULL);

  /* Destroy */
  rc = cupertino_list_tile_destroy(tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Create with subtitle populated in desc */
  memset(&desc, 0, sizeof(desc));
  desc.title = "With Subtitle";
  desc.subtitle = "Initial Subtitle";
  rc = cupertino_list_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tile != NULL);
  rc = cupertino_list_tile_get_subtitle(tile, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Initial Subtitle", str);

  /* Test destroy with NULL tile->base */
  ui_list_item_base_destroy(tile->base);
  tile->base = NULL;
  rc = cupertino_list_tile_destroy(tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_list_tile_oom_mock(void) {
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_list_tile_mock_create_fail;
  extern int g_cupertino_list_tile_mock_destroy_fail;
#endif
  struct cupertino_list_tile_descriptor desc;
  struct cupertino_list_tile *tile = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "OOM Tile";

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail malloc for struct cupertino_list_tile */
  g_malloc_fail_countdown = 0;
  rc = cupertino_list_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(tile == NULL);

  /* Fail malloc for ui_list_item_base */
  g_malloc_fail_countdown = 1;
  rc = cupertino_list_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(tile == NULL);
  g_malloc_fail_countdown = -1;

  /* Fail ui_list_item_base_create */
  g_cupertino_list_tile_mock_create_fail = 1;
  rc = cupertino_list_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(tile == NULL);
  g_cupertino_list_tile_mock_create_fail = 0;

  /* Create with desc->title == NULL */
  memset(&desc, 0, sizeof(desc));
  rc = cupertino_list_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tile != NULL);

  /* Test destroy failure */
  g_cupertino_list_tile_mock_destroy_fail = 1;
  rc = cupertino_list_tile_destroy(tile);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_list_tile_mock_destroy_fail = 0;

  rc = cupertino_list_tile_destroy(tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = cupertino_list_tile_create(dummy_engine, NULL, &tile);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

TEST test_list_tile_dynamic_reflow(void) {
  struct cupertino_list_tile_descriptor desc;
  struct cupertino_list_tile *tile = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  int is_stacked = 0;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "Wi-Fi";
  desc.additional_info = "Home Network";
  desc.style = CUPERTINO_LIST_TILE_VALUE1;

  rc = cupertino_list_tile_create(dummy_engine, &desc, &tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Reference scale 1.0 (standard) */
  rc = cupertino_list_tile_set_scale_factor(tile, 1.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_is_stacked(tile, &is_stacked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_stacked);

  rc = cupertino_list_tile_compute_height(tile, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(CUPERTINO_LIST_TILE_HEIGHT_DEFAULT, h, 0.01f);

  /* Accessibility scale >= 1.5 (AX1+) -> auto switches to stacked vertical */
  rc = cupertino_list_tile_set_scale_factor(tile, 1.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_is_stacked(tile, &is_stacked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_stacked);

  rc = cupertino_list_tile_compute_height(tile, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* (44 + 24) * 1.5 = 102pt */
  ASSERT_IN_RANGE(102.0f, h, 0.01f);

  /* Clear additional_info and re-set scale >= 1.5 -> is_stacked remains 0 */
  rc = cupertino_list_tile_set_additional_info(tile, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_set_scale_factor(tile, 1.6f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_is_stacked(tile, &is_stacked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_stacked);

  /* Reset to scale 1.0 */
  rc = cupertino_list_tile_set_scale_factor(tile, 1.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_is_stacked(tile, &is_stacked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_stacked);

  /* Null and invalid checks */
  rc = cupertino_list_tile_set_scale_factor(NULL, 1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_set_scale_factor(tile, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_set_scale_factor(tile, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_is_stacked(NULL, &is_stacked);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_is_stacked(tile, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_compute_height(NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_tile_compute_height(tile, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_list_tile_destroy(tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(cupertino_list_tile_suite) {
  RUN_TEST(test_list_tile_invalid_args);
  RUN_TEST(test_list_tile_lifecycle_and_properties);
  RUN_TEST(test_list_tile_dynamic_reflow);
  RUN_TEST(test_list_tile_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_list_tile_suite);
  GREATEST_MAIN_END();
}
