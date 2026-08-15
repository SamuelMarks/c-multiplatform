/* clang-format off */
#include "../include/ui_css_speech.h"
#include "../include/ui_cssom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define TEST_ASSERT(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "Assertion failed: %s at %s:%d\n", #cond, __FILE__,      \
              __LINE__);                                                       \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

static void test_properties(const char *decl_prop, const char *decl_val,
                            void (*verify)(struct ui_css_speech_properties *)) {
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_css_computed_style *style = NULL;
  struct ui_css_speech_properties props;

  ui_css_stylesheet_create(&sheet);
  ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(rule, decl_prop, decl_val, 0);
  ui_css_stylesheet_append_rule(sheet, rule);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");
  ui_css_resolve_style(sheet, node, &style);

  if (ui_css_speech_parse(style, &props) == UI_ERROR_NONE) {
    if (verify)
      verify(&props);
    ui_css_speech_cleanup(&props);
  }

  ui_css_computed_style_destroy(style);
  (void)ui_dom_node_destroy(node);
  ui_css_stylesheet_destroy(sheet);
}

static void verify_speak_never(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->speak == UI_CSS_SPEAK_NEVER);
}
static void verify_speak_as(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->speak_as_flags & UI_CSS_SPEAK_AS_LITERAL_PUNCTUATION);
  TEST_ASSERT(props->speak_as_flags & UI_CSS_SPEAK_AS_NO_PUNCTUATION);
}
static void verify_pause(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->pause_before.strength == UI_CSS_SPEECH_STRENGTH_NONE);
  TEST_ASSERT(props->pause_after.strength == UI_CSS_SPEECH_STRENGTH_X_WEAK);
}
static void verify_pause2(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->pause_after.strength == UI_CSS_SPEECH_STRENGTH_MEDIUM);
}
static void verify_pause3(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->pause_after.strength == UI_CSS_SPEECH_STRENGTH_X_STRONG);
}
static void verify_rest(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->rest_before.strength == UI_CSS_SPEECH_STRENGTH_NONE);
  TEST_ASSERT(props->rest_after.strength == UI_CSS_SPEECH_STRENGTH_NONE);
}
static void verify_cue(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->cue_after.uri != NULL);
  TEST_ASSERT(strcmp(props->cue_after.uri, "a.wav") == 0);
  TEST_ASSERT(props->cue_after.has_volume == 0);
  TEST_ASSERT(props->cue_before.uri == NULL);
}
static void verify_cue_quotes(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->cue_before.uri != NULL);
  TEST_ASSERT(strcmp(props->cue_before.uri, "b.wav") == 0);
}
static void verify_voice_vol(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_volume.has_keyword == 1);
  TEST_ASSERT(props->voice_volume.keyword == UI_CSS_VOICE_VOLUME_SILENT);
}
static void verify_voice_vol2(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_volume.has_keyword == 1);
  TEST_ASSERT(props->voice_volume.keyword == UI_CSS_VOICE_VOLUME_X_SOFT);
}
static void verify_voice_vol3(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_volume.has_keyword == 1);
  TEST_ASSERT(props->voice_volume.keyword == UI_CSS_VOICE_VOLUME_SOFT);
}
static void verify_voice_vol4(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_volume.has_keyword == 1);
  TEST_ASSERT(props->voice_volume.keyword == UI_CSS_VOICE_VOLUME_MEDIUM);
}
static void verify_voice_vol5(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_volume.has_keyword == 1);
  TEST_ASSERT(props->voice_volume.keyword == UI_CSS_VOICE_VOLUME_X_LOUD);
}
static void verify_voice_vol6(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_volume.has_db == 1);
  TEST_ASSERT(props->voice_volume.db.value == 10.0f);
}
static void verify_voice_rate(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_rate.has_keyword == 1);
  TEST_ASSERT(props->voice_rate.keyword == UI_CSS_VOICE_RATE_NORMAL);
}
static void verify_voice_rate2(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_rate.has_keyword == 1);
  TEST_ASSERT(props->voice_rate.keyword == UI_CSS_VOICE_RATE_SLOW);
}
static void verify_voice_rate3(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_rate.has_keyword == 1);
  TEST_ASSERT(props->voice_rate.keyword == UI_CSS_VOICE_RATE_MEDIUM);
}
static void verify_voice_rate4(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_rate.has_keyword == 1);
  TEST_ASSERT(props->voice_rate.keyword == UI_CSS_VOICE_RATE_FAST);
}
static void verify_voice_rate5(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_rate.has_keyword == 1);
  TEST_ASSERT(props->voice_rate.keyword == UI_CSS_VOICE_RATE_X_SLOW);
}
static void verify_voice_rate6(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_rate.has_percentage == 1);
  TEST_ASSERT(props->voice_rate.percentage.value == 50.0f);
}
static void verify_voice_pitch(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_pitch.has_keyword == 1);
  TEST_ASSERT(props->voice_pitch.keyword == UI_CSS_VOICE_PITCH_X_LOW);
}
static void verify_voice_pitch2(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_pitch.has_keyword == 1);
  TEST_ASSERT(props->voice_pitch.keyword == UI_CSS_VOICE_PITCH_LOW);
}
static void verify_voice_pitch3(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_pitch.has_keyword == 1);
  TEST_ASSERT(props->voice_pitch.keyword == UI_CSS_VOICE_PITCH_MEDIUM);
}
static void verify_voice_pitch4(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_pitch.has_keyword == 1);
  TEST_ASSERT(props->voice_pitch.keyword == UI_CSS_VOICE_PITCH_HIGH);
}
static void verify_voice_pitch5(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_pitch.has_keyword == 1);
  TEST_ASSERT(props->voice_pitch.keyword == UI_CSS_VOICE_PITCH_X_HIGH);
}
static void verify_voice_pitch6(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_pitch.has_semitones == 1);
  TEST_ASSERT(props->voice_pitch.semitones.value == 2.0f);
}
static void verify_voice_pitch7(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_pitch.has_percentage == 1);
  TEST_ASSERT(props->voice_pitch.percentage.value == 10.0f);
}
static void verify_voice_pitch8(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_pitch.has_frequency == 1);
  TEST_ASSERT(props->voice_pitch.frequency.value == 100.0f);
  TEST_ASSERT(props->voice_pitch.frequency.unit == UI_CSS_UNIT_HZ);
}
static void verify_voice_stress(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_stress == UI_CSS_VOICE_STRESS_MODERATE);
}
static void verify_voice_stress2(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_stress == UI_CSS_VOICE_STRESS_NONE);
}
static void verify_voice_stress3(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_stress == UI_CSS_VOICE_STRESS_REDUCED);
}
static void verify_speak_always(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->speak == UI_CSS_SPEAK_ALWAYS);
}
static void verify_voice_stress4(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->voice_stress == UI_CSS_VOICE_STRESS_STRONG);
}
static void verify_voice_duration(struct ui_css_speech_properties *props) {
  TEST_ASSERT(props->has_voice_duration == 1);
  TEST_ASSERT(props->voice_duration.value == 5.0f);
  TEST_ASSERT(props->voice_duration.unit == UI_CSS_UNIT_S);
}

