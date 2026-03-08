#include "m3/m3_adaptive.h"

int CMP_CALL m3_window_size_class(CMPScalar width, cmp_u32 *out_class) {
  if (out_class == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (width < 600.0f) {
    *out_class = M3_WINDOW_CLASS_COMPACT;
  } else if (width < 840.0f) {
    *out_class = M3_WINDOW_CLASS_MEDIUM;
  } else if (width < 1200.0f) {
    *out_class = M3_WINDOW_CLASS_EXPANDED;
  } else {
    *out_class = M3_WINDOW_CLASS_EXTRA_LARGE;
  }
  return CMP_OK;
}

int CMP_CALL m3_adaptive_list_detail_measure(const M3AdaptiveLayout *layout,
                                             CMPScalar *out_width,
                                             CMPScalar *out_height) {
  if (layout == NULL || out_width == NULL || out_height == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (layout->safe_width < 0.0f || layout->safe_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  *out_width = layout->safe_width;
  *out_height = layout->safe_height;
  return CMP_OK;
}

int CMP_CALL m3_adaptive_list_detail_layout(const M3AdaptiveLayout *layout,
                                            CMPRect bounds,
                                            CMPRect *out_primary,
                                            CMPRect *out_secondary,
                                            CMPBool is_rtl) {
  CMPScalar spacing;
  CMPScalar avail_w;
  CMPScalar primary_w;

  if (layout == NULL || out_primary == NULL || out_secondary == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (bounds.width < 0.0f || bounds.height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (layout->hinge.is_separating && (layout->hinge.bounds.height >= layout->hinge.bounds.width || layout->hinge.posture == M3_POSTURE_HALF_OPENED_BOOK)) {
    out_primary->y = bounds.y;
    out_primary->height = bounds.height;
    out_secondary->y = bounds.y;
    out_secondary->height = bounds.height;
    if (is_rtl) {
      out_secondary->x = bounds.x;
      out_secondary->width = layout->hinge.bounds.x - bounds.x;
      out_primary->x = layout->hinge.bounds.x + layout->hinge.bounds.width;
      out_primary->width = bounds.x + bounds.width - out_primary->x;
    } else {
      out_primary->x = bounds.x;
      out_primary->width = layout->hinge.bounds.x - bounds.x;
      out_secondary->x = layout->hinge.bounds.x + layout->hinge.bounds.width;
      out_secondary->width = bounds.x + bounds.width - out_secondary->x;
    }
    if (out_primary->width < 0.0f) out_primary->width = 0.0f;
    if (out_secondary->width < 0.0f) out_secondary->width = 0.0f;
    return CMP_OK;
  }


  if (layout->window_class == M3_WINDOW_CLASS_COMPACT) {
    *out_primary = bounds;
    out_secondary->x = bounds.x;
    out_secondary->y = bounds.y;
    out_secondary->width = 0.0f;
    out_secondary->height = 0.0f;
  } else if (layout->window_class == M3_WINDOW_CLASS_MEDIUM) {
    spacing = 24.0f;
    avail_w = bounds.width - spacing;
    if (avail_w < 0.0f) {
      avail_w = 0.0f;
    }

    out_primary->height = bounds.height;
    out_primary->width = avail_w / 3.0f;
    out_secondary->height = bounds.height;
    out_secondary->width = avail_w - out_primary->width;

    if (is_rtl) {
      out_secondary->x = bounds.x;
      out_primary->x = bounds.x + out_secondary->width + spacing;
    } else {
      out_primary->x = bounds.x;
      out_secondary->x = bounds.x + out_primary->width + spacing;
    }
    out_primary->y = bounds.y;
    out_secondary->y = bounds.y;
  } else { /* EXPANDED or EXTRA_LARGE */
    spacing = 24.0f;
    primary_w = 360.0f;
    if (primary_w > bounds.width) {
      primary_w = bounds.width;
    }

    out_primary->height = bounds.height;
    out_primary->width = primary_w;
    out_secondary->height = bounds.height;
    out_secondary->width = bounds.width - primary_w - spacing;
    if (out_secondary->width < 0.0f) {
      out_secondary->width = 0.0f;
    }

    if (is_rtl) {
      out_secondary->x = bounds.x;
      out_primary->x = bounds.x + out_secondary->width + spacing;
    } else {
      out_primary->x = bounds.x;
      out_secondary->x = bounds.x + primary_w + spacing;
    }
    out_primary->y = bounds.y;
    out_secondary->y = bounds.y;
  }

  return CMP_OK;
}

int CMP_CALL m3_adaptive_feed_layout(const M3AdaptiveLayout *layout,
                                     CMPRect bounds, CMPRect *out_primary,
                                     CMPRect *out_secondary, CMPBool is_rtl) {
  CMPScalar margins, spacing;

  if (layout == NULL || out_primary == NULL || out_secondary == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (bounds.width < 0.0f || bounds.height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (layout->hinge.is_separating && (layout->hinge.bounds.height >= layout->hinge.bounds.width || layout->hinge.posture == M3_POSTURE_HALF_OPENED_BOOK)) {
    out_primary->y = bounds.y;
    out_primary->height = bounds.height;
    out_secondary->y = bounds.y;
    out_secondary->height = bounds.height;
    if (is_rtl) {
      out_secondary->x = bounds.x;
      out_secondary->width = layout->hinge.bounds.x - bounds.x;
      out_primary->x = layout->hinge.bounds.x + layout->hinge.bounds.width;
      out_primary->width = bounds.x + bounds.width - out_primary->x;
    } else {
      out_primary->x = bounds.x;
      out_primary->width = layout->hinge.bounds.x - bounds.x;
      out_secondary->x = layout->hinge.bounds.x + layout->hinge.bounds.width;
      out_secondary->width = bounds.x + bounds.width - out_secondary->x;
    }
    if (out_primary->width < 0.0f) out_primary->width = 0.0f;
    if (out_secondary->width < 0.0f) out_secondary->width = 0.0f;
    return CMP_OK;
  }


  if (layout->window_class == M3_WINDOW_CLASS_COMPACT) {
    *out_primary = bounds;
    out_secondary->x = bounds.x;
    out_secondary->y = bounds.y;
    out_secondary->width = 0.0f;
    out_secondary->height = 0.0f;
  } else if (layout->window_class == M3_WINDOW_CLASS_MEDIUM) {
    margins = 24.0f;
    out_primary->x = bounds.x + margins;
    out_primary->y = bounds.y;
    out_primary->height = bounds.height;
    out_primary->width = bounds.width - margins * 2.0f;
    if (out_primary->width < 0.0f)
      out_primary->width = 0.0f;

    out_secondary->x = bounds.x;
    out_secondary->y = bounds.y;
    out_secondary->width = 0.0f;
    out_secondary->height = 0.0f;
  } else { /* EXPANDED or EXTRA_LARGE */
    margins = 24.0f;
    spacing = 24.0f;

    out_secondary->width = 300.0f;
    if (out_secondary->width > bounds.width) {
      out_secondary->width = bounds.width;
    }

    out_primary->width =
        bounds.width - out_secondary->width - spacing - margins * 2.0f;
    if (out_primary->width < 0.0f)
      out_primary->width = 0.0f;
    if (out_primary->width > 840.0f) { /* Max feed width */
      out_primary->width = 840.0f;
    }

    if (is_rtl) {
      out_secondary->x = bounds.x + margins;
      out_primary->x = bounds.x + bounds.width - margins - out_primary->width; out_secondary->x = out_primary->x - spacing - out_secondary->width;
    } else {
      out_primary->x = bounds.x + margins +
                       (bounds.width - margins * 2.0f - out_primary->width -
                        out_secondary->width - spacing) *
                           0.5f;
      out_secondary->x = out_primary->x + out_primary->width + spacing;
    }
    out_primary->y = bounds.y;
    out_primary->height = bounds.height;
    out_secondary->y = bounds.y;
    out_secondary->height = bounds.height;
  }

  return CMP_OK;
}

int CMP_CALL m3_adaptive_supporting_pane_layout(const M3AdaptiveLayout *layout,
                                                CMPRect bounds,
                                                CMPRect *out_primary,
                                                CMPRect *out_secondary,
                                                CMPBool is_rtl) {
  CMPScalar margins, spacing;

  if (layout == NULL || out_primary == NULL || out_secondary == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (bounds.width < 0.0f || bounds.height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (layout->hinge.is_separating && (layout->hinge.bounds.height >= layout->hinge.bounds.width || layout->hinge.posture == M3_POSTURE_HALF_OPENED_BOOK)) {
    out_primary->y = bounds.y;
    out_primary->height = bounds.height;
    out_secondary->y = bounds.y;
    out_secondary->height = bounds.height;
    if (is_rtl) {
      out_secondary->x = bounds.x;
      out_secondary->width = layout->hinge.bounds.x - bounds.x;
      out_primary->x = layout->hinge.bounds.x + layout->hinge.bounds.width;
      out_primary->width = bounds.x + bounds.width - out_primary->x;
    } else {
      out_primary->x = bounds.x;
      out_primary->width = layout->hinge.bounds.x - bounds.x;
      out_secondary->x = layout->hinge.bounds.x + layout->hinge.bounds.width;
      out_secondary->width = bounds.x + bounds.width - out_secondary->x;
    }
    if (out_primary->width < 0.0f) out_primary->width = 0.0f;
    if (out_secondary->width < 0.0f) out_secondary->width = 0.0f;
    return CMP_OK;
  }


  if (layout->window_class == M3_WINDOW_CLASS_COMPACT ||
      layout->window_class == M3_WINDOW_CLASS_MEDIUM) {
    *out_primary = bounds;
    out_secondary->x = bounds.x;
    out_secondary->y = bounds.y;
    out_secondary->width = 0.0f;
    out_secondary->height = 0.0f;
  } else { /* EXPANDED or EXTRA_LARGE */
    margins = 24.0f;
    spacing = 24.0f;

    out_secondary->width = 360.0f;
    if (out_secondary->width > bounds.width) {
      out_secondary->width = bounds.width;
    }

    out_primary->height = bounds.height;
    out_primary->width =
        bounds.width - out_secondary->width - spacing - margins * 2.0f;
    if (out_primary->width < 0.0f)
      out_primary->width = 0.0f;

    if (is_rtl) {
      out_secondary->x = bounds.x + margins;
      out_primary->x = bounds.x + bounds.width - margins - out_primary->width; out_secondary->x = out_primary->x - spacing - out_secondary->width;
    } else {
      out_primary->x = bounds.x + margins;
      out_secondary->x = out_primary->x + out_primary->width + spacing;
    }
    out_primary->y = bounds.y;
    out_secondary->y = bounds.y;
    out_secondary->height = bounds.height;
  }

  return CMP_OK;
}