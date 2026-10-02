/**
 * @file test_cupertino_spring.c
 * @brief Unit tests for Cupertino and Apple CASpringAnimation physics engine.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_spring.h"
#include <math.h>
/* clang-format on */

SUITE(cupertino_spring_suite);

TEST test_spring_presets(void) {
  struct cupertino_spring_config cfg;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_spring_get_preset(CUPERTINO_SPRING_PRESET_DEFAULT, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spring_get_preset((enum cupertino_spring_preset)99, &cfg);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spring_get_preset(CUPERTINO_SPRING_PRESET_COUNT, &cfg);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Default preset: CASpringAnimation matching mass=1, stiffness=100,
   * damping=10 */
  rc = cupertino_spring_get_preset(CUPERTINO_SPRING_PRESET_DEFAULT, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(cfg.mass - 1.0f) < 1e-5f);
  ASSERT(fabs(cfg.stiffness - 100.0f) < 1e-5f);
  ASSERT(fabs(cfg.damping - 10.0f) < 1e-5f);
  ASSERT(fabs(cfg.initial_velocity - 0.0f) < 1e-5f);
  ASSERT(fabs(cfg.initial_position - 0.0f) < 1e-5f);
  ASSERT(fabs(cfg.target_position - 1.0f) < 1e-5f);

  /* Bouncy preset */
  rc = cupertino_spring_get_preset(CUPERTINO_SPRING_PRESET_BOUNCY, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(cfg.stiffness - 180.0f) < 1e-5f);
  ASSERT(fabs(cfg.damping - 12.0f) < 1e-5f);

  /* Snappy preset */
  rc = cupertino_spring_get_preset(CUPERTINO_SPRING_PRESET_SNAPPY, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(cfg.stiffness - 300.0f) < 1e-5f);
  ASSERT(fabs(cfg.damping - 25.0f) < 1e-5f);

  /* Interactive preset */
  rc = cupertino_spring_get_preset(CUPERTINO_SPRING_PRESET_INTERACTIVE, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(cfg.stiffness - 250.0f) < 1e-5f);
  ASSERT(fabs(cfg.damping - 30.0f) < 1e-5f);

  /* Sheet dismiss critically damped preset */
  rc = cupertino_spring_get_preset(CUPERTINO_SPRING_PRESET_SHEET_DISMISS, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(cfg.stiffness - 200.0f) < 1e-5f);
  ASSERT(fabs(cfg.damping - 28.28427f) < 1e-4f);

  PASS();
}

TEST test_spring_evaluation_regimes(void) {
  struct cupertino_spring_config cfg;
  float pos = 0.0f;
  float vel = 0.0f;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_spring_evaluate(NULL, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  memset(&cfg, 0, sizeof(cfg));
  cfg.mass = 1.0f;
  cfg.stiffness = 100.0f;
  cfg.damping = 10.0f;
  cfg.initial_position = 0.0f;
  cfg.target_position = 1.0f;

  rc = cupertino_spring_evaluate(&cfg, 0.5f, NULL, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spring_evaluate(&cfg, 0.5f, &pos, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_spring_evaluate(&cfg, -0.1f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  cfg.mass = 0.0f;
  rc = cupertino_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  cfg.mass = 1.0f;
  cfg.stiffness = 0.0f;
  rc = cupertino_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  cfg.stiffness = 100.0f;
  cfg.damping = -1.0f;
  rc = cupertino_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* 1. Underdamped (zeta < 1.0, e.g. mass=1, stiffness=100, damping=10 -> zeta
   * = 10 / (2*10) = 0.5) */
  cfg.damping = 10.0f;
  /* At t = 0: position should be initial_position (0.0), velocity = 0.0 */
  rc = cupertino_spring_evaluate(&cfg, 0.0f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(pos - 0.0f) < 1e-5f);
  ASSERT(fabs(vel - 0.0f) < 1e-5f);

  /* At t = 0.2: moving toward target */
  rc = cupertino_spring_evaluate(&cfg, 0.2f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(pos > 0.0f);
  ASSERT(vel > 0.0f);

  /* At large t: should converge to target_position (1.0) with zero velocity */
  rc = cupertino_spring_evaluate(&cfg, 5.0f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(pos - 1.0f) < 1e-3f);
  ASSERT(fabs(vel) < 1e-3f);

  /* 2. Critically damped (zeta == 1.0, e.g. mass=1, stiffness=100, damping = 2
   * * sqrt(100) = 20) */
  cfg.damping = 20.0f;
  rc = cupertino_spring_evaluate(&cfg, 0.0f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(pos - 0.0f) < 1e-5f);

  rc = cupertino_spring_evaluate(&cfg, 0.3f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(pos > 0.0f && pos < 1.0f);

  rc = cupertino_spring_evaluate(&cfg, 5.0f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(pos - 1.0f) < 1e-3f);

  /* 3. Overdamped (zeta > 1.0, e.g. damping = 30.0 -> zeta = 30 / 20 = 1.5) */
  cfg.damping = 30.0f;
  rc = cupertino_spring_evaluate(&cfg, 0.0f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(pos - 0.0f) < 1e-5f);

  rc = cupertino_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(pos > 0.0f && pos < 1.0f);

  rc = cupertino_spring_evaluate(&cfg, 5.0f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(pos - 1.0f) < 1e-3f);

  PASS();
}

TEST test_rubber_band_physics(void) {
  float offset = 0.0f;
  float distance = 0.0f;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_rubber_band_offset(50.0f, 0.0f, 0.55f, &offset);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_rubber_band_offset(50.0f, 600.0f, 0.0f, &offset);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_rubber_band_offset(50.0f, 600.0f, 0.55f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_rubber_band_inverse(50.0f, 0.0f, 0.55f, &distance);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_rubber_band_inverse(50.0f, 600.0f, 0.0f, &distance);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_rubber_band_inverse(50.0f, 600.0f, 0.55f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  /* Offset >= dimension is invalid for inverse */
  rc = cupertino_rubber_band_inverse(600.0f, 600.0f, 0.55f, &distance);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Positive rubber-band offset */
  rc = cupertino_rubber_band_offset(100.0f, 800.0f, 0.55f, &offset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(offset > 0.0f && offset < 100.0f);

  /* Inversion matches original distance */
  rc = cupertino_rubber_band_inverse(offset, 800.0f, 0.55f, &distance);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(distance - 100.0f) < 1e-3f);

  /* Negative distance (overscroll top / left) */
  rc = cupertino_rubber_band_offset(-120.0f, 800.0f, 0.55f, &offset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(offset < 0.0f && offset > -120.0f);

  rc = cupertino_rubber_band_inverse(offset, 800.0f, 0.55f, &distance);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(distance - (-120.0f)) < 1e-3f);

  PASS();
}

TEST test_momentum_scroll_and_detents(void) {
  float delta = 0.0f;
  float vel = 0.0f;
  float detents[4];
  size_t nearest_idx = 0;
  float target_pos = 0.0f;
  ui_error_t rc;

  /* Momentum scroll invalid args */
  rc = cupertino_momentum_scroll(500.0f, 0.0f, 1.0f, &delta, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_momentum_scroll(500.0f, 1.0f, 1.0f, &delta, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_momentum_scroll(500.0f, 0.998f, -0.5f, &delta, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_momentum_scroll(500.0f, 0.998f, 1.0f, NULL, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_momentum_scroll(500.0f, 0.998f, 1.0f, &delta, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid momentum evaluation */
  rc = cupertino_momentum_scroll(500.0f, CUPERTINO_SCROLL_DECELERATION_NORMAL,
                                 1.0f, &delta, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(vel < 500.0f && vel > 0.0f);
  ASSERT(delta > 0.0f);

  /* Detent snapping invalid args */
  detents[0] = 0.0f;
  detents[1] = 200.0f;
  detents[2] = 400.0f;
  detents[3] = 600.0f;

  rc = cupertino_detent_find_nearest(100.0f, 0.0f, NULL, 4, 0.998f,
                                     &nearest_idx, &target_pos);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_detent_find_nearest(100.0f, 0.0f, detents, 0, 0.998f,
                                     &nearest_idx, &target_pos);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_detent_find_nearest(100.0f, 0.0f, detents, 4, 0.0f,
                                     &nearest_idx, &target_pos);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_detent_find_nearest(100.0f, 0.0f, detents, 4, 1.0f,
                                     &nearest_idx, &target_pos);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_detent_find_nearest(100.0f, 0.0f, detents, 4, 0.998f, NULL,
                                     &target_pos);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_detent_find_nearest(100.0f, 0.0f, detents, 4, 0.998f,
                                     &nearest_idx, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Zero velocity: snaps to closest detent to current position */
  /* current = 190.0 -> closest is detents[1] = 200.0 */
  rc = cupertino_detent_find_nearest(190.0f, 0.0f, detents, 4, 0.998f,
                                     &nearest_idx, &target_pos);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, nearest_idx);
  ASSERT(fabs(target_pos - 200.0f) < 1e-4f);

  /* Current = 10.0, high upward velocity: flings forward to detents[3] = 600.0
   */
  rc = cupertino_detent_find_nearest(10.0f, 1.2f, detents, 4, 0.998f,
                                     &nearest_idx, &target_pos);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, nearest_idx);
  ASSERT(fabs(target_pos - 600.0f) < 1e-4f);

  PASS();
}

SUITE(cupertino_spring_suite) {
  RUN_TEST(test_spring_presets);
  RUN_TEST(test_spring_evaluation_regimes);
  RUN_TEST(test_rubber_band_physics);
  RUN_TEST(test_momentum_scroll_and_detents);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_spring_suite);
  GREATEST_MAIN_END();
}
