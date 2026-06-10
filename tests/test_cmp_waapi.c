/* clang-format off */
#include "greatest.h"
#include "cmp_waapi.h"
/* clang-format on */

TEST test_doc_timeline(void) {
  cmp_waapi_doc_timeline_t tl;
  ASSERT_EQ(0, cmp_waapi_doc_timeline_init(&tl));
  ASSERT_EQ(0.0f, tl.base.current_time);
  ASSERT_EQ(0.0f, tl.origin_time);
  ASSERT_EQ(-1, cmp_waapi_doc_timeline_init(NULL));
  PASS();
}

TEST test_effect(void) {
  cmp_waapi_effect_t effect;
  int dummy_target = 1;
  ASSERT_EQ(0, cmp_waapi_effect_init(&effect, &dummy_target));
  ASSERT_EQ(&dummy_target, effect.target);
  ASSERT_EQ(0.0f, effect.iteration_duration);
  ASSERT_EQ(0.0f, effect.active_duration);
  ASSERT_EQ(0.0f, effect.end_time);
  ASSERT_EQ(-1, cmp_waapi_effect_init(NULL, NULL));
  PASS();
}

TEST test_animation(void) {
  cmp_waapi_animation_t anim;
  ASSERT_EQ(0, cmp_waapi_animation_init(&anim, "my-anim"));
  ASSERT_STR_EQ("my-anim", anim.id);
  ASSERT_EQ(NULL, anim.effect);
  ASSERT_EQ(NULL, anim.timeline);
  ASSERT_EQ(0.0f, anim.start_time);
  ASSERT_EQ(0.0f, anim.current_time);
  ASSERT_EQ(1.0f, anim.playback_rate);

  ASSERT_EQ(0, cmp_waapi_animation_free(&anim));
  ASSERT_EQ(NULL, anim.id);

  ASSERT_EQ(-1, cmp_waapi_animation_init(NULL, "test"));
  ASSERT_EQ(-1, cmp_waapi_animation_free(NULL));
  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_doc_timeline);
  RUN_TEST(test_effect);
  RUN_TEST(test_animation);
  GREATEST_MAIN_END();
}
