#include "m3/m3_navigation.h"

#include <string.h>

#ifdef M3_TESTING
#define M3_NAV_TEST_FAIL_NONE 0u
#define M3_NAV_TEST_FAIL_RESOLVE_MODE 1u
#define M3_NAV_TEST_FAIL_LAYOUT_ITEM_WIDTH_NEGATIVE 2u
#define M3_NAV_TEST_FAIL_LAYOUT_MODE_INVALID 3u
#define M3_NAV_TEST_FAIL_INDICATOR_RECT_NEGATIVE 4u
#define M3_NAV_TEST_FAIL_ITEM_RECT_NEGATIVE 5u
#define M3_NAV_TEST_FAIL_STYLE_INIT 6u
#define M3_NAV_TEST_FAIL_HIT_TEST_POS_NEGATIVE 7u

static m3_u32 g_m3_nav_test_fail_point = M3_NAV_TEST_FAIL_NONE;

static M3Bool m3_nav_test_consume_fail(m3_u32 point)
{
    if (g_m3_nav_test_fail_point != point) {
        return M3_FALSE;
    }
    g_m3_nav_test_fail_point = M3_NAV_TEST_FAIL_NONE;
    return M3_TRUE;
}

int M3_CALL m3_navigation_test_set_fail_point(m3_u32 point)
{
    g_m3_nav_test_fail_point = point;
    return M3_OK;
}

int M3_CALL m3_navigation_test_clear_fail_points(void)
{
    g_m3_nav_test_fail_point = M3_NAV_TEST_FAIL_NONE;
    return M3_OK;
}
#endif

typedef struct M3NavigationLayout {
    m3_u32 mode;
    M3Scalar start_x;
    M3Scalar start_y;
    M3Scalar item_width;
    M3Scalar item_height;
    M3Scalar spacing;
    M3Scalar content_width;
    M3Scalar content_height;
} M3NavigationLayout;

static int m3_navigation_validate_color(const M3Color *color)
{
    if (color == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!(color->r >= 0.0f && color->r <= 1.0f)) {
        return M3_ERR_RANGE;
    }
    if (!(color->g >= 0.0f && color->g <= 1.0f)) {
        return M3_ERR_RANGE;
    }
    if (!(color->b >= 0.0f && color->b <= 1.0f)) {
        return M3_ERR_RANGE;
    }
    if (!(color->a >= 0.0f && color->a <= 1.0f)) {
        return M3_ERR_RANGE;
    }
    return M3_OK;
}

static int m3_navigation_validate_edges(const M3LayoutEdges *edges)
{
    if (edges == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f || edges->bottom < 0.0f) {
        return M3_ERR_RANGE;
    }
    return M3_OK;
}

