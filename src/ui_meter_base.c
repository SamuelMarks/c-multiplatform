/* clang-format off */
#include "ui_meter_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

enum ui_error ui_meter_base_create(struct ui_meter_base **out_meter) {
  struct ui_meter_base *meter;
  struct ui_component *base_comp;
  enum ui_error err;

  if (!out_meter) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  meter = (struct ui_meter_base *)UI_MALLOC(sizeof(struct ui_meter_base));
  if (!meter) {
    ui_component_destroy(base_comp);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  meter->base = *base_comp;
  UI_FREE(base_comp);

  meter->value = 0.0f;
  meter->min_val = 0.0f;
  meter->max_val = 1.0f;
  meter->low_val = 0.0f;
  meter->high_val = 1.0f;
  meter->optimum_val = 0.5f;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &meter->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    UI_FREE(meter);
    return err;
  }

  err = ui_dom_node_set_tag_name(meter->base.shadow_root, "ui-meter");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(meter->base.shadow_root);
    UI_FREE(meter);
    return err;
  }

  ui_dom_node_set_attribute(meter->base.shadow_root, "role", "meter");
  ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuemin", "0");
  ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuemax", "1");
  ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuenow", "0");

  *out_meter = meter;
  return UI_ERROR_NONE;
}

static enum ui_error update_meter_attributes(struct ui_meter_base *meter) {
  char buf[32];

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->value);
#else
  sprintf(buf, "%f", meter->value);
#endif
  ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuenow", buf);
  ui_dom_node_set_attribute(meter->base.shadow_root, "value", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->min_val);
#else
  sprintf(buf, "%f", meter->min_val);
#endif
  ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuemin", buf);
  ui_dom_node_set_attribute(meter->base.shadow_root, "min", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->max_val);
#else
  sprintf(buf, "%f", meter->max_val);
#endif
  ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuemax", buf);
  ui_dom_node_set_attribute(meter->base.shadow_root, "max", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->low_val);
#else
  sprintf(buf, "%f", meter->low_val);
#endif
  ui_dom_node_set_attribute(meter->base.shadow_root, "low", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->high_val);
#else
  sprintf(buf, "%f", meter->high_val);
#endif
  ui_dom_node_set_attribute(meter->base.shadow_root, "high", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->optimum_val);
#else
  sprintf(buf, "%f", meter->optimum_val);
#endif
  ui_dom_node_set_attribute(meter->base.shadow_root, "optimum", buf);
  return UI_ERROR_NONE;
}

enum ui_error ui_meter_base_set_value(struct ui_meter_base *meter,
                                      float value) {
  if (!meter) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  meter->value = value;
  (void)update_meter_attributes(meter);
  return UI_ERROR_NONE;
}

enum ui_error ui_meter_base_set_bounds(struct ui_meter_base *meter,
                                       float min_val, float max_val) {
  if (!meter) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  meter->min_val = min_val;
  meter->max_val = max_val;
  (void)update_meter_attributes(meter);
  return UI_ERROR_NONE;
}

enum ui_error ui_meter_base_set_thresholds(struct ui_meter_base *meter,
                                           float low_val, float high_val,
                                           float optimum_val) {
  if (!meter) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  meter->low_val = low_val;
  meter->high_val = high_val;
  meter->optimum_val = optimum_val;
  (void)update_meter_attributes(meter);
  return UI_ERROR_NONE;
}

enum ui_error ui_meter_base_bind_value(struct ui_meter_base *widget,
                                       struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->value_signal = signal;
  return UI_ERROR_NONE;
}
