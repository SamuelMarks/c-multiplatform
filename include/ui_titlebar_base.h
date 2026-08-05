#ifndef UI_TITLEBAR_BASE_H
#define UI_TITLEBAR_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_signal.h"
/* clang-format on */

struct ui_titlebar_base;
struct ui_arena;

/**
 * @brief Results for hit testing in the titlebar area.
 */
enum ui_titlebar_hit_test_result {
  UI_TITLEBAR_HIT_TEST_NONE,
  UI_TITLEBAR_HIT_TEST_DRAG_AREA,
  UI_TITLEBAR_HIT_TEST_MINIMIZE_BTN,
  UI_TITLEBAR_HIT_TEST_MAXIMIZE_BTN,
  UI_TITLEBAR_HIT_TEST_CLOSE_BTN
};

/**
 * @brief Configuration for a custom titlebar.
 */
struct ui_titlebar_config {
  ui_bool_t draggable;
  float height;
};

/**
 * @brief Creates a custom titlebar component.
 *
 * @param arena The memory arena.
 * @param config The configuration.
 * @param out_titlebar Output pointer for the created component.
 * @return ui_error_t
 */
ui_error_t ui_titlebar_base_create(struct ui_arena *arena,
                                   const struct ui_titlebar_config *config,
                                   struct ui_titlebar_base **out_titlebar);

/**
 * @brief Destroys a custom titlebar component.
 *
 * @param titlebar The component.
 * @return ui_error_t
 */
ui_error_t ui_titlebar_base_destroy(struct ui_titlebar_base *titlebar);

/**
 * @brief Performs a hit test to determine what area was clicked.
 *
 * @param titlebar The component.
 * @param x The X coordinate.
 * @param y The Y coordinate.
 * @param out_result Output pointer for the hit test result.
 * @return ui_error_t
 */
ui_error_t
ui_titlebar_base_hit_test(struct ui_titlebar_base *titlebar, float x, float y,
                          enum ui_titlebar_hit_test_result *out_result);

/**
 * @brief Adds a rectangular hit area for a specific button type.
 *
 * @param titlebar The component.
 * @param btn_type The button type.
 * @param x The X coordinate of the rect.
 * @param y The Y coordinate of the rect.
 * @param w The width of the rect.
 * @param h The height of the rect.
 * @return ui_error_t
 */
ui_error_t
ui_titlebar_base_add_button_rect(struct ui_titlebar_base *titlebar,
                                 enum ui_titlebar_hit_test_result btn_type,
                                 float x, float y, float w, float h);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TITLEBAR_BASE_H */
