/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

SUITE(cmp_media_query_suite);

TEST test_media_query_evaluate(void) {
  cmp_media_query_t query;
  cmp_media_query_env_t env;
  int matches;

  memset(&query, 0, sizeof(cmp_media_query_t));
  memset(&env, 0, sizeof(cmp_media_query_env_t));

  query.min_width = 320.0f;
  query.max_width = 480.0f;
  query.orientation = -1;
  query.hover = -1;
  query.pointer = -1;
  query.update = -1;
  query.light_level = -1;

  env.viewport_width = 400.0f;
  env.viewport_height = 800.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_media_query_evaluate(&query, &env, &matches));
  ASSERT_EQ(1, matches);

  env.viewport_width = 500.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_media_query_evaluate(&query, &env, &matches));
  ASSERT_EQ(0, matches);

  PASS();
}

TEST test_pointer_media_evaluate(void) {
  cmp_media_query_t query;
  cmp_media_query_env_t env;
  int matches;

  memset(&query, 0, sizeof(cmp_media_query_t));
  memset(&env, 0, sizeof(cmp_media_query_env_t));

  query.hover = 1;
  query.pointer = 1; /* coarse */

  env.has_hover = 1;
  env.is_pointer_coarse = 1;

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_media_evaluate(&query, &env, &matches));
  ASSERT_EQ(1, matches);

  env.is_pointer_coarse = 0; /* fine pointer like a mouse */
  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_media_evaluate(&query, &env, &matches));
  ASSERT_EQ(0, matches);

  PASS();
}

TEST test_update_media_evaluate(void) {
  cmp_media_query_t query;
  cmp_media_query_env_t env;
  int matches;

  memset(&query, 0, sizeof(cmp_media_query_t));
  memset(&env, 0, sizeof(cmp_media_query_env_t));

  query.update = 1; /* fast */
  env.update_frequency = 1;

  ASSERT_EQ(CMP_SUCCESS, cmp_update_media_evaluate(&query, &env, &matches));
  ASSERT_EQ(1, matches);

  env.update_frequency = 0; /* slow/none */
  ASSERT_EQ(CMP_SUCCESS, cmp_update_media_evaluate(&query, &env, &matches));
  ASSERT_EQ(0, matches);

  PASS();
}

TEST test_light_level_evaluate(void) {
  cmp_media_query_t query;
  cmp_media_query_env_t env;
  int matches;

  memset(&query, 0, sizeof(cmp_media_query_t));
  memset(&env, 0, sizeof(cmp_media_query_env_t));

  query.light_level = 1; /* normal */
  env.light_level = 1;

  ASSERT_EQ(CMP_SUCCESS, cmp_light_level_evaluate(&query, &env, &matches));
  ASSERT_EQ(1, matches);

  env.light_level = 0; /* dim */
  ASSERT_EQ(CMP_SUCCESS, cmp_light_level_evaluate(&query, &env, &matches));
  ASSERT_EQ(0, matches);

  PASS();
}

TEST test_container_query_evaluate(void) {
  cmp_container_query_t query;
  cmp_container_ctx_t *ctx = NULL;
  int matches;

  memset(&query, 0, sizeof(cmp_container_query_t));

  query.min_width = 200.0f;
  query.max_width = 0.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_container_ctx_create(
                             &ctx, CMP_CONTAINER_TYPE_INLINE_SIZE, "card"));
  ctx->inline_size = 250.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_container_query_evaluate(&query, ctx, &matches));
  ASSERT_EQ(1, matches);

  ctx->inline_size = 150.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_container_query_evaluate(&query, ctx, &matches));
  ASSERT_EQ(0, matches);

  cmp_container_ctx_destroy(ctx);
  PASS();
}

TEST test_style_query_evaluate(void) {
  cmp_style_query_t query;
  cmp_style_query_t container_styles[2];
  int matches;

  query.property_name = "--theme";
  query.property_value = "dark";

  container_styles[0].property_name = "--color";
  container_styles[0].property_value = "red";
  container_styles[1].property_name = "--theme";
  container_styles[1].property_value = "dark";

  ASSERT_EQ(CMP_SUCCESS,
            cmp_style_query_evaluate(&query, container_styles, 2, &matches));
  ASSERT_EQ(1, matches);

  container_styles[1].property_value = "light";
  ASSERT_EQ(CMP_SUCCESS,
            cmp_style_query_evaluate(&query, container_styles, 2, &matches));
  ASSERT_EQ(0, matches);

  PASS();
}

TEST test_content_visibility_evaluate(void) {
  cmp_rect_t viewport = {0.0f, 0.0f, 1920.0f, 1080.0f};
  cmp_rect_t node_rect = {100.0f, 100.0f, 200.0f, 200.0f};
  int is_visible;

  ASSERT_EQ(CMP_SUCCESS, cmp_content_visibility_evaluate(
                             CMP_CONTENT_VISIBILITY_AUTO, &viewport, &node_rect,
                             &is_visible));
  ASSERT_EQ(1, is_visible);

  node_rect.y = 2000.0f; /* Offscreen */
  ASSERT_EQ(CMP_SUCCESS, cmp_content_visibility_evaluate(
                             CMP_CONTENT_VISIBILITY_AUTO, &viewport, &node_rect,
                             &is_visible));
  ASSERT_EQ(0, is_visible);

  PASS();
}

TEST test_contain_evaluate(void) {
  int isolates_layout, isolates_paint;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_contain_evaluate(CMP_CONTAIN_NONE, &isolates_layout,
                                 &isolates_paint));
  ASSERT_EQ(0, isolates_layout);
  ASSERT_EQ(0, isolates_paint);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_contain_evaluate(CMP_CONTAIN_STRICT, &isolates_layout,
                                 &isolates_paint));
  ASSERT_EQ(1, isolates_layout);
  ASSERT_EQ(1, isolates_paint);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_contain_evaluate(CMP_CONTAIN_LAYOUT, &isolates_layout,
                                 &isolates_paint));
  ASSERT_EQ(1, isolates_layout);
  ASSERT_EQ(0, isolates_paint);

  PASS();
}

static int resize_count = 0;
static void on_resize_callback(cmp_resize_observer_t *observer,
                               cmp_layout_node_t *node, float w, float h) {
  (void)observer;
  (void)node;
  (void)w;
  (void)h;
  resize_count++;
}

TEST test_resize_observer(void) {
  cmp_resize_observer_t *obs = NULL;
  cmp_layout_node_t *node = NULL;

  cmp_layout_node_create(&node);

  resize_count = 0;
  ASSERT_EQ(CMP_SUCCESS,
            cmp_resize_observer_create(&obs, on_resize_callback, NULL));
  ASSERT_NEQ(NULL, obs);

  ASSERT_EQ(CMP_SUCCESS, cmp_resize_observer_notify(obs, node, 100.0f, 100.0f));
  ASSERT_EQ(1, resize_count);

  ASSERT_EQ(CMP_SUCCESS, cmp_resize_observer_destroy(obs));
  cmp_layout_node_destroy(node);
  PASS();
}

SUITE(cmp_media_query_suite) {
  RUN_TEST(test_media_query_evaluate);
  RUN_TEST(test_pointer_media_evaluate);
  RUN_TEST(test_update_media_evaluate);
  RUN_TEST(test_light_level_evaluate);
  RUN_TEST(test_container_query_evaluate);
  RUN_TEST(test_style_query_evaluate);
  RUN_TEST(test_content_visibility_evaluate);
  RUN_TEST(test_contain_evaluate);
  RUN_TEST(test_resize_observer);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_media_query_suite);
  GREATEST_MAIN_END();
}
