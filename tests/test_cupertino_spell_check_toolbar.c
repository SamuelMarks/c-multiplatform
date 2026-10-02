/**
 * @file test_cupertino_spell_check_toolbar.c
 * @brief Unit tests for iOS Spell Check Suggestions Toolbar conforming to Apple
 * HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_spell_check_toolbar.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

TEST test_spell_check_toolbar_lifecycle_and_show(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_spell_check_toolbar_descriptor desc;
  struct cupertino_spell_check_toolbar *tb = NULL;
  const char *suggestions[3];
  const char *word = NULL;
  size_t count = 0;
  float x = 0.0f;
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  int is_vis = 0;
  ui_error_t rc;

  suggestions[0] = "accommodate";
  suggestions[1] = "accumulate";
  suggestions[2] = "acclamation";

  memset(&desc, 0, sizeof(desc));
  desc.allow_add_to_dictionary = 1;

  /* Null checks */
  rc = cupertino_spell_check_toolbar_create(NULL, &desc, &tb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_create(dummy_engine, NULL, &tb);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, tb);

  /* Show */
  rc = cupertino_spell_check_toolbar_show(NULL, 100.0f, 200.0f, 80.0f, 20.0f,
                                          "acommodate", suggestions, 3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_show(tb, 100.0f, 200.0f, 80.0f, 20.0f,
                                          NULL, suggestions, 3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_spell_check_toolbar_show(tb, 100.0f, 200.0f, 80.0f, 20.0f,
                                          "acommodate", suggestions, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_spell_check_toolbar_is_visible(tb, &is_vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_vis);

  rc = cupertino_spell_check_toolbar_get_misspelled_word(tb, &word);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("acommodate", word);

  rc = cupertino_spell_check_toolbar_get_suggestion_count(tb, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, count);

  rc = cupertino_spell_check_toolbar_get_bounds(tb, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_GT(w, 100.0f);
  ASSERT_GT(h, 30.0f);

  /* Hide */
  rc = cupertino_spell_check_toolbar_hide(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_spell_check_toolbar_is_visible(tb, &is_vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_vis);

  rc = cupertino_spell_check_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_spell_check_toolbar_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_spell_check_toolbar_selection_and_dictionary(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_spell_check_toolbar_descriptor desc;
  struct cupertino_spell_check_toolbar *tb = NULL;
  const char *suggestions[2];
  const char *chosen = NULL;
  int is_vis = 0;
  int added = 0;
  size_t count = 0;
  float x = 0.0f;
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  suggestions[0] = "definitely";
  suggestions[1] = "definitive";

  memset(&desc, 0, sizeof(desc));
  desc.allow_add_to_dictionary = 1;

  rc = cupertino_spell_check_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Show near top boundary (should flip arrow below) */
  rc = cupertino_spell_check_toolbar_show(tb, 50.0f, 10.0f, 60.0f, 18.0f,
                                          "definately", suggestions, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, tb->arrow_on_bottom);

  /* Select suggestion */
  rc = cupertino_spell_check_toolbar_select_suggestion(tb, 99, &chosen);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_spell_check_toolbar_select_suggestion(tb, 0, &chosen);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("definitely", chosen);

  /* Auto-dismissed */
  rc = cupertino_spell_check_toolbar_is_visible(tb, &is_vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_vis);

  /* Show again and add to dictionary */
  rc = cupertino_spell_check_toolbar_show(tb, 50.0f, 150.0f, 60.0f, 18.0f,
                                          "definately", suggestions, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, tb->arrow_on_bottom);

  rc = cupertino_spell_check_toolbar_add_to_dictionary(tb, &added);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, added);

  rc = cupertino_spell_check_toolbar_is_visible(tb, &is_vis);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_vis);

  /* Null checks */
  rc = cupertino_spell_check_toolbar_hide(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_is_visible(NULL, &is_vis);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_is_visible(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_get_suggestion_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_get_suggestion_count(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_select_suggestion(NULL, 0, &chosen);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_select_suggestion(tb, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_add_to_dictionary(NULL, &added);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_add_to_dictionary(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_get_misspelled_word(NULL, &chosen);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_get_misspelled_word(tb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_get_bounds(NULL, &x, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_spell_check_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test disabled dictionary branch */
  desc.allow_add_to_dictionary = 0;
  rc = cupertino_spell_check_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_spell_check_toolbar_show(tb, 50.0f, 150.0f, 60.0f, 18.0f,
                                          "definately", suggestions, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_spell_check_toolbar_add_to_dictionary(tb, &added);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, added);
  rc = cupertino_spell_check_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_spell_check_toolbar_oom(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_spell_check_toolbar_descriptor desc;
  struct cupertino_spell_check_toolbar *tb = NULL;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  g_malloc_fail_countdown = 0;
  rc = cupertino_spell_check_toolbar_create(dummy_engine, &desc, &tb);
  g_malloc_fail_countdown = -1;
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, tb);

  PASS();
}

TEST test_spell_check_toolbar_edge_cases(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_spell_check_toolbar_descriptor desc;
  struct cupertino_spell_check_toolbar *tb = NULL;
  const char *suggestions[6];
  float x = 0.0f;
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  rc = cupertino_spell_check_toolbar_create(dummy_engine, &desc, &tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Case 1: Suggestions with NULL entries and >
   * CUPERTINO_SPELL_CHECK_MAX_SUGGESTIONS (5) */
  suggestions[0] = "a";
  suggestions[1] = NULL;
  suggestions[2] = "c";
  suggestions[3] = "d";
  suggestions[4] = "e";
  suggestions[5] = "f";

  rc = cupertino_spell_check_toolbar_show(tb, 0.0f, 0.0f, 10.0f, 10.0f, "word",
                                          suggestions, 6);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(5, tb->suggestion_count);
  ASSERT_STR_EQ("", tb->suggestions[1]);

  /* Case 2: total_w >= 160.0f (long suggestion words so total_w exceeds 160) */
  suggestions[0] =
      "extraordinarilylongwordthatshouldmakethecontentwidthexceed160pts";
  suggestions[1] = "anotherextremelylongwordtoexceedwidth";
  rc = cupertino_spell_check_toolbar_show(tb, 0.0f, 0.0f, 10.0f, 10.0f, "word",
                                          suggestions, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_GT(tb->bubble_w, 160.0f);

  /* Case 3: cupertino_spell_check_toolbar_create with NULL out_toolbar */
  rc = cupertino_spell_check_toolbar_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Direct call with NULL toolbar */
  rc = cupertino_spell_check_recompute_bounds(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Case 4: cupertino_spell_check_toolbar_show invalid arguments combinations
   */
  rc = cupertino_spell_check_toolbar_show(tb, 0.0f, 0.0f, 10.0f, 10.0f, "word",
                                          NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_spell_check_toolbar_show(tb, 0.0f, 0.0f, 10.0f, 10.0f, "word",
                                          NULL, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, tb->suggestion_count);

  /* Case 5: cupertino_spell_check_toolbar_get_bounds null args individually */
  rc = cupertino_spell_check_toolbar_get_bounds(tb, NULL, &y, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_get_bounds(tb, &x, NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_get_bounds(tb, &x, &y, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spell_check_toolbar_get_bounds(tb, &x, &y, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_spell_check_mock_recompute_fail;
  g_cupertino_spell_check_mock_recompute_fail = 1;
  rc = cupertino_spell_check_toolbar_show(tb, 0.0f, 0.0f, 10.0f, 10.0f, "word",
                                          suggestions, 2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_spell_check_mock_recompute_fail = 0;
#endif

  rc = cupertino_spell_check_toolbar_destroy(tb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(cupertino_spell_check_toolbar_suite) {
  RUN_TEST(test_spell_check_toolbar_lifecycle_and_show);
  RUN_TEST(test_spell_check_toolbar_selection_and_dictionary);
  RUN_TEST(test_spell_check_toolbar_oom);
  RUN_TEST(test_spell_check_toolbar_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_spell_check_toolbar_suite);
  GREATEST_MAIN_END();
}