static int m3_navigation_validate_text_style(const M3TextStyle *style, M3Bool require_family)
{
    int rc;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (require_family == M3_TRUE && style->utf8_family == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (style->size_px <= 0) {
        return M3_ERR_RANGE;
    }
    if (style->weight < 100 || style->weight > 900) {
        return M3_ERR_RANGE;
    }
    if (style->italic != M3_FALSE && style->italic != M3_TRUE) {
        return M3_ERR_RANGE;
    }

    rc = m3_navigation_validate_color(&style->color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

static int m3_navigation_validate_style(const M3NavigationStyle *style, M3Bool require_family)
{
    int rc;
    M3Scalar padding_width;
    M3Scalar padding_height;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    switch (style->mode) {
        case M3_NAV_MODE_AUTO:
        case M3_NAV_MODE_BAR:
        case M3_NAV_MODE_RAIL:
        case M3_NAV_MODE_DRAWER:
            break;
        default:
            return M3_ERR_RANGE;
    }

    if (style->bar_height <= 0.0f || style->rail_width <= 0.0f || style->drawer_width <= 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->item_height <= 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->item_min_width < 0.0f || style->item_spacing < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->indicator_thickness < 0.0f || style->indicator_corner < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->breakpoint_rail < 0.0f || style->breakpoint_drawer < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->breakpoint_drawer < style->breakpoint_rail) {
        return M3_ERR_RANGE;
    }

    rc = m3_navigation_validate_edges(&style->padding);
    if (rc != M3_OK) {
        return rc;
    }

    padding_width = style->padding.left + style->padding.right;
    padding_height = style->padding.top + style->padding.bottom;
    if (style->bar_height < padding_height) {
        return M3_ERR_RANGE;
    }
    if (style->rail_width < padding_width || style->drawer_width < padding_width) {
        return M3_ERR_RANGE;
    }

    rc = m3_navigation_validate_text_style(&style->text_style, require_family);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_navigation_validate_color(&style->selected_text_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_navigation_validate_color(&style->indicator_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_navigation_validate_color(&style->background_color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

static int m3_navigation_validate_backend(const M3TextBackend *backend)
{
    if (backend == NULL || backend->vtable == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_OK;
}

static int m3_navigation_validate_items(const M3NavigationItem *items, m3_usize count)
{
    m3_usize i;

    if (count == 0) {
        return M3_OK;
    }
    if (items == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    for (i = 0; i < count; ++i) {
        if (items[i].utf8_label == NULL && items[i].utf8_len != 0) {
            return M3_ERR_INVALID_ARGUMENT;
        }
    }
    return M3_OK;
}

static int m3_navigation_validate_measure_spec(M3MeasureSpec spec)
{
    if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY && spec.mode != M3_MEASURE_AT_MOST) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
        return M3_ERR_RANGE;
    }
    return M3_OK;
}

static int m3_navigation_validate_rect(const M3Rect *rect)
{
    if (rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (rect->width < 0.0f || rect->height < 0.0f) {
        return M3_ERR_RANGE;
    }
    return M3_OK;
}

static int m3_navigation_resolve_mode(const M3NavigationStyle *style, M3Scalar width, m3_u32 *out_mode)
{
#ifdef M3_TESTING
    if (m3_nav_test_consume_fail(M3_NAV_TEST_FAIL_RESOLVE_MODE)) {
        return M3_ERR_UNKNOWN;
    }
#endif
    if (style == NULL || out_mode == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (width < 0.0f) {
        return M3_ERR_RANGE;
    }

    if (style->mode != M3_NAV_MODE_AUTO) {
        *out_mode = style->mode;
        return M3_OK;
    }

    if (width < style->breakpoint_rail) {
        *out_mode = M3_NAV_MODE_BAR;
    } else if (width < style->breakpoint_drawer) {
        *out_mode = M3_NAV_MODE_RAIL;
    } else {
        *out_mode = M3_NAV_MODE_DRAWER;
    }
    return M3_OK;
}

static int m3_navigation_measure_content(const M3NavigationStyle *style, m3_u32 mode, m3_usize item_count,
    M3Scalar *out_width, M3Scalar *out_height)
{
    M3Scalar width;
    M3Scalar height;
    M3Scalar spacing;

    if (style == NULL || out_width == NULL || out_height == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    spacing = style->item_spacing;
    if (spacing < 0.0f) {
        return M3_ERR_RANGE;
    }

    if (mode == M3_NAV_MODE_BAR) {
        width = style->padding.left + style->padding.right;
        if (item_count > 0) {
            width += style->item_min_width * (M3Scalar)item_count;
            width += spacing * (M3Scalar)(item_count - 1);
        }
        height = style->bar_height;
    } else if (mode == M3_NAV_MODE_RAIL || mode == M3_NAV_MODE_DRAWER) {
        width = (mode == M3_NAV_MODE_RAIL) ? style->rail_width : style->drawer_width;
        height = style->padding.top + style->padding.bottom;
        if (item_count > 0) {
            height += style->item_height * (M3Scalar)item_count;
            height += spacing * (M3Scalar)(item_count - 1);
        }
    } else {
        return M3_ERR_RANGE;
    }

    if (width < 0.0f || height < 0.0f) {
        return M3_ERR_RANGE;
    }

    *out_width = width;
    *out_height = height;
    return M3_OK;
}

static int m3_navigation_compute_layout(const M3Navigation *nav, M3NavigationLayout *out_layout)
{
    M3NavigationStyle style;
    M3Rect bounds;
    M3Scalar available_width;
    M3Scalar available_height;
    M3Scalar spacing;
    M3Scalar total_spacing;
    m3_u32 mode;
    int rc;

    if (nav == NULL || out_layout == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    style = nav->style;
    bounds = nav->bounds;

    rc = m3_navigation_validate_style(&style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_navigation_validate_rect(&bounds);
    if (rc != M3_OK) {
        return rc;
    }

    available_width = bounds.width - style.padding.left - style.padding.right;
    available_height = bounds.height - style.padding.top - style.padding.bottom;
    if (available_width < 0.0f || available_height < 0.0f) {
        return M3_ERR_RANGE;
    }

    rc = m3_navigation_resolve_mode(&style, bounds.width, &mode);
    if (rc != M3_OK) {
        return rc;
    }
#ifdef M3_TESTING
    if (m3_nav_test_consume_fail(M3_NAV_TEST_FAIL_LAYOUT_MODE_INVALID)) {
        mode = 99u;
    }
#endif

    spacing = style.item_spacing;
    if (spacing < 0.0f) {
        return M3_ERR_RANGE;
    }

    memset(out_layout, 0, sizeof(*out_layout));
    out_layout->mode = mode;
    out_layout->spacing = spacing;
    out_layout->start_x = bounds.x + style.padding.left;
    out_layout->start_y = bounds.y + style.padding.top;

    if (mode == M3_NAV_MODE_BAR) {
        out_layout->item_height = style.item_height;
        if (out_layout->item_height <= 0.0f) {
            return M3_ERR_RANGE;
        }
        if (nav->item_count > 0 && available_height <= 0.0f) {
            return M3_ERR_RANGE;
        }
        if (out_layout->item_height > available_height && available_height > 0.0f) {
            out_layout->item_height = available_height;
        }
        if (nav->item_count > 0) {
            total_spacing = spacing * (M3Scalar)(nav->item_count - 1);
            if (available_width < total_spacing) {
                return M3_ERR_RANGE;
            }
            out_layout->item_width = (available_width - total_spacing) / (M3Scalar)nav->item_count;
#ifdef M3_TESTING
            if (m3_nav_test_consume_fail(M3_NAV_TEST_FAIL_LAYOUT_ITEM_WIDTH_NEGATIVE)) {
                out_layout->item_width = -1.0f;
            }
#endif
            if (out_layout->item_width < 0.0f) {
                return M3_ERR_RANGE;
            }
        } else {
            out_layout->item_width = 0.0f;
        }
        out_layout->content_width = available_width;
        out_layout->content_height = out_layout->item_height;
        out_layout->start_y = bounds.y + style.padding.top + (available_height - out_layout->item_height) * 0.5f;
    } else if (mode == M3_NAV_MODE_RAIL || mode == M3_NAV_MODE_DRAWER) {
        out_layout->item_height = style.item_height;
        if (out_layout->item_height <= 0.0f) {
            return M3_ERR_RANGE;
        }
        if (nav->item_count > 0 && available_width <= 0.0f) {
            return M3_ERR_RANGE;
        }
        out_layout->item_width = available_width;
        if (nav->item_count > 0) {
            out_layout->content_height = out_layout->item_height * (M3Scalar)nav->item_count;
            out_layout->content_height += spacing * (M3Scalar)(nav->item_count - 1);
        } else {
            out_layout->content_height = 0.0f;
        }
        out_layout->content_width = out_layout->item_width;
    } else {
        return M3_ERR_RANGE;
    }

    return M3_OK;
}

static int m3_navigation_hit_test(const M3Navigation *nav, const M3NavigationLayout *layout, m3_i32 x, m3_i32 y,
    m3_usize *out_index)
{
    M3Scalar fx;
    M3Scalar fy;
    M3Scalar stride;
    M3Scalar pos;
    m3_usize index;

    if (nav == NULL || layout == NULL || out_index == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_index = M3_NAV_INVALID_INDEX;
    if (nav->item_count == 0) {
        return M3_OK;
    }

    fx = (M3Scalar)x;
    fy = (M3Scalar)y;

    if (layout->mode == M3_NAV_MODE_BAR) {
        if (fx < layout->start_x || fx > layout->start_x + layout->content_width) {
            return M3_OK;
        }
        if (fy < layout->start_y || fy > layout->start_y + layout->item_height) {
            return M3_OK;
        }
        stride = layout->item_width + layout->spacing;
        if (stride <= 0.0f) {
            return M3_ERR_RANGE;
        }
        pos = fx - layout->start_x;
#ifdef M3_TESTING
        if (m3_nav_test_consume_fail(M3_NAV_TEST_FAIL_HIT_TEST_POS_NEGATIVE)) {
            pos = -1.0f;
        }
#endif
        if (pos < 0.0f) {
            return M3_OK;
        }
        index = (m3_usize)(pos / stride);
        if (index >= nav->item_count) {
            return M3_OK;
        }
        if (pos > (M3Scalar)index * stride + layout->item_width) {
            return M3_OK;
        }
        *out_index = index;
        return M3_OK;
    }

    if (fx < layout->start_x || fx > layout->start_x + layout->item_width) {
        return M3_OK;
    }
    if (fy < layout->start_y || fy > layout->start_y + layout->content_height) {
        return M3_OK;
    }
    stride = layout->item_height + layout->spacing;
    if (stride <= 0.0f) {
        return M3_ERR_RANGE;
    }
    pos = fy - layout->start_y;
#ifdef M3_TESTING
    if (m3_nav_test_consume_fail(M3_NAV_TEST_FAIL_HIT_TEST_POS_NEGATIVE)) {
        pos = -1.0f;
    }
#endif
    if (pos < 0.0f) {
        return M3_OK;
    }
    index = (m3_usize)(pos / stride);
    if (index >= nav->item_count) {
        return M3_OK;
    }
    if (pos > (M3Scalar)index * stride + layout->item_height) {
        return M3_OK;
    }
    *out_index = index;
    return M3_OK;
}

static int m3_navigation_widget_measure(void *widget, M3MeasureSpec width, M3MeasureSpec height, M3Size *out_size)
{
    M3Navigation *nav;
    M3Scalar content_width;
    M3Scalar content_height;
    m3_u32 mode;
    int rc;

    if (widget == NULL || out_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_navigation_validate_measure_spec(width);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_navigation_validate_measure_spec(height);
    if (rc != M3_OK) {
        return rc;
    }

    nav = (M3Navigation *)widget;
    rc = m3_navigation_validate_style(&nav->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }

    if (nav->style.mode == M3_NAV_MODE_AUTO) {
        if (width.mode == M3_MEASURE_UNSPECIFIED) {
            mode = M3_NAV_MODE_BAR;
        } else {
            rc = m3_navigation_resolve_mode(&nav->style, width.size, &mode);
            if (rc != M3_OK) {
                return rc;
            }
        }
    } else {
        mode = nav->style.mode;
    }

    rc = m3_navigation_measure_content(&nav->style, mode, nav->item_count, &content_width, &content_height);
    if (rc != M3_OK) {
        return rc;
    }

    if (width.mode == M3_MEASURE_EXACTLY) {
        out_size->width = width.size;
    } else if (width.mode == M3_MEASURE_AT_MOST) {
        out_size->width = (content_width > width.size) ? width.size : content_width;
    } else {
        out_size->width = content_width;
    }

    if (height.mode == M3_MEASURE_EXACTLY) {
        out_size->height = height.size;
    } else if (height.mode == M3_MEASURE_AT_MOST) {
        out_size->height = (content_height > height.size) ? height.size : content_height;
    } else {
        out_size->height = content_height;
    }

    return M3_OK;
}

static int m3_navigation_widget_layout(void *widget, M3Rect bounds)
{
    M3Navigation *nav;
    M3NavigationLayout layout;
    int rc;

    if (widget == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_navigation_validate_rect(&bounds);
    if (rc != M3_OK) {
        return rc;
    }

    nav = (M3Navigation *)widget;
    nav->bounds = bounds;

    rc = m3_navigation_compute_layout(nav, &layout);
    if (rc != M3_OK) {
        return rc;
    }
    nav->active_mode = layout.mode;
    return M3_OK;
}

static int m3_navigation_widget_paint(void *widget, M3PaintContext *ctx)
{
    M3Navigation *nav;
    M3NavigationLayout layout;
    M3Rect item_rect;
    M3Rect indicator_rect;
    M3TextMetrics metrics;
    M3Scalar text_x;
    M3Scalar text_y;
    M3Scalar indicator_thickness;
    M3Color text_color;
    m3_usize i;
    int rc;

    if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (ctx->gfx->vtable == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (ctx->gfx->vtable->draw_rect == NULL) {
        return M3_ERR_UNSUPPORTED;
    }
    if (ctx->gfx->text_vtable == NULL || ctx->gfx->text_vtable->draw_text == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    nav = (M3Navigation *)widget;
    rc = m3_navigation_validate_style(&nav->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_navigation_validate_rect(&nav->bounds);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_navigation_validate_items(nav->items, nav->item_count);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_navigation_compute_layout(nav, &layout);
    if (rc != M3_OK) {
        return rc;
    }

    if (nav->style.background_color.a > 0.0f) {
        rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &nav->bounds, nav->style.background_color, 0.0f);
        if (rc != M3_OK) {
            return rc;
        }
    }

    indicator_thickness = nav->style.indicator_thickness;
    if (indicator_thickness < 0.0f) {
        return M3_ERR_RANGE;
    }

    if (nav->selected_index != M3_NAV_INVALID_INDEX && nav->selected_index < nav->item_count && indicator_thickness > 0.0f) {
        item_rect.x = layout.start_x;
        item_rect.y = layout.start_y;
        item_rect.width = layout.item_width;
        item_rect.height = layout.item_height;
        if (layout.mode == M3_NAV_MODE_BAR) {
            item_rect.x += (layout.item_width + layout.spacing) * (M3Scalar)nav->selected_index;
        } else {
            item_rect.y += (layout.item_height + layout.spacing) * (M3Scalar)nav->selected_index;
        }

        indicator_rect = item_rect;
        if (layout.mode == M3_NAV_MODE_BAR) {
            indicator_rect.y = item_rect.y + item_rect.height - indicator_thickness;
            indicator_rect.height = indicator_thickness;
        } else {
            indicator_rect.width = indicator_thickness;
        }
#ifdef M3_TESTING
        if (m3_nav_test_consume_fail(M3_NAV_TEST_FAIL_INDICATOR_RECT_NEGATIVE)) {
            indicator_rect.width = -1.0f;
        }
#endif

        if (indicator_rect.width < 0.0f || indicator_rect.height < 0.0f) {
            return M3_ERR_RANGE;
        }

        rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &indicator_rect, nav->style.indicator_color,
            nav->style.indicator_corner);
        if (rc != M3_OK) {
            return rc;
        }
    }

    if (nav->item_count == 0) {
        return M3_OK;
    }

    if (nav->text_backend.vtable == NULL || nav->text_backend.vtable->measure_text == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    for (i = 0; i < nav->item_count; ++i) {
        item_rect.x = layout.start_x;
        item_rect.y = layout.start_y;
        item_rect.width = layout.item_width;
        item_rect.height = layout.item_height;
        if (layout.mode == M3_NAV_MODE_BAR) {
            item_rect.x += (layout.item_width + layout.spacing) * (M3Scalar)i;
        } else {
            item_rect.y += (layout.item_height + layout.spacing) * (M3Scalar)i;
        }

#ifdef M3_TESTING
        if (m3_nav_test_consume_fail(M3_NAV_TEST_FAIL_ITEM_RECT_NEGATIVE)) {
            item_rect.width = -1.0f;
        }
#endif
        if (item_rect.width < 0.0f || item_rect.height < 0.0f) {
            return M3_ERR_RANGE;
        }

        if (nav->items[i].utf8_len == 0) {
            continue;
        }

        rc = m3_text_measure_utf8(&nav->text_backend, nav->font, nav->items[i].utf8_label, nav->items[i].utf8_len, &metrics);
        if (rc != M3_OK) {
            return rc;
        }

        text_x = item_rect.x + (item_rect.width - metrics.width) * 0.5f;
        text_y = item_rect.y + (item_rect.height - metrics.height) * 0.5f + metrics.baseline;

        text_color = nav->style.text_style.color;
        if (i == nav->selected_index) {
            text_color = nav->style.selected_text_color;
        }

        rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, nav->font, nav->items[i].utf8_label, nav->items[i].utf8_len,
            text_x, text_y, text_color);
        if (rc != M3_OK) {
            return rc;
        }
    }

    return M3_OK;
}

static int m3_navigation_widget_event(void *widget, const M3InputEvent *event, M3Bool *out_handled)
{
    M3Navigation *nav;
    M3NavigationLayout layout;
    m3_usize index;
    m3_usize previous;
    int rc;

    if (widget == NULL || event == NULL || out_handled == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_handled = M3_FALSE;
    nav = (M3Navigation *)widget;

    if (nav->widget.flags & M3_WIDGET_FLAG_DISABLED) {
        return M3_OK;
    }

    if (event->type != M3_INPUT_POINTER_DOWN && event->type != M3_INPUT_POINTER_UP) {
        return M3_OK;
    }

    rc = m3_navigation_compute_layout(nav, &layout);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_navigation_hit_test(nav, &layout, event->data.pointer.x, event->data.pointer.y, &index);
    if (rc != M3_OK) {
        return rc;
    }

    if (event->type == M3_INPUT_POINTER_DOWN) {
        if (nav->pressed_index != M3_NAV_INVALID_INDEX) {
            return M3_ERR_STATE;
        }
        if (index == M3_NAV_INVALID_INDEX) {
            return M3_OK;
        }
        nav->pressed_index = index;
        *out_handled = M3_TRUE;
        return M3_OK;
    }

    if (nav->pressed_index == M3_NAV_INVALID_INDEX) {
        return M3_OK;
    }

    if (index == nav->pressed_index) {
        previous = nav->selected_index;
        nav->selected_index = index;
        if (nav->on_select != NULL) {
            rc = nav->on_select(nav->on_select_ctx, nav, index);
            if (rc != M3_OK) {
                nav->selected_index = previous;
                nav->pressed_index = M3_NAV_INVALID_INDEX;
                return rc;
            }
        }
    }

    nav->pressed_index = M3_NAV_INVALID_INDEX;
    *out_handled = M3_TRUE;
    return M3_OK;
}

static int m3_navigation_widget_get_semantics(void *widget, M3Semantics *out_semantics)
{
    M3Navigation *nav;

    if (widget == NULL || out_semantics == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    nav = (M3Navigation *)widget;
    out_semantics->role = M3_SEMANTIC_NONE;
    out_semantics->flags = 0;
    if (nav->widget.flags & M3_WIDGET_FLAG_DISABLED) {
        out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
    }
    if (nav->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
        out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
    }
    out_semantics->utf8_label = NULL;
    out_semantics->utf8_hint = NULL;
    out_semantics->utf8_value = NULL;
    return M3_OK;
}

static int m3_navigation_widget_destroy(void *widget)
{
    M3Navigation *nav;
    int rc;

    if (widget == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    nav = (M3Navigation *)widget;
    rc = M3_OK;
    if (nav->owns_font == M3_TRUE && (nav->font.id != 0u || nav->font.generation != 0u)) {
        if (nav->text_backend.vtable != NULL && nav->text_backend.vtable->destroy_font != NULL) {
            rc = nav->text_backend.vtable->destroy_font(nav->text_backend.ctx, nav->font);
        } else {
            rc = M3_ERR_UNSUPPORTED;
        }
    }

    nav->font.id = 0u;
    nav->font.generation = 0u;
    nav->items = NULL;
    nav->item_count = 0;
    nav->selected_index = M3_NAV_INVALID_INDEX;
    nav->pressed_index = M3_NAV_INVALID_INDEX;
    nav->active_mode = M3_NAV_MODE_AUTO;
    nav->bounds.x = 0.0f;
    nav->bounds.y = 0.0f;
    nav->bounds.width = 0.0f;
    nav->bounds.height = 0.0f;
    nav->text_backend.ctx = NULL;
    nav->text_backend.vtable = NULL;
    nav->widget.ctx = NULL;
    nav->widget.vtable = NULL;
    nav->on_select = NULL;
    nav->on_select_ctx = NULL;
    nav->owns_font = M3_FALSE;
    return rc;
}

static const M3WidgetVTable g_m3_navigation_widget_vtable = {
    m3_navigation_widget_measure,
    m3_navigation_widget_layout,
    m3_navigation_widget_paint,
    m3_navigation_widget_event,
    m3_navigation_widget_get_semantics,
    m3_navigation_widget_destroy
};

int M3_CALL m3_navigation_style_init(M3NavigationStyle *style)
{
    int rc;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    memset(style, 0, sizeof(*style));

    rc = m3_text_style_init(&style->text_style);
#ifdef M3_TESTING
    if (m3_nav_test_consume_fail(M3_NAV_TEST_FAIL_STYLE_INIT)) {
        rc = M3_ERR_UNKNOWN;
    }
#endif
    if (rc != M3_OK) {
        return rc;
    }

    style->mode = M3_NAV_MODE_AUTO;
    style->bar_height = M3_NAV_DEFAULT_BAR_HEIGHT;
    style->rail_width = M3_NAV_DEFAULT_RAIL_WIDTH;
    style->drawer_width = M3_NAV_DEFAULT_DRAWER_WIDTH;
    style->item_height = M3_NAV_DEFAULT_ITEM_HEIGHT;
    style->item_min_width = M3_NAV_DEFAULT_ITEM_MIN_WIDTH;
    style->item_spacing = M3_NAV_DEFAULT_ITEM_SPACING;
    style->indicator_thickness = M3_NAV_DEFAULT_INDICATOR_THICKNESS;
    style->indicator_corner = M3_NAV_DEFAULT_INDICATOR_CORNER;
    style->breakpoint_rail = M3_NAV_DEFAULT_BREAKPOINT_RAIL;
    style->breakpoint_drawer = M3_NAV_DEFAULT_BREAKPOINT_DRAWER;
    style->background_color.r = 0.0f;
    style->background_color.g = 0.0f;
    style->background_color.b = 0.0f;
    style->background_color.a = 0.0f;
    style->selected_text_color.r = 0.0f;
    style->selected_text_color.g = 0.0f;
    style->selected_text_color.b = 0.0f;
    style->selected_text_color.a = 1.0f;
    style->indicator_color.r = 0.0f;
    style->indicator_color.g = 0.0f;
    style->indicator_color.b = 0.0f;
    style->indicator_color.a = 1.0f;
    return M3_OK;
}

int M3_CALL m3_navigation_init(M3Navigation *nav, const M3TextBackend *backend, const M3NavigationStyle *style,
    const M3NavigationItem *items, m3_usize item_count, m3_usize selected_index)
{
    m3_u32 mode;
    int rc;

    if (nav == NULL || backend == NULL || style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_navigation_validate_backend(backend);
    if (rc != M3_OK) {
        return rc;
    }
    if (backend->vtable->create_font == NULL || backend->vtable->destroy_font == NULL
        || backend->vtable->measure_text == NULL || backend->vtable->draw_text == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    rc = m3_navigation_validate_style(style, M3_TRUE);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_navigation_validate_items(items, item_count);
    if (rc != M3_OK) {
        return rc;
    }

    if (selected_index != M3_NAV_INVALID_INDEX && selected_index >= item_count) {
        return M3_ERR_RANGE;
    }

    rc = m3_navigation_resolve_mode(style, 0.0f, &mode);
    if (rc != M3_OK) {
        return rc;
    }

    memset(nav, 0, sizeof(*nav));
    nav->text_backend = *backend;
    nav->style = *style;
    nav->items = items;
    nav->item_count = item_count;
    nav->selected_index = selected_index;
    nav->pressed_index = M3_NAV_INVALID_INDEX;
    nav->active_mode = mode;
    nav->bounds.x = 0.0f;
    nav->bounds.y = 0.0f;
    nav->bounds.width = 0.0f;
    nav->bounds.height = 0.0f;

    rc = m3_text_font_create(backend, &style->text_style, &nav->font);
    if (rc != M3_OK) {
        return rc;
    }

    nav->owns_font = M3_TRUE;
    nav->widget.ctx = nav;
    nav->widget.vtable = &g_m3_navigation_widget_vtable;
    nav->widget.handle.id = 0u;
    nav->widget.handle.generation = 0u;
    nav->widget.flags = M3_WIDGET_FLAG_FOCUSABLE;
    return M3_OK;
}

int M3_CALL m3_navigation_set_items(M3Navigation *nav, const M3NavigationItem *items, m3_usize item_count)
{
    int rc;

    if (nav == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_navigation_validate_items(items, item_count);
    if (rc != M3_OK) {
        return rc;
    }

    nav->items = items;
    nav->item_count = item_count;
    if (nav->selected_index != M3_NAV_INVALID_INDEX && nav->selected_index >= item_count) {
        nav->selected_index = M3_NAV_INVALID_INDEX;
    }
    if (nav->pressed_index != M3_NAV_INVALID_INDEX && nav->pressed_index >= item_count) {
        nav->pressed_index = M3_NAV_INVALID_INDEX;
    }
    return M3_OK;
}

int M3_CALL m3_navigation_set_style(M3Navigation *nav, const M3NavigationStyle *style)
{
    M3Handle new_font;
    int rc;

    if (nav == NULL || style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_navigation_validate_style(style, M3_TRUE);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_text_font_create(&nav->text_backend, &style->text_style, &new_font);
    if (rc != M3_OK) {
        return rc;
    }

    if (nav->owns_font == M3_TRUE) {
        rc = m3_text_font_destroy(&nav->text_backend, nav->font);
        if (rc != M3_OK) {
            m3_text_font_destroy(&nav->text_backend, new_font);
            return rc;
        }
    }

    nav->style = *style;
    nav->font = new_font;
    nav->owns_font = M3_TRUE;
    return M3_OK;
}

int M3_CALL m3_navigation_set_selected(M3Navigation *nav, m3_usize selected_index)
{
    if (nav == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (selected_index != M3_NAV_INVALID_INDEX && selected_index >= nav->item_count) {
        return M3_ERR_RANGE;
    }
    nav->selected_index = selected_index;
    return M3_OK;
}

int M3_CALL m3_navigation_get_selected(const M3Navigation *nav, m3_usize *out_selected)
{
    if (nav == NULL || out_selected == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_selected = nav->selected_index;
    return M3_OK;
}

int M3_CALL m3_navigation_set_on_select(M3Navigation *nav, M3NavigationOnSelect on_select, void *ctx)
{
    if (nav == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    nav->on_select = on_select;
    nav->on_select_ctx = ctx;
    return M3_OK;
}

int M3_CALL m3_navigation_get_mode(const M3Navigation *nav, m3_u32 *out_mode)
{
    int rc;

    if (nav == NULL || out_mode == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_navigation_validate_rect(&nav->bounds);
    if (rc != M3_OK) {
        return rc;
    }

    return m3_navigation_resolve_mode(&nav->style, nav->bounds.width, out_mode);
}

#ifdef M3_TESTING
int M3_CALL m3_navigation_test_validate_color(const M3Color *color)
{
    return m3_navigation_validate_color(color);
}

int M3_CALL m3_navigation_test_validate_edges(const M3LayoutEdges *edges)
{
    return m3_navigation_validate_edges(edges);
}

int M3_CALL m3_navigation_test_validate_text_style(const M3TextStyle *style, M3Bool require_family)
{
    return m3_navigation_validate_text_style(style, require_family);
}

int M3_CALL m3_navigation_test_validate_style(const M3NavigationStyle *style, M3Bool require_family)
{
    return m3_navigation_validate_style(style, require_family);
}

int M3_CALL m3_navigation_test_validate_items(const M3NavigationItem *items, m3_usize count)
{
    return m3_navigation_validate_items(items, count);
}

int M3_CALL m3_navigation_test_validate_measure_spec(M3MeasureSpec spec)
{
    return m3_navigation_validate_measure_spec(spec);
}

int M3_CALL m3_navigation_test_validate_rect(const M3Rect *rect)
{
    return m3_navigation_validate_rect(rect);
}

int M3_CALL m3_navigation_test_resolve_mode(const M3NavigationStyle *style, M3Scalar width, m3_u32 *out_mode)
{
    return m3_navigation_resolve_mode(style, width, out_mode);
}

int M3_CALL m3_navigation_test_measure_content(const M3NavigationStyle *style, m3_u32 mode, m3_usize item_count,
    M3Scalar *out_width, M3Scalar *out_height)
{
    return m3_navigation_measure_content(style, mode, item_count, out_width, out_height);
}

int M3_CALL m3_navigation_test_compute_layout(const M3Navigation *nav, M3NavigationLayoutTest *out_layout)
{
    M3NavigationLayout layout;
    M3NavigationLayout *layout_ptr;
    int rc;

    layout_ptr = (out_layout == NULL) ? NULL : &layout;
    rc = m3_navigation_compute_layout(nav, layout_ptr);
    if (rc != M3_OK) {
        return rc;
    }
    if (out_layout == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    out_layout->mode = layout.mode;
    out_layout->start_x = layout.start_x;
    out_layout->start_y = layout.start_y;
    out_layout->item_width = layout.item_width;
    out_layout->item_height = layout.item_height;
    out_layout->spacing = layout.spacing;
    out_layout->content_width = layout.content_width;
    out_layout->content_height = layout.content_height;
    return M3_OK;
}

int M3_CALL m3_navigation_test_hit_test(const M3Navigation *nav, const M3NavigationLayoutTest *layout, m3_i32 x, m3_i32 y,
    m3_usize *out_index)
{
    M3NavigationLayout internal;
    const M3NavigationLayout *layout_ptr;

    layout_ptr = NULL;
    if (layout != NULL) {
        internal.mode = layout->mode;
        internal.start_x = layout->start_x;
        internal.start_y = layout->start_y;
        internal.item_width = layout->item_width;
        internal.item_height = layout->item_height;
        internal.spacing = layout->spacing;
        internal.content_width = layout->content_width;
        internal.content_height = layout->content_height;
        layout_ptr = &internal;
    }

    return m3_navigation_hit_test(nav, layout_ptr, x, y, out_index);
}
#endif
