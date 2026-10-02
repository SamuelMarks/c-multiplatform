/**
 * @file test_cupertino_list_section.c
 * @brief Unit tests for Cupertino List Section component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_list_section.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_list_section_suite);

TEST test_list_section_invalid_args(void) {
  struct cupertino_list_section_descriptor desc;
  struct cupertino_list_section *section = NULL;
  struct cupertino_list_tile *tile = NULL;
  struct ui_list_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *str = NULL;
  size_t count = 0;
  size_t idx = 0;
  int visible = 0;
  float inset = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Create invalid */
  rc = cupertino_list_section_create(NULL, &desc, &section);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_create(dummy_engine, NULL, &section);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy invalid */
  rc = cupertino_list_section_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create a valid section for method argument checks */
  desc.header = "Header";
  desc.footer = "Footer";
  rc = cupertino_list_section_create(dummy_engine, &desc, &section);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(section != NULL);

  /* Create and add a tile so index 0 is valid (< tile_count) */
  {
    struct cupertino_list_tile_descriptor td;
    memset(&td, 0, sizeof(td));
    td.title = "Dummy";
    rc = cupertino_list_tile_create(dummy_engine, &td, &tile);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_list_section_add_tile(section, tile, &idx);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Header / footer invalid */
  rc = cupertino_list_section_set_header(NULL, "Header");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_get_header(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_get_header(section, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_list_section_set_footer(NULL, "Footer");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_get_footer(NULL, &str);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_get_footer(section, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Add tile invalid */
  rc = cupertino_list_section_add_tile(NULL, tile, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_add_tile(section, NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Tile count / get tile invalid */
  rc = cupertino_list_section_get_tile_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_get_tile_count(section, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_list_section_get_tile(NULL, 0, &tile);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_get_tile(section, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_get_tile(section, 5, &tile);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Separator visible / inset invalid */
  rc = cupertino_list_section_is_separator_visible(NULL, 0, &visible);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_is_separator_visible(section, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_is_separator_visible(section, 5, &visible);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_list_section_get_separator_inset(NULL, 0, &inset);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_get_separator_inset(section, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_get_separator_inset(section, 5, &inset);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Base invalid */
  rc = cupertino_list_section_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_list_section_get_base(section, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Clean up */
  rc = cupertino_list_section_destroy(section);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_destroy(tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_list_section_lifecycle_and_tiles(void) {
  struct cupertino_list_section_descriptor desc;
  struct cupertino_list_tile_descriptor tile_desc1, tile_desc2;
  struct cupertino_list_section *section = NULL;
  struct cupertino_list_tile *tile1 = NULL;
  struct cupertino_list_tile *tile2 = NULL;
  struct cupertino_list_tile *retrieved_tile = NULL;
  struct ui_list_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *str = NULL;
  size_t count = 0;
  size_t idx = 0;
  int visible = 0;
  float inset = 0.0f;
  ui_error_t rc;

  /* Test section with NULL header and footer */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_LIST_SECTION_PLAIN;
  rc = cupertino_list_section_create(dummy_engine, &desc, &section);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(section != NULL);

  rc = cupertino_list_section_get_header(section, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(str == NULL);

  rc = cupertino_list_section_get_footer(section, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(str == NULL);

  rc = cupertino_list_section_set_header(section, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_section_get_header(section, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(str == NULL);

  rc = cupertino_list_section_set_footer(section, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_section_get_footer(section, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(str == NULL);

  /* Add tile without out_index pointer */
  memset(&tile_desc1, 0, sizeof(tile_desc1));
  tile_desc1.title = "Standard Tile";
  rc = cupertino_list_tile_create(dummy_engine, &tile_desc1, &tile1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_list_section_add_tile(section, tile1, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Standard style: last tile separator is visible */
  rc = cupertino_list_section_is_separator_visible(section, 0, &visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, visible);

  /* Destroy with section->base == NULL */
  base = section->base;
  section->base = NULL;
  rc = cupertino_list_section_destroy(section);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_list_base_destroy(base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_destroy(tile1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Create section in Inset Grouped mode */
  memset(&desc, 0, sizeof(desc));
  desc.style = CUPERTINO_LIST_SECTION_INSET_GROUPED;
  desc.header = "CONNECTIVITY";
  desc.footer = "Manage cellular and local network connections.";

  rc = cupertino_list_section_create(dummy_engine, &desc, &section);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(section != NULL);

  /* Check base */
  rc = cupertino_list_section_get_base(section, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Check header and footer */
  rc = cupertino_list_section_get_header(section, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("CONNECTIVITY", str);

  rc = cupertino_list_section_get_footer(section, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Manage cellular and local network connections.", str);

  /* Mutate header / footer */
  rc = cupertino_list_section_set_header(section, "NETWORK");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_section_get_header(section, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("NETWORK", str);

  rc = cupertino_list_section_set_footer(section, "Network settings.");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_section_get_footer(section, &str);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Network settings.", str);

  /* Create two tiles */
  memset(&tile_desc1, 0, sizeof(tile_desc1));
  tile_desc1.title = "Airplane Mode";
  tile_desc1.leading_icon = "airplane";
  rc = cupertino_list_tile_create(dummy_engine, &tile_desc1, &tile1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&tile_desc2, 0, sizeof(tile_desc2));
  tile_desc2.title = "Personal Hotspot";
  tile_desc2.leading_icon = ""; /* No icon */
  rc = cupertino_list_tile_create(dummy_engine, &tile_desc2, &tile2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add tiles to section */
  rc = cupertino_list_section_add_tile(section, tile1, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  rc = cupertino_list_section_add_tile(section, tile2, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  rc = cupertino_list_section_get_tile_count(section, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, count);

  /* Get tile by index */
  rc = cupertino_list_section_get_tile(section, 0, &retrieved_tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(tile1, retrieved_tile);

  rc = cupertino_list_section_get_tile(section, 1, &retrieved_tile);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(tile2, retrieved_tile);

  /* Separator visibility in Inset Grouped: tile 0 is visible, tile 1 (last) is
   * hidden */
  rc = cupertino_list_section_is_separator_visible(section, 0, &visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, visible);

  rc = cupertino_list_section_is_separator_visible(section, 1, &visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, visible);

  /* Separator leading inset */
  rc = cupertino_list_section_get_separator_inset(section, 0, &inset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(inset > 55.9f && inset < 56.1f);

  rc = cupertino_list_section_get_separator_inset(section, 1, &inset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(inset > 15.9f && inset < 16.1f);

  /* Fill tiles up to MAX to test overflow */
  while (section->tile_count < CUPERTINO_LIST_SECTION_MAX_TILES) {
    rc = cupertino_list_section_add_tile(section, tile1, &idx);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Exceeding max tiles */
  rc = cupertino_list_section_add_tile(section, tile1, &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* Clean up */
  rc = cupertino_list_section_destroy(section);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_list_tile_destroy(tile1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_list_tile_destroy(tile2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_list_section_mock_failures(void) {
  struct cupertino_list_section_descriptor desc;
  struct cupertino_list_tile_descriptor tile_desc;
  struct cupertino_list_section *section = NULL;
  struct cupertino_list_tile *tile = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  size_t idx = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.header = "Mock Header";
  desc.footer = "Mock Footer";

  memset(&tile_desc, 0, sizeof(tile_desc));
  tile_desc.title = "Mock Tile";

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_cupertino_list_section_mock_base_create_fail;
    extern int g_cupertino_list_section_mock_set_orientation_fail;
    extern int g_cupertino_list_section_mock_base_destroy_fail;
    extern int g_cupertino_list_section_mock_tile_get_base_fail;
    extern int g_cupertino_list_section_mock_append_item_fail;
    extern struct ui_list_base *g_cupertino_list_section_last_created_base;

    /* Base create fail */
    g_cupertino_list_section_mock_base_create_fail = 1;
    rc = cupertino_list_section_create(dummy_engine, &desc, &section);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    ASSERT(section == NULL);
    g_cupertino_list_section_mock_base_create_fail = 0;

    /* Set orientation fail with destroy success */
    g_cupertino_list_section_mock_set_orientation_fail = 1;
    g_cupertino_list_section_mock_base_destroy_fail = 0;
    rc = cupertino_list_section_create(dummy_engine, &desc, &section);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    ASSERT(section == NULL);

    /* Set orientation fail with destroy fail */
    g_cupertino_list_section_mock_set_orientation_fail = 1;
    g_cupertino_list_section_mock_base_destroy_fail = 1;
    rc = cupertino_list_section_create(dummy_engine, &desc, &section);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    ASSERT(section == NULL);
    g_cupertino_list_section_mock_set_orientation_fail = 0;
    g_cupertino_list_section_mock_base_destroy_fail = 0;
    if (g_cupertino_list_section_last_created_base) {
      rc = ui_list_base_destroy(g_cupertino_list_section_last_created_base);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      g_cupertino_list_section_last_created_base = NULL;
    }

    /* Create valid section and tile */
    rc = cupertino_list_section_create(dummy_engine, &desc, &section);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_list_tile_create(dummy_engine, &tile_desc, &tile);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* Add tile: tile_get_base fail */
    g_cupertino_list_section_mock_tile_get_base_fail = 1;
    rc = cupertino_list_section_add_tile(section, tile, &idx);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_list_section_mock_tile_get_base_fail = 0;

    /* Add tile: append_item fail */
    g_cupertino_list_section_mock_append_item_fail = 1;
    rc = cupertino_list_section_add_tile(section, tile, &idx);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_list_section_mock_append_item_fail = 0;

    /* Destroy fail */
    g_cupertino_list_section_mock_base_destroy_fail = 1;
    rc = cupertino_list_section_destroy(section);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_cupertino_list_section_mock_base_destroy_fail = 0;

    /* Normal destroy */
    rc = cupertino_list_section_destroy(section);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = cupertino_list_tile_destroy(tile);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
#endif

  PASS();
}

TEST test_list_section_oom_mock(void) {
  struct cupertino_list_section_descriptor desc;
  struct cupertino_list_section *section = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.header = "OOM Section";

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail malloc for struct cupertino_list_section */
  g_malloc_fail_countdown = 0;
  rc = cupertino_list_section_create(dummy_engine, &desc, &section);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(section == NULL);

  /* Fail malloc for ui_list_base */
  g_malloc_fail_countdown = 1;
  rc = cupertino_list_section_create(dummy_engine, &desc, &section);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(section == NULL);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_list_section_create(NULL, &desc, &section);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_list_section_suite) {
  RUN_TEST(test_list_section_invalid_args);
  RUN_TEST(test_list_section_lifecycle_and_tiles);
  RUN_TEST(test_list_section_mock_failures);
  RUN_TEST(test_list_section_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_list_section_suite);
  GREATEST_MAIN_END();
}
