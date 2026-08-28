/**
 * @file ui_css_speech.c
 * @brief ui_css_speech.c implementation.
 */
/* clang-format off */
#include "../include/ui_css_speech.h"
#include "../include/ui_cssom.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "strtok_posix.h"
/* clang-format on */

#if defined(_MSC_VER)
/** @cond */
#define UI_STRTOK(str, delim, ctx) strtok_s((str), (delim), (ctx))
/** @endcond */
#else
/** @cond */
#define UI_STRTOK(str, delim, ctx) strtok_r((str), (delim), (ctx))
/** @endcond */
/**
 * @brief dup_string.
 * @param s Parameter s.
 * @param out_str Parameter out_str.
 * @return Return value.
 */
static ui_error_t dup_string(const char *s, char **out_str) {
  size_t len;
  char *p;
  len = strlen(s) + 1;
  p = (char *)C_MULTIPLATFORM_MALLOC(len);
  if (!p) {
    *out_str = NULL;
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memcpy(p, s, len);
  *out_str = p;
  return UI_ERROR_NONE;
}
#endif

/**
 * @brief skip_whitespace.
 * @param p_str Parameter p_str.
 * @return Return value.
 */
static ui_error_t skip_whitespace(const char **p_str) {
  while (isspace((unsigned char)**p_str)) {
    (*p_str)++;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief parse_speech_strength.
 * @param str Parameter str.
 * @param out_strength Parameter out_strength.
 * @return Return value.
 */
static void parse_speech_strength(const char *str,
                                  enum ui_css_speech_strength *out_strength) {
  if (strcmp(str, "none") == 0) {
    *out_strength = UI_CSS_SPEECH_STRENGTH_NONE;
  } else if (strcmp(str, "x-weak") == 0) {
    *out_strength = UI_CSS_SPEECH_STRENGTH_X_WEAK;
  } else if (strcmp(str, "weak") == 0) {
    *out_strength = UI_CSS_SPEECH_STRENGTH_WEAK;
  } else if (strcmp(str, "medium") == 0) {
    *out_strength = UI_CSS_SPEECH_STRENGTH_MEDIUM;
  } else if (strcmp(str, "strong") == 0) {
    *out_strength = UI_CSS_SPEECH_STRENGTH_STRONG;
  } else if (strcmp(str, "x-strong") == 0) {
    *out_strength = UI_CSS_SPEECH_STRENGTH_X_STRONG;
  } else {
    /* should not be reached for well-formed keywords, but handle safely */
    *out_strength = UI_CSS_SPEECH_STRENGTH_NONE;
  }
}

/**
 * @brief parse_pause_or_rest.
 * @param str Parameter str.
 * @param out_val Parameter out_val.
 * @return Return value.
 */
static ui_error_t parse_pause_or_rest(const char *str,
                                      struct ui_css_speech_pause *out_val) {
  ui_error_t parse_rc;
  out_val->has_time = 0;
  out_val->strength = UI_CSS_SPEECH_STRENGTH_NONE;

  parse_rc = ui_css_parse_value(str, &out_val->time);
  (void)parse_rc;
  if (parse_rc == UI_ERROR_NONE && (out_val->time.unit == UI_CSS_UNIT_S ||
                                    out_val->time.unit == UI_CSS_UNIT_MS)) {
    out_val->has_time = 1;
    return UI_ERROR_NONE;
  } else {
    enum ui_css_speech_strength strength;
    parse_speech_strength(str, &strength);
    if (strength == UI_CSS_SPEECH_STRENGTH_NONE && strcmp(str, "none") != 0) {
      return UI_ERROR_PARSE_FAILED;
    }
    out_val->strength = strength;
    return UI_ERROR_NONE;
  }
}

/**
 * @brief parse_cue.
 * @param str Parameter str.
 * @param out_val Parameter out_val.
 * @return Return value.
 */
static ui_error_t parse_cue(const char *str,
                            struct ui_css_speech_cue *out_val) {
  out_val->uri = NULL;
  out_val->has_volume = 0;
  if (strcmp(str, "none") == 0)
    return UI_ERROR_NONE;

  if (strncmp(str, "url(", 4) == 0) {
    const char *start = str + 4;
    const char *end = strchr(start, ')');
    if (end) {
      size_t len = (size_t)(end - start);
      if (start[0] == '"' || start[0] == '\'') {
        start++;
        len -= 2;
      }
      out_val->uri = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
      if (out_val->uri) {
        memcpy(out_val->uri, start, len);
        out_val->uri[len] = '\0';
      }

      /* Look for decibel value after url(...) */
      {
        const char *after = end + 1;
        {
          ui_error_t sw_rc = skip_whitespace(&after);
          (void)sw_rc;
        }
        if (*after) {
          ui_error_t pv_rc = ui_css_parse_value(after, &out_val->volume_db);
          (void)pv_rc;
          if (pv_rc == UI_ERROR_NONE) {
            out_val->has_volume = 1;
          }
        }
      }
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief parse_voice_volume.
 * @param str Parameter str.
 * @param out_val Parameter out_val.
 * @return Return value.
 */
static ui_error_t parse_voice_volume(const char *str,
                                     struct ui_css_voice_volume *out_val) {
  out_val->has_keyword = 0;
  out_val->has_db = 0;

  if (strcmp(str, "silent") == 0) {
    out_val->has_keyword = 1;
    out_val->keyword = UI_CSS_VOICE_VOLUME_SILENT;
    return UI_ERROR_NONE;
  }

  {
    ui_error_t parse_rc = ui_css_parse_value(str, &out_val->db);
    (void)parse_rc;
    if (parse_rc == UI_ERROR_NONE && out_val->db.unit == UI_CSS_UNIT_DB) {
      out_val->has_db = 1;
      return UI_ERROR_NONE;
    }
  }

  out_val->has_keyword = 1;
  if (strcmp(str, "x-soft") == 0)
    out_val->keyword = UI_CSS_VOICE_VOLUME_X_SOFT;
  else if (strcmp(str, "soft") == 0)
    out_val->keyword = UI_CSS_VOICE_VOLUME_SOFT;
  else if (strcmp(str, "medium") == 0)
    out_val->keyword = UI_CSS_VOICE_VOLUME_MEDIUM;
  else if (strcmp(str, "loud") == 0)
    out_val->keyword = UI_CSS_VOICE_VOLUME_LOUD;
  else if (strcmp(str, "x-loud") == 0)
    out_val->keyword = UI_CSS_VOICE_VOLUME_X_LOUD;
  else
    out_val->has_keyword = 0;
  return UI_ERROR_NONE;
}

/**
 * @brief parse_voice_rate.
 * @param str Parameter str.
 * @param out_val Parameter out_val.
 * @return Return value.
 */
static ui_error_t parse_voice_rate(const char *str,
                                   struct ui_css_voice_rate *out_val) {
  out_val->has_keyword = 0;
  out_val->has_percentage = 0;

  {
    ui_error_t parse_rc = ui_css_parse_value(str, &out_val->percentage);
    (void)parse_rc;
    if (parse_rc == UI_ERROR_NONE &&
        out_val->percentage.unit == UI_CSS_UNIT_PERCENT) {
      out_val->has_percentage = 1;
      return UI_ERROR_NONE;
    }
  }

  out_val->has_keyword = 1;
  if (strcmp(str, "normal") == 0)
    out_val->keyword = UI_CSS_VOICE_RATE_NORMAL;
  else if (strcmp(str, "x-slow") == 0)
    out_val->keyword = UI_CSS_VOICE_RATE_X_SLOW;
  else if (strcmp(str, "slow") == 0)
    out_val->keyword = UI_CSS_VOICE_RATE_SLOW;
  else if (strcmp(str, "medium") == 0)
    out_val->keyword = UI_CSS_VOICE_RATE_MEDIUM;
  else if (strcmp(str, "fast") == 0)
    out_val->keyword = UI_CSS_VOICE_RATE_FAST;
  else if (strcmp(str, "x-fast") == 0)
    out_val->keyword = UI_CSS_VOICE_RATE_X_FAST;
  else
    out_val->has_keyword = 0;
  return UI_ERROR_NONE;
}

/**
 * @brief parse_voice_pitch.
 * @param str Parameter str.
 * @param out_val Parameter out_val.
 * @return Return value.
 */
static ui_error_t parse_voice_pitch(const char *str,
                                    struct ui_css_voice_pitch *out_val) {
  char token_buf[128];
  char *token;
  char *next_token = NULL;

  out_val->is_absolute = 0;
  out_val->has_keyword = 0;
  out_val->has_frequency = 0;
  out_val->has_semitones = 0;
  out_val->has_percentage = 0;

  UI_STRNCPY(token_buf, sizeof(token_buf), str, sizeof(token_buf) - 1);
  token_buf[sizeof(token_buf) - 1] = '\0';

  token = UI_STRTOK(token_buf, " ", &next_token);
  while (token) {
    if (strcmp(token, "absolute") == 0) {
      out_val->is_absolute = 1;
    } else if (strcmp(token, "x-low") == 0) {
      out_val->has_keyword = 1;
      out_val->keyword = UI_CSS_VOICE_PITCH_X_LOW;
    } else if (strcmp(token, "low") == 0) {
      out_val->has_keyword = 1;
      out_val->keyword = UI_CSS_VOICE_PITCH_LOW;
    } else if (strcmp(token, "medium") == 0) {
      out_val->has_keyword = 1;
      out_val->keyword = UI_CSS_VOICE_PITCH_MEDIUM;
    } else if (strcmp(token, "high") == 0) {
      out_val->has_keyword = 1;
      out_val->keyword = UI_CSS_VOICE_PITCH_HIGH;
    } else if (strcmp(token, "x-high") == 0) {
      out_val->has_keyword = 1;
      out_val->keyword = UI_CSS_VOICE_PITCH_X_HIGH;
    } else {
      struct ui_css_value val;
      {
        ui_error_t rc = ui_css_parse_value(token, &val);
        (void)rc;
        if (rc == UI_ERROR_NONE) {
          if (val.unit == UI_CSS_UNIT_HZ || val.unit == UI_CSS_UNIT_KHZ) {
            out_val->has_frequency = 1;
            out_val->frequency = val;
          } else if (val.unit == UI_CSS_UNIT_ST) {
            out_val->has_semitones = 1;
            out_val->semitones = val;
          } else if (val.unit == UI_CSS_UNIT_PERCENT) {
            out_val->has_percentage = 1;
            out_val->percentage = val;
          }
        }
      }
    }
    token = UI_STRTOK(NULL, " ", &next_token);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_css_speech_parse.
 * @param style Parameter style.
 * @param out_props Parameter out_props.
 * @return Return value.
 */
ui_error_t ui_css_speech_parse(const struct ui_css_computed_style *style,
                               struct ui_css_speech_properties *out_props) {
  const char *val_str;
  ui_error_t attr_rc;

  if (!style || !out_props)
    return UI_ERROR_INVALID_ARGUMENT;

  memset(out_props, 0, sizeof(struct ui_css_speech_properties));

  /* Default values */
  out_props->speak = UI_CSS_SPEAK_AUTO;
  out_props->speak_as_flags = UI_CSS_SPEAK_AS_NORMAL;
  out_props->voice_stress = UI_CSS_VOICE_STRESS_NORMAL;

  attr_rc = ui_css_computed_style_get_property(style, "speak", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    if (strcmp(val_str, "never") == 0)
      out_props->speak = UI_CSS_SPEAK_NEVER;
    else if (strcmp(val_str, "always") == 0)
      out_props->speak = UI_CSS_SPEAK_ALWAYS;
  }

  attr_rc = ui_css_computed_style_get_property(style, "speak-as", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    if (strstr(val_str, "spell-out"))
      out_props->speak_as_flags |= UI_CSS_SPEAK_AS_SPELL_OUT;
    if (strstr(val_str, "digits"))
      out_props->speak_as_flags |= UI_CSS_SPEAK_AS_DIGITS;
    if (strstr(val_str, "literal-punctuation"))
      out_props->speak_as_flags |= UI_CSS_SPEAK_AS_LITERAL_PUNCTUATION;
    if (strstr(val_str, "no-punctuation"))
      out_props->speak_as_flags |= UI_CSS_SPEAK_AS_NO_PUNCTUATION;
  }

  attr_rc = ui_css_computed_style_get_property(style, "pause-before", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    ui_error_t parse_rc =
        parse_pause_or_rest(val_str, &out_props->pause_before);
    (void)parse_rc;
  }

  attr_rc = ui_css_computed_style_get_property(style, "pause-after", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    ui_error_t parse_rc = parse_pause_or_rest(val_str, &out_props->pause_after);
    (void)parse_rc;
  }

  attr_rc = ui_css_computed_style_get_property(style, "rest-before", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    ui_error_t parse_rc = parse_pause_or_rest(
        val_str, (struct ui_css_speech_pause *)&out_props->rest_before);
    (void)parse_rc;
  }

  attr_rc = ui_css_computed_style_get_property(style, "rest-after", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    ui_error_t parse_rc = parse_pause_or_rest(
        val_str, (struct ui_css_speech_pause *)&out_props->rest_after);
    (void)parse_rc;
  }

  attr_rc = ui_css_computed_style_get_property(style, "cue-before", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    ui_error_t parse_rc = parse_cue(val_str, &out_props->cue_before);
    (void)parse_rc;
  }

  attr_rc = ui_css_computed_style_get_property(style, "cue-after", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    ui_error_t parse_rc = parse_cue(val_str, &out_props->cue_after);
    (void)parse_rc;
  }

  attr_rc = ui_css_computed_style_get_property(style, "voice-volume", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    ui_error_t parse_rc = parse_voice_volume(val_str, &out_props->voice_volume);
    (void)parse_rc;
  }

  attr_rc = ui_css_computed_style_get_property(style, "voice-family", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    {
#if defined(_MSC_VER)
      out_props->voice_family = C_MULTIPLATFORM_STRDUP(val_str);
#else
      {
        ui_error_t dup_rc = dup_string(val_str, &out_props->voice_family);
        if (dup_rc != UI_ERROR_NONE)
          return dup_rc;
      }
#endif
    }
  }

  attr_rc = ui_css_computed_style_get_property(style, "voice-rate", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    ui_error_t parse_rc = parse_voice_rate(val_str, &out_props->voice_rate);
    (void)parse_rc;
  }

  attr_rc = ui_css_computed_style_get_property(style, "voice-pitch", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    ui_error_t parse_rc = parse_voice_pitch(val_str, &out_props->voice_pitch);
    (void)parse_rc;
  }

  attr_rc = ui_css_computed_style_get_property(style, "voice-range", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    ui_error_t parse_rc = parse_voice_pitch(val_str, &out_props->voice_range);
    (void)parse_rc;
  }

  attr_rc = ui_css_computed_style_get_property(style, "voice-stress", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    if (strcmp(val_str, "strong") == 0)
      out_props->voice_stress = UI_CSS_VOICE_STRESS_STRONG;
    else if (strcmp(val_str, "moderate") == 0)
      out_props->voice_stress = UI_CSS_VOICE_STRESS_MODERATE;
    else if (strcmp(val_str, "none") == 0)
      out_props->voice_stress = UI_CSS_VOICE_STRESS_NONE;
    else if (strcmp(val_str, "reduced") == 0)
      out_props->voice_stress = UI_CSS_VOICE_STRESS_REDUCED;
  }

  attr_rc =
      ui_css_computed_style_get_property(style, "voice-duration", &val_str);
  (void)attr_rc;
  if (attr_rc == UI_ERROR_NONE) {
    if (strcmp(val_str, "auto") != 0) {
      ui_error_t pd_rc =
          ui_css_parse_value(val_str, &out_props->voice_duration);
      (void)pd_rc;
      if (pd_rc == UI_ERROR_NONE)
        out_props->has_voice_duration = 1;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_css_speech_cleanup.
 * @param props Parameter props.
 * @return Return value.
 */
ui_error_t ui_css_speech_cleanup(struct ui_css_speech_properties *props) {
  if (!props)
    return UI_ERROR_INVALID_ARGUMENT;

  if (props->cue_before.uri)
    C_MULTIPLATFORM_FREE(props->cue_before.uri);
  if (props->cue_after.uri)
    C_MULTIPLATFORM_FREE(props->cue_after.uri);
  if (props->voice_family)
    C_MULTIPLATFORM_FREE(props->voice_family);

  props->cue_before.uri = NULL;
  props->cue_after.uri = NULL;
  props->voice_family = NULL;
  return UI_ERROR_NONE;
}
