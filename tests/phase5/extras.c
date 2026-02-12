#include "m3/m3_extras.h"
#include "test_utils.h"

#include <string.h>

int M3_CALL m3_extras_test_set_tooltip_size_range_fail(M3Bool enable);
int M3_CALL m3_extras_test_set_badge_size_range_fail(M3Bool enable);
int M3_CALL m3_extras_test_validate_size(const M3Size *size);
int M3_CALL m3_extras_test_validate_text_metrics(const M3TextMetrics *metrics);

static int test_text_style_fill(M3TextStyle *style) {
  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  style->utf8_family = "Test";
  style->size_px = 12;
  style->weight = 400;
  style->italic = M3_FALSE;
  style->color.r = 0.1f;
  style->color.g = 0.2f;
  style->color.b = 0.3f;
  style->color.a = 1.0f;
  return M3_OK;
}

static int test_metrics_fill(M3TextMetrics *metrics, M3Scalar width,
                             M3Scalar height) {
  if (metrics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  metrics->width = width;
  metrics->height = height;
  if (height > 0.0f) {
    metrics->baseline = height * 0.8f;
  } else {
    metrics->baseline = 0.0f;
  }
  return M3_OK;
}

static int m3_near(M3Scalar a, M3Scalar b, M3Scalar tol) {
  M3Scalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

int main(void) {
  M3Color color;
  M3LayoutEdges edges;
  M3TextStyle text_style;
  M3TooltipStyle tooltip_style;
  M3TooltipStyle tooltip_style_rich;
  M3TooltipContent tooltip_content;
  M3TooltipAnchor tooltip_anchor;
  M3TooltipPlacement tooltip_placement;
  M3Size tooltip_size;
  M3Rect overlay;
  M3Rect bounds;
  m3_u32 direction;
  M3TextMetrics title_metrics;
  M3TextMetrics body_metrics;
  M3BadgeStyle badge_style;
  M3BadgeContent badge_content;
  M3BadgePlacement badge_placement;
  M3Size badge_size;
  M3Rect badge_bounds;

  color.r = 0.0f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_color(NULL), M3_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  M3_TEST_EXPECT(m3_extras_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  M3_TEST_EXPECT(m3_extras_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.2f;
  M3_TEST_EXPECT(m3_extras_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.2f;
  M3_TEST_EXPECT(m3_extras_test_validate_color(&color), M3_ERR_RANGE);
  color.a = 1.0f;
  M3_TEST_OK(m3_extras_test_validate_color(&color));

  M3_TEST_EXPECT(m3_extras_test_validate_edges(NULL), M3_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.left = 0.0f;
  edges.right = -1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.right = 0.0f;
  edges.top = -1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.top = 0.0f;
  edges.bottom = -1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.bottom = 0.0f;
  M3_TEST_OK(m3_extras_test_validate_edges(&edges));

  M3_TEST_EXPECT(m3_extras_test_validate_rect(NULL), M3_ERR_INVALID_ARGUMENT);
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_rect(&bounds), M3_ERR_RANGE);
  bounds.width = 1.0f;
  bounds.height = -1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_rect(&bounds), M3_ERR_RANGE);
  bounds.height = 1.0f;
  M3_TEST_OK(m3_extras_test_validate_rect(&bounds));

  M3_TEST_EXPECT(m3_extras_test_validate_size(NULL), M3_ERR_INVALID_ARGUMENT);
  tooltip_size.width = -1.0f;
  tooltip_size.height = 1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_size(&tooltip_size), M3_ERR_RANGE);
  tooltip_size.width = 1.0f;
  tooltip_size.height = -1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_size(&tooltip_size), M3_ERR_RANGE);
  tooltip_size.height = 1.0f;
  M3_TEST_OK(m3_extras_test_validate_size(&tooltip_size));

  M3_TEST_EXPECT(m3_extras_test_validate_text_metrics(NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_extras_test_validate_text_style(NULL, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(test_text_style_fill(&text_style));
  text_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_extras_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  text_style.utf8_family = "Test";
  text_style.size_px = 0;
  M3_TEST_EXPECT(m3_extras_test_validate_text_style(&text_style, M3_FALSE),
                 M3_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 99;
  M3_TEST_EXPECT(m3_extras_test_validate_text_style(&text_style, M3_FALSE),
                 M3_ERR_RANGE);
  text_style.weight = 901;
  M3_TEST_EXPECT(m3_extras_test_validate_text_style(&text_style, M3_FALSE),
                 M3_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 2;
  M3_TEST_EXPECT(m3_extras_test_validate_text_style(&text_style, M3_FALSE),
                 M3_ERR_RANGE);
  text_style.italic = M3_FALSE;
  text_style.color.r = -0.1f;
  M3_TEST_EXPECT(m3_extras_test_validate_text_style(&text_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(test_text_style_fill(&text_style));
  M3_TEST_OK(m3_extras_test_validate_text_style(&text_style, M3_FALSE));

  M3_TEST_EXPECT(m3_tooltip_style_init_plain(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  M3_TEST_ASSERT(tooltip_style.variant == M3_TOOLTIP_VARIANT_PLAIN);
  M3_TEST_ASSERT(m3_near(tooltip_style.padding.left,
                         M3_TOOLTIP_PLAIN_DEFAULT_PADDING_X, 0.001f));
  M3_TEST_ASSERT(m3_near(tooltip_style.padding.top,
                         M3_TOOLTIP_PLAIN_DEFAULT_PADDING_Y, 0.001f));
  M3_TEST_ASSERT(m3_near(tooltip_style.min_height,
                         M3_TOOLTIP_PLAIN_DEFAULT_MIN_HEIGHT, 0.001f));

  M3_TEST_EXPECT(m3_tooltip_style_init_rich(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_tooltip_style_init_rich(&tooltip_style_rich));
  M3_TEST_ASSERT(tooltip_style_rich.variant == M3_TOOLTIP_VARIANT_RICH);
  M3_TEST_ASSERT(m3_near(tooltip_style_rich.padding.left,
                         M3_TOOLTIP_RICH_DEFAULT_PADDING_X, 0.001f));
  M3_TEST_ASSERT(m3_near(tooltip_style_rich.padding.top,
                         M3_TOOLTIP_RICH_DEFAULT_PADDING_Y, 0.001f));
  M3_TEST_ASSERT(m3_near(tooltip_style_rich.min_height,
                         M3_TOOLTIP_RICH_DEFAULT_MIN_HEIGHT, 0.001f));

  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_style(NULL, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);

  tooltip_style.variant = 99;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.padding.left = -1.0f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.min_width = -1.0f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.max_width = -1.0f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.min_width = 10.0f;
  tooltip_style.max_width = 5.0f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.min_height = 10.0f;
  tooltip_style.max_height = 5.0f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.corner_radius = -1.0f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.anchor_gap = -1.0f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.title_body_spacing = -1.0f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.text_style.size_px = 0;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_style.background_color.r = -0.1f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_style(&tooltip_style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  tooltip_style.text_style.utf8_family = "Test";
  M3_TEST_OK(m3_extras_test_validate_tooltip_style(&tooltip_style, M3_TRUE));

  M3_TEST_OK(m3_tooltip_style_init_rich(&tooltip_style_rich));
  tooltip_style_rich.title_style.size_px = 0;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style_rich, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_rich(&tooltip_style_rich));
  tooltip_style_rich.body_style.size_px = 0;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style_rich, M3_FALSE),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_rich(&tooltip_style_rich));
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_style(&tooltip_style_rich, M3_TRUE),
      M3_ERR_INVALID_ARGUMENT);
  tooltip_style_rich.title_style.utf8_family = "Test";
  tooltip_style_rich.body_style.utf8_family = "Test";
  M3_TEST_OK(
      m3_extras_test_validate_tooltip_style(&tooltip_style_rich, M3_TRUE));

  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_anchor(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  tooltip_anchor.type = 99;
  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_anchor(&tooltip_anchor),
                 M3_ERR_RANGE);
  tooltip_anchor.type = M3_TOOLTIP_ANCHOR_RECT;
  tooltip_anchor.rect.x = 0.0f;
  tooltip_anchor.rect.y = 0.0f;
  tooltip_anchor.rect.width = -1.0f;
  tooltip_anchor.rect.height = 1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_anchor(&tooltip_anchor),
                 M3_ERR_RANGE);
  tooltip_anchor.rect.width = 1.0f;
  tooltip_anchor.rect.height = 1.0f;
  M3_TEST_OK(m3_extras_test_validate_tooltip_anchor(&tooltip_anchor));
  tooltip_anchor.type = M3_TOOLTIP_ANCHOR_POINT;
  M3_TEST_OK(m3_extras_test_validate_tooltip_anchor(&tooltip_anchor));

  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_placement(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  tooltip_placement.direction = 99;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_START;
  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_placement(&tooltip_placement),
                 M3_ERR_RANGE);
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = 99;
  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_placement(&tooltip_placement),
                 M3_ERR_RANGE);
  tooltip_placement.align = M3_TOOLTIP_ALIGN_CENTER;
  M3_TEST_OK(m3_extras_test_validate_tooltip_placement(&tooltip_placement));

  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_content(NULL, &tooltip_content),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_content(&tooltip_style, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(test_metrics_fill(&tooltip_content.body_metrics, 10.0f, 8.0f));
  M3_TEST_OK(test_metrics_fill(&tooltip_content.title_metrics, 12.0f, 8.0f));
  tooltip_content.has_title = 2;
  tooltip_content.has_body = M3_TRUE;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      M3_ERR_INVALID_ARGUMENT);
  tooltip_content.has_title = M3_FALSE;
  tooltip_content.has_body = 2;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      M3_ERR_INVALID_ARGUMENT);
  tooltip_content.has_body = M3_FALSE;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      M3_ERR_INVALID_ARGUMENT);
  tooltip_content.has_body = M3_TRUE;
  tooltip_content.body_metrics.width = -1.0f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      M3_ERR_RANGE);
  M3_TEST_OK(test_metrics_fill(&tooltip_content.body_metrics, 10.0f, 8.0f));
  tooltip_content.body_metrics.baseline = -1.0f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      M3_ERR_RANGE);
  M3_TEST_OK(test_metrics_fill(&tooltip_content.body_metrics, 10.0f, 8.0f));
  tooltip_content.has_title = M3_TRUE;
  tooltip_content.title_metrics.height = -1.0f;
  M3_TEST_EXPECT(
      m3_extras_test_validate_tooltip_content(&tooltip_style, &tooltip_content),
      M3_ERR_RANGE);
  M3_TEST_OK(test_metrics_fill(&tooltip_content.title_metrics, 12.0f, 8.0f));
  tooltip_content.has_title = M3_FALSE;
  M3_TEST_OK(m3_extras_test_validate_tooltip_content(&tooltip_style,
                                                     &tooltip_content));

  M3_TEST_OK(m3_tooltip_style_init_rich(&tooltip_style_rich));
  tooltip_content.has_title = M3_FALSE;
  tooltip_content.has_body = M3_FALSE;
  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_content(&tooltip_style_rich,
                                                         &tooltip_content),
                 M3_ERR_INVALID_ARGUMENT);
  tooltip_content.has_title = M3_TRUE;
  tooltip_content.title_metrics.width = -1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_content(&tooltip_style_rich,
                                                         &tooltip_content),
                 M3_ERR_RANGE);
  M3_TEST_OK(test_metrics_fill(&tooltip_content.title_metrics, 12.0f, 8.0f));
  tooltip_content.has_body = M3_TRUE;
  tooltip_content.body_metrics.height = -1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_tooltip_content(&tooltip_style_rich,
                                                         &tooltip_content),
                 M3_ERR_RANGE);
  M3_TEST_OK(test_metrics_fill(&tooltip_content.body_metrics, 10.0f, 6.0f));
  M3_TEST_OK(m3_extras_test_validate_tooltip_content(&tooltip_style_rich,
                                                     &tooltip_content));

  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  M3_TEST_OK(test_metrics_fill(&body_metrics, 20.0f, 10.0f));
  M3_TEST_OK(test_metrics_fill(&title_metrics, 12.0f, 8.0f));
  tooltip_content.body_metrics = body_metrics;
  tooltip_content.title_metrics = title_metrics;
  tooltip_content.has_title = M3_FALSE;
  tooltip_content.has_body = M3_TRUE;
  M3_TEST_EXPECT(
      m3_tooltip_compute_content_size(NULL, &tooltip_content, &tooltip_size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_tooltip_compute_content_size(&tooltip_style, NULL, &tooltip_size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_tooltip_compute_content_size(&tooltip_style, &tooltip_content, NULL),
      M3_ERR_INVALID_ARGUMENT);
  tooltip_style.variant = 99;
  M3_TEST_EXPECT(m3_tooltip_compute_content_size(
                     &tooltip_style, &tooltip_content, &tooltip_size),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  M3_TEST_OK(m3_extras_test_set_tooltip_size_range_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_tooltip_compute_content_size(
                     &tooltip_style, &tooltip_content, &tooltip_size),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_compute_content_size(&tooltip_style, &tooltip_content,
                                             &tooltip_size));
  M3_TEST_ASSERT(m3_near(tooltip_size.width, 36.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(tooltip_size.height, 24.0f, 0.001f));
  tooltip_style.min_width = 40.0f;
  tooltip_style.min_height = 30.0f;
  M3_TEST_OK(m3_tooltip_compute_content_size(&tooltip_style, &tooltip_content,
                                             &tooltip_size));
  M3_TEST_ASSERT(m3_near(tooltip_size.width, 40.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(tooltip_size.height, 30.0f, 0.001f));
  tooltip_style.min_width = 0.0f;
  tooltip_style.min_height = 0.0f;
  tooltip_style.max_width = 32.0f;
  tooltip_style.max_height = 16.0f;
  M3_TEST_OK(m3_tooltip_compute_content_size(&tooltip_style, &tooltip_content,
                                             &tooltip_size));
  M3_TEST_ASSERT(m3_near(tooltip_size.width, 32.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(tooltip_size.height, 16.0f, 0.001f));
  tooltip_content.has_body = M3_FALSE;
  M3_TEST_EXPECT(m3_tooltip_compute_content_size(
                     &tooltip_style, &tooltip_content, &tooltip_size),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_tooltip_style_init_rich(&tooltip_style_rich));
  M3_TEST_OK(test_metrics_fill(&title_metrics, 50.0f, 12.0f));
  M3_TEST_OK(test_metrics_fill(&body_metrics, 30.0f, 10.0f));
  tooltip_content.title_metrics = title_metrics;
  tooltip_content.body_metrics = body_metrics;
  tooltip_content.has_title = M3_TRUE;
  tooltip_content.has_body = M3_TRUE;
  M3_TEST_OK(m3_tooltip_compute_content_size(&tooltip_style_rich,
                                             &tooltip_content, &tooltip_size));
  M3_TEST_ASSERT(m3_near(tooltip_size.width, 74.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(tooltip_size.height, 42.0f, 0.001f));
  tooltip_style_rich.min_height = 0.0f;
  tooltip_content.has_body = M3_FALSE;
  M3_TEST_OK(m3_tooltip_compute_content_size(&tooltip_style_rich,
                                             &tooltip_content, &tooltip_size));
  M3_TEST_ASSERT(m3_near(tooltip_size.width, 74.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(tooltip_size.height, 28.0f, 0.001f));
  tooltip_content.has_title = M3_FALSE;
  tooltip_content.has_body = M3_TRUE;
  M3_TEST_OK(m3_tooltip_compute_content_size(&tooltip_style_rich,
                                             &tooltip_content, &tooltip_size));
  M3_TEST_ASSERT(m3_near(tooltip_size.width, 54.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(tooltip_size.height, 26.0f, 0.001f));

  M3_TEST_EXPECT(m3_tooltip_compute_bounds(NULL, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_anchor.type = M3_TOOLTIP_ANCHOR_RECT;
  tooltip_anchor.rect.x = 0.0f;
  tooltip_anchor.rect.y = 0.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_START;
  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 100.0f;
  overlay.height = 100.0f;
  tooltip_size.width = -1.0f;
  tooltip_size.height = 10.0f;
  M3_TEST_EXPECT(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 M3_ERR_RANGE);
  tooltip_style.variant = 99;
  tooltip_size.width = 10.0f;
  M3_TEST_EXPECT(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_anchor.type = M3_TOOLTIP_ANCHOR_POINT;
  tooltip_anchor.point.x = 50.0f;
  tooltip_anchor.point.y = 50.0f;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  tooltip_anchor.type = 99;
  M3_TEST_EXPECT(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 M3_ERR_RANGE);
  tooltip_anchor.type = M3_TOOLTIP_ANCHOR_RECT;
  tooltip_anchor.rect.x = 0.0f;
  tooltip_anchor.rect.y = 0.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_placement.direction = 99;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_START;
  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 100.0f;
  overlay.height = 100.0f;
  tooltip_size.width = 10.0f;
  tooltip_size.height = 10.0f;
  M3_TEST_EXPECT(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 M3_ERR_RANGE);
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = 99;
  M3_TEST_EXPECT(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 M3_ERR_RANGE);
  tooltip_placement.align = M3_TOOLTIP_ALIGN_START;
  overlay.width = -1.0f;
  M3_TEST_EXPECT(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 M3_ERR_RANGE);
  overlay.width = 100.0f;
  tooltip_size.width = -1.0f;
  M3_TEST_EXPECT(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                           &tooltip_placement, &overlay,
                                           &tooltip_size, &bounds, &direction),
                 M3_ERR_RANGE);

  M3_TEST_OK(m3_tooltip_style_init_plain(&tooltip_style));
  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 100.0f;
  overlay.height = 100.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 10.0f;
  tooltip_anchor.type = M3_TOOLTIP_ANCHOR_RECT;
  tooltip_anchor.rect.x = 10.0f;
  tooltip_anchor.rect.y = 10.0f;
  tooltip_anchor.rect.width = 20.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_START;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_DOWN);
  M3_TEST_ASSERT(m3_near(bounds.x, 10.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 24.0f, 0.001f));

  tooltip_anchor.rect.x = 40.0f;
  tooltip_anchor.rect.y = 80.0f;
  tooltip_anchor.rect.width = 20.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_size.height = 20.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_CENTER;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_UP);
  M3_TEST_ASSERT(m3_near(bounds.x, 40.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 56.0f, 0.001f));

  overlay.height = 30.0f;
  tooltip_anchor.rect.x = 60.0f;
  tooltip_anchor.rect.y = 20.0f;
  tooltip_anchor.rect.width = 15.0f;
  tooltip_anchor.rect.height = 5.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 25.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_END;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_UP);
  M3_TEST_ASSERT(m3_near(bounds.x, 55.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 0.0f, 0.001f));

  overlay.height = 100.0f;
  tooltip_anchor.rect.x = 5.0f;
  tooltip_anchor.rect.y = 60.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 10.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_UP;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_START;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_UP);
  M3_TEST_ASSERT(m3_near(bounds.x, 5.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 46.0f, 0.001f));

  tooltip_anchor.rect.x = 30.0f;
  tooltip_anchor.rect.y = 10.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_size.height = 20.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_UP;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_CENTER;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_DOWN);
  M3_TEST_ASSERT(m3_near(bounds.x, 25.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 24.0f, 0.001f));

  overlay.height = 30.0f;
  tooltip_anchor.rect.x = 40.0f;
  tooltip_anchor.rect.y = 5.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 5.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 25.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_UP;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_END;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_DOWN);
  M3_TEST_ASSERT(m3_near(bounds.x, 30.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 5.0f, 0.001f));

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
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_RIGHT;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_START;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_RIGHT);
  M3_TEST_ASSERT(m3_near(bounds.x, 24.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 10.0f, 0.001f));

  tooltip_anchor.rect.x = 40.0f;
  tooltip_anchor.rect.y = 20.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_RIGHT;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_CENTER;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_LEFT);
  M3_TEST_ASSERT(m3_near(bounds.x, 16.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 20.0f, 0.001f));

  overlay.width = 30.0f;
  overlay.height = 30.0f;
  tooltip_anchor.rect.x = 20.0f;
  tooltip_anchor.rect.y = 10.0f;
  tooltip_anchor.rect.width = 5.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_size.width = 25.0f;
  tooltip_size.height = 10.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_RIGHT;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_END;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_LEFT);
  M3_TEST_ASSERT(m3_near(bounds.x, 0.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 10.0f, 0.001f));

  overlay.width = 100.0f;
  overlay.height = 60.0f;
  tooltip_anchor.rect.x = 70.0f;
  tooltip_anchor.rect.y = 5.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 10.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_LEFT;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_START;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_LEFT);
  M3_TEST_ASSERT(m3_near(bounds.x, 46.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 5.0f, 0.001f));

  tooltip_anchor.rect.x = 10.0f;
  tooltip_anchor.rect.y = 15.0f;
  tooltip_anchor.rect.width = 10.0f;
  tooltip_anchor.rect.height = 10.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_LEFT;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_CENTER;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_RIGHT);
  M3_TEST_ASSERT(m3_near(bounds.x, 24.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 15.0f, 0.001f));

  overlay.width = 30.0f;
  overlay.height = 20.0f;
  tooltip_anchor.rect.x = 5.0f;
  tooltip_anchor.rect.y = 18.0f;
  tooltip_anchor.rect.width = 5.0f;
  tooltip_anchor.rect.height = 5.0f;
  tooltip_size.width = 25.0f;
  tooltip_size.height = 10.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_LEFT;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_END;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(direction == M3_TOOLTIP_DIRECTION_RIGHT);
  M3_TEST_ASSERT(m3_near(bounds.x, 5.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.y, 10.0f, 0.001f));

  overlay.width = 15.0f;
  overlay.height = 8.0f;
  tooltip_anchor.rect.x = 0.0f;
  tooltip_anchor.rect.y = 0.0f;
  tooltip_anchor.rect.width = 1.0f;
  tooltip_anchor.rect.height = 1.0f;
  tooltip_size.width = 20.0f;
  tooltip_size.height = 10.0f;
  tooltip_placement.direction = M3_TOOLTIP_DIRECTION_DOWN;
  tooltip_placement.align = M3_TOOLTIP_ALIGN_START;
  M3_TEST_OK(m3_tooltip_compute_bounds(&tooltip_style, &tooltip_anchor,
                                       &tooltip_placement, &overlay,
                                       &tooltip_size, &bounds, &direction));
  M3_TEST_ASSERT(m3_near(bounds.width, 15.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(bounds.height, 8.0f, 0.001f));

  M3_TEST_EXPECT(m3_badge_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_badge_style_init(&badge_style));
  M3_TEST_ASSERT(
      m3_near(badge_style.min_size, M3_BADGE_DEFAULT_MIN_SIZE, 0.001f));
  M3_TEST_ASSERT(
      m3_near(badge_style.dot_diameter, M3_BADGE_DEFAULT_DOT_DIAMETER, 0.001f));
  M3_TEST_EXPECT(m3_extras_test_validate_badge_style(NULL, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);
  badge_style.min_size = -1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_badge_style(&badge_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_badge_style_init(&badge_style));
  badge_style.padding_x = -1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_badge_style(&badge_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_badge_style_init(&badge_style));
  badge_style.corner_radius = -1.0f;
  M3_TEST_EXPECT(m3_extras_test_validate_badge_style(&badge_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_badge_style_init(&badge_style));
  badge_style.text_style.size_px = 0;
  M3_TEST_EXPECT(m3_extras_test_validate_badge_style(&badge_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_badge_style_init(&badge_style));
  badge_style.background_color.r = -0.1f;
  M3_TEST_EXPECT(m3_extras_test_validate_badge_style(&badge_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_badge_style_init(&badge_style));
  M3_TEST_EXPECT(m3_extras_test_validate_badge_style(&badge_style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  badge_style.text_style.utf8_family = "Test";
  M3_TEST_OK(m3_extras_test_validate_badge_style(&badge_style, M3_TRUE));

  M3_TEST_EXPECT(m3_badge_placement_init_icon(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_badge_placement_init_icon(&badge_placement));
  M3_TEST_ASSERT(badge_placement.corner == M3_BADGE_CORNER_TOP_RIGHT);
  M3_TEST_EXPECT(m3_badge_placement_init_navigation(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_badge_placement_init_navigation(&badge_placement));
  M3_TEST_ASSERT(badge_placement.corner == M3_BADGE_CORNER_TOP_RIGHT);

  M3_TEST_EXPECT(m3_extras_test_validate_badge_content(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  badge_content.has_text = 2;
  M3_TEST_EXPECT(m3_extras_test_validate_badge_content(&badge_content),
                 M3_ERR_INVALID_ARGUMENT);
  badge_content.has_text = M3_TRUE;
  M3_TEST_OK(test_metrics_fill(&badge_content.text_metrics, -1.0f, 4.0f));
  M3_TEST_EXPECT(m3_extras_test_validate_badge_content(&badge_content),
                 M3_ERR_RANGE);
  M3_TEST_OK(test_metrics_fill(&badge_content.text_metrics, 4.0f, 2.0f));
  M3_TEST_OK(m3_extras_test_validate_badge_content(&badge_content));
  badge_content.has_text = M3_FALSE;
  M3_TEST_OK(m3_extras_test_validate_badge_content(&badge_content));

  M3_TEST_EXPECT(m3_badge_compute_size(NULL, &badge_content, &badge_size),
                 M3_ERR_INVALID_ARGUMENT);
  badge_style.min_size = -1.0f;
  M3_TEST_EXPECT(
      m3_badge_compute_size(&badge_style, &badge_content, &badge_size),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_badge_style_init(&badge_style));
  badge_content.has_text = 2;
  M3_TEST_EXPECT(
      m3_badge_compute_size(&badge_style, &badge_content, &badge_size),
      M3_ERR_INVALID_ARGUMENT);
  badge_content.has_text = M3_FALSE;
  M3_TEST_OK(m3_extras_test_set_badge_size_range_fail(M3_TRUE));
  M3_TEST_EXPECT(
      m3_badge_compute_size(&badge_style, &badge_content, &badge_size),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_badge_style_init(&badge_style));
  badge_content.has_text = M3_FALSE;
  M3_TEST_OK(m3_badge_compute_size(&badge_style, &badge_content, &badge_size));
  M3_TEST_ASSERT(m3_near(badge_size.width, badge_style.dot_diameter, 0.001f));
  M3_TEST_ASSERT(m3_near(badge_size.height, badge_style.dot_diameter, 0.001f));
  badge_content.has_text = M3_TRUE;
  M3_TEST_OK(test_metrics_fill(&badge_content.text_metrics, 2.0f, 20.0f));
  M3_TEST_OK(m3_badge_compute_size(&badge_style, &badge_content, &badge_size));
  M3_TEST_ASSERT(m3_near(badge_size.width, 24.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(badge_size.height, 24.0f, 0.001f));
  M3_TEST_OK(test_metrics_fill(&badge_content.text_metrics, 4.0f, 5.0f));
  M3_TEST_OK(m3_badge_compute_size(&badge_style, &badge_content, &badge_size));
  M3_TEST_ASSERT(m3_near(badge_size.height, badge_style.min_size, 0.001f));

  M3_TEST_EXPECT(m3_extras_test_validate_badge_placement(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  badge_placement.corner = 99;
  M3_TEST_EXPECT(m3_extras_test_validate_badge_placement(&badge_placement),
                 M3_ERR_RANGE);
  badge_placement.corner = M3_BADGE_CORNER_TOP_LEFT;
  M3_TEST_OK(m3_extras_test_validate_badge_placement(&badge_placement));

  M3_TEST_OK(m3_badge_style_init(&badge_style));
  badge_style.dot_diameter = 10.0f;
  badge_content.has_text = M3_FALSE;
  badge_placement.offset_x = 0.0f;
  badge_placement.offset_y = 0.0f;
  M3_TEST_EXPECT(m3_badge_compute_bounds(&badge_style, &badge_content, NULL,
                                         &badge_placement, &badge_bounds),
                 M3_ERR_INVALID_ARGUMENT);
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
  M3_TEST_EXPECT(m3_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                         &badge_placement, &badge_bounds),
                 M3_ERR_RANGE);
  badge_placement.corner = M3_BADGE_CORNER_TOP_LEFT;
  M3_TEST_EXPECT(m3_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                         &badge_placement, &badge_bounds),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_badge_style_init(&badge_style));
  badge_style.dot_diameter = 10.0f;
  badge_content.has_text = M3_FALSE;
  badge_placement.corner = 99;
  M3_TEST_EXPECT(m3_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                         &badge_placement, &badge_bounds),
                 M3_ERR_RANGE);
  badge_placement.corner = M3_BADGE_CORNER_TOP_LEFT;
  M3_TEST_OK(m3_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                     &badge_placement, &badge_bounds));
  M3_TEST_ASSERT(m3_near(badge_bounds.x, 5.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(badge_bounds.y, 5.0f, 0.001f));
  badge_placement.corner = M3_BADGE_CORNER_TOP_RIGHT;
  M3_TEST_OK(m3_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                     &badge_placement, &badge_bounds));
  M3_TEST_ASSERT(m3_near(badge_bounds.x, 25.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(badge_bounds.y, 5.0f, 0.001f));
  badge_placement.corner = M3_BADGE_CORNER_BOTTOM_RIGHT;
  M3_TEST_OK(m3_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                     &badge_placement, &badge_bounds));
  M3_TEST_ASSERT(m3_near(badge_bounds.x, 25.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(badge_bounds.y, 25.0f, 0.001f));
  badge_placement.corner = M3_BADGE_CORNER_BOTTOM_LEFT;
  M3_TEST_OK(m3_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                     &badge_placement, &badge_bounds));
  M3_TEST_ASSERT(m3_near(badge_bounds.x, 5.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(badge_bounds.y, 25.0f, 0.001f));

  bounds.width = -1.0f;
  M3_TEST_EXPECT(m3_badge_compute_bounds(&badge_style, &badge_content, &bounds,
                                         &badge_placement, &badge_bounds),
                 M3_ERR_RANGE);

  return 0;
}
