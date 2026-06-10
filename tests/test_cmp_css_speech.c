/* clang-format off */
#include "greatest.h"
#include "cmp_css_speech.h"
/* clang-format on */

TEST test_voice_group_init_free(void) {
  cmp_prop_voice_group_t group;
  const char *family[] = {"female", "male", NULL};

  ASSERT_EQ(0, cmp_prop_voice_group_init(&group, "loud", family, "fast", "high",
                                         "x-high", "strong", "2s"));
  ASSERT_STR_EQ("loud", group.voice_volume);
  ASSERT_STR_EQ("female", group.voice_family[0]);
  ASSERT_STR_EQ("male", group.voice_family[1]);
  ASSERT_EQ(NULL, group.voice_family[2]);
  ASSERT_STR_EQ("fast", group.voice_rate);
  ASSERT_STR_EQ("high", group.voice_pitch);
  ASSERT_STR_EQ("x-high", group.voice_range);
  ASSERT_STR_EQ("strong", group.voice_stress);
  ASSERT_STR_EQ("2s", group.voice_duration);

  ASSERT_EQ(0, cmp_prop_voice_group_free(&group));
  ASSERT_EQ(NULL, group.voice_volume);
  ASSERT_EQ(NULL, group.voice_family);
  PASS();
}

TEST test_speak_init_free(void) {
  cmp_prop_speak_t prop;
  ASSERT_EQ(0, cmp_prop_speak_init(&prop, CMP_SPEAK_ALWAYS, "spell-out"));
  ASSERT_EQ(CMP_SPEAK_ALWAYS, prop.speak);
  ASSERT_STR_EQ("spell-out", prop.speak_as);

  ASSERT_EQ(0, cmp_prop_speak_free(&prop));
  ASSERT_EQ(NULL, prop.speak_as);
  PASS();
}

TEST test_pause_init_free(void) {
  cmp_prop_pause_t prop;
  ASSERT_EQ(0, cmp_prop_pause_init(&prop, "1s", "2s"));
  ASSERT_STR_EQ("1s", prop.pause_before);
  ASSERT_STR_EQ("2s", prop.pause_after);

  ASSERT_EQ(0, cmp_prop_pause_free(&prop));
  ASSERT_EQ(NULL, prop.pause_before);
  PASS();
}

TEST test_rest_init_free(void) {
  cmp_prop_rest_t prop;
  ASSERT_EQ(0, cmp_prop_rest_init(&prop, "500ms", "100ms"));
  ASSERT_STR_EQ("500ms", prop.rest_before);
  ASSERT_STR_EQ("100ms", prop.rest_after);

  ASSERT_EQ(0, cmp_prop_rest_free(&prop));
  ASSERT_EQ(NULL, prop.rest_before);
  PASS();
}

TEST test_cue_init_free(void) {
  cmp_prop_cue_t prop;
  ASSERT_EQ(0, cmp_prop_cue_init(&prop, "url(audio1.wav)", "url(audio2.wav)"));
  ASSERT_STR_EQ("url(audio1.wav)", prop.cue_before);
  ASSERT_STR_EQ("url(audio2.wav)", prop.cue_after);

  ASSERT_EQ(0, cmp_prop_cue_free(&prop));
  ASSERT_EQ(NULL, prop.cue_before);
  PASS();
}

TEST test_play_during_init_free(void) {
  cmp_prop_play_during_t prop;
  ASSERT_EQ(0, cmp_prop_play_during_init(&prop, "url(bg.wav) mix repeat"));
  ASSERT_STR_EQ("url(bg.wav) mix repeat", prop.play_during);

  ASSERT_EQ(0, cmp_prop_play_during_free(&prop));
  ASSERT_EQ(NULL, prop.play_during);
  PASS();
}

TEST test_at_rule_media_speech_init_free(void) {
  cmp_at_rule_media_speech_t rule;
  const char *family[] = {"auto", NULL};

  ASSERT_EQ(0, cmp_at_rule_media_speech_init(&rule));

  ASSERT_EQ(0,
            cmp_prop_voice_group_init(&rule.voice, "medium", family, "medium",
                                      "medium", "medium", "normal", "auto"));
  ASSERT_EQ(0, cmp_prop_speak_init(&rule.speak, CMP_SPEAK_AUTO, "normal"));
  ASSERT_EQ(0, cmp_prop_pause_init(&rule.pause, "none", "none"));
  ASSERT_EQ(0, cmp_prop_rest_init(&rule.rest, "none", "none"));
  ASSERT_EQ(0, cmp_prop_cue_init(&rule.cue, "none", "none"));
  ASSERT_EQ(0, cmp_prop_play_during_init(&rule.play_during, "auto"));

  ASSERT_EQ(0, cmp_at_rule_media_speech_free(&rule));
  ASSERT_EQ(NULL, rule.voice.voice_volume);
  ASSERT_EQ(NULL, rule.speak.speak_as);
  PASS();
}

SUITE(cmp_css_speech_suite) {
  RUN_TEST(test_voice_group_init_free);
  RUN_TEST(test_speak_init_free);
  RUN_TEST(test_pause_init_free);
  RUN_TEST(test_rest_init_free);
  RUN_TEST(test_cue_init_free);
  RUN_TEST(test_play_during_init_free);
  RUN_TEST(test_at_rule_media_speech_init_free);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_css_speech_suite);
  GREATEST_MAIN_END();
}
