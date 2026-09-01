/**
 * @file ui_meter_base.c
 * @brief ui_meter_base.c implementation.
 */
/* clang-format off */
#include "ui_meter_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

/**
 * @brief ui_meter_base_create.
 * @param out_meter Parameter out_meter.
 * @return Return value.
 */
ui_error_t ui_meter_base_create(struct ui_meter_base **out_meter) {
  struct ui_meter_base *meter;
  struct ui_component *base_comp;
  ui_error_t err;

  if (!out_meter) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  meter = (struct ui_meter_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_meter_base));
  if (!meter) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(base_comp);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    return UI_ERROR_OUT_OF_MEMORY;
  }

  meter->base = *base_comp;
  C_MULTIPLATFORM_FREE(base_comp);

  meter->value = 0.0f;
  meter->min_val = 0.0f;
  meter->max_val = 1.0f;
  meter->low_val = 0.0f;
  meter->high_val = 1.0f;
  meter->optimum_val = 0.5f;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &meter->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(meter);
    return err;
  }

  err = ui_dom_node_set_tag_name(meter->base.shadow_root, "ui-meter");
  if (err != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(meter->base.shadow_root);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    C_MULTIPLATFORM_FREE(meter);
    return err;
  }

  err = ui_dom_node_set_attribute(meter->base.shadow_root, "role", "meter");
  { (void)err; }
  err =
      ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuemin", "0");
  { (void)err; }
  err =
      ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuemax", "1");
  { (void)err; }
  err =
      ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuenow", "0");
  { (void)err; }

  *out_meter = meter;
  return UI_ERROR_NONE;
}

/**
 * @brief update_meter_attributes.
 * @param meter Parameter meter.
 * @return Return value.
 */
static ui_error_t update_meter_attributes(struct ui_meter_base *meter) {
  char buf[32];
  ui_error_t err;

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->value);
#else
  sprintf(buf, "%f", meter->value);
#endif
  err =
      ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuenow", buf);
  { (void)err; }
  err = ui_dom_node_set_attribute(meter->base.shadow_root, "value", buf);
  { (void)err; }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->min_val);
#else
  sprintf(buf, "%f", meter->min_val);
#endif
  err =
      ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuemin", buf);
  { (void)err; }
  err = ui_dom_node_set_attribute(meter->base.shadow_root, "min", buf);
  { (void)err; }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->max_val);
#else
  sprintf(buf, "%f", meter->max_val);
#endif
  err =
      ui_dom_node_set_attribute(meter->base.shadow_root, "aria-valuemax", buf);
  { (void)err; }
  err = ui_dom_node_set_attribute(meter->base.shadow_root, "max", buf);
  { (void)err; }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->low_val);
#else
  sprintf(buf, "%f", meter->low_val);
#endif
  err = ui_dom_node_set_attribute(meter->base.shadow_root, "low", buf);
  { (void)err; }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->high_val);
#else
  sprintf(buf, "%f", meter->high_val);
#endif
  err = ui_dom_node_set_attribute(meter->base.shadow_root, "high", buf);
  { (void)err; }

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%f", meter->optimum_val);
#else
  sprintf(buf, "%f", meter->optimum_val);
#endif
  err = ui_dom_node_set_attribute(meter->base.shadow_root, "optimum", buf);
  { (void)err; }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_meter_base_set_value.
 * @param meter Parameter meter.
 * @param value Parameter value.
 * @return Return value.
 */
ui_error_t ui_meter_base_set_value(struct ui_meter_base *meter, float value) {
  ui_error_t err;
  if (!meter) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  meter->value = value;
  err = update_meter_attributes(meter);
  { (void)err; }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_meter_base_set_bounds.
 * @param meter Parameter meter.
 * @param min_val Parameter min_val.
 * @param max_val Parameter max_val.
 * @return Return value.
 */
ui_error_t ui_meter_base_set_bounds(struct ui_meter_base *meter, float min_val,
                                    float max_val) {
  ui_error_t err;
  if (!meter) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  meter->min_val = min_val;
  meter->max_val = max_val;
  err = update_meter_attributes(meter);
  { (void)err; }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_meter_base_set_thresholds.
 * @param meter Parameter meter.
 * @param low_val Parameter low_val.
 * @param high_val Parameter high_val.
 * @param optimum_val Parameter optimum_val.
 * @return Return value.
 */
ui_error_t ui_meter_base_set_thresholds(struct ui_meter_base *meter,
                                        float low_val, float high_val,
                                        float optimum_val) {
  ui_error_t err;
  if (!meter) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  meter->low_val = low_val;
  meter->high_val = high_val;
  meter->optimum_val = optimum_val;
  err = update_meter_attributes(meter);
  { (void)err; }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_meter_base_bind_value.
 * @param widget Parameter widget.
 * @param signal Parameter signal.
 * @return Return value.
 */
ui_error_t ui_meter_base_bind_value(struct ui_meter_base *widget,
                                    struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->value_signal = signal;
  return UI_ERROR_NONE;
}
