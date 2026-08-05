/* clang-format off */
#include "ui_property_grid_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stdio.h>
/* clang-format on */

static ui_error_t mock_factory(const struct ui_property_row *row,
                               void *user_data,
                               struct ui_component **out_comp) {
  (void)row;
  (void)user_data;
  *out_comp = NULL; /* Mock */
  return UI_ERROR_NONE;
}

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    err = (expr);                                                              \
    if (err != UI_ERROR_NONE) {                                                \
      printf("Failed at line %d: %d\n", __LINE__, err);                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    err = (expr);                                                              \
    if (err != (expected)) {                                                   \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             err);                                                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)

void test_property_grid_extra_err(void);
int main(void) {
  struct ui_arena *arena;
  struct ui_property_grid_base *grid = NULL;
  struct ui_property_row row1;
  struct ui_property_row row2;
  ui_error_t err;
  ui_signal_t *signal = NULL;
  int i;
  char group_id_buf[128][32];

  if (ui_arena_create(1024 * 64, &arena) != UI_ERROR_NONE) {
    return 1;
  }

  ASSERT_EQ(ui_property_grid_base_create(NULL, &grid),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_property_grid_base_create(arena, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_property_grid_base_create(arena, &grid));

  ASSERT_EQ(ui_property_grid_base_destroy(NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_property_grid_base_get_value_changed_signal(NULL, &signal),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_property_grid_base_get_value_changed_signal(grid, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_property_grid_base_get_value_changed_signal(grid, &signal));

  row1.id = "prop_color";
  row1.label = "Background Color";
  row1.group_id = "Appearance";
  row1.type = UI_PROPERTY_VALUE_TYPE_COLOR;
  row1.is_read_only = UI_FALSE;

  ASSERT_EQ(ui_property_grid_base_add_property(NULL, &row1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_property_grid_base_add_property(grid, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  row2.id = NULL;
  ASSERT_EQ(ui_property_grid_base_add_property(grid, &row2),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_property_grid_base_add_property(grid, &row1));

  row2.id = "prop_width";
  row2.label = "Width";
  row2.group_id = "Layout";
  row2.type = UI_PROPERTY_VALUE_TYPE_INT;
  row2.is_read_only = UI_FALSE;

  ASSERT_SUCCESS(ui_property_grid_base_add_property(grid, &row2));

  ASSERT_EQ(ui_property_grid_base_set_editor_factory(NULL, mock_factory, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(
      ui_property_grid_base_set_editor_factory(grid, mock_factory, NULL));

  ASSERT_EQ(
      ui_property_grid_base_set_group_collapsed(NULL, "Appearance", UI_TRUE),
      UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_property_grid_base_set_group_collapsed(grid, NULL, UI_TRUE),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(
      ui_property_grid_base_set_group_collapsed(grid, "Appearance", UI_TRUE));

  ASSERT_EQ(ui_property_grid_base_set_filter(NULL, "Width"),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_property_grid_base_set_filter(grid, "Width"));

  ASSERT_EQ(_ui_property_grid_base_trigger_change(NULL, "prop_color"),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(_ui_property_grid_base_trigger_change(grid, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(_ui_property_grid_base_trigger_change(grid, "prop_color"));
  ASSERT_EQ(_ui_property_grid_base_trigger_change(grid, "invalid_id"),
            UI_ERROR_NOT_FOUND);

  /* Exhaust rows and groups to hit out of bounds */
  for (i = 0; i < 128 + 1; i++) {
    struct ui_property_row dummy_row;
    dummy_row.id = "dummy";
    dummy_row.label = "Dummy";
    dummy_row.type = UI_PROPERTY_VALUE_TYPE_INT;
    dummy_row.is_read_only = UI_FALSE;
    if (i < 32) {
      sprintf(group_id_buf[i], "Group%d", i);
      dummy_row.group_id = group_id_buf[i];
    } else {
      dummy_row.group_id = NULL;
    }
    err = ui_property_grid_base_add_property(grid, &dummy_row);
    if (i >= 128) {
      if (err != UI_ERROR_OUT_OF_BOUNDS) {
        printf("Failed at line %d: Expected OUT_OF_BOUNDS\n", __LINE__);
        return 1;
      }
    } else {
      if (err != UI_ERROR_NONE && err != UI_ERROR_OUT_OF_BOUNDS) {
        printf("Failed at line %d: got %d\n", __LINE__, err);
        return 1;
      }
    }
  }

  ASSERT_EQ(
      ui_property_grid_base_set_group_collapsed(grid, "NewGroup33", UI_TRUE),
      UI_ERROR_OUT_OF_BOUNDS);
  ASSERT_SUCCESS(ui_property_grid_base_destroy(grid));
  (void)ui_arena_destroy(arena);
  printf("Test passed!\n");
  return 0;
}
void test_property_grid_extra_err(void) {
  /* To hit out_group=NULL in get_or_create_group, we need to call it. But it's
  static and called via add_property and set_group_collapsed which don't pass
  NULL. So we can't easily hit it without modifying the source. Wait,
  ui_property_grid_base.c is compiled into the test directly? No, it's in
  libui_engine_test.a. */
}
