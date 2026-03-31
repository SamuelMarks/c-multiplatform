/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_svg_suite);

TEST test_svg_viewbox(void) {
  cmp_svg_viewbox_t vb;
  float out_x, out_y, scale_x, scale_y;

  vb.x = 0.0f;
  vb.y = 0.0f;
  vb.width = 100.0f;
  vb.height = 100.0f;
  vb.aspect_ratio.align = CMP_SVG_ASPECT_RATIO_NONE;
  vb.aspect_ratio.meet_or_slice = CMP_SVG_ASPECT_RATIO_MEET;

  /* Test NONE */
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_viewbox_evaluate(&vb, 200.0f, 300.0f, &out_x,
                                                  &out_y, &scale_x, &scale_y));
  ASSERT_EQ(0.0f, out_x);
  ASSERT_EQ(0.0f, out_y);
  ASSERT_EQ(2.0f, scale_x);
  ASSERT_EQ(3.0f, scale_y);

  /* Test xMidYMid meet */
  vb.aspect_ratio.align = CMP_SVG_ASPECT_RATIO_XMID_YMID;
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_viewbox_evaluate(&vb, 200.0f, 300.0f, &out_x,
                                                  &out_y, &scale_x, &scale_y));
  ASSERT_EQ(0.0f, out_x);  /* 200 - 100*2 = 0 / 2 = 0 */
  ASSERT_EQ(50.0f, out_y); /* 300 - 100*2 = 100 / 2 = 50 */
  ASSERT_EQ(2.0f, scale_x);
  ASSERT_EQ(2.0f, scale_y);

  /* Test xMaxYMax slice */
  vb.aspect_ratio.align = CMP_SVG_ASPECT_RATIO_XMAX_YMAX;
  vb.aspect_ratio.meet_or_slice = CMP_SVG_ASPECT_RATIO_SLICE;
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_viewbox_evaluate(&vb, 200.0f, 300.0f, &out_x,
                                                  &out_y, &scale_x, &scale_y));
  ASSERT_EQ(-100.0f, out_x); /* 200 - 100*3 = -100 */
  ASSERT_EQ(0.0f, out_y);    /* 300 - 100*3 = 0 */
  ASSERT_EQ(3.0f, scale_x);
  ASSERT_EQ(3.0f, scale_y);

  /* Test with offset viewbox and xMinYMin meet */
  vb.x = -50.0f;
  vb.y = -50.0f;
  vb.aspect_ratio.align = CMP_SVG_ASPECT_RATIO_XMIN_YMIN;
  vb.aspect_ratio.meet_or_slice = CMP_SVG_ASPECT_RATIO_MEET;
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_viewbox_evaluate(&vb, 200.0f, 300.0f, &out_x,
                                                  &out_y, &scale_x, &scale_y));
  ASSERT_EQ(100.0f, out_x); /* 0 - (-50)*2 = 100 */
  ASSERT_EQ(100.0f, out_y); /* 0 - (-50)*2 = 100 */
  ASSERT_EQ(2.0f, scale_x);
  ASSERT_EQ(2.0f, scale_y);

  /* Test Zero bounds fallback */
  vb.width = 0.0f;
  vb.height = 0.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_viewbox_evaluate(&vb, 200.0f, 300.0f, &out_x,
                                                  &out_y, &scale_x, &scale_y));
  ASSERT_EQ(0.0f, out_x);
  ASSERT_EQ(0.0f, out_y);
  ASSERT_EQ(1.0f, scale_x);
  ASSERT_EQ(1.0f, scale_y);

  /* Test xMidYMin and xMinYMax */
  vb.width = 100.0f;
  vb.height = 100.0f;
  vb.x = 0.0f;
  vb.y = 0.0f;
  vb.aspect_ratio.align = CMP_SVG_ASPECT_RATIO_XMID_YMIN;
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_viewbox_evaluate(&vb, 200.0f, 300.0f, &out_x,
                                                  &out_y, &scale_x, &scale_y));
  ASSERT_EQ(0.0f, out_x);
  ASSERT_EQ(0.0f, out_y);

  vb.aspect_ratio.align = CMP_SVG_ASPECT_RATIO_XMIN_YMAX;
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_viewbox_evaluate(&vb, 200.0f, 300.0f, &out_x,
                                                  &out_y, &scale_x, &scale_y));
  ASSERT_EQ(0.0f, out_x);
  ASSERT_EQ(100.0f, out_y);

  PASS();
}
TEST test_svg_path_tessellate(void) {
  float input[] = {0.0f, 0.0f, 10.0f, 10.0f};
  float input_bezier[] = {0.0f, 0.0f, 0.0f, 10.0f, 10.0f, 10.0f, 10.0f, 0.0f};
  float input_arc[] = {0.0f, 0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 1.0f, 10.0f, 0.0f};
  float *output = NULL;
  size_t count = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_path_tessellate(CMP_SVG_PATH_POLYGON, input, 4,
                                                 &output, &count));
  ASSERT_NEQ(NULL, output);
  ASSERT_EQ(2, count);
  ASSERT_EQ(10.0f, output[2]);

  CMP_FREE(output);
  output = NULL;
  count = 0;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_svg_path_tessellate(CMP_SVG_PATH_BEZIER, input_bezier, 8,
                                    &output, &count));
  ASSERT_NEQ(NULL, output);
  ASSERT(count >= 8); /* Should have generated intermediate points */
  CMP_FREE(output);
  output = NULL;
  count = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_path_tessellate(CMP_SVG_PATH_ARC, input_arc, 9,
                                                 &output, &count));
  ASSERT_NEQ(NULL, output);
  ASSERT(count >= 4); /* Should have generated intermediate arc points */
  CMP_FREE(output);

  PASS();
}

