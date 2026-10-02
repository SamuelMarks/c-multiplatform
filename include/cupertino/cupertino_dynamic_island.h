/**
 * @file cupertino_dynamic_island.h
 * @brief Dynamic Island morphing container and HIG content templates.
 */

#ifndef CUPERTINO_CUPERTINO_DYNAMIC_ISLAND_H
#define CUPERTINO_CUPERTINO_DYNAMIC_ISLAND_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_ISLAND_COMPACT_WIDTH 126.0f
#define CUPERTINO_ISLAND_COMPACT_HEIGHT 37.0f
#define CUPERTINO_ISLAND_MINIMAL_SIZE 37.0f
#define CUPERTINO_ISLAND_EXPANDED_WIDTH 370.0f
#define CUPERTINO_ISLAND_EXPANDED_HEIGHT 160.0f
#define CUPERTINO_ISLAND_TOP_MARGIN 11.0f

/**
 * @enum cupertino_island_state
 * @brief Dynamic Island morphing display states.
 */
enum cupertino_island_state {
  CUPERTINO_ISLAND_COMPACT = 0, /**< Standard hardware-conforming pill. */
  CUPERTINO_ISLAND_MINIMAL,     /**< Small detached circular indicator dot. */
  CUPERTINO_ISLAND_EXPANDED     /**< Full interactive rich control bubble. */
};

/**
 * @enum cupertino_island_template
 * @brief Dynamic Island content presentation templates.
 */
enum cupertino_island_template {
  CUPERTINO_ISLAND_TEMPLATE_NONE = 0,  /**< Empty / custom children. */
  CUPERTINO_ISLAND_TEMPLATE_CALL,      /**< VoIP phone call with audio wave. */
  CUPERTINO_ISLAND_TEMPLATE_MEDIA,     /**< Now Playing music visualizer. */
  CUPERTINO_ISLAND_TEMPLATE_TIMER,     /**< Countdown progress timer. */
  CUPERTINO_ISLAND_TEMPLATE_NAVIGATION /**< Turn-by-turn navigation guidance. */
};

/**
 * @struct cupertino_dynamic_island_descriptor
 * @brief Configuration descriptor for Dynamic Island container.
 */
struct cupertino_dynamic_island_descriptor {
  enum cupertino_island_state initial_state; /**< Starting morph state. */
  enum cupertino_island_template
      initial_template; /**< Starting content template. */
  float screen_width;   /**< Screen width for centering. */
};

/**
 * @struct cupertino_dynamic_island
 * @brief Dynamic Island container instance.
 */
struct cupertino_dynamic_island {
  enum cupertino_island_state state;
  enum cupertino_island_template template_type;
  float screen_width;
  float current_width;
  float current_height;
  float current_corner_radius;
  float target_width;
  float target_height;
  float target_corner_radius;
  float morph_progress; /**< Spring interpolation progress [0.0, 1.0]. */
  int is_animating;
  ui_color_t background_color;
};

/**
 * @brief Creates a new Dynamic Island container instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_island Pointer to receive newly created island instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dynamic_island_create(
    struct ui_engine *engine,
    const struct cupertino_dynamic_island_descriptor *desc,
    struct cupertino_dynamic_island **out_island);

/**
 * @brief Destroys a Dynamic Island instance.
 *
 * @param island Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_dynamic_island_destroy(struct cupertino_dynamic_island *island);

/**
 * @brief Transitions Dynamic Island to a new morph state.
 *
 * @param island Target island.
 * @param state Target state (Compact, Minimal, Expanded).
 * @param animated 1 for smooth spring transition, 0 for immediate jump.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dynamic_island_set_state(
    struct cupertino_dynamic_island *island, enum cupertino_island_state state,
    int animated);

/**
 * @brief Gets current morph state of Dynamic Island.
 *
 * @param island Target island.
 * @param out_state Pointer to receive state enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dynamic_island_get_state(
    const struct cupertino_dynamic_island *island,
    enum cupertino_island_state *out_state);

/**
 * @brief Sets content presentation template.
 *
 * @param island Target island.
 * @param template_type Template variant (Call, Media, Timer, Navigation).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dynamic_island_set_template(
    struct cupertino_dynamic_island *island,
    enum cupertino_island_template template_type);

/**
 * @brief Gets current content presentation template.
 *
 * @param island Target island.
 * @param out_template Pointer to receive template enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dynamic_island_get_template(
    const struct cupertino_dynamic_island *island,
    enum cupertino_island_template *out_template);

/**
 * @brief Advances spring morphing animation frame.
 *
 * @param island Target island.
 * @param delta_ms Elapsed time in milliseconds.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dynamic_island_tick(
    struct cupertino_dynamic_island *island, float delta_ms);

/**
 * @brief Computes physical bounding geometry for the island container.
 *
 * @param island Target island.
 * @param out_x Pointer to receive top-left X coordinate.
 * @param out_y Pointer to receive top-left Y coordinate.
 * @param out_w Pointer to receive bounding width.
 * @param out_h Pointer to receive bounding height.
 * @param out_radius Pointer to receive continuous squircle corner radius.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dynamic_island_get_bounds(
    const struct cupertino_dynamic_island *island, float *out_x, float *out_y,
    float *out_w, float *out_h, float *out_radius);

/**
 * @brief Handles touch tap or long-press on island to toggle expansion.
 *
 * @param island Target island.
 * @param is_long_press 1 for long-press, 0 for tap.
 * @param out_state_changed Pointer to receive 1 if state changed, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dynamic_island_handle_touch(
    struct cupertino_dynamic_island *island, int is_long_press,
    int *out_state_changed);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_DYNAMIC_ISLAND_H */
