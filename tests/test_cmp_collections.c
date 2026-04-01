/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_compositional_layout(void) {
  cmp_collection_t *collection = NULL;
  cmp_collection_section_t *flow_sec = NULL;
  cmp_collection_section_t *carousel_sec = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_collection_create(&collection));

  /* Standard wrapping flow layout */
  ASSERT_EQ(CMP_SUCCESS, cmp_collection_section_create(&flow_sec));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_collection_section_set_flow_layout(flow_sec, 200.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_collection_add_section(collection, flow_sec));

  /* Orthogonal App Store style carousel */
  ASSERT_EQ(CMP_SUCCESS, cmp_collection_section_create(&carousel_sec));
  ASSERT_EQ(CMP_SUCCESS, cmp_collection_section_set_orthogonal_behavior(
                             carousel_sec, CMP_ORTHOGONAL_PAGING));
  ASSERT_EQ(CMP_SUCCESS, cmp_collection_add_section(collection, carousel_sec));

  ASSERT_EQ(CMP_SUCCESS, cmp_collection_destroy(collection));
  PASS();
}

TEST test_diffable_datasource(void) {
  cmp_diffable_datasource_t *ds = NULL;
  const uint64_t state_v1[] = {0xA1, 0xB2, 0xC3};
  const uint64_t state_v2[] = {0xA1, 0xC3, 0xD4}; /* B2 deleted, D4 inserted */

  ASSERT_EQ(CMP_SUCCESS, cmp_diffable_datasource_create(&ds));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_diffable_datasource_apply_snapshot(ds, state_v1, 3));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_diffable_datasource_apply_snapshot(ds, state_v2, 3));

  ASSERT_EQ(CMP_SUCCESS, cmp_diffable_datasource_destroy(ds));
  PASS();
}

TEST test_null_args(void) {
  cmp_collection_t *c = NULL;
  cmp_collection_section_t *s = NULL;
  cmp_diffable_datasource_t *ds = NULL;
  cmp_ui_node_t dummy;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_collection_create(NULL));
  cmp_collection_create(&c);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_collection_section_create(NULL));
  cmp_collection_section_create(&s);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_collection_section_set_flow_layout(NULL, 100.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_collection_section_set_flow_layout(s, 0.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_collection_section_set_orthogonal_behavior(
                NULL, CMP_ORTHOGONAL_NONE));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_collection_add_section(NULL, s));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_collection_add_section(c, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_diffable_datasource_create(NULL));
  cmp_diffable_datasource_create(&ds);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_diffable_datasource_apply_snapshot(NULL, NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_diffable_datasource_apply_snapshot(ds, NULL, 5));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_map_view_mount(NULL, 0.0f, 0.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_web_view_mount(NULL, "a"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_web_view_mount(&dummy, NULL));

  cmp_collection_add_section(c, s);
  cmp_collection_destroy(c);
  cmp_diffable_datasource_destroy(ds);
  PASS();
}

SUITE(collections_suite) {
  RUN_TEST(test_compositional_layout);
  RUN_TEST(test_diffable_datasource);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(collections_suite);
  GREATEST_MAIN_END();
}
