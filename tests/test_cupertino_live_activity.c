/**
 * @file test_cupertino_live_activity.c
 * @brief Unit tests for Cupertino Live Activity & StandBy Lock Screen Cards
 * conforming to Apple HIG.
 */

/* clang-format off */
#include "cupertino/cupertino_live_activity.h"
#include "greatest.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

TEST test_live_activity_lifecycle_and_defaults(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_live_activity_descriptor desc;
  struct cupertino_live_activity *activity = NULL;
  struct ui_card_base *card = NULL;
  ui_error_t rc;

  /* Test creation with all NULL optional fields and negative numeric values */
  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_LIVE_ACTIVITY_LAYOUT_COMPACT;
  desc.title = NULL;
  desc.subtitle = NULL;
  desc.duration_seconds = -5.0f; /* Defaults to 60.0f */
  desc.elapsed_seconds = -1.0f;  /* Defaults to 0.0f */
  desc.ambient_lux = -10.0f;     /* Defaults to 100.0f */

  rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, activity);
  ASSERT_EQ(60.0f, activity->duration_seconds);
  ASSERT_EQ(0.0f, activity->elapsed_seconds);
  ASSERT_EQ(1, activity->is_timer_active);
  ASSERT_EQ(100.0f, activity->ambient_lux);

  /* Destroy with activity->base_card == NULL */
  card = activity->base_card;
  activity->base_card = NULL;
  rc = cupertino_live_activity_destroy(activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_card_base_destroy(card);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test with elapsed_seconds >= duration_seconds */
  memset(&desc, 0, sizeof(desc));
  desc.duration_seconds = 30.0f;
  desc.elapsed_seconds = 30.0f;
  rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, activity->is_timer_active);
  rc = cupertino_live_activity_destroy(activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test valid creation with full descriptor */
  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_LIVE_ACTIVITY_LAYOUT_COMPACT;
  desc.title = "Delivery";
  desc.subtitle = "Arriving in 15 mins";
  desc.duration_seconds = 1800.0f;
  desc.elapsed_seconds = 900.0f;
  desc.standby_mode_enabled = 0;
  desc.ambient_lux = 100.0f;

  /* Null checks */
  rc = cupertino_live_activity_create(NULL, &desc, &activity);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_live_activity_create(dummy_engine, NULL, &activity);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_live_activity_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, activity);
  ASSERT_EQ(CUPERTINO_LIVE_ACTIVITY_LAYOUT_COMPACT, activity->layout);
  ASSERT_IN_RANGE(360.0f, activity->width, 0.01f);
  ASSERT_IN_RANGE(88.0f, activity->height, 0.01f);
  ASSERT_IN_RANGE(24.0f, activity->corner_radius, 0.01f);
  ASSERT_EQ(0, activity->is_night_mode);

  /* Base card inspection */
  rc = cupertino_live_activity_get_card_base(activity, &card);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, card);

  rc = cupertino_live_activity_get_card_base(NULL, &card);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_live_activity_get_card_base(activity, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_live_activity_destroy(activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_live_activity_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_live_activity_layout_modes(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_live_activity_descriptor desc;
  struct cupertino_live_activity *activity = NULL;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_LIVE_ACTIVITY_LAYOUT_COMPACT;
  rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Minimal layout */
  rc = cupertino_live_activity_set_layout(
      activity, CUPERTINO_LIVE_ACTIVITY_LAYOUT_MINIMAL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_LIVE_ACTIVITY_LAYOUT_MINIMAL, activity->layout);
  ASSERT_IN_RANGE(88.0f, activity->width, 0.01f);
  ASSERT_IN_RANGE(88.0f, activity->height, 0.01f);
  ASSERT_IN_RANGE(22.0f, activity->corner_radius, 0.01f);

  /* Expanded layout */
  rc = cupertino_live_activity_set_layout(
      activity, CUPERTINO_LIVE_ACTIVITY_LAYOUT_EXPANDED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_LIVE_ACTIVITY_LAYOUT_EXPANDED, activity->layout);
  ASSERT_IN_RANGE(360.0f, activity->width, 0.01f);
  ASSERT_IN_RANGE(160.0f, activity->height, 0.01f);
  ASSERT_IN_RANGE(28.0f, activity->corner_radius, 0.01f);

  /* Fallback / default layout branch */
  rc = cupertino_live_activity_set_layout(
      activity, (enum cupertino_live_activity_layout)999);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(360.0f, activity->width, 0.01f);
  ASSERT_IN_RANGE(88.0f, activity->height, 0.01f);
  ASSERT_IN_RANGE(24.0f, activity->corner_radius, 0.01f);

  /* Null checks */
  rc = cupertino_live_activity_set_layout(
      NULL, CUPERTINO_LIVE_ACTIVITY_LAYOUT_COMPACT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern ui_error_t test_cupertino_live_activity_update_palette(
        struct cupertino_live_activity * a);
    extern ui_error_t test_cupertino_live_activity_apply_geometry(
        struct cupertino_live_activity * a);

    rc = test_cupertino_live_activity_update_palette(NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = test_cupertino_live_activity_apply_geometry(NULL);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }
#endif

  rc = cupertino_live_activity_destroy(activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_live_activity_subsecond_timer(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_live_activity_descriptor desc;
  struct cupertino_live_activity *activity = NULL;
  float progress = 0.0f;
  char time_str[32];
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_LIVE_ACTIVITY_LAYOUT_COMPACT;
  desc.duration_seconds = 10.0f;
  desc.elapsed_seconds = 0.0f;

  rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Advance timer by 2.5 seconds */
  rc = cupertino_live_activity_tick_timer(activity, 2.5f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.25f, progress, 0.001f);
  ASSERT_EQ(1, activity->is_timer_active);

  /* Get formatted string (rem = 7.5s -> "00:07.50") */
  rc = cupertino_live_activity_get_timer_string(activity, time_str,
                                                sizeof(time_str));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("00:07.50", time_str);

  /* Advance past end */
  rc = cupertino_live_activity_tick_timer(activity, 8.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(1.0f, progress, 0.001f);
  ASSERT_EQ(0, activity->is_timer_active);

  /* Tick when timer is not active */
  rc = cupertino_live_activity_tick_timer(activity, 1.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(1.0f, progress, 0.001f);

  /* Test p > 1.0f branch when inactive and elapsed exceeds duration */
  activity->is_timer_active = 0;
  activity->elapsed_seconds = 20.0f;
  activity->duration_seconds = 10.0f;
  rc = cupertino_live_activity_tick_timer(activity, 0.5f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.0f, progress);

  /* Timer string when elapsed >= duration (rem < 0 branch) */
  activity->elapsed_seconds = 15.0f;
  rc = cupertino_live_activity_get_timer_string(activity, time_str,
                                                sizeof(time_str));
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("00:00.00", time_str);

  /* Tick when duration is 0 */
  activity->duration_seconds = 0.0f;
  rc = cupertino_live_activity_tick_timer(activity, 1.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, progress);

  /* Null checks */
  rc = cupertino_live_activity_tick_timer(NULL, 1.0f, &progress);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_live_activity_tick_timer(activity, -1.0f, &progress);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_live_activity_tick_timer(activity, 1.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_live_activity_get_timer_string(NULL, time_str,
                                                sizeof(time_str));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_live_activity_get_timer_string(activity, NULL,
                                                sizeof(time_str));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_live_activity_get_timer_string(activity, time_str, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_live_activity_destroy(activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_live_activity_standby_night_mode(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_live_activity_descriptor desc;
  struct cupertino_live_activity *activity = NULL;
  int is_night = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_LIVE_ACTIVITY_LAYOUT_COMPACT;
  desc.standby_mode_enabled = 1;
  desc.ambient_lux = 50.0f; /* Well lit */

  rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, activity->is_night_mode);

  /* Drop ambient light to 0.5 lux -> night mode deep red should activate */
  rc = cupertino_live_activity_set_ambient_light(activity, 0.5f, &is_night);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_night);
  ASSERT_EQ(1, activity->is_night_mode);

  /* Increase light back to 20 lux -> night mode exits */
  rc = cupertino_live_activity_set_ambient_light(activity, 20.0f, &is_night);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_night);
  ASSERT_EQ(0, activity->is_night_mode);

  /* Disable standby mode */
  rc = cupertino_live_activity_set_standby_mode(activity, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_live_activity_set_ambient_light(activity, 0.2f, &is_night);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_night); /* Standby disabled -> no night mode */

  /* Null checks */
  rc = cupertino_live_activity_set_ambient_light(NULL, 0.5f, &is_night);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_live_activity_set_ambient_light(activity, -1.0f, &is_night);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_live_activity_set_ambient_light(activity, 0.5f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_live_activity_set_standby_mode(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_live_activity_destroy(activity);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_live_activity_mock_failures(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_live_activity_descriptor desc;
  struct cupertino_live_activity *activity = NULL;
  int is_night = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.title = "Mock Title";
  desc.subtitle = "Mock Subtitle";

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_cupertino_live_activity_mock_card_create_fail;
    extern int g_cupertino_live_activity_mock_card_set_title_fail;
    extern int g_cupertino_live_activity_mock_card_set_subtitle_fail;
    extern int g_cupertino_live_activity_mock_card_destroy_fail;
    extern int g_cupertino_live_activity_mock_apply_geometry_fail;
    extern int g_cupertino_live_activity_mock_update_palette_fail;
    extern struct ui_card_base *g_cupertino_live_activity_last_created_card;

    /* Card create fail */
    g_cupertino_live_activity_mock_card_create_fail = 1;
    rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    g_cupertino_live_activity_mock_card_create_fail = 0;

    /* Card set title fail (destroy success) */
    g_cupertino_live_activity_mock_card_set_title_fail = 1;
    g_cupertino_live_activity_mock_card_destroy_fail = 0;
    rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    /* Card set title fail (destroy fail) */
    g_cupertino_live_activity_mock_card_set_title_fail = 1;
    g_cupertino_live_activity_mock_card_destroy_fail = 1;
    rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_live_activity_mock_card_set_title_fail = 0;
    g_cupertino_live_activity_mock_card_destroy_fail = 0;
    if (g_cupertino_live_activity_last_created_card) {
      rc = ui_card_base_destroy(g_cupertino_live_activity_last_created_card);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      g_cupertino_live_activity_last_created_card = NULL;
    }

    /* Card set subtitle fail (destroy success) */
    g_cupertino_live_activity_mock_card_set_subtitle_fail = 1;
    g_cupertino_live_activity_mock_card_destroy_fail = 0;
    rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    /* Card set subtitle fail (destroy fail) */
    g_cupertino_live_activity_mock_card_set_subtitle_fail = 1;
    g_cupertino_live_activity_mock_card_destroy_fail = 1;
    rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_live_activity_mock_card_set_subtitle_fail = 0;
    g_cupertino_live_activity_mock_card_destroy_fail = 0;
    if (g_cupertino_live_activity_last_created_card) {
      rc = ui_card_base_destroy(g_cupertino_live_activity_last_created_card);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      g_cupertino_live_activity_last_created_card = NULL;
    }

    /* Apply geometry fail (destroy success) */
    g_cupertino_live_activity_mock_apply_geometry_fail = 1;
    g_cupertino_live_activity_mock_card_destroy_fail = 0;
    rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    /* Apply geometry fail (destroy fail) */
    g_cupertino_live_activity_mock_apply_geometry_fail = 1;
    g_cupertino_live_activity_mock_card_destroy_fail = 1;
    rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_live_activity_mock_apply_geometry_fail = 0;
    g_cupertino_live_activity_mock_card_destroy_fail = 0;
    if (g_cupertino_live_activity_last_created_card) {
      rc = ui_card_base_destroy(g_cupertino_live_activity_last_created_card);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      g_cupertino_live_activity_last_created_card = NULL;
    }

    /* Update palette fail (destroy success) */
    g_cupertino_live_activity_mock_update_palette_fail = 1;
    g_cupertino_live_activity_mock_card_destroy_fail = 0;
    rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

    /* Update palette fail (destroy fail) */
    g_cupertino_live_activity_mock_update_palette_fail = 1;
    g_cupertino_live_activity_mock_card_destroy_fail = 1;
    rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_live_activity_mock_update_palette_fail = 0;
    g_cupertino_live_activity_mock_card_destroy_fail = 0;
    if (g_cupertino_live_activity_last_created_card) {
      rc = ui_card_base_destroy(g_cupertino_live_activity_last_created_card);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      g_cupertino_live_activity_last_created_card = NULL;
    }

    /* Valid create for set_ambient_light fail and destroy fail */
    rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    g_cupertino_live_activity_mock_update_palette_fail = 1;
    rc = cupertino_live_activity_set_ambient_light(activity, 50.0f, &is_night);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_live_activity_mock_update_palette_fail = 0;

    /* Destroy fail branch */
    g_cupertino_live_activity_mock_card_destroy_fail = 1;
    rc = cupertino_live_activity_destroy(activity);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_cupertino_live_activity_mock_card_destroy_fail = 0;
    if (g_cupertino_live_activity_last_created_card) {
      rc = ui_card_base_destroy(g_cupertino_live_activity_last_created_card);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      g_cupertino_live_activity_last_created_card = NULL;
    }
  }
#endif

  PASS();
}

TEST test_live_activity_oom(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct cupertino_live_activity_descriptor desc;
  struct cupertino_live_activity *activity = NULL;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.layout = CUPERTINO_LIVE_ACTIVITY_LAYOUT_COMPACT;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, activity);

  g_malloc_fail_countdown = 1;
  rc = cupertino_live_activity_create(dummy_engine, &desc, &activity);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, activity);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_live_activity_create(NULL, &desc, &activity);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_live_activity_suite) {
  RUN_TEST(test_live_activity_lifecycle_and_defaults);
  RUN_TEST(test_live_activity_layout_modes);
  RUN_TEST(test_live_activity_subsecond_timer);
  RUN_TEST(test_live_activity_standby_night_mode);
  RUN_TEST(test_live_activity_mock_failures);
  RUN_TEST(test_live_activity_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_live_activity_suite);
  GREATEST_MAIN_END();
}
