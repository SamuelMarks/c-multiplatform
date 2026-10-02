/**
 * @file cupertino_status_bar.h
 * @brief iOS Safe Area & Translucent Status Bar component conforming to Apple
 * HIG.
 */

#ifndef CUPERTINO_CUPERTINO_STATUS_BAR_H
#define CUPERTINO_CUPERTINO_STATUS_BAR_H

/* clang-format off */
#include "ui_error.h"
#include "ui_safe_area_manager.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum cupertino_status_bar_style
 * @brief Text and icon rendering appearance modes for the status bar.
 */
enum cupertino_status_bar_style {
  CUPERTINO_STATUS_BAR_STYLE_DEFAULT =
      0, /**< Dark content (black text/icons for light backdrops). */
  CUPERTINO_STATUS_BAR_STYLE_LIGHT_CONTENT, /**< Light content (white text/icons
                                               for dark backdrops). */
  CUPERTINO_STATUS_BAR_STYLE_TRANSLUCENT    /**< Translucent blur matching
                                               navigation bar material. */
};

/**
 * @enum cupertino_hardware_device_preset
 * @brief Standard Apple hardware safe area preset configurations.
 */
enum cupertino_hardware_device_preset {
  CUPERTINO_DEVICE_PRESET_DYNAMIC_ISLAND =
      0, /**< iPhone 14 Pro/15/16: Top 54pt, Bottom 34pt. */
  CUPERTINO_DEVICE_PRESET_NOTCH,   /**< iPhone X/11/12/13/14: Top 47pt, Bottom
                                      34pt. */
  CUPERTINO_DEVICE_PRESET_CLASSIC, /**< Classic iPhone with Home button: Top
                                      20pt, Bottom 0pt. */
  CUPERTINO_DEVICE_PRESET_IPAD     /**< iPad standard: Top 24pt, Bottom 20pt. */
};

/**
 * @struct cupertino_status_bar_descriptor
 * @brief Initialization descriptor for the status bar and safe area host.
 */
struct cupertino_status_bar_descriptor {
  enum cupertino_status_bar_style style; /**< Visual appearance style. */
  enum cupertino_hardware_device_preset
      preset;                    /**< Hardware geometry profile. */
  const char *initial_time_text; /**< Initial clock string (or NULL). */
  float initial_battery_level;   /**< Battery fraction in [0.0, 1.0]. */
  int is_charging;               /**< 1 if connected to power. */
  int cellular_bars;             /**< Cellular reception bars [0-4]. */
  int wifi_bars;                 /**< Wi-Fi signal arcs [0-3]. */
};

/**
 * @struct cupertino_status_bar
 * @brief Status bar instance integrating hardware safe area and status
 * readouts.
 */
struct cupertino_status_bar {
  enum cupertino_status_bar_style style; /**< Active style. */
  struct ui_safe_area_insets insets;     /**< Active safe area margins. */
  char time_text[32];                    /**< Digital clock readout. */
  float battery_level;                   /**< Battery percentage fraction. */
  int is_charging;                       /**< Battery charging state. */
  int cellular_bars;                     /**< Cellular signal level. */
  int wifi_bars;                         /**< Wi-Fi signal level. */
  int is_hidden;                         /**< Hidden visibility flag. */
  struct ui_safe_area_manager *manager;  /**< Bound safe area manager. */
};

/**
 * @brief Creates a new Cupertino status bar and safe area host.
 *
 * @param engine Pointer to ui_engine instance.
 * @param desc Configuration descriptor.
 * @param out_status_bar Pointer to receive allocated status bar.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_status_bar_create(struct ui_engine *engine,
                            const struct cupertino_status_bar_descriptor *desc,
                            struct cupertino_status_bar **out_status_bar);

/**
 * @brief Destroys a Cupertino status bar instance.
 *
 * @param bar Status bar to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_status_bar_destroy(struct cupertino_status_bar *bar);

/**
 * @brief Updates the visual style (Dark, Light, or Translucent).
 *
 * @param bar Target status bar.
 * @param style Appearance style.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_status_bar_set_style(
    struct cupertino_status_bar *bar, enum cupertino_status_bar_style style);

/**
 * @brief Gets the current visual style.
 *
 * @param bar Target status bar.
 * @param out_style Pointer to receive style.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_status_bar_get_style(const struct cupertino_status_bar *bar,
                               enum cupertino_status_bar_style *out_style);

/**
 * @brief Applies hardware safe area insets to the status bar.
 *
 * @param bar Target status bar.
 * @param insets Pointer to safe area insets.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_status_bar_apply_insets(
    struct cupertino_status_bar *bar, const struct ui_safe_area_insets *insets);

/**
 * @brief Gets current safe area insets.
 *
 * @param bar Target status bar.
 * @param out_insets Pointer to receive insets.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_status_bar_get_insets(const struct cupertino_status_bar *bar,
                                struct ui_safe_area_insets *out_insets);

/**
 * @brief Binds a CDK safe area manager for dynamic hardware updates.
 *
 * @param bar Target status bar.
 * @param manager Safe area manager instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_status_bar_set_safe_area_manager(
    struct cupertino_status_bar *bar, struct ui_safe_area_manager *manager);

/**
 * @brief Sets the clock readout text string (e.g. "9:41").
 *
 * @param bar Target status bar.
 * @param time_text Clock text (or NULL to default to "9:41").
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_status_bar_set_time_text(
    struct cupertino_status_bar *bar, const char *time_text);

/**
 * @brief Gets the current clock readout text string.
 *
 * @param bar Target status bar.
 * @param out_time_text Pointer to receive pointer to time string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_status_bar_get_time_text(
    const struct cupertino_status_bar *bar, const char **out_time_text);

/**
 * @brief Sets the battery percentage and charging indicator.
 *
 * @param bar Target status bar.
 * @param level Battery fraction in [0.0, 1.0].
 * @param is_charging 1 if charging, 0 if on battery.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_status_bar_set_battery(
    struct cupertino_status_bar *bar, float level, int is_charging);

/**
 * @brief Gets the battery status.
 *
 * @param bar Target status bar.
 * @param out_level Pointer to receive battery level fraction.
 * @param out_is_charging Pointer to receive charging flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_status_bar_get_battery(const struct cupertino_status_bar *bar,
                                 float *out_level, int *out_is_charging);

/**
 * @brief Sets cellular and Wi-Fi signal indicator levels.
 *
 * @param bar Target status bar.
 * @param cellular_bars Signal strength [0-4].
 * @param wifi_bars Signal strength [0-3].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_status_bar_set_signal(
    struct cupertino_status_bar *bar, int cellular_bars, int wifi_bars);

/**
 * @brief Gets cellular and Wi-Fi signal indicator levels.
 *
 * @param bar Target status bar.
 * @param out_cellular_bars Pointer to receive cellular bars.
 * @param out_wifi_bars Pointer to receive Wi-Fi arcs.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_status_bar_get_signal(const struct cupertino_status_bar *bar,
                                int *out_cellular_bars, int *out_wifi_bars);

/**
 * @brief Computes status bar layout bounds based on screen width and safe area.
 *
 * @param bar Target status bar.
 * @param screen_w Physical or point width of the screen.
 * @param out_w Pointer to receive computed bar width.
 * @param out_h Pointer to receive computed bar height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_status_bar_get_bounds(const struct cupertino_status_bar *bar,
                                float screen_w, float *out_w, float *out_h);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_STATUS_BAR_H */
