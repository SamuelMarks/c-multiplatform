/* clang-format off */
#include "cmp_css_transforms.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_transform_group(void) {
  cmp_prop_transform_group_t group;

  ASSERT_EQ(-1, cmp_prop_transform_group_init(
                    NULL, NULL, NULL, CMP_TRANSFORM_STYLE_FLAT, NULL, NULL,
                    CMP_BACKFACE_VISIBILITY_VISIBLE));

  ASSERT_EQ(0, cmp_prop_transform_group_init(&group, "scale(2)", "50% 50%",
                                             CMP_TRANSFORM_STYLE_PRESERVE_3D,
                                             "1000px", "top",
                                             CMP_BACKFACE_VISIBILITY_HIDDEN));
  ASSERT_STR_EQ("scale(2)", group.transform);
  ASSERT_STR_EQ("50% 50%", group.transform_origin);
  ASSERT_EQ(CMP_TRANSFORM_STYLE_PRESERVE_3D, group.transform_style);
  ASSERT_STR_EQ("1000px", group.perspective);
  ASSERT_STR_EQ("top", group.perspective_origin);
  ASSERT_EQ(CMP_BACKFACE_VISIBILITY_HIDDEN, group.backface);

  ASSERT_EQ(0, cmp_prop_transform_group_free(&group));
  ASSERT_EQ(NULL, group.transform);
  ASSERT_EQ(NULL, group.transform_origin);
  ASSERT_EQ(NULL, group.perspective);
  ASSERT_EQ(NULL, group.perspective_origin);

  /* Partial init */
  ASSERT_EQ(0, cmp_prop_transform_group_init(
                   &group, "rotate(45deg)", NULL, CMP_TRANSFORM_STYLE_FLAT,
                   NULL, NULL, CMP_BACKFACE_VISIBILITY_VISIBLE));
  ASSERT_STR_EQ("rotate(45deg)", group.transform);
  ASSERT_EQ(NULL, group.perspective);
  ASSERT_EQ(0, cmp_prop_transform_group_free(&group));

  ASSERT_EQ(-1, cmp_prop_transform_group_free(NULL));

  PASS();
}

TEST test_independent_transform(void) {
  cmp_prop_independent_transform_t group;

  ASSERT_EQ(-1, cmp_prop_independent_transform_init(NULL, NULL, NULL, NULL));

  ASSERT_EQ(0, cmp_prop_independent_transform_init(&group, "10px 20px", "1turn",
                                                   "2 0.5"));
  ASSERT_STR_EQ("10px 20px", group.translate);
  ASSERT_STR_EQ("1turn", group.rotate);
  ASSERT_STR_EQ("2 0.5", group.scale);

  ASSERT_EQ(0, cmp_prop_independent_transform_free(&group));
  ASSERT_EQ(NULL, group.translate);
  ASSERT_EQ(NULL, group.rotate);
  ASSERT_EQ(NULL, group.scale);

  ASSERT_EQ(0,
            cmp_prop_independent_transform_init(&group, NULL, "90deg", NULL));
  ASSERT_STR_EQ("90deg", group.rotate);
  ASSERT_EQ(NULL, group.translate);
  ASSERT_EQ(0, cmp_prop_independent_transform_free(&group));

  ASSERT_EQ(-1, cmp_prop_independent_transform_free(NULL));

  PASS();
}

TEST test_transform_funcs(void) {
  cmp_transform_funcs_t func;

  ASSERT_EQ(-1, cmp_transform_funcs_init(NULL, CMP_TRANSFORM_FUNC_MATRIX,
                                         "1, 0, 0, 1, 0, 0"));
  ASSERT_EQ(-1,
            cmp_transform_funcs_init(&func, CMP_TRANSFORM_FUNC_MATRIX, NULL));

  ASSERT_EQ(0, cmp_transform_funcs_init(&func, CMP_TRANSFORM_FUNC_MATRIX3D,
                                        "1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1"));
  ASSERT_EQ(CMP_TRANSFORM_FUNC_MATRIX3D, func.type);
  ASSERT_STR_EQ("1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1", func.args);

  ASSERT_EQ(0, cmp_transform_funcs_free(&func));
  ASSERT_EQ(NULL, func.args);

  ASSERT_EQ(-1, cmp_transform_funcs_free(NULL));

  PASS();
}

TEST test_motion_path_group(void) {
  cmp_prop_motion_path_group_t group;

  ASSERT_EQ(
      -1, cmp_prop_motion_path_group_init(NULL, NULL, NULL, NULL, NULL, NULL));

  ASSERT_EQ(0, cmp_prop_motion_path_group_init(&group,
                                               "path('M 0 0 L 100 100')", "50%",
                                               "auto", "center", "auto 90deg"));
  ASSERT_STR_EQ("path('M 0 0 L 100 100')", group.path);
  ASSERT_STR_EQ("50%", group.distance);
  ASSERT_STR_EQ("auto", group.position);
  ASSERT_STR_EQ("center", group.anchor);
  ASSERT_STR_EQ("auto 90deg", group.rotate);

  ASSERT_EQ(0, cmp_prop_motion_path_group_free(&group));
  ASSERT_EQ(NULL, group.path);

  ASSERT_EQ(0, cmp_prop_motion_path_group_init(&group, "none", NULL, NULL, NULL,
                                               NULL));
  ASSERT_STR_EQ("none", group.path);
  ASSERT_EQ(NULL, group.distance);
  ASSERT_EQ(0, cmp_prop_motion_path_group_free(&group));

  ASSERT_EQ(-1, cmp_prop_motion_path_group_free(NULL));

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_transform_group);
  RUN_TEST(test_independent_transform);
  RUN_TEST(test_transform_funcs);
  RUN_TEST(test_motion_path_group);
  GREATEST_MAIN_END();
}