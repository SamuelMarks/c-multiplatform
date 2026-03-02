#include "m3/m3_segmented.h"
#include "cmpc/cmp_core.h"
#include <stdlib.h>
#include <string.h>

static int m3_segmented_measure(void *widget, CMPMeasureSpec width,
                                CMPMeasureSpec height, CMPSize *out_size);
static int m3_segmented_layout(void *widget, CMPRect bounds);
static int m3_segmented_paint(void *widget, CMPPaintContext *ctx);
static int m3_segmented_event(void *widget, const CMPInputEvent *event,
                              CMPBool *out_handled);
static int m3_segmented_get_semantics(void *widget,
                                      CMPSemantics *out_semantics);
static int m3_segmented_destroy(void *widget);

static const CMPWidgetVTable m3_segmented_vtable = {
    m3_segmented_measure, m3_segmented_layout,        m3_segmented_paint,
    m3_segmented_event,   m3_segmented_get_semantics, m3_segmented_destroy};

CMP_API int CMP_CALL m3_segmented_style_init(M3SegmentedStyle *style) {
  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  m3_button_style_init_outlined(&style->segment_style);

  style->border_color.r = 0.474f;
  style->border_color.g = 0.454f;
  style->border_color.b = 0.494f;
  style->border_color.a = 1.0f;

  style->corner_radius = 20.0f;
  style->border_width = 1.0f;

  return CMP_OK;
}

static int CMP_CALL m3_segmented_on_button_press(void *ctx, M3Button *button) {
  M3SegmentedGroup *group = (M3SegmentedGroup *)ctx;
  cmp_usize i;

  for (i = 0; i < group->segment_count; i++) {
    if (group->segments[i] == button) {
      if (!group->multi_select) {
        group->selected_index = i;
      }
      if (group->on_change != NULL) {
        return group->on_change(group->on_change_ctx, group, i);
      }
      return CMP_OK;
    }
  }

  return CMP_OK;
}

CMP_API int CMP_CALL m3_segmented_init(M3SegmentedGroup *group,
                                       const M3SegmentedStyle *style,
                                       CMPTextBackend text_backend) {
  if (group == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  group->widget.vtable = &m3_segmented_vtable;
  group->text_backend = text_backend;
  group->style = *style;
  group->segment_count = 0;
  group->capacity = 4;
  group->segments = (M3Button **)malloc(group->capacity * sizeof(M3Button *));
  if (group->segments == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  group->selected_index = 0;
  group->multi_select = CMP_FALSE;

  group->bounds.x = 0.0f;
  group->bounds.y = 0.0f;
  group->bounds.width = 0.0f;
  group->bounds.height = 0.0f;

  group->on_change = NULL;
  group->on_change_ctx = NULL;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_segmented_add(M3SegmentedGroup *group,
                                      const char *utf8_label) {
  M3Button *btn;
  if (group == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (group->segment_count >= group->capacity) {
    cmp_usize new_cap = group->capacity * 2;
    M3Button **new_segs =
        (M3Button **)realloc(group->segments, new_cap * sizeof(M3Button *));
    if (new_segs == NULL) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
    group->segments = new_segs;
    group->capacity = new_cap;
  }

  btn = (M3Button *)malloc(sizeof(M3Button));
  if (btn == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  m3_button_init(btn, &group->text_backend, &group->style.segment_style,
                 utf8_label, utf8_label ? strlen(utf8_label) : 0);
  m3_button_set_on_click(btn, m3_segmented_on_button_press, group);

  group->segments[group->segment_count] = btn;
  group->segment_count += 1;

  return CMP_OK;
}

CMP_API int CMP_CALL m3_segmented_set_on_change(M3SegmentedGroup *group,
                                                M3SegmentedOnChange on_change,
                                                void *ctx) {
  if (group == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  group->on_change = on_change;
  group->on_change_ctx = ctx;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_segmented_set_selected(M3SegmentedGroup *group,
                                               cmp_usize index) {
  if (group == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  group->selected_index = index;
  return CMP_OK;
}

CMP_API int CMP_CALL m3_segmented_test_helper(void) { return CMP_OK; }

static int m3_segmented_measure(void *widget, CMPMeasureSpec width,
                                CMPMeasureSpec height, CMPSize *out_size) {
  M3SegmentedGroup *group = (M3SegmentedGroup *)widget;
  CMPSize child_size;
  cmp_usize i;
  CMPScalar total_w = 0.0f;
  CMPScalar max_h = 0.0f;
  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};

  if (group == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < group->segment_count; i++) {
    group->segments[i]->widget.vtable->measure(&group->segments[i]->widget,
                                               unspec, unspec, &child_size);
    total_w += child_size.width;
    if (child_size.height > max_h) {
      max_h = child_size.height;
    }
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else {
    out_size->width = total_w;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else {
    out_size->height = max_h;
  }

  return CMP_OK;
}

static int m3_segmented_layout(void *widget, CMPRect bounds) {
  M3SegmentedGroup *group = (M3SegmentedGroup *)widget;
  CMPRect child_bounds;
  CMPScalar seg_width;
  CMPScalar current_x;
  cmp_usize i;

  if (group == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  group->bounds = bounds;

  if (group->segment_count == 0) {
    return CMP_OK;
  }

  seg_width = bounds.width / (CMPScalar)group->segment_count;
  current_x = bounds.x;

  for (i = 0; i < group->segment_count; i++) {
    child_bounds.x = current_x;
    child_bounds.y = bounds.y;
    child_bounds.width = seg_width;
    child_bounds.height = bounds.height;
    group->segments[i]->widget.vtable->layout(&group->segments[i]->widget,
                                              child_bounds);
    current_x += seg_width;
  }

  return CMP_OK;
}

static int m3_segmented_paint(void *widget, CMPPaintContext *ctx) {
  M3SegmentedGroup *group = (M3SegmentedGroup *)widget;
  cmp_usize i;

  if (group == NULL || ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (ctx->gfx != NULL) {
    ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &group->bounds,
                                group->style.border_color,
                                group->style.corner_radius);
  }

  for (i = 0; i < group->segment_count; i++) {
    group->segments[i]->widget.vtable->paint(&group->segments[i]->widget, ctx);
  }

  return CMP_OK;
}

static int m3_segmented_event(void *widget, const CMPInputEvent *event,
                              CMPBool *out_handled) {
  M3SegmentedGroup *group = (M3SegmentedGroup *)widget;
  cmp_usize i;

  if (group == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  for (i = 0; i < group->segment_count; i++) {
    CMPBool child_handled = CMP_FALSE;
    group->segments[i]->widget.vtable->event(&group->segments[i]->widget, event,
                                             &child_handled);
    if (child_handled) {
      *out_handled = CMP_TRUE;
      return CMP_OK;
    }
  }

  return CMP_OK;
}

static int m3_segmented_get_semantics(void *widget,
                                      CMPSemantics *out_semantics) {
  M3SegmentedGroup *group = (M3SegmentedGroup *)widget;
  if (group == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = CMP_SEMANTIC_FLAG_FOCUSABLE;
  out_semantics->utf8_label = "Segmented Group";
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_segmented_destroy(void *widget) {
  M3SegmentedGroup *group = (M3SegmentedGroup *)widget;
  cmp_usize i;
  if (group == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (group->segments != NULL) {
    for (i = 0; i < group->segment_count; i++) {
      group->segments[i]->widget.vtable->destroy(&group->segments[i]->widget);
      free(group->segments[i]);
    }
    free(group->segments);
  }
  return CMP_OK;
}
