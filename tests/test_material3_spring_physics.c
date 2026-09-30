/**
 * @file test_material3_spring_physics.c
 * @brief Tests verifying second-order differential equation solver across
 * spring damping regimes.
 */

/* clang-format off */
#include "greatest.h"
#include "material3/md3_spring.h"
#include "ui_error.h"
#include <math.h>
/* clang-format on */

TEST test_md3_spring_underdamped(void) {
  struct md3_spring_config cfg;
  float pos = 0.0f;
  float vel = 0.0f;
  ui_error_t rc;

  /* Underdamped (damping_ratio < 1.0) */
  cfg.damping_ratio = 0.5f;
  cfg.stiffness = 100.0f;
  cfg.mass = 1.0f;
  cfg.initial_position = 0.0f;
  cfg.target_position = 10.0f;
  cfg.initial_velocity = 0.0f;

  rc = md3_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(pos > 0.0f);
  PASS();
}

TEST test_md3_spring_critically_damped(void) {
  struct md3_spring_config cfg;
  float pos = 0.0f;
  float vel = 0.0f;
  ui_error_t rc;

  /* Critically damped (damping_ratio == 1.0) */
  cfg.damping_ratio = 1.0f;
  cfg.stiffness = 100.0f;
  cfg.mass = 1.0f;
  cfg.initial_position = 0.0f;
  cfg.target_position = 10.0f;
  cfg.initial_velocity = 0.0f;

  rc = md3_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(pos > 0.0f && pos <= 10.0f);
  PASS();
}

TEST test_md3_spring_overdamped(void) {
  struct md3_spring_config cfg;
  float pos = 0.0f;
  float vel = 0.0f;
  ui_error_t rc;

  /* Overdamped (damping_ratio > 1.0) */
  cfg.damping_ratio = 1.5f;
  cfg.stiffness = 100.0f;
  cfg.mass = 1.0f;
  cfg.initial_position = 0.0f;
  cfg.target_position = 10.0f;
  cfg.initial_velocity = 0.0f;

  rc = md3_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(pos > 0.0f && pos <= 10.0f);
  PASS();
}

TEST test_md3_spring_config(void) {
  struct md3_spring_config cfg;
  ui_error_t rc;

  /* Invalid argument checks */
  rc = md3_spring_get_config(MD3_SPRING_TOKEN_BOUNCY, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_spring_get_config((enum md3_spring_token) - 1, &cfg);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_spring_get_config((enum md3_spring_token)MD3_SPRING_TOKEN_COUNT,
                             &cfg);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_spring_get_config((enum md3_spring_token)99, &cfg);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid tokens */
  rc = md3_spring_get_config(MD3_SPRING_TOKEN_FAST_SPATIAL, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(700.0f, cfg.stiffness);
  ASSERT_EQ(0.9f, cfg.damping_ratio);

  rc = md3_spring_get_config(MD3_SPRING_TOKEN_FAST_EFFECTS, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(800.0f, cfg.stiffness);
  ASSERT_EQ(0.7f, cfg.damping_ratio);

  rc = md3_spring_get_config(MD3_SPRING_TOKEN_SLOW_SPATIAL, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(300.0f, cfg.stiffness);
  ASSERT_EQ(0.95f, cfg.damping_ratio);

  rc = md3_spring_get_config(MD3_SPRING_TOKEN_SLOW_EFFECTS, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(200.0f, cfg.stiffness);
  ASSERT_EQ(0.8f, cfg.damping_ratio);

  rc = md3_spring_get_config(MD3_SPRING_TOKEN_BOUNCY, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(400.0f, cfg.stiffness);
  ASSERT_EQ(0.5f, cfg.damping_ratio);

  PASS();
}

TEST test_md3_spring_evaluate_invalid_and_negative_elapsed(void) {
  struct md3_spring_config cfg;
  float pos = 0.0f;
  float vel = 0.0f;
  ui_error_t rc;

  cfg.damping_ratio = 0.5f;
  cfg.stiffness = 100.0f;
  cfg.mass = 1.0f;
  cfg.initial_position = 0.0f;
  cfg.target_position = 10.0f;
  cfg.initial_velocity = 0.0f;

  /* Invalid arguments */
  rc = md3_spring_evaluate(NULL, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_spring_evaluate(&cfg, 0.5f, NULL, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_spring_evaluate(&cfg, 0.5f, &pos, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  cfg.mass = 0.0f;
  rc = md3_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  cfg.mass = -1.0f;
  rc = md3_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  cfg.mass = 1.0f;

  cfg.stiffness = 0.0f;
  rc = md3_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  cfg.stiffness = -10.0f;
  rc = md3_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  cfg.stiffness = 100.0f;

  cfg.damping_ratio = -0.1f;
  rc = md3_spring_evaluate(&cfg, 0.5f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  cfg.damping_ratio = 0.5f;

  /* Negative elapsed time clamped to 0.0f */
  rc = md3_spring_evaluate(&cfg, -1.0f, &pos, &vel);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(md3_spring_physics_suite) {
  RUN_TEST(test_md3_spring_config);
  RUN_TEST(test_md3_spring_evaluate_invalid_and_negative_elapsed);
  RUN_TEST(test_md3_spring_underdamped);
  RUN_TEST(test_md3_spring_critically_damped);
  RUN_TEST(test_md3_spring_overdamped);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_spring_physics_suite);
  GREATEST_MAIN_END();
}
