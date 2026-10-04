/**
 * @file md3_action_widgets.h
 * @brief Material 3 & Expressive Action Widgets (fab_menu, button_group,
 * toggle_button, toolbar).
 */

#ifndef MATERIAL3_MD3_ACTION_WIDGETS_H
#define MATERIAL3_MD3_ACTION_WIDGETS_H

/* clang-format off */
#include "material3/md3_button.h"
#include "material3/md3_fab.h"
#include "ui_button_group_base.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_speed_dial_base.h"
#include "ui_toggle_base.h"
#include "ui_toolbar_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md3_button_group_orientation
 * @brief Button group orientation.
 */
enum md3_button_group_orientation {
  MD3_BUTTON_GROUP_HORIZONTAL = 0,
  MD3_BUTTON_GROUP_VERTICAL,
  MD3_BUTTON_GROUP_ORIENTATION_COUNT
};

/**
 * @enum md3_button_group_selection_mode
 * @brief Selection synchronization mode.
 */
enum md3_button_group_selection_mode {
  MD3_BUTTON_GROUP_SELECTION_NONE = 0,
  MD3_BUTTON_GROUP_SELECTION_SINGLE,
  MD3_BUTTON_GROUP_SELECTION_MULTI,
  MD3_BUTTON_GROUP_SELECTION_MODE_COUNT
};

/**
 * @struct md3_button_group
 * @brief Material 3 Button Group wrapping ui_button_group_base.
 */
struct md3_button_group {
  struct ui_button_group_base *base;
  enum md3_button_group_orientation orientation;
  enum md3_button_group_selection_mode selection_mode;
  struct md3_button **buttons;
  size_t button_count;
  size_t capacity;
  int *selected_states;
  int focused_index;
};

/**
 * @brief Creates a Material 3 Button Group.
 *
 * @param engine Pointer to ui_engine instance.
 * @param orientation Orientation of the group (horizontal or vertical).
 * @param out_group Pointer to receive allocated button group.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_button_group_create(
    struct ui_engine *engine, enum md3_button_group_orientation orientation,
    struct md3_button_group **out_group);

/**
 * @brief Destroys a Material 3 button group.
 *
 * @param group The button group to destroy.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_button_group_destroy(struct md3_button_group *group);

/**
 * @brief Adds a child button to the group.
 *
 * @param group The button group.
 * @param button Button to append.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_button_group_add_button(
    struct md3_button_group *group, struct md3_button *button);

/**
 * @brief Sets the selection mode of the group.
 *
 * @param group The button group.
 * @param mode Selection mode.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_button_group_set_selection_mode(
    struct md3_button_group *group, enum md3_button_group_selection_mode mode);

/**
 * @brief Sets selection state for a specific button index.
 *
 * @param group The button group.
 * @param index Index of button.
 * @param selected 1 to select, 0 to deselect.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_button_group_set_selected(
    struct md3_button_group *group, size_t index, int selected);

/**
 * @brief Retrieves selection state for a button index.
 *
 * @param group The button group.
 * @param index Index of button.
 * @param out_selected Output pointer for selection state.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_button_group_is_selected(
    const struct md3_button_group *group, size_t index, int *out_selected);

/**
 * @brief Roving tabindex navigation helper for arrow keys.
 *
 * @param group The button group.
 * @param delta Direction step (+1 forward, -1 backward).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_button_group_navigate(struct md3_button_group *group, int delta);

/**
 * @struct md3_toggle_button
 * @brief Material 3 Toggle Button wrapping ui_toggle_base.
 */
struct md3_toggle_button {
  struct ui_toggle_base *base;
  int is_selected;
  char text[64];
  char icon_name[32];
};

/**
 * @brief Creates a Material 3 Toggle Button.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_toggle Pointer to receive allocated toggle button.
 * @param out_cva Optional pointer to receive Control Value Accessor.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_toggle_button_create(
    struct ui_engine *engine, struct md3_toggle_button **out_toggle,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Material 3 toggle button.
 *
 * @param toggle The toggle button.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_toggle_button_destroy(struct md3_toggle_button *toggle);

/**
 * @brief Sets selected/pressed state on toggle button.
 *
 * @param toggle The toggle button.
 * @param selected 1 if selected, 0 if unselected.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_toggle_button_set_selected(struct md3_toggle_button *toggle, int selected);

/**
 * @brief Retrieves selected state from toggle button.
 *
 * @param toggle The toggle button.
 * @param out_selected Output pointer for selection state.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_toggle_button_is_selected(
    const struct md3_toggle_button *toggle, int *out_selected);

/**
 * @brief Sets text label on toggle button.
 *
 * @param toggle The toggle button.
 * @param text The label text.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_toggle_button_set_text(struct md3_toggle_button *toggle, const char *text);

/**
 * @brief Sets icon token on toggle button.
 *
 * @param toggle The toggle button.
 * @param icon_name The icon name.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_toggle_button_set_icon(
    struct md3_toggle_button *toggle, const char *icon_name);

/**
 * @enum md3_toolbar_variant
 * @brief Toolbar height metrics and density.
 */
enum md3_toolbar_variant {
  MD3_TOOLBAR_STANDARD = 0, /* 48dp height */
  MD3_TOOLBAR_DENSE,        /* 40dp height */
  MD3_TOOLBAR_VARIANT_COUNT
};

/**
 * @struct md3_toolbar_item
 * @brief Item slot hosted within toolbar.
 */
struct md3_toolbar_item {
  int id;
  float width;
  int is_overflow;
  char title[64];
  struct md3_toolbar_item *next;
};

/**
 * @struct md3_toolbar
 * @brief Material 3 Toolbar wrapping ui_toolbar_base.
 */
struct md3_toolbar {
  struct ui_toolbar_base *base;
  enum md3_toolbar_variant variant;
  float available_width;
  struct md3_toolbar_item *items;
  size_t item_count;
  size_t visible_count;
  size_t overflow_count;
  int focused_index;
};

/**
 * @brief Creates a Material 3 Toolbar.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_toolbar Pointer to receive allocated toolbar.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_toolbar_create(struct ui_engine *engine, struct md3_toolbar **out_toolbar);

/**
 * @brief Destroys a Material 3 toolbar.
 *
 * @param toolbar The toolbar to destroy.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_toolbar_destroy(struct md3_toolbar *toolbar);

/**
 * @brief Sets toolbar density variant (Standard 48dp or Dense 40dp).
 *
 * @param toolbar The toolbar.
 * @param variant The density variant.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_toolbar_set_variant(
    struct md3_toolbar *toolbar, enum md3_toolbar_variant variant);

/**
 * @brief Sets toolbar title.
 *
 * @param toolbar The toolbar.
 * @param title The title string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_toolbar_set_title(struct md3_toolbar *toolbar, const char *title);

/**
 * @brief Adds an action item to toolbar.
 *
 * @param toolbar The toolbar.
 * @param id Unique item identifier.
 * @param title Item label.
 * @param width Layout width consumed in dp.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_toolbar_add_action(
    struct md3_toolbar *toolbar, int id, const char *title, float width);

/**
 * @brief Performs automatic responsive overflow calculation given available
 * width.
 *
 * @param toolbar The toolbar.
 * @param available_width Available container width in dp.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_toolbar_calculate_overflow(
    struct md3_toolbar *toolbar, float available_width);

/**
 * @brief Roving tabindex navigation helper for toolbar actions.
 *
 * @param toolbar The toolbar.
 * @param delta Direction step (+1 forward, -1 backward).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_toolbar_navigate(struct md3_toolbar *toolbar, int delta);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_ACTION_WIDGETS_H */
