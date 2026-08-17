/**
 * @file ui_css_speech.h
 * @brief CSS Speech properties and parsing.
 *
 * This header defines structures, enumerations, and functions for handling
 * CSS speech properties (e.g., speak, voice-volume, voice-rate, pauses, and
 * cues).
 */

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
  UI_CSS_SPEAK_AUTO,  /**< Automatic speaking behavior. */
  UI_CSS_SPEAK_NEVER, /**< Never speak. */
  UI_CSS_SPEAK_ALWAYS /**< Always speak. */
};

/**
 * @brief CSS speak-as property flags.
 */
enum ui_css_speak_as_flag {
  UI_CSS_SPEAK_AS_NORMAL = 0,         /**< Speak as normal. */
  UI_CSS_SPEAK_AS_SPELL_OUT = 1 << 0, /**< Spell out the text. */
  UI_CSS_SPEAK_AS_DIGITS = 1 << 1,    /**< Read digits individually. */
  UI_CSS_SPEAK_AS_LITERAL_PUNCTUATION =
      1 << 2,                             /**< Speak punctuation literally. */
  UI_CSS_SPEAK_AS_NO_PUNCTUATION = 1 << 3 /**< Do not speak punctuation. */
};

/**
 * @brief Speech strength keyword.
 */
enum ui_css_speech_strength {
  UI_CSS_SPEECH_STRENGTH_NONE,    /**< No speech strength. */
  UI_CSS_SPEECH_STRENGTH_X_WEAK,  /**< Extra weak speech strength. */
  UI_CSS_SPEECH_STRENGTH_WEAK,    /**< Weak speech strength. */
  UI_CSS_SPEECH_STRENGTH_MEDIUM,  /**< Medium speech strength. */
  UI_CSS_SPEECH_STRENGTH_STRONG,  /**< Strong speech strength. */
  UI_CSS_SPEECH_STRENGTH_X_STRONG /**< Extra strong speech strength. */
};

/**
 * @brief CSS pause-before/after property.
 */
struct ui_css_speech_pause {
  int has_time; /**< 1 if a specific time is provided, 0 otherwise. */
  struct ui_css_value time;             /**< The pause time value. */
  enum ui_css_speech_strength strength; /**< The strength of the pause. */
};

/**
 * @brief CSS rest-before/after property.
 */
struct ui_css_speech_rest {
  int has_time; /**< 1 if a specific time is provided, 0 otherwise. */
  struct ui_css_value time;             /**< The rest time value. */
  enum ui_css_speech_strength strength; /**< The strength of the rest. */
};

/**
 * @brief CSS cue-before/after property.
 */
struct ui_css_speech_cue {
  char *uri; /**< The URI of the cue media. */
  struct ui_css_value
      volume_db;  /**< The volume level of the cue in decibels. */
  int has_volume; /**< 1 if volume is specified, 0 otherwise. */
};

/**
 * @brief Voice volume keyword.
 */
enum ui_css_voice_volume_keyword {
  UI_CSS_VOICE_VOLUME_SILENT, /**< Silent volume. */
  UI_CSS_VOICE_VOLUME_X_SOFT, /**< Extra soft volume. */
  UI_CSS_VOICE_VOLUME_SOFT,   /**< Soft volume. */
  UI_CSS_VOICE_VOLUME_MEDIUM, /**< Medium volume. */
  UI_CSS_VOICE_VOLUME_LOUD,   /**< Loud volume. */
  UI_CSS_VOICE_VOLUME_X_LOUD  /**< Extra loud volume. */
};

/**
 * @brief CSS voice-volume property.
 */
struct ui_css_voice_volume {
  enum ui_css_voice_volume_keyword keyword; /**< The voice volume keyword. */
  int has_keyword;        /**< 1 if a keyword is used, 0 otherwise. */
  struct ui_css_value db; /**< The specific decibel level. */
  int has_db; /**< 1 if a specific decibel level is provided, 0 otherwise. */
};

/**
 * @brief Voice rate keyword.
 */
enum ui_css_voice_rate_keyword {
  UI_CSS_VOICE_RATE_NORMAL, /**< Normal rate. */
  UI_CSS_VOICE_RATE_X_SLOW, /**< Extra slow rate. */
  UI_CSS_VOICE_RATE_SLOW,   /**< Slow rate. */
  UI_CSS_VOICE_RATE_MEDIUM, /**< Medium rate. */
  UI_CSS_VOICE_RATE_FAST,   /**< Fast rate. */
  UI_CSS_VOICE_RATE_X_FAST  /**< Extra fast rate. */
};

