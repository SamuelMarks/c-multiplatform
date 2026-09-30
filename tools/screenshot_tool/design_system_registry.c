/**
 * @file design_system_registry.c
 * @brief Registry implementation for pluggable design system screenshot
 * drivers.
 */

/* clang-format off */
#include "design_system_renderer.h"
#include <string.h>
/* clang-format on */

#define MAX_DRIVERS 16

static const struct design_system_driver *g_drivers[MAX_DRIVERS];
static size_t g_driver_count = 0;

ui_error_t ds_registry_register(const struct design_system_driver *driver) {
  size_t i;
  if (!driver || !driver->name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  for (i = 0; i < g_driver_count; ++i) {
    if (strcmp(g_drivers[i]->name, driver->name) == 0) {
      g_drivers[i] = driver;
      return UI_ERROR_NONE;
    }
  }
  if (g_driver_count >= MAX_DRIVERS) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  g_drivers[g_driver_count++] = driver;
  return UI_ERROR_NONE;
}

ui_error_t ds_registry_find(const char *name,
                            const struct design_system_driver **out_driver) {
  size_t i;
  if (!out_driver) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_driver = NULL;
  if (!name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  for (i = 0; i < g_driver_count; ++i) {
    if (strcmp(g_drivers[i]->name, name) == 0) {
      *out_driver = g_drivers[i];
      return UI_ERROR_NONE;
    }
  }
  return UI_ERROR_NOT_FOUND;
}

ui_error_t
ds_registry_get_all(const struct design_system_driver *const **out_drivers,
                    size_t *out_count) {
  if (!out_drivers || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_drivers = g_drivers;
  *out_count = g_driver_count;
  return UI_ERROR_NONE;
}

ui_error_t ds_registry_reset(void) {
  g_driver_count = 0;
  return UI_ERROR_NONE;
}