int main(void) {
  struct ui_css_stylesheet *sheet = NULL;
  struct ui_css_rule *rule = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_css_computed_style *style = NULL;
  struct ui_css_speech_properties props;
  ui_error_t rc;

  /* Basic valid coverage */
  test_properties("speak", "never", verify_speak_never);
  test_properties("speak", "always", verify_speak_always);
  test_properties("speak", "auto", NULL);
  test_properties("speak-as", "literal-punctuation no-punctuation",
                  verify_speak_as);
  test_properties("pause-before", "250ms", NULL);
  test_properties("pause-before", "1s", NULL);
  test_properties("pause-before", "10px",
                  NULL); /* invalid unit falls to keyword parsing */
  test_properties("voice-volume", "10px", NULL);
  test_properties("voice-rate", "10px", NULL);
  test_properties("voice-pitch", "10px", NULL);

  /* Missing coverage tests */
  test_properties("cue-after", "url(a.wav", NULL);
  test_properties("cue-after", "url(a.wav) x", NULL);
  test_properties("voice-pitch", "10khz", NULL);
  test_properties("voice-pitch", "10xyz", NULL);
  test_properties("voice-duration", "auto", NULL);
  test_properties("voice-duration", "unknown", NULL);
  test_properties("voice-stress", "unknown", NULL);
  test_properties("pause-before", "none", NULL);
  test_properties("pause-after", "x-weak", verify_pause);
  test_properties("pause-before", "weak", NULL);
  test_properties("pause-after", "medium", verify_pause2);
  test_properties("pause-before", "strong", NULL);
  test_properties("pause-after", "x-strong", verify_pause3);
  test_properties("rest-before", "none", NULL);
  test_properties("rest-after", "none", verify_rest);
  test_properties("cue-after", "url(a.wav)", verify_cue);
  test_properties("cue-before", "url(\"b.wav\")", verify_cue_quotes);
  test_properties("cue-after", "none", NULL);
  test_properties("voice-volume", "silent", verify_voice_vol);
  test_properties("voice-volume", "x-soft", verify_voice_vol2);
  test_properties("voice-volume", "soft", verify_voice_vol3);
  test_properties("voice-volume", "medium", verify_voice_vol4);
  test_properties("voice-volume", "x-loud", verify_voice_vol5);
  test_properties("voice-volume", "10db", verify_voice_vol6);
  test_properties("voice-volume", "unknown", NULL);
  test_properties("voice-rate", "normal", verify_voice_rate);
  test_properties("voice-rate", "slow", verify_voice_rate2);
  test_properties("voice-rate", "medium", verify_voice_rate3);
  test_properties("voice-rate", "fast", verify_voice_rate4);
  test_properties("voice-rate", "x-slow", verify_voice_rate5);
  test_properties("voice-rate", "50%", verify_voice_rate6);
  test_properties("voice-rate", "unknown", NULL);
  test_properties("voice-pitch", "x-low", verify_voice_pitch);
  test_properties("voice-pitch", "low", verify_voice_pitch2);
  test_properties("voice-pitch", "medium", verify_voice_pitch3);
  test_properties("voice-pitch", "high", verify_voice_pitch4);
  test_properties("voice-pitch", "x-high", verify_voice_pitch5);
  test_properties("voice-pitch", "2st", verify_voice_pitch6);
  test_properties("voice-pitch", "10%", verify_voice_pitch7);
  test_properties("voice-pitch", "100hz", verify_voice_pitch8);
  test_properties("voice-range", "medium", NULL);
  test_properties("voice-stress", "moderate", verify_voice_stress);
  test_properties("voice-stress", "none", verify_voice_stress2);
  test_properties("voice-stress", "reduced", verify_voice_stress3);
  test_properties("voice-stress", "strong", verify_voice_stress4);
  test_properties("voice-duration", "5s", verify_voice_duration);

  /* Primary initial test */
  rc = ui_css_stylesheet_create(&sheet);
  TEST_ASSERT(rc == UI_ERROR_NONE);
  rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(rule, "speak", "always", 0);
  ui_css_rule_append_declaration(rule, "speak-as", "spell-out digits", 0);
  ui_css_rule_append_declaration(rule, "pause-before", "250ms", 0);
  ui_css_rule_append_declaration(rule, "rest-after", "strong", 0);
  ui_css_rule_append_declaration(rule, "cue-before", "url(bell.aiff) -2db", 0);
  ui_css_rule_append_declaration(rule, "voice-volume", "loud", 0);
  ui_css_rule_append_declaration(rule, "voice-family", "female", 0);
  ui_css_rule_append_declaration(rule, "voice-rate", "x-fast", 0);
  ui_css_rule_append_declaration(rule, "voice-pitch", "absolute 500Hz", 0);

  ui_css_stylesheet_append_rule(sheet, rule);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");
  ui_css_resolve_style(sheet, node, &style);

  rc = ui_css_speech_parse(style, &props);
  TEST_ASSERT(rc == UI_ERROR_NONE);

  TEST_ASSERT(props.speak == UI_CSS_SPEAK_ALWAYS);
  TEST_ASSERT(props.speak_as_flags & UI_CSS_SPEAK_AS_SPELL_OUT);
  TEST_ASSERT(props.speak_as_flags & UI_CSS_SPEAK_AS_DIGITS);
  TEST_ASSERT(props.pause_before.has_time == 1);
  TEST_ASSERT(props.pause_before.time.value == 250.0f);
  TEST_ASSERT(props.rest_after.has_time == 0);
  TEST_ASSERT(props.rest_after.strength == UI_CSS_SPEECH_STRENGTH_STRONG);
  TEST_ASSERT(props.cue_before.uri != NULL);
  TEST_ASSERT(strcmp(props.cue_before.uri, "bell.aiff") == 0);
  TEST_ASSERT(props.cue_before.has_volume == 1);
  TEST_ASSERT(props.cue_before.volume_db.value == -2.0f);
  TEST_ASSERT(props.voice_volume.has_keyword == 1);
  TEST_ASSERT(props.voice_volume.keyword == UI_CSS_VOICE_VOLUME_LOUD);
  TEST_ASSERT(props.voice_family != NULL);
  TEST_ASSERT(strcmp(props.voice_family, "female") == 0);
  TEST_ASSERT(props.voice_rate.has_keyword == 1);
  TEST_ASSERT(props.voice_rate.keyword == UI_CSS_VOICE_RATE_X_FAST);
  TEST_ASSERT(props.voice_pitch.is_absolute == 1);
  TEST_ASSERT(props.voice_pitch.has_frequency == 1);
  TEST_ASSERT(props.voice_pitch.frequency.value == 500.0f);

  ui_css_speech_cleanup(&props);
  ui_css_computed_style_destroy(style);
  (void)ui_dom_node_destroy(node);
  ui_css_stylesheet_destroy(sheet);

  /* empty string edge cases to hit !str branches in parse_* inside
   * ui_css_speech_parse */
  rc = ui_css_stylesheet_create(&sheet);
  rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
  ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
  ui_css_rule_append_declaration(rule, "pause-after", "", 0);
  ui_css_rule_append_declaration(rule, "rest-before", "", 0);
  ui_css_rule_append_declaration(rule, "cue-after", "", 0);
  ui_css_rule_append_declaration(rule, "voice-volume", "", 0);
  ui_css_rule_append_declaration(rule, "voice-rate", "", 0);
  ui_css_rule_append_declaration(rule, "voice-pitch", "", 0);
  ui_css_rule_append_declaration(rule, "voice-range", "", 0);

  ui_css_stylesheet_append_rule(sheet, rule);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_set_tag_name(node, "div");
  ui_css_resolve_style(sheet, node, &style);

  rc = ui_css_speech_parse(style, &props);
  TEST_ASSERT(props.voice_volume.has_keyword == 0);  /* overridden to empty */
  TEST_ASSERT(props.voice_rate.has_keyword == 0);    /* overridden */
  TEST_ASSERT(props.voice_pitch.has_frequency == 0); /* overridden */

  ui_css_speech_cleanup(&props);
  ui_css_computed_style_destroy(style);
  (void)ui_dom_node_destroy(node);
  ui_css_stylesheet_destroy(sheet);

  /* NULL pointer checks */
  TEST_ASSERT(ui_css_speech_parse(NULL, &props) == UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_speech_parse(style, NULL) == UI_ERROR_INVALID_ARGUMENT);
  TEST_ASSERT(ui_css_speech_cleanup(NULL) == UI_ERROR_INVALID_ARGUMENT);

  /* OOM injections for speech parse specifically */
  {
    g_malloc_fail_countdown = -1;
    rc = ui_css_stylesheet_create(&sheet);
    if (rc == UI_ERROR_NONE) {
      rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
      if (rc == UI_ERROR_NONE) {
        (void)ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG,
                                          "div");
        (void)ui_css_rule_append_declaration(rule, "cue-before",
                                             "url('test.wav')", 0);
        (void)ui_css_rule_append_declaration(rule, "voice-family", "male", 0);
        (void)ui_css_rule_append_declaration(rule, "pause-before", "250ms", 0);
        (void)ui_css_rule_append_declaration(rule, "rest-after", "strong", 0);
        (void)ui_css_rule_append_declaration(rule, "voice-pitch",
                                             "absolute 500Hz", 0);
        (void)ui_css_rule_append_declaration(rule, "voice-rate", "x-fast", 0);
        (void)ui_css_rule_append_declaration(rule, "voice-volume", "loud", 0);
        (void)ui_css_stylesheet_append_rule(sheet, rule);
      }

      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
      if (rc == UI_ERROR_NONE) {
        (void)ui_dom_node_set_tag_name(node, "div");
        rc = ui_css_resolve_style(sheet, node, &style);
        if (rc == UI_ERROR_NONE) {
          int j;
          for (j = 0; j < 15; j++) {
            g_malloc_fail_countdown = j;
            rc = ui_css_speech_parse(style, &props);
            if (rc == UI_ERROR_NONE) {
              ui_css_speech_cleanup(&props);
              break;
            } else {
              ui_css_speech_cleanup(&props);
            }
          }
          g_malloc_fail_countdown = -1;
          ui_css_computed_style_destroy(style);
        }
        (void)ui_dom_node_destroy(node);
      }
      ui_css_stylesheet_destroy(sheet);
    }
  }

  /* OOM injections */
  {
    int i;
    for (i = 0; i < 1000; i++) {
      g_malloc_fail_countdown = i;

      rc = ui_css_stylesheet_create(&sheet);
      if (rc == UI_ERROR_NONE) {
        rc = ui_css_rule_create(UI_CSS_RULE_TYPE_STYLE, &rule);
        if (rc == UI_ERROR_NONE) {
          ui_css_rule_append_selector(rule, UI_CSS_SELECTOR_TYPE_TAG, "div");
          ui_css_rule_append_declaration(rule, "cue-before", "url('test.wav')",
                                         0);
          ui_css_rule_append_declaration(rule, "voice-family", "male", 0);
          ui_css_rule_append_declaration(rule, "pause-before", "250ms", 0);
          ui_css_rule_append_declaration(rule, "rest-after", "strong", 0);
          ui_css_rule_append_declaration(rule, "voice-pitch", "absolute 500Hz",
                                         0);
          ui_css_rule_append_declaration(rule, "voice-rate", "x-fast", 0);
          ui_css_rule_append_declaration(rule, "voice-volume", "loud", 0);
          ui_css_stylesheet_append_rule(sheet, rule);
        }

        rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
        if (rc == UI_ERROR_NONE) {
          ui_dom_node_set_tag_name(node, "div");
          rc = ui_css_resolve_style(sheet, node, &style);
          if (rc == UI_ERROR_NONE) {
            ui_css_computed_style_destroy(style);
          }
          (void)ui_dom_node_destroy(node);
        }
        ui_css_stylesheet_destroy(sheet);
      }
      if (rc == UI_ERROR_NONE) {
        break; /* passed all the way through */
      }
    }
    g_malloc_fail_countdown = -1;
  }

  printf("test_ui_css_speech passed.\n");
  return 0;
}