/**
 * @brief CSS voice-rate property.
 */
struct ui_css_voice_rate {
  enum ui_css_voice_rate_keyword keyword; /**< The voice rate keyword. */
  int has_keyword;                /**< 1 if a keyword is used, 0 otherwise. */
  struct ui_css_value percentage; /**< The specific rate percentage. */
  int has_percentage; /**< 1 if a specific percentage is provided, 0 otherwise.
                       */
};

/**
 * @brief Voice pitch keyword.
 */
enum ui_css_voice_pitch_keyword {
  UI_CSS_VOICE_PITCH_X_LOW,  /**< Extra low pitch. */
  UI_CSS_VOICE_PITCH_LOW,    /**< Low pitch. */
  UI_CSS_VOICE_PITCH_MEDIUM, /**< Medium pitch. */
  UI_CSS_VOICE_PITCH_HIGH,   /**< High pitch. */
  UI_CSS_VOICE_PITCH_X_HIGH  /**< Extra high pitch. */
};

/**
 * @brief CSS voice-pitch property.
 */
struct ui_css_voice_pitch {
  int is_absolute; /**< 1 if pitch is an absolute value, 0 if relative. */
  enum ui_css_voice_pitch_keyword keyword; /**< The pitch keyword. */
  int has_keyword;               /**< 1 if a keyword is used, 0 otherwise. */
  struct ui_css_value frequency; /**< Pitch frequency value. */
  int has_frequency; /**< 1 if a frequency is specified, 0 otherwise. */
  struct ui_css_value semitones; /**< Pitch offset in semitones. */
  int has_semitones; /**< 1 if semitones are specified, 0 otherwise. */
  struct ui_css_value percentage; /**< Pitch adjustment percentage. */
  int has_percentage; /**< 1 if percentage is specified, 0 otherwise. */
};

/**
 * @brief Voice stress property.
 */
enum ui_css_voice_stress {
  UI_CSS_VOICE_STRESS_NORMAL,   /**< Normal voice stress. */
  UI_CSS_VOICE_STRESS_STRONG,   /**< Strong voice stress. */
  UI_CSS_VOICE_STRESS_MODERATE, /**< Moderate voice stress. */
  UI_CSS_VOICE_STRESS_NONE,     /**< No voice stress. */
  UI_CSS_VOICE_STRESS_REDUCED   /**< Reduced voice stress. */
};

/**
 * @brief Aggregated CSS speech properties.
 */
struct ui_css_speech_properties {
  enum ui_css_speak speak; /**< The speak property. */
  int speak_as_flags;      /**< Bitmask of `ui_css_speak_as_flag`. */

  struct ui_css_speech_pause
      pause_before; /**< The pause-before configuration. */
  struct ui_css_speech_pause pause_after; /**< The pause-after configuration. */

  struct ui_css_speech_rest rest_before; /**< The rest-before configuration. */
  struct ui_css_speech_rest rest_after;  /**< The rest-after configuration. */

  struct ui_css_speech_cue cue_before; /**< The cue-before configuration. */
  struct ui_css_speech_cue cue_after;  /**< The cue-after configuration. */

  struct ui_css_voice_volume
      voice_volume; /**< The voice-volume configuration. */

  char *voice_family; /**< A string indicating the voice-family list. */

  struct ui_css_voice_rate voice_rate; /**< The voice-rate configuration. */

  struct ui_css_voice_pitch voice_pitch; /**< The voice-pitch configuration. */
  struct ui_css_voice_pitch voice_range; /**< The voice-range configuration. */

  enum ui_css_voice_stress voice_stress; /**< The voice-stress configuration. */

  struct ui_css_value voice_duration; /**< The duration for speech readout. */
  int has_voice_duration; /**< 1 if a specific duration is provided, 0
                             otherwise. */
};

/**
 * @brief Forward declaration of the computed style structure.
 */
struct ui_css_computed_style;

/**
 * @brief Parses the CSS speech properties from a computed style.
 *
 * @param style Pointer to the computed style.
 * @param out_props Pointer to the structure to populate with parsed properties.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_speech_parse(const struct ui_css_computed_style *style,
                               struct ui_css_speech_properties *out_props);

/**
 * @brief Cleans up allocated resources within a speech properties struct.
 *
 * @param props Pointer to the properties structure to clean up.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_speech_cleanup(struct ui_css_speech_properties *props);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_SPEECH_H */
