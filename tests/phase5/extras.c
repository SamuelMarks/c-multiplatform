#include "cmpc/cmp_extras.h"
#include "test_utils.h"

#include <string.h>

int CMP_CALL cmp_extras_test_set_tooltip_size_range_fail(CMPBool enable);
int CMP_CALL cmp_extras_test_set_badge_size_range_fail(CMPBool enable);
int CMP_CALL cmp_extras_test_validate_size(const CMPSize *size);
int CMP_CALL cmp_extras_test_validate_text_metrics(const CMPTextMetrics *metrics);

static int test_text_style_fill(CMPTextStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  style->utf8_family = "Test";
  style->size_px = 12;
  style->weight = 400;
  style->italic = CMP_FALSE;
  style->color.r = 0.1f;
  style->color.g = 0.2f;
  style->color.b = 0.3f;
  style->color.a = 1.0f;
  return CMP_OK;
}

static int test_metrics_fill(CMPTextMetrics *metrics, CMPScalar width,
                             CMPScalar height) {
  if (metrics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  metrics->width = width;
  metrics->height = height;
  if (height > 0.0f) {
    metrics->baseline = height * 0.8f;
  } else {
    metrics->baseline = 0.0f;
  }
  return CMP_OK;
}

static int cmp_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

int main(void) {
  CMPColor color;
  CMPLayoutEdges edges;
  CMPTextStyle text_style;
  CMPTooltipStyle tooltip_style;
  CMPTooltipStyle tooltip_style_rich;
  CMPTooltipContent tooltip_content;
  CMPTooltipAnchor tooltip_anchor;
  CMPTooltipPlacement tooltip_placement;
  CMPSize tooltip_size;
  CMPRect overlay;
  CMPRect bounds;
  cmp_u32 direction;
  CMPTextMetrics title_metrics;
  CMPTextMetrics body_metrics;
  CMPBadgeStyle badge_style;
  CMPBadgeContent badge_content;
  CMPBadgePlacement badge_placement;
  CMPSize badge_size;
  CMPRect badge_bounds;

  color.r = 0.0f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_color(NULL), CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.2f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.2f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 1.0f;
  CMP_TEST_OK(cmp_extras_test_validate_color(&color));

  CMP_TEST_EXPECT(cmp_extras_test_validate_edges(NULL), CMP_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.left = 0.0f;
  edges.right = -1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.right = 0.0f;
  edges.top = -1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.top = 0.0f;
  edges.bottom = -1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.bottom = 0.0f;
  CMP_TEST_OK(cmp_extras_test_validate_edges(&edges));

  CMP_TEST_EXPECT(cmp_extras_test_validate_rect(NULL), CMP_ERR_INVALID_ARGUMENT);
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_rect(&bounds), CMP_ERR_RANGE);
  bounds.width = 1.0f;
  bounds.height = -1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_rect(&bounds), CMP_ERR_RANGE);
  bounds.height = 1.0f;
  CMP_TEST_OK(cmp_extras_test_validate_rect(&bounds));

  CMP_TEST_EXPECT(cmp_extras_test_validate_size(NULL), CMP_ERR_INVALID_ARGUMENT);
  tooltip_size.width = -1.0f;
  tooltip_size.height = 1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_size(&tooltip_size), CMP_ERR_RANGE);
  tooltip_size.width = 1.0f;
  tooltip_size.height = -1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_size(&tooltip_size), CMP_ERR_RANGE);
  tooltip_size.height = 1.0f;
  CMP_TEST_OK(cmp_extras_test_validate_size(&tooltip_size));

  CMP_TEST_EXPECT(cmp_extras_test_validate_text_metrics(NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_extras_test_validate_text_style(NULL, CMP_FALSE),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(test_text_style_fill(&text_style));
  text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(cmp_extras_test_validate_text_style(&text_style, CMP_TRUE),
                 CMP_ERR_INVALID_ARGUMENT);
  text_style.utf8_family = "Test";
  text_style.size_px = 0;
  CMP_TEST_EXPECT(cmp_extras_test_validate_text_style(&text_style, CMP_FALSE),
                 CMP_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 99;
  CMP_TEST_EXPECT(cmp_extras_test_validate_text_style(&text_style, CMP_FALSE),
                 CMP_ERR_RANGE);
  text_style.weight = 901;
  CMP_TEST_EXPECT(cmp_extras_test_validate_text_style(&text_style, CMP_FALSE),
                 CMP_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 2;
  CMP_TEST_EXPECT(cmp_extras_test_validate_text_style(&text_style, CMP_FALSE),
                 CMP_ERR_RANGE);
  text_style.italic = CMP_FALSE;
  text_style.color.r = -0.1f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_text_style(&text_style, CMP_FALSE),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(test_text_style_fill(&text_style));
  CMP_TEST_OK(cmp_extras_test_validate_text_style(&text_style, CMP_FALSE));

  CMP_TEST_EXPECT(cmp_tooltip_style_init_plain(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  CMP_TEST_ASSERT(tooltip_style.variant == CMP_TOOLTIP_VARIANT_PLAIN);
  CMP_TEST_ASSERT(cmp_near(tooltip_style.padding.left,
                         CMP_TOOLTIP_PLAIN_DEFAULT_PADDING_X, 0.001f));
  CMP_TEST_ASSERT(cmp_near(tooltip_style.padding.top,
                         CMP_TOOLTIP_PLAIN_DEFAULT_PADDING_Y, 0.001f));
  CMP_TEST_ASSERT(cmp_near(tooltip_style.min_height,
                         CMP_TOOLTIP_PLAIN_DEFAULT_MIN_HEIGHT, 0.001f));

  CMP_TEST_EXPECT(cmp_tooltip_style_init_rich(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_tooltip_style_init_rich(&tooltip_style_rich));
  CMP_TEST_ASSERT(tooltip_style_rich.variant == CMP_TOOLTIP_VARIANT_RICH);
  CMP_TEST_ASSERT(cmp_near(tooltip_style_rich.padding.left,
                         CMP_TOOLTIP_RICH_DEFAULT_PADDING_X, 0.001f));
  CMP_TEST_ASSERT(cmp_near(tooltip_style_rich.padding.top,
                         CMP_TOOLTIP_RICH_DEFAULT_PADDING_Y, 0.001f));
  CMP_TEST_ASSERT(cmp_near(tooltip_style_rich.min_height,
                         CMP_TOOLTIP_RICH_DEFAULT_MIN_HEIGHT, 0.001f));

  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_style(NULL, CMP_FALSE),
                 CMP_ERR_INVALID_ARGUMENT);

  tooltip_style.variant = 99;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.padding.left = -1.0f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.min_width = -1.0f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.max_width = -1.0f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.min_width = 10.0f;
  tooltip_style.max_width = 5.0f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.min_height = 10.0f;
  tooltip_style.max_height = 5.0f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.anchor_gap = -1.0f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.title_body_spacing = -1.0f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.text_style.size_px = 0;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.background_color.r = -0.1f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_TRUE),
                 CMP_ERR_INVALID_ARGUMENT);
  tooltip_style.text_style.utf8_family = "Test";
  CMP_TEST_OK(cmp_extras_test_validate_tooltip_style(&tooltip_style, CMP_TRUE));

  CMP_TEST_OK(cmp_tooltip_style_init_rich(&tooltip_style_rich));
  tooltip_style_rich.title_style.size_px = 0;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style_rich, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_rich(&tooltip_style_rich));
  tooltip_style_rich.body_style.size_px = 0;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style_rich, CMP_FALSE),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_rich(&tooltip_style_rich));
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_style(&tooltip_style_rich, CMP_TRUE),
      CMP_ERR_INVALID_ARGUMENT);
  tooltip_style_rich.title_style.utf8_family = "Test";
  tooltip_style_rich.body_style.utf8_family = "Test";
  CMP_TEST_OK(
      cmp_extras_test_validate_tooltip_style(&tooltip_style_rich, CMP_TRUE));

  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_anchor(NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  tooltip_anchor.type = 99;
  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_anchor(&tooltip_anchor),
                 CMP_ERR_RANGE);
  tooltip_anchor.type = CMP_TOOLTIP_ANCHOR_RECT;
  tooltip_anchor.rect.x = 0.0f;
  tooltip_anchor.rect.y = 0.0f;
  tooltip_anchor.rect.width = -1.0f;
  tooltip_anchor.rect.height = 1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_anchor(&tooltip_anchor),
                 CMP_ERR_RANGE);
  tooltip_anchor.rect.width = 1.0f;
  tooltip_anchor.rect.height = 1.0f;
  CMP_TEST_OK(cmp_extras_test_validate_tooltip_anchor(&tooltip_anchor));
  tooltip_anchor.type = CMP_TOOLTIP_ANCHOR_POINT;
  CMP_TEST_OK(cmp_extras_test_validate_tooltip_anchor(&tooltip_anchor));

  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_placement(NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  tooltip_placement.direction = 99;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_START;
  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_placement(&tooltip_placement),
                 CMP_ERR_RANGE);
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = 99;
  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_placement(&tooltip_placement),
                 CMP_ERR_RANGE);
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_CENTER;
  CMP_TEST_OK(cmp_extras_test_validate_tooltip_placement(&tooltip_placement));

  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_content(NULL, &tooltip_content),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_content(&tooltip_style, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(test_metrics_fill(&tooltip_content.body_metrics, 10.0f, 8.0f));
  CMP_TEST_OK(test_metrics_fill(&tooltip_content.title_metrics, 12.0f, 8.0f));
  tooltip_content.has_title = 2;
  tooltip_content.has_body = CMP_TRUE;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      CMP_ERR_INVALID_ARGUMENT);
  tooltip_content.has_title = CMP_FALSE;
  tooltip_content.has_body = 2;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      CMP_ERR_INVALID_ARGUMENT);
  tooltip_content.has_body = CMP_FALSE;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      CMP_ERR_INVALID_ARGUMENT);
  tooltip_content.has_body = CMP_TRUE;
  tooltip_content.body_metrics.width = -1.0f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      CMP_ERR_RANGE);
  CMP_TEST_OK(test_metrics_fill(&tooltip_content.body_metrics, 10.0f, 8.0f));
  tooltip_content.body_metrics.baseline = -1.0f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      CMP_ERR_RANGE);
  CMP_TEST_OK(test_metrics_fill(&tooltip_content.body_metrics, 10.0f, 8.0f));
  tooltip_content.has_title = CMP_TRUE;
  tooltip_content.title_metrics.height = -1.0f;
  CMP_TEST_EXPECT(
      cmp_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      CMP_ERR_RANGE);
  CMP_TEST_OK(test_metrics_fill(&tooltip_content.title_metrics, 12.0f, 8.0f));
  tooltip_content.has_title = CMP_FALSE;
  CMP_TEST_OK(cmp_extras_test_validate_tooltip_content(&tooltip_style,
                                                     &tooltip_content));

  CMP_TEST_OK(cmp_tooltip_style_init_rich(&tooltip_style_rich));
  tooltip_content.has_title = CMP_FALSE;
  tooltip_content.has_body = CMP_FALSE;
  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_content(&tooltip_style_rich,
                                                         &tooltip_content),
                 CMP_ERR_INVALID_ARGUMENT);
  tooltip_content.has_title = CMP_TRUE;
  tooltip_content.title_metrics.width = -1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_content(&tooltip_style_rich,
                                                         &tooltip_content),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(test_metrics_fill(&tooltip_content.title_metrics, 12.0f, 8.0f));
  tooltip_content.has_body = CMP_TRUE;
  tooltip_content.body_metrics.height = -1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_tooltip_content(&tooltip_style_rich,
                                                         &tooltip_content),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(test_metrics_fill(&tooltip_content.body_metrics, 10.0f, 6.0f));
  CMP_TEST_OK(cmp_extras_test_validate_tooltip_content(&tooltip_style_rich,
                                                     &tooltip_content));

  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  CMP_TEST_OK(test_metrics_fill(&body_metrics, 20.0f, 10.0f));
  CMP_TEST_OK(test_metrics_fill(&title_metrics, 12.0f, 8.0f));
  tooltip_content.body_metrics = body_metrics;
  tooltip_content.title_metrics = title_metrics;
  tooltip_content.has_title = CMP_FALSE;
  tooltip_content.has_body = CMP_TRUE;
  CMP_TEST_EXPECT(
      cmp_tooltip_compute_content_size(NULL, &tooltip_content, &tooltip_size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_tooltip_compute_content_size(&tooltip_style, NULL, &tooltip_size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_tooltip_compute_content_size(&tooltip_style, &tooltip_content, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  tooltip_style.variant = 99;
  CMP_TEST_EXPECT(cmp_tooltip_compute_content_size(
                     &tooltip_style, &tooltip_content, &tooltip_size),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  CMP_TEST_OK(cmp_extras_test_set_tooltip_size_range_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_tooltip_compute_content_size(
                     &tooltip_style, &tooltip_content, &tooltip_size),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_compute_content_size(&tooltip_style, &tooltip_content,
                                             &tooltip_size));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.width, 36.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.height, 24.0f, 0.001f));
  tooltip_style.min_width = 40.0f;
  tooltip_style.min_height = 30.0f;
  CMP_TEST_OK(cmp_tooltip_compute_content_size(&tooltip_style, &tooltip_content,
                                             &tooltip_size));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.width, 40.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.height, 30.0f, 0.001f));
  tooltip_style.min_width = 0.0f;
  tooltip_style.min_height = 0.0f;
  tooltip_style.max_width = 32.0f;
  tooltip_style.max_height = 16.0f;
  CMP_TEST_OK(cmp_tooltip_compute_content_size(&tooltip_style, &tooltip_content,
                                             &tooltip_size));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.width, 32.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.height, 16.0f, 0.001f));
  tooltip_content.has_body = CMP_FALSE;
  CMP_TEST_EXPECT(cmp_tooltip_compute_content_size(
                     &tooltip_style, &tooltip_content, &tooltip_size),
                 CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_tooltip_style_init_rich(&tooltip_style_rich));
  CMP_TEST_OK(test_metrics_fill(&title_metrics, 50.0f, 12.0f));
  CMP_TEST_OK(test_metrics_fill(&body_metrics, 30.0f, 10.0f));
  tooltip_content.title_metrics = title_metrics;
  tooltip_content.body_metrics = body_metrics;
  tooltip_content.has_title = CMP_TRUE;
  tooltip_content.has_body = CMP_TRUE;
  CMP_TEST_OK(cmp_tooltip_compute_content_size(&tooltip_style_rich,
                                             &tooltip_content, &tooltip_size));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.width, 74.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.height, 42.0f, 0.001f));
  tooltip_style_rich.min_height = 0.0f;
  tooltip_content.has_body = CMP_FALSE;
  CMP_TEST_OK(cmp_tooltip_compute_content_size(&tooltip_style_rich,
                                             &tooltip_content, &tooltip_size));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.width, 74.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.height, 28.0f, 0.001f));
  tooltip_content.has_title = CMP_FALSE;
  tooltip_content.has_body = CMP_TRUE;
  CMP_TEST_OK(cmp_tooltip_compute_content_size(&tooltip_style_rich,
                                             &tooltip_content, &tooltip_size));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.width, 54.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(tooltip_size.height, 26.0f, 0.001f));

  CMP_TEST_EXPECT(cmp_tooltip_compute_bounds(NULL, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_anchor.type = CMP_TOOLTIP_ANCHOR_RECT;
  tooltip_anchor.rect.x = 0.0f;
  tooltip_anchor.rect.y = 0.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_START;
  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 100.0f;
  overlay.height = 100.0f;
  tooltip_size.width = -1.0f;
  tooltip_size.height = 10.0f;
  CMP_TEST_EXPECT(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 CMP_ERR_RANGE);
  tooltip_style.variant = 99;
  tooltip_size.width = 10.0f;
  CMP_TEST_EXPECT(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_anchor.type = CMP_TOOLTIP_ANCHOR_POINT;
  tooltip_anchor.point.x = 50.0f;
  tooltip_anchor.point.y = 50.0f;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  tooltip_anchor.type = 99;
  CMP_TEST_EXPECT(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 CMP_ERR_RANGE);
  tooltip_anchor.type = CMP_TOOLTIP_ANCHOR_RECT;
  tooltip_anchor.rect.x = 0.0f;
  tooltip_anchor.rect.y = 0.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_placement.direction = 99;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_START;
  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 100.0f;
  overlay.height = 100.0f;
  tooltip_size.width = 10.0f;
  tooltip_size.height = 10.0f;
  CMP_TEST_EXPECT(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 CMP_ERR_RANGE);
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = 99;
  CMP_TEST_EXPECT(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 CMP_ERR_RANGE);
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_START;
  overlay.width = -1.0f;
  CMP_TEST_EXPECT(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 CMP_ERR_RANGE);
  overlay.width = 100.0f;
  tooltip_size.width = -1.0f;
  CMP_TEST_EXPECT(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 CMP_ERR_RANGE);

  CMP_TEST_OK(cmp_tooltip_style_init_plain(&tooltip_style));
  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 100.0f;
  overlay.height = 100.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 10.0f;
  tooltip_anchor.type = CMP_TOOLTIP_ANCHOR_RECT;
  tooltip_anchor.rect.x = 10.0f;
  tooltip_anchor.rect.y = 10.0f;
  tooltip_anchor.rect.width = 20.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_START;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_DOWN);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 10.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 24.0f, 0.001f));

  tooltip_anchor.rect.x = 40.0f;
  tooltip_anchor.rect.y = 80.0f;
  tooltip_anchor.rect.width = 20.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_size.height = 20.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_CENTER;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_UP);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 40.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 56.0f, 0.001f));

  overlay.height = 30.0f;
  tooltip_anchor.rect.x = 60.0f;
  tooltip_anchor.rect.y = 20.0f;
  tooltip_anchor.rect.width = 15.0f;
  tooltip_anchor.rect.height = 5.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 25.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_END;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_UP);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 55.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 0.0f, 0.001f));

  overlay.height = 100.0f;
  tooltip_anchor.rect.x = 5.0f;
  tooltip_anchor.rect.y = 60.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 10.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_UP;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_START;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_UP);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 5.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 46.0f, 0.001f));

  tooltip_anchor.rect.x = 30.0f;
  tooltip_anchor.rect.y = 10.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_size.height = 20.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_UP;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_CENTER;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_DOWN);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 25.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 24.0f, 0.001f));

  overlay.height = 30.0f;
  tooltip_anchor.rect.x = 40.0f;
  tooltip_anchor.rect.y = 5.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 5.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 25.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_UP;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_END;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_DOWN);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 30.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 5.0f, 0.001f));

  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 60.0f;
  overlay.height = 60.0f;
  tooltip_anchor.rect.x = 10.0f;
  tooltip_anchor.rect.y = 10.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 10.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_RIGHT;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_START;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_RIGHT);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 24.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 10.0f, 0.001f));

  tooltip_anchor.rect.x = 40.0f;
  tooltip_anchor.rect.y = 20.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_RIGHT;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_CENTER;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_LEFT);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 16.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 20.0f, 0.001f));

  overlay.width = 30.0f;
  overlay.height = 30.0f;
  tooltip_anchor.rect.x = 20.0f;
  tooltip_anchor.rect.y = 10.0f;
  tooltip_anchor.rect.width = 5.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_size.width = 25.0f;
  tooltip_size.height = 10.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_RIGHT;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_END;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_LEFT);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 0.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 10.0f, 0.001f));

  overlay.width = 100.0f;
  overlay.height = 60.0f;
  tooltip_anchor.rect.x = 70.0f;
  tooltip_anchor.rect.y = 5.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 10.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_LEFT;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_START;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_LEFT);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 46.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 5.0f, 0.001f));

  tooltip_anchor.rect.x = 10.0f;
  tooltip_anchor.rect.y = 15.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_LEFT;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_CENTER;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_RIGHT);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 24.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 15.0f, 0.001f));

  overlay.width = 30.0f;
  overlay.height = 20.0f;
  tooltip_anchor.rect.x = 5.0f;
  tooltip_anchor.rect.y = 18.0f;
  tooltip_anchor.rect.width = 5.0f;
  tooltip_anchor.rect.height = 5.0f;
  tooltip_size.width = 25.0f;
  tooltip_size.height = 10.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_LEFT;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_END;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(direction == CMP_TOOLTIP_DIRECTION_RIGHT);
  CMP_TEST_ASSERT(cmp_near(bounds.x, 5.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.y, 10.0f, 0.001f));

  overlay.width = 15.0f;
  overlay.height = 8.0f;
  tooltip_anchor.rect.x = 0.0f;
  tooltip_anchor.rect.y = 0.0f;
  tooltip_anchor.rect.width = 1.0f;
  tooltip_anchor.rect.height = 1.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 10.0f;
  tooltip_placement.direction = CMP_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = CMP_TOOLTIP_ALIGN_START;
  CMP_TEST_OK(cmp_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  CMP_TEST_ASSERT(cmp_near(bounds.width, 15.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bounds.height, 8.0f, 0.001f));

  CMP_TEST_EXPECT(cmp_badge_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_badge_style_init(&badge_style));
  CMP_TEST_ASSERT(
      cmp_near(badge_style.min_size, CMP_BADGE_DEFAULT_MIN_SIZE, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(badge_style.dot_diameter, CMP_BADGE_DEFAULT_DOT_DIAMETER, 0.001f));
  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_style(NULL, CMP_FALSE),
                 CMP_ERR_INVALID_ARGUMENT);
  badge_style.min_size = -1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_style(&badge_style, CMP_FALSE),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_badge_style_init(&badge_style));
  badge_style.padding_x = -1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_style(&badge_style, CMP_FALSE),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_badge_style_init(&badge_style));
  badge_style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_style(&badge_style, CMP_FALSE),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_badge_style_init(&badge_style));
  badge_style.text_style.size_px = 0;
  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_style(&badge_style, CMP_FALSE),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_badge_style_init(&badge_style));
  badge_style.background_color.r = -0.1f;
  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_style(&badge_style, CMP_FALSE),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_badge_style_init(&badge_style));
  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_style(&badge_style, CMP_TRUE),
                 CMP_ERR_INVALID_ARGUMENT);
  badge_style.text_style.utf8_family = "Test";
  CMP_TEST_OK(cmp_extras_test_validate_badge_style(&badge_style, CMP_TRUE));

  CMP_TEST_EXPECT(cmp_badge_placement_init_icon(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_badge_placement_init_icon(&badge_placement));
  CMP_TEST_ASSERT(badge_placement.corner == CMP_BADGE_CORNER_TOP_RIGHT);
  CMP_TEST_EXPECT(cmp_badge_placement_init_navigation(NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_badge_placement_init_navigation(&badge_placement));
  CMP_TEST_ASSERT(badge_placement.corner == CMP_BADGE_CORNER_TOP_RIGHT);

  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_content(NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  badge_content.has_text = 2;
  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_content(&badge_content),
                 CMP_ERR_INVALID_ARGUMENT);
  badge_content.has_text = CMP_TRUE;
  CMP_TEST_OK(test_metrics_fill(&badge_content.text_metrics, -1.0f, 4.0f));
  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_content(&badge_content),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(test_metrics_fill(&badge_content.text_metrics, 4.0f, 2.0f));
  CMP_TEST_OK(cmp_extras_test_validate_badge_content(&badge_content));
  badge_content.has_text = CMP_FALSE;
  CMP_TEST_OK(cmp_extras_test_validate_badge_content(&badge_content));

  CMP_TEST_EXPECT(cmp_badge_compute_size(NULL, &badge_content, &badge_size),
                 CMP_ERR_INVALID_ARGUMENT);
  badge_style.min_size = -1.0f;
  CMP_TEST_EXPECT(
      cmp_badge_compute_size(&badge_style, &badge_content, &badge_size),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_badge_style_init(&badge_style));
  badge_content.has_text = 2;
  CMP_TEST_EXPECT(
      cmp_badge_compute_size(&badge_style, &badge_content, &badge_size),
      CMP_ERR_INVALID_ARGUMENT);
  badge_content.has_text = CMP_FALSE;
  CMP_TEST_OK(cmp_extras_test_set_badge_size_range_fail(CMP_TRUE));
  CMP_TEST_EXPECT(
      cmp_badge_compute_size(&badge_style, &badge_content, &badge_size),
      CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_badge_style_init(&badge_style));
  badge_content.has_text = CMP_FALSE;
  CMP_TEST_OK(cmp_badge_compute_size(&badge_style, &badge_content, &badge_size));
  CMP_TEST_ASSERT(cmp_near(badge_size.width, badge_style.dot_diameter, 0.001f));
  CMP_TEST_ASSERT(cmp_near(badge_size.height, badge_style.dot_diameter, 0.001f));
  badge_content.has_text = CMP_TRUE;
  CMP_TEST_OK(test_metrics_fill(&badge_content.text_metrics, 2.0f, 20.0f));
  CMP_TEST_OK(cmp_badge_compute_size(&badge_style, &badge_content, &badge_size));
  CMP_TEST_ASSERT(cmp_near(badge_size.width, 24.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(badge_size.height, 24.0f, 0.001f));
  CMP_TEST_OK(test_metrics_fill(&badge_content.text_metrics, 4.0f, 5.0f));
  CMP_TEST_OK(cmp_badge_compute_size(&badge_style, &badge_content, &badge_size));
  CMP_TEST_ASSERT(cmp_near(badge_size.height, badge_style.min_size, 0.001f));

  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_placement(NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  badge_placement.corner = 99;
  CMP_TEST_EXPECT(cmp_extras_test_validate_badge_placement(&badge_placement),
                 CMP_ERR_RANGE);
  badge_placement.corner = CMP_BADGE_CORNER_TOP_LEFT;
  CMP_TEST_OK(cmp_extras_test_validate_badge_placement(&badge_placement));

  CMP_TEST_OK(cmp_badge_style_init(&badge_style));
  badge_style.dot_diameter = 10.0f;
  badge_content.has_text = CMP_FALSE;
  badge_placement.offset_x = 0.0f;
  badge_placement.offset_y = 0.0f;
  CMP_TEST_EXPECT(cmp_badge_compute_bounds(&badge_style, &badge_content, NULL,
                                         &badge_placement, &badge_bounds),
                 CMP_ERR_INVALID_ARGUMENT);
  badge_bounds.x = 0.0f;
  badge_bounds.y = 0.0f;
  badge_bounds.width = 0.0f;
  badge_bounds.height = 0.0f;
  badge_placement.corner = 99;
  bounds.x = 10.0f;
  bounds.y = 10.0f;
  bounds.width = 20.0f;
  bounds.height = 20.0f;
  badge_style.min_size = -1.0f;
  CMP_TEST_EXPECT(cmp_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                         &badge_placement, &badge_bounds),
                 CMP_ERR_RANGE);
  badge_placement.corner = CMP_BADGE_CORNER_TOP_LEFT;
  CMP_TEST_EXPECT(cmp_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                         &badge_placement, &badge_bounds),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_badge_style_init(&badge_style));
  badge_style.dot_diameter = 10.0f;
  badge_content.has_text = CMP_FALSE;
  badge_placement.corner = 99;
  CMP_TEST_EXPECT(cmp_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                         &badge_placement, &badge_bounds),
                 CMP_ERR_RANGE);
  badge_placement.corner = CMP_BADGE_CORNER_TOP_LEFT;
  CMP_TEST_OK(cmp_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                     &badge_placement, &badge_bounds));
  CMP_TEST_ASSERT(cmp_near(badge_bounds.x, 5.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(badge_bounds.y, 5.0f, 0.001f));
  badge_placement.corner = CMP_BADGE_CORNER_TOP_RIGHT;
  CMP_TEST_OK(cmp_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                     &badge_placement, &badge_bounds));
  CMP_TEST_ASSERT(cmp_near(badge_bounds.x, 25.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(badge_bounds.y, 5.0f, 0.001f));
  badge_placement.corner = CMP_BADGE_CORNER_BOTTOM_RIGHT;
  CMP_TEST_OK(cmp_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                     &badge_placement, &badge_bounds));
  CMP_TEST_ASSERT(cmp_near(badge_bounds.x, 25.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(badge_bounds.y, 25.0f, 0.001f));
  badge_placement.corner = CMP_BADGE_CORNER_BOTTOM_LEFT;
  CMP_TEST_OK(cmp_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                     &badge_placement, &badge_bounds));
  CMP_TEST_ASSERT(cmp_near(badge_bounds.x, 5.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(badge_bounds.y, 25.0f, 0.001f));

  bounds.width = -1.0f;
  CMP_TEST_EXPECT(cmp_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                         &badge_placement, &badge_bounds),
                 CMP_ERR_RANGE);

  return 0;
}
