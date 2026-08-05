#ifndef UI_CSS_SPEECH_H
#define UI_CSS_SPEECH_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * @brief CSS speak property.
 */
enum ui_css_speak {
  UI_CSS_SPEAK_AUTO,
  UI_CSS_SPEAK_NEVER,
  UI_CSS_SPEAK_ALWAYS
};

/**
 * @brief CSS speak-as property flags.
 */
enum ui_css_speak_as_flag {
  UI_CSS_SPEAK_AS_NORMAL = 0,
  UI_CSS_SPEAK_AS_SPELL_OUT = 1 << 0,
  UI_CSS_SPEAK_AS_DIGITS = 1 << 1,
  UI_CSS_SPEAK_AS_LITERAL_PUNCTUATION = 1 << 2,
  UI_CSS_SPEAK_AS_NO_PUNCTUATION = 1 << 3
};

/**
 * @brief Speech strength keyword.
 */
enum ui_css_speech_strength {
  UI_CSS_SPEECH_STRENGTH_NONE,
  UI_CSS_SPEECH_STRENGTH_X_WEAK,
  UI_CSS_SPEECH_STRENGTH_WEAK,
  UI_CSS_SPEECH_STRENGTH_MEDIUM,
  UI_CSS_SPEECH_STRENGTH_STRONG,
  UI_CSS_SPEECH_STRENGTH_X_STRONG
};

/**
 * @brief CSS pause-before/after property.
 */
struct ui_css_speech_pause {
  int has_time;
  struct ui_css_value time;
  enum ui_css_speech_strength strength;
};

/**
 * @brief CSS rest-before/after property.
 */
struct ui_css_speech_rest {
  int has_time;
  struct ui_css_value time;
  enum ui_css_speech_strength strength;
};

/**
 * @brief CSS cue-before/after property.
 */
struct ui_css_speech_cue {
  char *uri;
  struct ui_css_value volume_db;
  int has_volume;
};

/**
 * @brief Voice volume keyword.
 */
enum ui_css_voice_volume_keyword {
  UI_CSS_VOICE_VOLUME_SILENT,
  UI_CSS_VOICE_VOLUME_X_SOFT,
  UI_CSS_VOICE_VOLUME_SOFT,
  UI_CSS_VOICE_VOLUME_MEDIUM,
  UI_CSS_VOICE_VOLUME_LOUD,
  UI_CSS_VOICE_VOLUME_X_LOUD
};

/**
 * @brief CSS voice-volume property.
 */
struct ui_css_voice_volume {
  enum ui_css_voice_volume_keyword keyword;
  int has_keyword;
  struct ui_css_value db;
  int has_db;
};

/**
 * @brief Voice rate keyword.
 */
enum ui_css_voice_rate_keyword {
  UI_CSS_VOICE_RATE_NORMAL,
  UI_CSS_VOICE_RATE_X_SLOW,
  UI_CSS_VOICE_RATE_SLOW,
  UI_CSS_VOICE_RATE_MEDIUM,
  UI_CSS_VOICE_RATE_FAST,
  UI_CSS_VOICE_RATE_X_FAST
};

/**
 * @brief CSS voice-rate property.
 */
struct ui_css_voice_rate {
  enum ui_css_voice_rate_keyword keyword;
  int has_keyword;
  struct ui_css_value percentage;
  int has_percentage;
};

/**
 * @brief Voice pitch keyword.
 */
enum ui_css_voice_pitch_keyword {
  UI_CSS_VOICE_PITCH_X_LOW,
  UI_CSS_VOICE_PITCH_LOW,
  UI_CSS_VOICE_PITCH_MEDIUM,
  UI_CSS_VOICE_PITCH_HIGH,
  UI_CSS_VOICE_PITCH_X_HIGH
};

/**
 * @brief CSS voice-pitch property.
 */
struct ui_css_voice_pitch {
  int is_absolute;
  enum ui_css_voice_pitch_keyword keyword;
  int has_keyword;
  struct ui_css_value frequency;
  int has_frequency;
  struct ui_css_value semitones;
  int has_semitones;
  struct ui_css_value percentage;
  int has_percentage;
};

/**
 * @brief Voice stress property.
 */
enum ui_css_voice_stress {
  UI_CSS_VOICE_STRESS_NORMAL,
  UI_CSS_VOICE_STRESS_STRONG,
  UI_CSS_VOICE_STRESS_MODERATE,
  UI_CSS_VOICE_STRESS_NONE,
  UI_CSS_VOICE_STRESS_REDUCED
};

/**
 * @brief Aggregated CSS speech properties.
 */
struct ui_css_speech_properties {
  enum ui_css_speak speak;
  int speak_as_flags;

  struct ui_css_speech_pause pause_before;
  struct ui_css_speech_pause pause_after;

  struct ui_css_speech_rest rest_before;
  struct ui_css_speech_rest rest_after;

  struct ui_css_speech_cue cue_before;
  struct ui_css_speech_cue cue_after;

  struct ui_css_voice_volume voice_volume;

  char *voice_family;

  struct ui_css_voice_rate voice_rate;

  struct ui_css_voice_pitch voice_pitch;
  struct ui_css_voice_pitch voice_range;

  enum ui_css_voice_stress voice_stress;

  struct ui_css_value voice_duration;
  int has_voice_duration;
};

struct ui_css_computed_style; /* Forward declare */

/**
 * @brief Parses the CSS speech properties from a computed style.
 *
 * @param style The computed style.
 * @param out_props Pointer to the struct to populate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_speech_parse(const struct ui_css_computed_style *style,
                               struct ui_css_speech_properties *out_props);

/**
 * @brief Cleans up allocated resources within a speech properties struct.
 *
 * @param props The struct to clean up.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_speech_cleanup(struct ui_css_speech_properties *props);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_SPEECH_H */