TEST test_svg_renderer_api(void) {
  cmp_svg_renderer_t *r = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_renderer_create(&r, 0.5f));
  ASSERT_NEQ(NULL, r);

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_renderer_move_to(r, 0.0f, 0.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_renderer_line_to(r, 10.0f, 0.0f));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_svg_renderer_quad_to(r, 10.0f, 10.0f, 0.0f, 10.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_renderer_close(r));

  ASSERT(r->vertex_count > 4);
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_renderer_destroy(r));
  PASS();
}

TEST test_svg_stroke(void) {
  cmp_svg_stroke_t stroke;
  float input[] = {0.0f, 0.0f, 10.0f, 10.0f};
  float *output = NULL;
  size_t count = 0;

  stroke.width = 2.0f;
  stroke.linecap = 0;
  stroke.linejoin = 0;
  stroke.miter_limit = 4.0f;
  stroke.dash.count = 0;
  stroke.dash.array = NULL;
  stroke.dash.offset = 0.0f;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_svg_stroke_evaluate(&stroke, input, 2, &output, &count));
  ASSERT_NEQ(NULL, output);
  ASSERT_EQ(4, count); /* Start Left, End Left, End Right, Start Right */

  CMP_FREE(output);
  PASS();
}

TEST test_svg_fill(void) {
  cmp_svg_fill_t fill;
  float input[] = {0.0f, 0.0f, 10.0f, 0.0f, 10.0f, 10.0f, 0.0f, 10.0f};
  float *output = NULL;
  size_t count = 0;

  fill.rule = CMP_SVG_FILL_NONZERO;
  fill.color.r = 1.0f;
  fill.color.g = 0.0f;
  fill.color.b = 0.0f;
  fill.color.a = 1.0f;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_svg_fill_evaluate(&fill, input, 4, &output, &count));
  ASSERT_NEQ(NULL, output);
  ASSERT_EQ(6, count); /* 2 triangles (4 vertices) */

  CMP_FREE(output);
  PASS();
}

TEST test_svg_dash(void) {
  cmp_svg_dash_t dash;
  float input[] = {0.0f, 0.0f, 10.0f, 10.0f};
  float **out_subpaths = NULL;
  size_t *out_counts = NULL;
  size_t subpath_count = 0;

  dash.count = 0;
  dash.array = NULL;
  dash.offset = 0.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_dash_evaluate(&dash, input, 2, &out_subpaths,
                                               &out_counts, &subpath_count));
  ASSERT_EQ(1, subpath_count);
  ASSERT_EQ(2, out_counts[0]);

  CMP_FREE(out_subpaths[0]);
  CMP_FREE(out_subpaths);
  CMP_FREE(out_counts);

  PASS();
}

