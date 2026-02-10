#include "m3/m3_dialogs.h"

static int m3_dialog_validate_color(const M3Color *color)
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

static int m3_dialog_validate_text_style(const M3TextStyle *style, M3Bool require_family)
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

    rc = m3_dialog_validate_color(&style->color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

static int m3_dialog_validate_edges(const M3LayoutEdges *edges)
{
    if (edges == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f || edges->bottom < 0.0f) {
        return M3_ERR_RANGE;
    }
    return M3_OK;
}

static int m3_dialog_validate_measure_spec(M3MeasureSpec spec)
{
    if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY && spec.mode != M3_MEASURE_AT_MOST) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
        return M3_ERR_RANGE;
    }
    return M3_OK;
}

static int m3_dialog_validate_rect(const M3Rect *rect)
{
    if (rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (rect->width < 0.0f || rect->height < 0.0f) {
        return M3_ERR_RANGE;
    }
    return M3_OK;
}

static int m3_dialog_validate_backend(const M3TextBackend *backend)
{
    if (backend == NULL || backend->vtable == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_OK;
}

static int m3_dialog_measure_optional_text(const M3TextBackend *backend, M3Handle font, const char *utf8, m3_usize len,
    M3TextMetrics *out_metrics)
{
    if (out_metrics == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (len == 0u) {
        out_metrics->width = 0.0f;
        out_metrics->height = 0.0f;
        out_metrics->baseline = 0.0f;
        return M3_OK;
    }

    return m3_text_measure_utf8(backend, font, utf8, len, out_metrics);
}

static M3Bool m3_dialog_point_in_rect(const M3Rect *rect, M3Scalar x, M3Scalar y)
{
    if (rect == NULL) {
        return M3_FALSE;
    }
    if (rect->width <= 0.0f || rect->height <= 0.0f) {
        return M3_FALSE;
    }
    if (x < rect->x || y < rect->y) {
        return M3_FALSE;
    }
    if (x > rect->x + rect->width || y > rect->y + rect->height) {
        return M3_FALSE;
    }
    return M3_TRUE;
}

#ifdef M3_TESTING
int M3_CALL m3_dialog_test_point_in_rect(const M3Rect *rect, M3Scalar x, M3Scalar y, M3Bool *out_inside)
{
    if (out_inside == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_inside = m3_dialog_point_in_rect(rect, x, y);
    return M3_OK;
}

int M3_CALL m3_dialog_test_validate_color(const M3Color *color)
{
    return m3_dialog_validate_color(color);
}

int M3_CALL m3_dialog_test_validate_text_style(const M3TextStyle *style, M3Bool require_family)
{
    return m3_dialog_validate_text_style(style, require_family);
}

int M3_CALL m3_dialog_test_validate_edges(const M3LayoutEdges *edges)
{
    return m3_dialog_validate_edges(edges);
}

int M3_CALL m3_dialog_test_validate_measure_spec(M3MeasureSpec spec)
{
    return m3_dialog_validate_measure_spec(spec);
}

int M3_CALL m3_dialog_test_validate_rect(const M3Rect *rect)
{
    return m3_dialog_validate_rect(rect);
}

int M3_CALL m3_dialog_test_validate_backend(const M3TextBackend *backend)
{
    return m3_dialog_validate_backend(backend);
}
#endif

static int m3_dialog_destroy_fonts(const M3TextBackend *backend, M3Handle *fonts, m3_usize count)
{
    int rc;
    int destroy_rc;
    m3_usize i;

    if (backend == NULL || fonts == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = M3_OK;
    for (i = 0; i < count; ++i) {
        if (fonts[i].id == 0u && fonts[i].generation == 0u) {
            continue;
        }
        destroy_rc = m3_text_font_destroy(backend, fonts[i]);
        if (rc == M3_OK && destroy_rc != M3_OK) {
            rc = destroy_rc;
        }
        fonts[i].id = 0u;
        fonts[i].generation = 0u;
    }

    return rc;
}

static int m3_alert_dialog_validate_style(const M3AlertDialogStyle *style, M3Bool require_family)
{
    int rc;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (style->corner_radius < 0.0f || style->min_width < 0.0f || style->max_width < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->max_width > 0.0f && style->max_width < style->min_width) {
        return M3_ERR_RANGE;
    }
    if (style->title_body_spacing < 0.0f || style->body_action_spacing < 0.0f || style->action_spacing < 0.0f
        || style->action_padding_x < 0.0f || style->action_padding_y < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->shadow_enabled != M3_FALSE && style->shadow_enabled != M3_TRUE) {
        return M3_ERR_RANGE;
    }
    if (style->scrim_enabled != M3_FALSE && style->scrim_enabled != M3_TRUE) {
        return M3_ERR_RANGE;
    }

    rc = m3_dialog_validate_edges(&style->padding);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_text_style(&style->title_style, require_family);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_text_style(&style->body_style, require_family);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_text_style(&style->action_style, require_family);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_dialog_validate_color(&style->background_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_color(&style->scrim_color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

static int m3_alert_dialog_metrics_update(M3AlertDialog *dialog)
{
    int rc;

    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (dialog->metrics_valid == M3_TRUE) {
        return M3_OK;
    }

    rc = m3_dialog_measure_optional_text(&dialog->text_backend, dialog->title_font, dialog->utf8_title, dialog->title_len,
        &dialog->title_metrics);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_measure_optional_text(&dialog->text_backend, dialog->body_font, dialog->utf8_body, dialog->body_len,
        &dialog->body_metrics);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_measure_optional_text(&dialog->text_backend, dialog->action_font, dialog->utf8_confirm, dialog->confirm_len,
        &dialog->confirm_metrics);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_measure_optional_text(&dialog->text_backend, dialog->action_font, dialog->utf8_dismiss, dialog->dismiss_len,
        &dialog->dismiss_metrics);
    if (rc != M3_OK) {
        return rc;
    }

    dialog->metrics_valid = M3_TRUE;
    return M3_OK;
}

static int m3_alert_dialog_compute_actions(const M3AlertDialog *dialog, M3Scalar *out_total_width, M3Scalar *out_height,
    M3Scalar *out_confirm_width, M3Scalar *out_dismiss_width)
{
    M3Scalar confirm_width;
    M3Scalar dismiss_width;
    M3Scalar height;
    M3Bool has_confirm;
    M3Bool has_dismiss;

    if (dialog == NULL || out_total_width == NULL || out_height == NULL || out_confirm_width == NULL || out_dismiss_width == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    has_confirm = (dialog->confirm_len > 0u) ? M3_TRUE : M3_FALSE;
    has_dismiss = (dialog->dismiss_len > 0u) ? M3_TRUE : M3_FALSE;

    confirm_width = 0.0f;
    dismiss_width = 0.0f;
    height = 0.0f;

    if (has_confirm == M3_TRUE) {
        confirm_width = dialog->confirm_metrics.width + dialog->style.action_padding_x * 2.0f;
        height = dialog->confirm_metrics.height + dialog->style.action_padding_y * 2.0f;
    }
    if (has_dismiss == M3_TRUE) {
        dismiss_width = dialog->dismiss_metrics.width + dialog->style.action_padding_x * 2.0f;
        if (dialog->dismiss_metrics.height + dialog->style.action_padding_y * 2.0f > height) {
            height = dialog->dismiss_metrics.height + dialog->style.action_padding_y * 2.0f;
        }
    }

    *out_total_width = confirm_width + dismiss_width;
    if (has_confirm == M3_TRUE && has_dismiss == M3_TRUE) {
        *out_total_width += dialog->style.action_spacing;
    }
    *out_height = height;
    *out_confirm_width = confirm_width;
    *out_dismiss_width = dismiss_width;
    return M3_OK;
}

static int m3_alert_dialog_layout_actions(M3AlertDialog *dialog)
{
    M3Scalar content_x;
    M3Scalar content_y;
    M3Scalar content_width;
    M3Scalar cursor_y;
    M3Scalar actions_width;
    M3Scalar actions_height;
    M3Scalar confirm_width;
    M3Scalar dismiss_width;
    M3Scalar x;
    int rc;

    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    content_x = dialog->bounds.x + dialog->style.padding.left;
    content_y = dialog->bounds.y + dialog->style.padding.top;
    content_width = dialog->bounds.width - dialog->style.padding.left - dialog->style.padding.right;
    if (content_width < 0.0f) {
        content_width = 0.0f;
    }

    cursor_y = content_y;
    if (dialog->title_len > 0u) {
        cursor_y += dialog->title_metrics.height;
    }
    if (dialog->title_len > 0u && dialog->body_len > 0u) {
        cursor_y += dialog->style.title_body_spacing;
    }
    if (dialog->body_len > 0u) {
        cursor_y += dialog->body_metrics.height;
    }
    if ((dialog->confirm_len > 0u || dialog->dismiss_len > 0u) && (dialog->title_len > 0u || dialog->body_len > 0u)) {
        cursor_y += dialog->style.body_action_spacing;
    }

    rc = m3_alert_dialog_compute_actions(dialog, &actions_width, &actions_height, &confirm_width, &dismiss_width);
    if (rc != M3_OK) {
        return rc;
    }

    if (actions_width <= 0.0f) {
        dialog->confirm_bounds.x = 0.0f;
        dialog->confirm_bounds.y = 0.0f;
        dialog->confirm_bounds.width = 0.0f;
        dialog->confirm_bounds.height = 0.0f;
        dialog->dismiss_bounds = dialog->confirm_bounds;
        return M3_OK;
    }

    x = content_x;
    if (content_width > actions_width) {
        x = content_x + (content_width - actions_width);
    }

    if (dialog->dismiss_len > 0u) {
        dialog->dismiss_bounds.x = x;
        dialog->dismiss_bounds.y = cursor_y;
        dialog->dismiss_bounds.width = dismiss_width;
        dialog->dismiss_bounds.height = actions_height;
        x += dismiss_width;
        if (dialog->confirm_len > 0u) {
            x += dialog->style.action_spacing;
        }
    } else {
        dialog->dismiss_bounds.x = 0.0f;
        dialog->dismiss_bounds.y = 0.0f;
        dialog->dismiss_bounds.width = 0.0f;
        dialog->dismiss_bounds.height = 0.0f;
    }

    if (dialog->confirm_len > 0u) {
        dialog->confirm_bounds.x = x;
        dialog->confirm_bounds.y = cursor_y;
        dialog->confirm_bounds.width = confirm_width;
        dialog->confirm_bounds.height = actions_height;
    } else {
        dialog->confirm_bounds.x = 0.0f;
        dialog->confirm_bounds.y = 0.0f;
        dialog->confirm_bounds.width = 0.0f;
        dialog->confirm_bounds.height = 0.0f;
    }

    return M3_OK;
}

static int m3_alert_dialog_widget_measure(void *widget, M3MeasureSpec width, M3MeasureSpec height, M3Size *out_size)
{
    M3AlertDialog *dialog;
    M3Scalar desired_width;
    M3Scalar desired_height;
    M3Scalar content_width;
    M3Scalar actions_width;
    M3Scalar actions_height;
    M3Scalar confirm_width;
    M3Scalar dismiss_width;
    int rc;

    if (widget == NULL || out_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_dialog_validate_measure_spec(width);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_measure_spec(height);
    if (rc != M3_OK) {
        return rc;
    }

    dialog = (M3AlertDialog *)widget;
    rc = m3_alert_dialog_validate_style(&dialog->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_alert_dialog_metrics_update(dialog);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_alert_dialog_compute_actions(dialog, &actions_width, &actions_height, &confirm_width, &dismiss_width);
    if (rc != M3_OK) {
        return rc;
    }

    content_width = dialog->title_metrics.width;
    if (dialog->body_metrics.width > content_width) {
        content_width = dialog->body_metrics.width;
    }
    if (actions_width > content_width) {
        content_width = actions_width;
    }

    desired_width = content_width + dialog->style.padding.left + dialog->style.padding.right;
    desired_height = dialog->style.padding.top + dialog->style.padding.bottom;

    if (dialog->title_len > 0u) {
        desired_height += dialog->title_metrics.height;
    }
    if (dialog->title_len > 0u && dialog->body_len > 0u) {
        desired_height += dialog->style.title_body_spacing;
    }
    if (dialog->body_len > 0u) {
        desired_height += dialog->body_metrics.height;
    }
    if ((dialog->confirm_len > 0u || dialog->dismiss_len > 0u) && (dialog->title_len > 0u || dialog->body_len > 0u)) {
        desired_height += dialog->style.body_action_spacing;
    }
    if (actions_height > 0.0f) {
        desired_height += actions_height;
    }

    if (desired_width < dialog->style.min_width) {
        desired_width = dialog->style.min_width;
    }
    if (dialog->style.max_width > 0.0f && desired_width > dialog->style.max_width) {
        desired_width = dialog->style.max_width;
    }

    if (width.mode == M3_MEASURE_EXACTLY) {
        out_size->width = width.size;
    } else if (width.mode == M3_MEASURE_AT_MOST) {
        out_size->width = (desired_width > width.size) ? width.size : desired_width;
    } else {
        out_size->width = desired_width;
    }

    if (height.mode == M3_MEASURE_EXACTLY) {
        out_size->height = height.size;
    } else if (height.mode == M3_MEASURE_AT_MOST) {
        out_size->height = (desired_height > height.size) ? height.size : desired_height;
    } else {
        out_size->height = desired_height;
    }

    return M3_OK;
}

static int m3_alert_dialog_widget_layout(void *widget, M3Rect bounds)
{
    M3AlertDialog *dialog;
    int rc;

    if (widget == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_dialog_validate_rect(&bounds);
    if (rc != M3_OK) {
        return rc;
    }

    dialog = (M3AlertDialog *)widget;
    rc = m3_alert_dialog_validate_style(&dialog->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_alert_dialog_metrics_update(dialog);
    if (rc != M3_OK) {
        return rc;
    }

    dialog->bounds = bounds;
    return m3_alert_dialog_layout_actions(dialog);
}

static int m3_alert_dialog_draw_action_text(const M3AlertDialog *dialog, const M3Rect *bounds,
    const M3TextMetrics *metrics, const char *utf8, m3_usize len, M3PaintContext *ctx)
{
    M3Scalar text_x;
    M3Scalar text_y;
    M3Scalar available_width;
    M3Scalar available_height;

    if (dialog == NULL || bounds == NULL || metrics == NULL || ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (len == 0u) {
        return M3_OK;
    }

    available_width = bounds->width - dialog->style.action_padding_x * 2.0f;
    available_height = bounds->height - dialog->style.action_padding_y * 2.0f;
    if (available_width < 0.0f) {
        available_width = 0.0f;
    }
    if (available_height < 0.0f) {
        available_height = 0.0f;
    }

    if (available_width <= metrics->width) {
        text_x = bounds->x + dialog->style.action_padding_x;
    } else {
        text_x = bounds->x + dialog->style.action_padding_x + (available_width - metrics->width) * 0.5f;
    }

    if (available_height <= metrics->height) {
        text_y = bounds->y + dialog->style.action_padding_y + metrics->baseline;
    } else {
        text_y = bounds->y + dialog->style.action_padding_y + (available_height - metrics->height) * 0.5f + metrics->baseline;
    }

    return ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, dialog->action_font, utf8, len, text_x, text_y,
        dialog->style.action_style.color);
}

static int m3_alert_dialog_widget_paint(void *widget, M3PaintContext *ctx)
{
    M3AlertDialog *dialog;
    M3Rect content_rect;
    const M3Rect *shadow_clip;
    M3Scalar cursor_y;
    int rc;

    if (widget == NULL || ctx == NULL || ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (ctx->gfx->vtable->draw_rect == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    shadow_clip = NULL;
    if (ctx->gfx->vtable->push_clip != NULL && ctx->gfx->vtable->pop_clip != NULL) {
        shadow_clip = &ctx->clip;
    }

    dialog = (M3AlertDialog *)widget;
    rc = m3_alert_dialog_validate_style(&dialog->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_alert_dialog_metrics_update(dialog);
    if (rc != M3_OK) {
        return rc;
    }

    if (dialog->style.scrim_enabled == M3_TRUE) {
        rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &ctx->clip, dialog->style.scrim_color, 0.0f);
        if (rc != M3_OK) {
            return rc;
        }
    }

    if (dialog->style.shadow_enabled == M3_TRUE) {
        rc = m3_shadow_paint(&dialog->style.shadow, ctx->gfx, &dialog->bounds, shadow_clip);
        if (rc != M3_OK) {
            return rc;
        }
    }

    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &dialog->bounds, dialog->style.background_color, dialog->style.corner_radius);
    if (rc != M3_OK) {
        return rc;
    }

    if (ctx->gfx->text_vtable == NULL || ctx->gfx->text_vtable->draw_text == NULL) {
        if (dialog->title_len > 0u || dialog->body_len > 0u || dialog->confirm_len > 0u || dialog->dismiss_len > 0u) {
            return M3_ERR_UNSUPPORTED;
        }
        return M3_OK;
    }

    content_rect.x = dialog->bounds.x + dialog->style.padding.left;
    content_rect.y = dialog->bounds.y + dialog->style.padding.top;
    content_rect.width = dialog->bounds.width - dialog->style.padding.left - dialog->style.padding.right;
    content_rect.height = dialog->bounds.height - dialog->style.padding.top - dialog->style.padding.bottom;
    if (content_rect.width < 0.0f) {
        content_rect.width = 0.0f;
    }
    if (content_rect.height < 0.0f) {
        content_rect.height = 0.0f;
    }

    cursor_y = content_rect.y;
    if (dialog->title_len > 0u) {
        rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, dialog->title_font, dialog->utf8_title, dialog->title_len,
            content_rect.x, cursor_y + dialog->title_metrics.baseline, dialog->style.title_style.color);
        if (rc != M3_OK) {
            return rc;
        }
        cursor_y += dialog->title_metrics.height;
    }
    if (dialog->title_len > 0u && dialog->body_len > 0u) {
        cursor_y += dialog->style.title_body_spacing;
    }
    if (dialog->body_len > 0u) {
        rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, dialog->body_font, dialog->utf8_body, dialog->body_len,
            content_rect.x, cursor_y + dialog->body_metrics.baseline, dialog->style.body_style.color);
        if (rc != M3_OK) {
            return rc;
        }
        cursor_y += dialog->body_metrics.height;
    }

    if ((dialog->confirm_len > 0u || dialog->dismiss_len > 0u) && (dialog->title_len > 0u || dialog->body_len > 0u)) {
        cursor_y += dialog->style.body_action_spacing;
        M3_UNUSED(cursor_y);
    }

    rc = m3_alert_dialog_draw_action_text(dialog, &dialog->dismiss_bounds, &dialog->dismiss_metrics, dialog->utf8_dismiss,
        dialog->dismiss_len, ctx);
    if (rc != M3_OK) {
        return rc;
    }

    return m3_alert_dialog_draw_action_text(dialog, &dialog->confirm_bounds, &dialog->confirm_metrics, dialog->utf8_confirm,
        dialog->confirm_len, ctx);
}

static int m3_alert_dialog_widget_event(void *widget, const M3InputEvent *event, M3Bool *out_handled)
{
    M3AlertDialog *dialog;
    M3Scalar x;
    M3Scalar y;
    int rc;

    if (widget == NULL || event == NULL || out_handled == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_handled = M3_FALSE;
    dialog = (M3AlertDialog *)widget;

    if (dialog->widget.flags & M3_WIDGET_FLAG_DISABLED) {
        return M3_OK;
    }

    if (event->type != M3_INPUT_POINTER_DOWN && event->type != M3_INPUT_POINTER_UP) {
        return M3_OK;
    }

    x = (M3Scalar)event->data.pointer.x;
    y = (M3Scalar)event->data.pointer.y;

    if (event->type == M3_INPUT_POINTER_DOWN) {
        if (dialog->pressed_confirm == M3_TRUE || dialog->pressed_dismiss == M3_TRUE) {
            return M3_ERR_STATE;
        }
        if (m3_dialog_point_in_rect(&dialog->confirm_bounds, x, y) == M3_TRUE) {
            dialog->pressed_confirm = M3_TRUE;
            *out_handled = M3_TRUE;
            return M3_OK;
        }
        if (m3_dialog_point_in_rect(&dialog->dismiss_bounds, x, y) == M3_TRUE) {
            dialog->pressed_dismiss = M3_TRUE;
            *out_handled = M3_TRUE;
            return M3_OK;
        }
        return M3_OK;
    }

    if (dialog->pressed_confirm == M3_TRUE) {
        dialog->pressed_confirm = M3_FALSE;
        *out_handled = M3_TRUE;
        if (m3_dialog_point_in_rect(&dialog->confirm_bounds, x, y) == M3_TRUE && dialog->on_action != NULL) {
            rc = dialog->on_action(dialog->on_action_ctx, dialog, M3_ALERT_DIALOG_ACTION_CONFIRM);
            if (rc != M3_OK) {
                return rc;
            }
        }
        return M3_OK;
    }

    if (dialog->pressed_dismiss == M3_TRUE) {
        dialog->pressed_dismiss = M3_FALSE;
        *out_handled = M3_TRUE;
        if (m3_dialog_point_in_rect(&dialog->dismiss_bounds, x, y) == M3_TRUE && dialog->on_action != NULL) {
            rc = dialog->on_action(dialog->on_action_ctx, dialog, M3_ALERT_DIALOG_ACTION_DISMISS);
            if (rc != M3_OK) {
                return rc;
            }
        }
        return M3_OK;
    }

    return M3_OK;
}

static int m3_alert_dialog_widget_get_semantics(void *widget, M3Semantics *out_semantics)
{
    M3AlertDialog *dialog;

    if (widget == NULL || out_semantics == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog = (M3AlertDialog *)widget;
    out_semantics->role = M3_SEMANTIC_NONE;
    out_semantics->flags = 0;
    if (dialog->widget.flags & M3_WIDGET_FLAG_DISABLED) {
        out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
    }
    if (dialog->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
        out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
    }
    out_semantics->utf8_label = dialog->utf8_title;
    out_semantics->utf8_hint = NULL;
    out_semantics->utf8_value = NULL;
    return M3_OK;
}

static int m3_alert_dialog_widget_destroy(void *widget)
{
    M3AlertDialog *dialog;
    M3Handle fonts[3];
    int rc;

    if (widget == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog = (M3AlertDialog *)widget;
    rc = M3_OK;
    if (dialog->owns_fonts == M3_TRUE) {
        fonts[0] = dialog->title_font;
        fonts[1] = dialog->body_font;
        fonts[2] = dialog->action_font;
        rc = m3_dialog_destroy_fonts(&dialog->text_backend, fonts, 3u);
    }

    dialog->title_font.id = 0u;
    dialog->title_font.generation = 0u;
    dialog->body_font.id = 0u;
    dialog->body_font.generation = 0u;
    dialog->action_font.id = 0u;
    dialog->action_font.generation = 0u;
    dialog->utf8_title = NULL;
    dialog->title_len = 0u;
    dialog->utf8_body = NULL;
    dialog->body_len = 0u;
    dialog->utf8_confirm = NULL;
    dialog->confirm_len = 0u;
    dialog->utf8_dismiss = NULL;
    dialog->dismiss_len = 0u;
    dialog->metrics_valid = M3_FALSE;
    dialog->owns_fonts = M3_FALSE;
    dialog->pressed_confirm = M3_FALSE;
    dialog->pressed_dismiss = M3_FALSE;
    dialog->on_action = NULL;
    dialog->on_action_ctx = NULL;
    dialog->text_backend.ctx = NULL;
    dialog->text_backend.vtable = NULL;
    dialog->widget.ctx = NULL;
    dialog->widget.vtable = NULL;
    return rc;
}

static const M3WidgetVTable g_m3_alert_dialog_widget_vtable = {
    m3_alert_dialog_widget_measure,
    m3_alert_dialog_widget_layout,
    m3_alert_dialog_widget_paint,
    m3_alert_dialog_widget_event,
    m3_alert_dialog_widget_get_semantics,
    m3_alert_dialog_widget_destroy
};

static int m3_fullscreen_dialog_validate_style(const M3FullScreenDialogStyle *style, M3Bool require_family)
{
    int rc;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (style->corner_radius < 0.0f || style->title_body_spacing < 0.0f || style->body_action_spacing < 0.0f
        || style->action_padding_x < 0.0f || style->action_padding_y < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->shadow_enabled != M3_FALSE && style->shadow_enabled != M3_TRUE) {
        return M3_ERR_RANGE;
    }
    if (style->scrim_enabled != M3_FALSE && style->scrim_enabled != M3_TRUE) {
        return M3_ERR_RANGE;
    }

    rc = m3_dialog_validate_edges(&style->padding);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_text_style(&style->title_style, require_family);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_text_style(&style->body_style, require_family);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_text_style(&style->action_style, require_family);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_dialog_validate_color(&style->background_color);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_color(&style->scrim_color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

static int m3_fullscreen_dialog_metrics_update(M3FullScreenDialog *dialog)
{
    int rc;

    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (dialog->metrics_valid == M3_TRUE) {
        return M3_OK;
    }

    rc = m3_dialog_measure_optional_text(&dialog->text_backend, dialog->title_font, dialog->utf8_title, dialog->title_len,
        &dialog->title_metrics);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_measure_optional_text(&dialog->text_backend, dialog->body_font, dialog->utf8_body, dialog->body_len,
        &dialog->body_metrics);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_measure_optional_text(&dialog->text_backend, dialog->action_font, dialog->utf8_action, dialog->action_len,
        &dialog->action_metrics);
    if (rc != M3_OK) {
        return rc;
    }

    dialog->metrics_valid = M3_TRUE;
    return M3_OK;
}

static int m3_fullscreen_dialog_compute_action(const M3FullScreenDialog *dialog, M3Scalar *out_width, M3Scalar *out_height)
{
    if (dialog == NULL || out_width == NULL || out_height == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (dialog->action_len == 0u) {
        *out_width = 0.0f;
        *out_height = 0.0f;
        return M3_OK;
    }

    *out_width = dialog->action_metrics.width + dialog->style.action_padding_x * 2.0f;
    *out_height = dialog->action_metrics.height + dialog->style.action_padding_y * 2.0f;
    return M3_OK;
}

static int m3_fullscreen_dialog_layout_action(M3FullScreenDialog *dialog)
{
    M3Scalar content_x;
    M3Scalar content_y;
    M3Scalar content_width;
    M3Scalar cursor_y;
    M3Scalar action_width;
    M3Scalar action_height;
    int rc;

    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    content_x = dialog->bounds.x + dialog->style.padding.left;
    content_y = dialog->bounds.y + dialog->style.padding.top;
    content_width = dialog->bounds.width - dialog->style.padding.left - dialog->style.padding.right;
    if (content_width < 0.0f) {
        content_width = 0.0f;
    }

    cursor_y = content_y;
    if (dialog->title_len > 0u) {
        cursor_y += dialog->title_metrics.height;
    }
    if (dialog->title_len > 0u && dialog->body_len > 0u) {
        cursor_y += dialog->style.title_body_spacing;
    }
    if (dialog->body_len > 0u) {
        cursor_y += dialog->body_metrics.height;
    }
    if (dialog->action_len > 0u && (dialog->title_len > 0u || dialog->body_len > 0u)) {
        cursor_y += dialog->style.body_action_spacing;
    }

    rc = m3_fullscreen_dialog_compute_action(dialog, &action_width, &action_height);
    if (rc != M3_OK) {
        return rc;
    }

    if (action_width <= 0.0f) {
        dialog->action_bounds.x = 0.0f;
        dialog->action_bounds.y = 0.0f;
        dialog->action_bounds.width = 0.0f;
        dialog->action_bounds.height = 0.0f;
        return M3_OK;
    }

    dialog->action_bounds.x = content_x;
    if (content_width > action_width) {
        dialog->action_bounds.x = content_x + (content_width - action_width);
    }
    dialog->action_bounds.y = cursor_y;
    dialog->action_bounds.width = action_width;
    dialog->action_bounds.height = action_height;
    return M3_OK;
}

static int m3_fullscreen_dialog_widget_measure(void *widget, M3MeasureSpec width, M3MeasureSpec height, M3Size *out_size)
{
    M3FullScreenDialog *dialog;
    M3Scalar desired_width;
    M3Scalar desired_height;
    M3Scalar content_width;
    M3Scalar action_width;
    M3Scalar action_height;
    int rc;

    if (widget == NULL || out_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_dialog_validate_measure_spec(width);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_measure_spec(height);
    if (rc != M3_OK) {
        return rc;
    }

    dialog = (M3FullScreenDialog *)widget;
    rc = m3_fullscreen_dialog_validate_style(&dialog->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_fullscreen_dialog_metrics_update(dialog);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_fullscreen_dialog_compute_action(dialog, &action_width, &action_height);
    if (rc != M3_OK) {
        return rc;
    }

    content_width = dialog->title_metrics.width;
    if (dialog->body_metrics.width > content_width) {
        content_width = dialog->body_metrics.width;
    }
    if (action_width > content_width) {
        content_width = action_width;
    }

    desired_width = content_width + dialog->style.padding.left + dialog->style.padding.right;
    desired_height = dialog->style.padding.top + dialog->style.padding.bottom;

    if (dialog->title_len > 0u) {
        desired_height += dialog->title_metrics.height;
    }
    if (dialog->title_len > 0u && dialog->body_len > 0u) {
        desired_height += dialog->style.title_body_spacing;
    }
    if (dialog->body_len > 0u) {
        desired_height += dialog->body_metrics.height;
    }
    if (dialog->action_len > 0u && (dialog->title_len > 0u || dialog->body_len > 0u)) {
        desired_height += dialog->style.body_action_spacing;
    }
    if (action_height > 0.0f) {
        desired_height += action_height;
    }

    if (width.mode == M3_MEASURE_EXACTLY) {
        out_size->width = width.size;
    } else if (width.mode == M3_MEASURE_AT_MOST) {
        out_size->width = (desired_width > width.size) ? width.size : desired_width;
    } else {
        out_size->width = desired_width;
    }

    if (height.mode == M3_MEASURE_EXACTLY) {
        out_size->height = height.size;
    } else if (height.mode == M3_MEASURE_AT_MOST) {
        out_size->height = (desired_height > height.size) ? height.size : desired_height;
    } else {
        out_size->height = desired_height;
    }

    return M3_OK;
}

static int m3_fullscreen_dialog_widget_layout(void *widget, M3Rect bounds)
{
    M3FullScreenDialog *dialog;
    int rc;

    if (widget == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_dialog_validate_rect(&bounds);
    if (rc != M3_OK) {
        return rc;
    }

    dialog = (M3FullScreenDialog *)widget;
    rc = m3_fullscreen_dialog_validate_style(&dialog->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_fullscreen_dialog_metrics_update(dialog);
    if (rc != M3_OK) {
        return rc;
    }

    dialog->bounds = bounds;
    return m3_fullscreen_dialog_layout_action(dialog);
}

static int m3_fullscreen_dialog_draw_action_text(const M3FullScreenDialog *dialog, const M3Rect *bounds,
    const M3TextMetrics *metrics, const char *utf8, m3_usize len, M3PaintContext *ctx)
{
    M3Scalar text_x;
    M3Scalar text_y;
    M3Scalar available_width;
    M3Scalar available_height;

    if (dialog == NULL || bounds == NULL || metrics == NULL || ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (len == 0u) {
        return M3_OK;
    }

    available_width = bounds->width - dialog->style.action_padding_x * 2.0f;
    available_height = bounds->height - dialog->style.action_padding_y * 2.0f;
    if (available_width < 0.0f) {
        available_width = 0.0f;
    }
    if (available_height < 0.0f) {
        available_height = 0.0f;
    }

    if (available_width <= metrics->width) {
        text_x = bounds->x + dialog->style.action_padding_x;
    } else {
        text_x = bounds->x + dialog->style.action_padding_x + (available_width - metrics->width) * 0.5f;
    }

    if (available_height <= metrics->height) {
        text_y = bounds->y + dialog->style.action_padding_y + metrics->baseline;
    } else {
        text_y = bounds->y + dialog->style.action_padding_y + (available_height - metrics->height) * 0.5f + metrics->baseline;
    }

    return ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, dialog->action_font, utf8, len, text_x, text_y,
        dialog->style.action_style.color);
}

static int m3_fullscreen_dialog_widget_paint(void *widget, M3PaintContext *ctx)
{
    M3FullScreenDialog *dialog;
    M3Rect content_rect;
    const M3Rect *shadow_clip;
    M3Scalar cursor_y;
    int rc;

    if (widget == NULL || ctx == NULL || ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (ctx->gfx->vtable->draw_rect == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    shadow_clip = NULL;
    if (ctx->gfx->vtable->push_clip != NULL && ctx->gfx->vtable->pop_clip != NULL) {
        shadow_clip = &ctx->clip;
    }

    dialog = (M3FullScreenDialog *)widget;
    rc = m3_fullscreen_dialog_validate_style(&dialog->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_fullscreen_dialog_metrics_update(dialog);
    if (rc != M3_OK) {
        return rc;
    }

    if (dialog->style.scrim_enabled == M3_TRUE) {
        rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &ctx->clip, dialog->style.scrim_color, 0.0f);
        if (rc != M3_OK) {
            return rc;
        }
    }

    if (dialog->style.shadow_enabled == M3_TRUE) {
        rc = m3_shadow_paint(&dialog->style.shadow, ctx->gfx, &dialog->bounds, shadow_clip);
        if (rc != M3_OK) {
            return rc;
        }
    }

    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &dialog->bounds, dialog->style.background_color, dialog->style.corner_radius);
    if (rc != M3_OK) {
        return rc;
    }

    if (ctx->gfx->text_vtable == NULL || ctx->gfx->text_vtable->draw_text == NULL) {
        if (dialog->title_len > 0u || dialog->body_len > 0u || dialog->action_len > 0u) {
            return M3_ERR_UNSUPPORTED;
        }
        return M3_OK;
    }

    content_rect.x = dialog->bounds.x + dialog->style.padding.left;
    content_rect.y = dialog->bounds.y + dialog->style.padding.top;
    content_rect.width = dialog->bounds.width - dialog->style.padding.left - dialog->style.padding.right;
    content_rect.height = dialog->bounds.height - dialog->style.padding.top - dialog->style.padding.bottom;

    cursor_y = content_rect.y;
    if (dialog->title_len > 0u) {
        rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, dialog->title_font, dialog->utf8_title, dialog->title_len,
            content_rect.x, cursor_y + dialog->title_metrics.baseline, dialog->style.title_style.color);
        if (rc != M3_OK) {
            return rc;
        }
        cursor_y += dialog->title_metrics.height;
    }
    if (dialog->title_len > 0u && dialog->body_len > 0u) {
        cursor_y += dialog->style.title_body_spacing;
    }
    if (dialog->body_len > 0u) {
        rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, dialog->body_font, dialog->utf8_body, dialog->body_len,
            content_rect.x, cursor_y + dialog->body_metrics.baseline, dialog->style.body_style.color);
        if (rc != M3_OK) {
            return rc;
        }
        cursor_y += dialog->body_metrics.height;
    }

    if (dialog->action_len > 0u && (dialog->title_len > 0u || dialog->body_len > 0u)) {
        cursor_y += dialog->style.body_action_spacing;
        M3_UNUSED(cursor_y);
    }

    return m3_fullscreen_dialog_draw_action_text(dialog, &dialog->action_bounds, &dialog->action_metrics, dialog->utf8_action,
        dialog->action_len, ctx);
}

static int m3_fullscreen_dialog_widget_event(void *widget, const M3InputEvent *event, M3Bool *out_handled)
{
    M3FullScreenDialog *dialog;
    M3Scalar x;
    M3Scalar y;
    int rc;

    if (widget == NULL || event == NULL || out_handled == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_handled = M3_FALSE;
    dialog = (M3FullScreenDialog *)widget;

    if (dialog->widget.flags & M3_WIDGET_FLAG_DISABLED) {
        return M3_OK;
    }

    if (event->type != M3_INPUT_POINTER_DOWN && event->type != M3_INPUT_POINTER_UP) {
        return M3_OK;
    }

    x = (M3Scalar)event->data.pointer.x;
    y = (M3Scalar)event->data.pointer.y;

    if (event->type == M3_INPUT_POINTER_DOWN) {
        if (dialog->pressed_action == M3_TRUE) {
            return M3_ERR_STATE;
        }
        if (m3_dialog_point_in_rect(&dialog->action_bounds, x, y) == M3_TRUE) {
            dialog->pressed_action = M3_TRUE;
            *out_handled = M3_TRUE;
        }
        return M3_OK;
    }

    if (dialog->pressed_action == M3_TRUE) {
        dialog->pressed_action = M3_FALSE;
        *out_handled = M3_TRUE;
        if (m3_dialog_point_in_rect(&dialog->action_bounds, x, y) == M3_TRUE && dialog->on_action != NULL) {
            rc = dialog->on_action(dialog->on_action_ctx, dialog);
            if (rc != M3_OK) {
                return rc;
            }
        }
    }

    return M3_OK;
}

static int m3_fullscreen_dialog_widget_get_semantics(void *widget, M3Semantics *out_semantics)
{
    M3FullScreenDialog *dialog;

    if (widget == NULL || out_semantics == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog = (M3FullScreenDialog *)widget;
    out_semantics->role = M3_SEMANTIC_NONE;
    out_semantics->flags = 0;
    if (dialog->widget.flags & M3_WIDGET_FLAG_DISABLED) {
        out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
    }
    if (dialog->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
        out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
    }
    out_semantics->utf8_label = dialog->utf8_title;
    out_semantics->utf8_hint = NULL;
    out_semantics->utf8_value = NULL;
    return M3_OK;
}

static int m3_fullscreen_dialog_widget_destroy(void *widget)
{
    M3FullScreenDialog *dialog;
    M3Handle fonts[3];
    int rc;

    if (widget == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog = (M3FullScreenDialog *)widget;
    rc = M3_OK;
    if (dialog->owns_fonts == M3_TRUE) {
        fonts[0] = dialog->title_font;
        fonts[1] = dialog->body_font;
        fonts[2] = dialog->action_font;
        rc = m3_dialog_destroy_fonts(&dialog->text_backend, fonts, 3u);
    }

    dialog->title_font.id = 0u;
    dialog->title_font.generation = 0u;
    dialog->body_font.id = 0u;
    dialog->body_font.generation = 0u;
    dialog->action_font.id = 0u;
    dialog->action_font.generation = 0u;
    dialog->utf8_title = NULL;
    dialog->title_len = 0u;
    dialog->utf8_body = NULL;
    dialog->body_len = 0u;
    dialog->utf8_action = NULL;
    dialog->action_len = 0u;
    dialog->metrics_valid = M3_FALSE;
    dialog->owns_fonts = M3_FALSE;
    dialog->pressed_action = M3_FALSE;
    dialog->on_action = NULL;
    dialog->on_action_ctx = NULL;
    dialog->text_backend.ctx = NULL;
    dialog->text_backend.vtable = NULL;
    dialog->widget.ctx = NULL;
    dialog->widget.vtable = NULL;
    return rc;
}

static const M3WidgetVTable g_m3_fullscreen_dialog_widget_vtable = {
    m3_fullscreen_dialog_widget_measure,
    m3_fullscreen_dialog_widget_layout,
    m3_fullscreen_dialog_widget_paint,
    m3_fullscreen_dialog_widget_event,
    m3_fullscreen_dialog_widget_get_semantics,
    m3_fullscreen_dialog_widget_destroy
};

static int m3_snackbar_validate_style(const M3SnackbarStyle *style, M3Bool require_family)
{
    int rc;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (style->corner_radius < 0.0f || style->min_width < 0.0f || style->max_width < 0.0f || style->min_height < 0.0f
        || style->action_spacing < 0.0f || style->action_padding_x < 0.0f || style->action_padding_y < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (style->max_width > 0.0f && style->max_width < style->min_width) {
        return M3_ERR_RANGE;
    }

    rc = m3_dialog_validate_edges(&style->padding);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_text_style(&style->message_style, require_family);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_text_style(&style->action_style, require_family);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_dialog_validate_color(&style->background_color);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

static int m3_snackbar_metrics_update(M3Snackbar *snackbar)
{
    int rc;

    if (snackbar == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (snackbar->metrics_valid == M3_TRUE) {
        return M3_OK;
    }

    rc = m3_dialog_measure_optional_text(&snackbar->text_backend, snackbar->message_font, snackbar->utf8_message,
        snackbar->message_len, &snackbar->message_metrics);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_measure_optional_text(&snackbar->text_backend, snackbar->action_font, snackbar->utf8_action,
        snackbar->action_len, &snackbar->action_metrics);
    if (rc != M3_OK) {
        return rc;
    }

    snackbar->metrics_valid = M3_TRUE;
    return M3_OK;
}

static int m3_snackbar_compute_action(const M3Snackbar *snackbar, M3Scalar *out_width, M3Scalar *out_height)
{
    if (snackbar == NULL || out_width == NULL || out_height == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (snackbar->action_len == 0u) {
        *out_width = 0.0f;
        *out_height = 0.0f;
        return M3_OK;
    }

    *out_width = snackbar->action_metrics.width + snackbar->style.action_padding_x * 2.0f;
    *out_height = snackbar->action_metrics.height + snackbar->style.action_padding_y * 2.0f;
    return M3_OK;
}

static int m3_snackbar_layout_action(M3Snackbar *snackbar)
{
    M3Scalar content_x;
    M3Scalar content_width;
    M3Scalar action_width;
    M3Scalar action_height;
    int rc;

    if (snackbar == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    content_x = snackbar->bounds.x + snackbar->style.padding.left;
    content_width = snackbar->bounds.width - snackbar->style.padding.left - snackbar->style.padding.right;
    if (content_width < 0.0f) {
        content_width = 0.0f;
    }

    rc = m3_snackbar_compute_action(snackbar, &action_width, &action_height);
    if (rc != M3_OK) {
        return rc;
    }

    if (action_width <= 0.0f) {
        snackbar->action_bounds.x = 0.0f;
        snackbar->action_bounds.y = 0.0f;
        snackbar->action_bounds.width = 0.0f;
        snackbar->action_bounds.height = 0.0f;
        return M3_OK;
    }

    snackbar->action_bounds.x = content_x;
    if (content_width > action_width) {
        snackbar->action_bounds.x = content_x + (content_width - action_width);
    }
    snackbar->action_bounds.y = snackbar->bounds.y + snackbar->style.padding.top;
    snackbar->action_bounds.width = action_width;
    snackbar->action_bounds.height = action_height;
    return M3_OK;
}

static int m3_snackbar_widget_measure(void *widget, M3MeasureSpec width, M3MeasureSpec height, M3Size *out_size)
{
    M3Snackbar *snackbar;
    M3Scalar desired_width;
    M3Scalar desired_height;
    M3Scalar content_width;
    M3Scalar action_width;
    M3Scalar action_height;
    int rc;

    if (widget == NULL || out_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_dialog_validate_measure_spec(width);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_dialog_validate_measure_spec(height);
    if (rc != M3_OK) {
        return rc;
    }

    snackbar = (M3Snackbar *)widget;
    rc = m3_snackbar_validate_style(&snackbar->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_snackbar_metrics_update(snackbar);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_snackbar_compute_action(snackbar, &action_width, &action_height);
    if (rc != M3_OK) {
        return rc;
    }

    content_width = snackbar->message_metrics.width;
    if (action_width > 0.0f) {
        content_width += snackbar->style.action_spacing + action_width;
    }

    desired_width = content_width + snackbar->style.padding.left + snackbar->style.padding.right;
    desired_height = snackbar->style.padding.top + snackbar->style.padding.bottom;

    if (snackbar->message_metrics.height > desired_height) {
        desired_height = snackbar->message_metrics.height + snackbar->style.padding.top + snackbar->style.padding.bottom;
    }
    if (action_height + snackbar->style.padding.top + snackbar->style.padding.bottom > desired_height) {
        desired_height = action_height + snackbar->style.padding.top + snackbar->style.padding.bottom;
    }

    if (desired_width < snackbar->style.min_width) {
        desired_width = snackbar->style.min_width;
    }
    if (snackbar->style.max_width > 0.0f && desired_width > snackbar->style.max_width) {
        desired_width = snackbar->style.max_width;
    }
    if (desired_height < snackbar->style.min_height) {
        desired_height = snackbar->style.min_height;
    }

    if (width.mode == M3_MEASURE_EXACTLY) {
        out_size->width = width.size;
    } else if (width.mode == M3_MEASURE_AT_MOST) {
        out_size->width = (desired_width > width.size) ? width.size : desired_width;
    } else {
        out_size->width = desired_width;
    }

    if (height.mode == M3_MEASURE_EXACTLY) {
        out_size->height = height.size;
    } else if (height.mode == M3_MEASURE_AT_MOST) {
        out_size->height = (desired_height > height.size) ? height.size : desired_height;
    } else {
        out_size->height = desired_height;
    }

    return M3_OK;
}

static int m3_snackbar_widget_layout(void *widget, M3Rect bounds)
{
    M3Snackbar *snackbar;
    int rc;

    if (widget == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_dialog_validate_rect(&bounds);
    if (rc != M3_OK) {
        return rc;
    }

    snackbar = (M3Snackbar *)widget;
    rc = m3_snackbar_validate_style(&snackbar->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_snackbar_metrics_update(snackbar);
    if (rc != M3_OK) {
        return rc;
    }

    snackbar->bounds = bounds;
    return m3_snackbar_layout_action(snackbar);
}

static int m3_snackbar_draw_action_text(const M3Snackbar *snackbar, const M3Rect *bounds, const M3TextMetrics *metrics,
    const char *utf8, m3_usize len, M3PaintContext *ctx)
{
    M3Scalar text_x;
    M3Scalar text_y;
    M3Scalar available_width;
    M3Scalar available_height;

    if (snackbar == NULL || bounds == NULL || metrics == NULL || ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (len == 0u) {
        return M3_OK;
    }

    available_width = bounds->width - snackbar->style.action_padding_x * 2.0f;
    available_height = bounds->height - snackbar->style.action_padding_y * 2.0f;
    if (available_width < 0.0f) {
        available_width = 0.0f;
    }
    if (available_height < 0.0f) {
        available_height = 0.0f;
    }

    if (available_width <= metrics->width) {
        text_x = bounds->x + snackbar->style.action_padding_x;
    } else {
        text_x = bounds->x + snackbar->style.action_padding_x + (available_width - metrics->width) * 0.5f;
    }

    if (available_height <= metrics->height) {
        text_y = bounds->y + snackbar->style.action_padding_y + metrics->baseline;
    } else {
        text_y = bounds->y + snackbar->style.action_padding_y + (available_height - metrics->height) * 0.5f + metrics->baseline;
    }

    return ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, snackbar->action_font, utf8, len, text_x, text_y,
        snackbar->style.action_style.color);
}

static int m3_snackbar_widget_paint(void *widget, M3PaintContext *ctx)
{
    M3Snackbar *snackbar;
    M3Rect content_rect;
    M3Scalar text_x;
    M3Scalar text_y;
    M3Scalar available_width;
    M3Scalar available_height;
    int rc;

    if (widget == NULL || ctx == NULL || ctx->gfx == NULL || ctx->gfx->vtable == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (ctx->gfx->vtable->draw_rect == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    snackbar = (M3Snackbar *)widget;
    rc = m3_snackbar_validate_style(&snackbar->style, M3_FALSE);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_snackbar_metrics_update(snackbar);
    if (rc != M3_OK) {
        return rc;
    }

    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &snackbar->bounds, snackbar->style.background_color, snackbar->style.corner_radius);
    if (rc != M3_OK) {
        return rc;
    }

    if (ctx->gfx->text_vtable == NULL || ctx->gfx->text_vtable->draw_text == NULL) {
        if (snackbar->message_len > 0u || snackbar->action_len > 0u) {
            return M3_ERR_UNSUPPORTED;
        }
        return M3_OK;
    }

    content_rect.x = snackbar->bounds.x + snackbar->style.padding.left;
    content_rect.y = snackbar->bounds.y + snackbar->style.padding.top;
    content_rect.width = snackbar->bounds.width - snackbar->style.padding.left - snackbar->style.padding.right;
    content_rect.height = snackbar->bounds.height - snackbar->style.padding.top - snackbar->style.padding.bottom;
    if (content_rect.width < 0.0f) {
        content_rect.width = 0.0f;
    }
    if (content_rect.height < 0.0f) {
        content_rect.height = 0.0f;
    }

    available_width = content_rect.width;
    if (snackbar->action_len > 0u) {
        available_width -= snackbar->action_bounds.width + snackbar->style.action_spacing;
        if (available_width < 0.0f) {
            available_width = 0.0f;
        }
    }

    available_height = content_rect.height;
    if (available_height < 0.0f) {
        available_height = 0.0f;
    }

    if (available_width <= snackbar->message_metrics.width) {
        text_x = content_rect.x;
    } else {
        text_x = content_rect.x;
    }

    if (available_height <= snackbar->message_metrics.height) {
        text_y = content_rect.y + snackbar->message_metrics.baseline;
    } else {
        text_y = content_rect.y + (available_height - snackbar->message_metrics.height) * 0.5f + snackbar->message_metrics.baseline;
    }

    if (snackbar->message_len > 0u) {
        rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, snackbar->message_font, snackbar->utf8_message, snackbar->message_len,
            text_x, text_y, snackbar->style.message_style.color);
        if (rc != M3_OK) {
            return rc;
        }
    }

    return m3_snackbar_draw_action_text(snackbar, &snackbar->action_bounds, &snackbar->action_metrics, snackbar->utf8_action,
        snackbar->action_len, ctx);
}

static int m3_snackbar_widget_event(void *widget, const M3InputEvent *event, M3Bool *out_handled)
{
    M3Snackbar *snackbar;
    M3Scalar x;
    M3Scalar y;
    int rc;

    if (widget == NULL || event == NULL || out_handled == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_handled = M3_FALSE;
    snackbar = (M3Snackbar *)widget;

    if (snackbar->widget.flags & M3_WIDGET_FLAG_DISABLED) {
        return M3_OK;
    }

    if (event->type != M3_INPUT_POINTER_DOWN && event->type != M3_INPUT_POINTER_UP) {
        return M3_OK;
    }

    x = (M3Scalar)event->data.pointer.x;
    y = (M3Scalar)event->data.pointer.y;

    if (event->type == M3_INPUT_POINTER_DOWN) {
        if (snackbar->pressed_action == M3_TRUE) {
            return M3_ERR_STATE;
        }
        if (m3_dialog_point_in_rect(&snackbar->action_bounds, x, y) == M3_TRUE) {
            snackbar->pressed_action = M3_TRUE;
            *out_handled = M3_TRUE;
        }
        return M3_OK;
    }

    if (snackbar->pressed_action == M3_TRUE) {
        snackbar->pressed_action = M3_FALSE;
        *out_handled = M3_TRUE;
        if (m3_dialog_point_in_rect(&snackbar->action_bounds, x, y) == M3_TRUE && snackbar->on_action != NULL) {
            rc = snackbar->on_action(snackbar->on_action_ctx, snackbar);
            if (rc != M3_OK) {
                return rc;
            }
        }
    }

    return M3_OK;
}

static int m3_snackbar_widget_get_semantics(void *widget, M3Semantics *out_semantics)
{
    M3Snackbar *snackbar;

    if (widget == NULL || out_semantics == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    snackbar = (M3Snackbar *)widget;
    out_semantics->role = M3_SEMANTIC_NONE;
    out_semantics->flags = 0;
    if (snackbar->widget.flags & M3_WIDGET_FLAG_DISABLED) {
        out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
    }
    if (snackbar->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
        out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
    }
    out_semantics->utf8_label = snackbar->utf8_message;
    out_semantics->utf8_hint = NULL;
    out_semantics->utf8_value = NULL;
    return M3_OK;
}

static int m3_snackbar_widget_destroy(void *widget)
{
    M3Snackbar *snackbar;
    M3Handle fonts[2];
    int rc;

    if (widget == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    snackbar = (M3Snackbar *)widget;
    rc = M3_OK;
    if (snackbar->owns_fonts == M3_TRUE) {
        fonts[0] = snackbar->message_font;
        fonts[1] = snackbar->action_font;
        rc = m3_dialog_destroy_fonts(&snackbar->text_backend, fonts, 2u);
    }

    snackbar->message_font.id = 0u;
    snackbar->message_font.generation = 0u;
    snackbar->action_font.id = 0u;
    snackbar->action_font.generation = 0u;
    snackbar->utf8_message = NULL;
    snackbar->message_len = 0u;
    snackbar->utf8_action = NULL;
    snackbar->action_len = 0u;
    snackbar->metrics_valid = M3_FALSE;
    snackbar->owns_fonts = M3_FALSE;
    snackbar->pressed_action = M3_FALSE;
    snackbar->on_action = NULL;
    snackbar->on_action_ctx = NULL;
    snackbar->text_backend.ctx = NULL;
    snackbar->text_backend.vtable = NULL;
    snackbar->widget.ctx = NULL;
    snackbar->widget.vtable = NULL;
    return rc;
}

static const M3WidgetVTable g_m3_snackbar_widget_vtable = {
    m3_snackbar_widget_measure,
    m3_snackbar_widget_layout,
    m3_snackbar_widget_paint,
    m3_snackbar_widget_event,
    m3_snackbar_widget_get_semantics,
    m3_snackbar_widget_destroy
};

int M3_CALL m3_alert_dialog_style_init(M3AlertDialogStyle *style)
{
    M3Color shadow_color;
    int rc;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_text_style_init(&style->title_style);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_text_style_init(&style->body_style);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_text_style_init(&style->action_style);
    if (rc != M3_OK) {
        return rc;
    }

    style->padding.left = M3_ALERT_DIALOG_DEFAULT_PADDING_X;
    style->padding.right = M3_ALERT_DIALOG_DEFAULT_PADDING_X;
    style->padding.top = M3_ALERT_DIALOG_DEFAULT_PADDING_Y;
    style->padding.bottom = M3_ALERT_DIALOG_DEFAULT_PADDING_Y;
    style->corner_radius = M3_ALERT_DIALOG_DEFAULT_CORNER_RADIUS;
    style->min_width = M3_ALERT_DIALOG_DEFAULT_MIN_WIDTH;
    style->max_width = M3_ALERT_DIALOG_DEFAULT_MAX_WIDTH;
    style->title_body_spacing = M3_ALERT_DIALOG_DEFAULT_TITLE_BODY_SPACING;
    style->body_action_spacing = M3_ALERT_DIALOG_DEFAULT_BODY_ACTION_SPACING;
    style->action_spacing = M3_ALERT_DIALOG_DEFAULT_ACTION_SPACING;
    style->action_padding_x = M3_ALERT_DIALOG_DEFAULT_ACTION_PADDING_X;
    style->action_padding_y = M3_ALERT_DIALOG_DEFAULT_ACTION_PADDING_Y;

    style->title_style.size_px = 20;
    style->title_style.weight = 500;
    style->body_style.size_px = 14;
    style->body_style.weight = 400;
    style->action_style.size_px = 14;
    style->action_style.weight = 500;

    style->title_style.color.r = 0.0f;
    style->title_style.color.g = 0.0f;
    style->title_style.color.b = 0.0f;
    style->title_style.color.a = 1.0f;

    style->body_style.color.r = 0.0f;
    style->body_style.color.g = 0.0f;
    style->body_style.color.b = 0.0f;
    style->body_style.color.a = 0.74f;

    style->action_style.color.r = 0.12f;
    style->action_style.color.g = 0.37f;
    style->action_style.color.b = 0.85f;
    style->action_style.color.a = 1.0f;

    style->background_color.r = 1.0f;
    style->background_color.g = 1.0f;
    style->background_color.b = 1.0f;
    style->background_color.a = 1.0f;

    style->scrim_color.r = 0.0f;
    style->scrim_color.g = 0.0f;
    style->scrim_color.b = 0.0f;
    style->scrim_color.a = 0.32f;

    rc = m3_shadow_init(&style->shadow);
    if (rc != M3_OK) {
        return rc;
    }

    shadow_color.r = 0.0f;
    shadow_color.g = 0.0f;
    shadow_color.b = 0.0f;
    shadow_color.a = 0.24f;
    rc = m3_shadow_set(&style->shadow, 0.0f, 8.0f, 16.0f, 0.0f, style->corner_radius, 4u, shadow_color);
    if (rc != M3_OK) {
        return rc;
    }

    style->shadow_enabled = M3_TRUE;
    style->scrim_enabled = M3_TRUE;
    return M3_OK;
}

int M3_CALL m3_alert_dialog_init(M3AlertDialog *dialog, const M3TextBackend *backend, const M3AlertDialogStyle *style,
    const char *utf8_title, m3_usize title_len, const char *utf8_body, m3_usize body_len)
{
    M3Handle fonts[2];
    int rc;
    int cleanup_rc;

    if (dialog == NULL || backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_title == NULL && title_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_body == NULL && body_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_dialog_validate_backend(backend);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_alert_dialog_validate_style(style, M3_TRUE);
    if (rc != M3_OK) {
        return rc;
    }

    dialog->text_backend = *backend;
    dialog->style = *style;
    dialog->utf8_title = utf8_title;
    dialog->title_len = title_len;
    dialog->utf8_body = utf8_body;
    dialog->body_len = body_len;
    dialog->utf8_confirm = NULL;
    dialog->confirm_len = 0u;
    dialog->utf8_dismiss = NULL;
    dialog->dismiss_len = 0u;
    dialog->metrics_valid = M3_FALSE;
    dialog->owns_fonts = M3_TRUE;
    dialog->pressed_confirm = M3_FALSE;
    dialog->pressed_dismiss = M3_FALSE;
    dialog->on_action = NULL;
    dialog->on_action_ctx = NULL;
    dialog->bounds.x = 0.0f;
    dialog->bounds.y = 0.0f;
    dialog->bounds.width = 0.0f;
    dialog->bounds.height = 0.0f;
    dialog->confirm_bounds = dialog->bounds;
    dialog->dismiss_bounds = dialog->bounds;

    dialog->title_font.id = 0u;
    dialog->title_font.generation = 0u;
    dialog->body_font.id = 0u;
    dialog->body_font.generation = 0u;
    dialog->action_font.id = 0u;
    dialog->action_font.generation = 0u;

    rc = m3_text_font_create(backend, &style->title_style, &dialog->title_font);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_text_font_create(backend, &style->body_style, &dialog->body_font);
    if (rc != M3_OK) {
        cleanup_rc = m3_dialog_destroy_fonts(backend, &dialog->title_font, 1u);
        if (cleanup_rc != M3_OK) {
            return cleanup_rc;
        }
        return rc;
    }
    rc = m3_text_font_create(backend, &style->action_style, &dialog->action_font);
    if (rc != M3_OK) {
        fonts[0] = dialog->title_font;
        fonts[1] = dialog->body_font;
        cleanup_rc = m3_dialog_destroy_fonts(backend, fonts, 2u);
        if (cleanup_rc != M3_OK) {
            return cleanup_rc;
        }
        return rc;
    }

    dialog->widget.ctx = dialog;
    dialog->widget.vtable = &g_m3_alert_dialog_widget_vtable;
    dialog->widget.handle.id = 0u;
    dialog->widget.handle.generation = 0u;
    dialog->widget.flags = 0u;
    return M3_OK;
}

int M3_CALL m3_alert_dialog_set_title(M3AlertDialog *dialog, const char *utf8_title, m3_usize title_len)
{
    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_title == NULL && title_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog->utf8_title = utf8_title;
    dialog->title_len = title_len;
    dialog->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_alert_dialog_set_body(M3AlertDialog *dialog, const char *utf8_body, m3_usize body_len)
{
    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_body == NULL && body_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog->utf8_body = utf8_body;
    dialog->body_len = body_len;
    dialog->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_alert_dialog_set_actions(M3AlertDialog *dialog, const char *utf8_confirm, m3_usize confirm_len,
    const char *utf8_dismiss, m3_usize dismiss_len)
{
    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_confirm == NULL && confirm_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_dismiss == NULL && dismiss_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog->utf8_confirm = utf8_confirm;
    dialog->confirm_len = confirm_len;
    dialog->utf8_dismiss = utf8_dismiss;
    dialog->dismiss_len = dismiss_len;
    dialog->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_alert_dialog_set_style(M3AlertDialog *dialog, const M3AlertDialogStyle *style)
{
    M3Handle new_title;
    M3Handle new_body;
    M3Handle new_action;
    M3Handle old_fonts[3];
    M3Handle new_fonts[3];
    int rc;
    int cleanup_rc;

    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_alert_dialog_validate_style(style, M3_TRUE);
    if (rc != M3_OK) {
        return rc;
    }

    new_title.id = 0u;
    new_title.generation = 0u;
    new_body.id = 0u;
    new_body.generation = 0u;
    new_action.id = 0u;
    new_action.generation = 0u;

    rc = m3_text_font_create(&dialog->text_backend, &style->title_style, &new_title);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_text_font_create(&dialog->text_backend, &style->body_style, &new_body);
    if (rc != M3_OK) {
        cleanup_rc = m3_dialog_destroy_fonts(&dialog->text_backend, &new_title, 1u);
        if (cleanup_rc != M3_OK) {
            return cleanup_rc;
        }
        return rc;
    }
    rc = m3_text_font_create(&dialog->text_backend, &style->action_style, &new_action);
    if (rc != M3_OK) {
        new_fonts[0] = new_title;
        new_fonts[1] = new_body;
        cleanup_rc = m3_dialog_destroy_fonts(&dialog->text_backend, new_fonts, 2u);
        if (cleanup_rc != M3_OK) {
            return cleanup_rc;
        }
        return rc;
    }

    if (dialog->owns_fonts == M3_TRUE) {
        old_fonts[0] = dialog->title_font;
        old_fonts[1] = dialog->body_font;
        old_fonts[2] = dialog->action_font;
        rc = m3_dialog_destroy_fonts(&dialog->text_backend, old_fonts, 3u);
        if (rc != M3_OK) {
            new_fonts[0] = new_title;
            new_fonts[1] = new_body;
            new_fonts[2] = new_action;
            cleanup_rc = m3_dialog_destroy_fonts(&dialog->text_backend, new_fonts, 3u);
            if (cleanup_rc != M3_OK) {
                return cleanup_rc;
            }
            return rc;
        }
    }

    dialog->style = *style;
    dialog->title_font = new_title;
    dialog->body_font = new_body;
    dialog->action_font = new_action;
    dialog->owns_fonts = M3_TRUE;
    dialog->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_alert_dialog_set_on_action(M3AlertDialog *dialog, M3AlertDialogOnAction on_action, void *ctx)
{
    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog->on_action = on_action;
    dialog->on_action_ctx = ctx;
    return M3_OK;
}

int M3_CALL m3_fullscreen_dialog_style_init(M3FullScreenDialogStyle *style)
{
    M3Color shadow_color;
    int rc;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_text_style_init(&style->title_style);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_text_style_init(&style->body_style);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_text_style_init(&style->action_style);
    if (rc != M3_OK) {
        return rc;
    }

    style->padding.left = M3_FULLSCREEN_DIALOG_DEFAULT_PADDING_X;
    style->padding.right = M3_FULLSCREEN_DIALOG_DEFAULT_PADDING_X;
    style->padding.top = M3_FULLSCREEN_DIALOG_DEFAULT_PADDING_Y;
    style->padding.bottom = M3_FULLSCREEN_DIALOG_DEFAULT_PADDING_Y;
    style->corner_radius = M3_FULLSCREEN_DIALOG_DEFAULT_CORNER_RADIUS;
    style->title_body_spacing = M3_FULLSCREEN_DIALOG_DEFAULT_TITLE_BODY_SPACING;
    style->body_action_spacing = M3_FULLSCREEN_DIALOG_DEFAULT_BODY_ACTION_SPACING;
    style->action_padding_x = M3_FULLSCREEN_DIALOG_DEFAULT_ACTION_PADDING_X;
    style->action_padding_y = M3_FULLSCREEN_DIALOG_DEFAULT_ACTION_PADDING_Y;

    style->title_style.size_px = 20;
    style->title_style.weight = 500;
    style->body_style.size_px = 14;
    style->body_style.weight = 400;
    style->action_style.size_px = 14;
    style->action_style.weight = 500;

    style->title_style.color.r = 0.0f;
    style->title_style.color.g = 0.0f;
    style->title_style.color.b = 0.0f;
    style->title_style.color.a = 1.0f;

    style->body_style.color.r = 0.0f;
    style->body_style.color.g = 0.0f;
    style->body_style.color.b = 0.0f;
    style->body_style.color.a = 0.74f;

    style->action_style.color.r = 0.12f;
    style->action_style.color.g = 0.37f;
    style->action_style.color.b = 0.85f;
    style->action_style.color.a = 1.0f;

    style->background_color.r = 1.0f;
    style->background_color.g = 1.0f;
    style->background_color.b = 1.0f;
    style->background_color.a = 1.0f;

    style->scrim_color.r = 0.0f;
    style->scrim_color.g = 0.0f;
    style->scrim_color.b = 0.0f;
    style->scrim_color.a = 0.0f;

    rc = m3_shadow_init(&style->shadow);
    if (rc != M3_OK) {
        return rc;
    }

    shadow_color.r = 0.0f;
    shadow_color.g = 0.0f;
    shadow_color.b = 0.0f;
    shadow_color.a = 0.24f;
    rc = m3_shadow_set(&style->shadow, 0.0f, 6.0f, 12.0f, 0.0f, style->corner_radius, 3u, shadow_color);
    if (rc != M3_OK) {
        return rc;
    }

    style->shadow_enabled = M3_FALSE;
    style->scrim_enabled = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_fullscreen_dialog_init(M3FullScreenDialog *dialog, const M3TextBackend *backend,
    const M3FullScreenDialogStyle *style, const char *utf8_title, m3_usize title_len, const char *utf8_body, m3_usize body_len)
{
    M3Handle fonts[2];
    int rc;
    int cleanup_rc;

    if (dialog == NULL || backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_title == NULL && title_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_body == NULL && body_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_dialog_validate_backend(backend);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_fullscreen_dialog_validate_style(style, M3_TRUE);
    if (rc != M3_OK) {
        return rc;
    }

    dialog->text_backend = *backend;
    dialog->style = *style;
    dialog->utf8_title = utf8_title;
    dialog->title_len = title_len;
    dialog->utf8_body = utf8_body;
    dialog->body_len = body_len;
    dialog->utf8_action = NULL;
    dialog->action_len = 0u;
    dialog->metrics_valid = M3_FALSE;
    dialog->owns_fonts = M3_TRUE;
    dialog->pressed_action = M3_FALSE;
    dialog->on_action = NULL;
    dialog->on_action_ctx = NULL;
    dialog->bounds.x = 0.0f;
    dialog->bounds.y = 0.0f;
    dialog->bounds.width = 0.0f;
    dialog->bounds.height = 0.0f;
    dialog->action_bounds = dialog->bounds;

    dialog->title_font.id = 0u;
    dialog->title_font.generation = 0u;
    dialog->body_font.id = 0u;
    dialog->body_font.generation = 0u;
    dialog->action_font.id = 0u;
    dialog->action_font.generation = 0u;

    rc = m3_text_font_create(backend, &style->title_style, &dialog->title_font);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_text_font_create(backend, &style->body_style, &dialog->body_font);
    if (rc != M3_OK) {
        cleanup_rc = m3_dialog_destroy_fonts(backend, &dialog->title_font, 1u);
        if (cleanup_rc != M3_OK) {
            return cleanup_rc;
        }
        return rc;
    }
    rc = m3_text_font_create(backend, &style->action_style, &dialog->action_font);
    if (rc != M3_OK) {
        fonts[0] = dialog->title_font;
        fonts[1] = dialog->body_font;
        cleanup_rc = m3_dialog_destroy_fonts(backend, fonts, 2u);
        if (cleanup_rc != M3_OK) {
            return cleanup_rc;
        }
        return rc;
    }

    dialog->widget.ctx = dialog;
    dialog->widget.vtable = &g_m3_fullscreen_dialog_widget_vtable;
    dialog->widget.handle.id = 0u;
    dialog->widget.handle.generation = 0u;
    dialog->widget.flags = 0u;
    return M3_OK;
}

int M3_CALL m3_fullscreen_dialog_set_title(M3FullScreenDialog *dialog, const char *utf8_title, m3_usize title_len)
{
    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_title == NULL && title_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog->utf8_title = utf8_title;
    dialog->title_len = title_len;
    dialog->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_fullscreen_dialog_set_body(M3FullScreenDialog *dialog, const char *utf8_body, m3_usize body_len)
{
    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_body == NULL && body_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog->utf8_body = utf8_body;
    dialog->body_len = body_len;
    dialog->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_fullscreen_dialog_set_action(M3FullScreenDialog *dialog, const char *utf8_action, m3_usize action_len)
{
    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_action == NULL && action_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog->utf8_action = utf8_action;
    dialog->action_len = action_len;
    dialog->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_fullscreen_dialog_set_style(M3FullScreenDialog *dialog, const M3FullScreenDialogStyle *style)
{
    M3Handle new_title;
    M3Handle new_body;
    M3Handle new_action;
    M3Handle old_fonts[3];
    M3Handle new_fonts[3];
    int rc;
    int cleanup_rc;

    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_fullscreen_dialog_validate_style(style, M3_TRUE);
    if (rc != M3_OK) {
        return rc;
    }

    new_title.id = 0u;
    new_title.generation = 0u;
    new_body.id = 0u;
    new_body.generation = 0u;
    new_action.id = 0u;
    new_action.generation = 0u;

    rc = m3_text_font_create(&dialog->text_backend, &style->title_style, &new_title);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_text_font_create(&dialog->text_backend, &style->body_style, &new_body);
    if (rc != M3_OK) {
        cleanup_rc = m3_dialog_destroy_fonts(&dialog->text_backend, &new_title, 1u);
        if (cleanup_rc != M3_OK) {
            return cleanup_rc;
        }
        return rc;
    }
    rc = m3_text_font_create(&dialog->text_backend, &style->action_style, &new_action);
    if (rc != M3_OK) {
        new_fonts[0] = new_title;
        new_fonts[1] = new_body;
        cleanup_rc = m3_dialog_destroy_fonts(&dialog->text_backend, new_fonts, 2u);
        if (cleanup_rc != M3_OK) {
            return cleanup_rc;
        }
        return rc;
    }

    if (dialog->owns_fonts == M3_TRUE) {
        old_fonts[0] = dialog->title_font;
        old_fonts[1] = dialog->body_font;
        old_fonts[2] = dialog->action_font;
        rc = m3_dialog_destroy_fonts(&dialog->text_backend, old_fonts, 3u);
        if (rc != M3_OK) {
            new_fonts[0] = new_title;
            new_fonts[1] = new_body;
            new_fonts[2] = new_action;
            cleanup_rc = m3_dialog_destroy_fonts(&dialog->text_backend, new_fonts, 3u);
            if (cleanup_rc != M3_OK) {
                return cleanup_rc;
            }
            return rc;
        }
    }

    dialog->style = *style;
    dialog->title_font = new_title;
    dialog->body_font = new_body;
    dialog->action_font = new_action;
    dialog->owns_fonts = M3_TRUE;
    dialog->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_fullscreen_dialog_set_on_action(M3FullScreenDialog *dialog, M3FullScreenDialogOnAction on_action, void *ctx)
{
    if (dialog == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    dialog->on_action = on_action;
    dialog->on_action_ctx = ctx;
    return M3_OK;
}

int M3_CALL m3_snackbar_style_init(M3SnackbarStyle *style)
{
    int rc;

    if (style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_text_style_init(&style->message_style);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_text_style_init(&style->action_style);
    if (rc != M3_OK) {
        return rc;
    }

    style->padding.left = M3_SNACKBAR_DEFAULT_PADDING_X;
    style->padding.right = M3_SNACKBAR_DEFAULT_PADDING_X;
    style->padding.top = M3_SNACKBAR_DEFAULT_PADDING_Y;
    style->padding.bottom = M3_SNACKBAR_DEFAULT_PADDING_Y;
    style->corner_radius = M3_SNACKBAR_DEFAULT_CORNER_RADIUS;
    style->min_width = M3_SNACKBAR_DEFAULT_MIN_WIDTH;
    style->max_width = M3_SNACKBAR_DEFAULT_MAX_WIDTH;
    style->min_height = M3_SNACKBAR_DEFAULT_MIN_HEIGHT;
    style->action_spacing = M3_SNACKBAR_DEFAULT_ACTION_SPACING;
    style->action_padding_x = M3_SNACKBAR_DEFAULT_ACTION_PADDING_X;
    style->action_padding_y = M3_SNACKBAR_DEFAULT_ACTION_PADDING_Y;

    style->message_style.size_px = 14;
    style->message_style.weight = 400;
    style->action_style.size_px = 14;
    style->action_style.weight = 500;

    style->message_style.color.r = 1.0f;
    style->message_style.color.g = 1.0f;
    style->message_style.color.b = 1.0f;
    style->message_style.color.a = 1.0f;

    style->action_style.color.r = 0.67f;
    style->action_style.color.g = 0.85f;
    style->action_style.color.b = 1.0f;
    style->action_style.color.a = 1.0f;

    style->background_color.r = 0.2f;
    style->background_color.g = 0.2f;
    style->background_color.b = 0.2f;
    style->background_color.a = 1.0f;

    return M3_OK;
}

int M3_CALL m3_snackbar_init(M3Snackbar *snackbar, const M3TextBackend *backend, const M3SnackbarStyle *style,
    const char *utf8_message, m3_usize message_len)
{
    int rc;
    int cleanup_rc;

    if (snackbar == NULL || backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_message == NULL && message_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_dialog_validate_backend(backend);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_snackbar_validate_style(style, M3_TRUE);
    if (rc != M3_OK) {
        return rc;
    }

    snackbar->text_backend = *backend;
    snackbar->style = *style;
    snackbar->utf8_message = utf8_message;
    snackbar->message_len = message_len;
    snackbar->utf8_action = NULL;
    snackbar->action_len = 0u;
    snackbar->metrics_valid = M3_FALSE;
    snackbar->owns_fonts = M3_TRUE;
    snackbar->pressed_action = M3_FALSE;
    snackbar->on_action = NULL;
    snackbar->on_action_ctx = NULL;
    snackbar->bounds.x = 0.0f;
    snackbar->bounds.y = 0.0f;
    snackbar->bounds.width = 0.0f;
    snackbar->bounds.height = 0.0f;
    snackbar->action_bounds = snackbar->bounds;

    snackbar->message_font.id = 0u;
    snackbar->message_font.generation = 0u;
    snackbar->action_font.id = 0u;
    snackbar->action_font.generation = 0u;

    rc = m3_text_font_create(backend, &style->message_style, &snackbar->message_font);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_text_font_create(backend, &style->action_style, &snackbar->action_font);
    if (rc != M3_OK) {
        cleanup_rc = m3_dialog_destroy_fonts(backend, &snackbar->message_font, 1u);
        if (cleanup_rc != M3_OK) {
            return cleanup_rc;
        }
        return rc;
    }

    snackbar->widget.ctx = snackbar;
    snackbar->widget.vtable = &g_m3_snackbar_widget_vtable;
    snackbar->widget.handle.id = 0u;
    snackbar->widget.handle.generation = 0u;
    snackbar->widget.flags = 0u;
    return M3_OK;
}

int M3_CALL m3_snackbar_set_message(M3Snackbar *snackbar, const char *utf8_message, m3_usize message_len)
{
    if (snackbar == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_message == NULL && message_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    snackbar->utf8_message = utf8_message;
    snackbar->message_len = message_len;
    snackbar->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_snackbar_set_action(M3Snackbar *snackbar, const char *utf8_action, m3_usize action_len)
{
    if (snackbar == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_action == NULL && action_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    snackbar->utf8_action = utf8_action;
    snackbar->action_len = action_len;
    snackbar->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_snackbar_set_style(M3Snackbar *snackbar, const M3SnackbarStyle *style)
{
    M3Handle new_message;
    M3Handle new_action;
    M3Handle old_fonts[2];
    M3Handle new_fonts[2];
    int rc;
    int cleanup_rc;

    if (snackbar == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_snackbar_validate_style(style, M3_TRUE);
    if (rc != M3_OK) {
        return rc;
    }

    new_message.id = 0u;
    new_message.generation = 0u;
    new_action.id = 0u;
    new_action.generation = 0u;

    rc = m3_text_font_create(&snackbar->text_backend, &style->message_style, &new_message);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_text_font_create(&snackbar->text_backend, &style->action_style, &new_action);
    if (rc != M3_OK) {
        cleanup_rc = m3_dialog_destroy_fonts(&snackbar->text_backend, &new_message, 1u);
        if (cleanup_rc != M3_OK) {
            return cleanup_rc;
        }
        return rc;
    }

    if (snackbar->owns_fonts == M3_TRUE) {
        old_fonts[0] = snackbar->message_font;
        old_fonts[1] = snackbar->action_font;
        rc = m3_dialog_destroy_fonts(&snackbar->text_backend, old_fonts, 2u);
        if (rc != M3_OK) {
            new_fonts[0] = new_message;
            new_fonts[1] = new_action;
            cleanup_rc = m3_dialog_destroy_fonts(&snackbar->text_backend, new_fonts, 2u);
            if (cleanup_rc != M3_OK) {
                return cleanup_rc;
            }
            return rc;
        }
    }

    snackbar->style = *style;
    snackbar->message_font = new_message;
    snackbar->action_font = new_action;
    snackbar->owns_fonts = M3_TRUE;
    snackbar->metrics_valid = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_snackbar_set_on_action(M3Snackbar *snackbar, M3SnackbarOnAction on_action, void *ctx)
{
    if (snackbar == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    snackbar->on_action = on_action;
    snackbar->on_action_ctx = ctx;
    return M3_OK;
}