TEST test_svg_node_lifecycle(void) {
  cmp_svg_node_t *node = NULL;
  cmp_svg_node_t *child = NULL;
  cmp_svg_node_t *clone = NULL;
  void *dummy_style;

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_node_create(&node));
  ASSERT_NEQ(NULL, node);

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_node_create(&child));
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_node_add_child(node, child));
  ASSERT_EQ(1, node->child_count);
  ASSERT_EQ(node, child->parent);

  dummy_style = (void *)&node;

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_css_bind(node, dummy_style));
  ASSERT_NEQ(NULL, node->css);
  ASSERT_EQ(dummy_style, node->css->computed_style);
  ASSERT_EQ(0, strcmp(node->css->element_name, "svg-node"));
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_use_instantiate(node, &clone));
  ASSERT_NEQ(NULL, clone);

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_node_destroy(clone));
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_node_destroy(node));

  PASS();
}

TEST test_svg_smil_tick(void) {
  cmp_svg_node_t *node = NULL;
  cmp_svg_smil_t anim;

  anim.duration = 1000.0f;
  anim.current_time = 0.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_node_create(&node));
  node->animations = &anim;

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_smil_tick(node, 500.0f));
  ASSERT_EQ(500.0f, anim.current_time);

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_smil_tick(node, 600.0f));
  ASSERT_EQ(1000.0f, anim.current_time); /* Cap test */

  /* Prevent pointer cascading logic to stack allocated anim */
  node->animations = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_node_destroy(node));
  PASS();
}

TEST test_svg_foreign_bind(void) {
  cmp_svg_node_t *node = NULL;
  cmp_layout_node_t dom_node;

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_node_create(&node));
  ASSERT_EQ(CMP_SUCCESS, cmp_svg_foreign_bind(node, &dom_node));

  ASSERT_EQ(1, node->is_foreign_object);
  ASSERT_EQ(&dom_node, node->dom_mapping);

  ASSERT_EQ(CMP_SUCCESS, cmp_svg_node_destroy(node));
  PASS();
}

TEST test_svg_filter_evaluate(void) {
  cmp_svg_filter_node_t filter;
  unsigned char in_pixels[16] = {255, 0,   0,   255,  /* Red */
                                 0,   255, 0,   255,  /* Green */
                                 0,   0,   255, 255,  /* Blue */
                                 255, 255, 255, 255}; /* White */
  unsigned char *out_pixels = NULL;

  /* Color Matrix Test (Grayscale Luma) */
  filter.type = CMP_SVG_FE_COLOR_MATRIX;
  filter.config = NULL;
  filter.next = NULL;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_svg_filter_evaluate(&filter, in_pixels, 2, 2, &out_pixels));
  ASSERT_NEQ(NULL, out_pixels);

  /* Red -> Luma approx 76 */
  ASSERT_EQ(76, out_pixels[0]);
  ASSERT_EQ(76, out_pixels[1]);
  ASSERT_EQ(76, out_pixels[2]);
  ASSERT_EQ(255, out_pixels[3]);

  /* Green -> Luma approx 149 */
  ASSERT_EQ(149, out_pixels[4]);

  CMP_FREE(out_pixels);

  /* Fallback Test */
  filter.type = CMP_SVG_FE_DISPLACEMENT_MAP;
  ASSERT_EQ(CMP_SUCCESS,
            cmp_svg_filter_evaluate(&filter, in_pixels, 2, 2, &out_pixels));
  ASSERT_NEQ(NULL, out_pixels);
  ASSERT_EQ(255, out_pixels[0]); /* Unchanged Red */
  ASSERT_EQ(0, out_pixels[1]);

  CMP_FREE(out_pixels);
  PASS();
}

SUITE(cmp_svg_suite) {
  RUN_TEST(test_svg_viewbox);
  RUN_TEST(test_svg_path_tessellate);
  RUN_TEST(test_svg_renderer_api);
  RUN_TEST(test_svg_stroke);
  RUN_TEST(test_svg_fill);
  RUN_TEST(test_svg_dash);
  RUN_TEST(test_svg_node_lifecycle);
  RUN_TEST(test_svg_smil_tick);
  RUN_TEST(test_svg_foreign_bind);
  RUN_TEST(test_svg_filter_evaluate);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_svg_suite);
  GREATEST_MAIN_END();
}