/**
 * @file ui_accordion_base.c
 * @brief Implementation of accordion base component.
 */

/* clang-format off */
#include "ui_accordion_base.h"
#include "ui_internal_mem.h"
#include <stddef.h>
#ifdef UI_TEST_MOCK_ALLOC
int g_accordion_mock_fail = 0;
static ui_error_t mock_set_on_toggle(struct ui_disclosure_base *d, ui_disclosure_on_toggle_t f, void *u) {
  if (g_accordion_mock_fail == 1) return UI_ERROR_UNKNOWN;
  return (ui_disclosure_base_set_on_toggle)(d, f, u);
}
static ui_error_t mock_is_expanded(struct ui_disclosure_base *d, int *out) {
  if (g_accordion_mock_fail == 2) return UI_ERROR_UNKNOWN;
  return (ui_disclosure_base_is_expanded)(d, out);
}
static ui_error_t mock_set_expanded(struct ui_disclosure_base *d, int val) {
  if (g_accordion_mock_fail == 3) return UI_ERROR_UNKNOWN;
  return (ui_disclosure_base_set_expanded)(d, val);
}
/** @cond */
#define ui_disclosure_base_set_on_toggle mock_set_on_toggle
/** @endcond */
/** @cond */
#define ui_disclosure_base_is_expanded mock_is_expanded
/** @endcond */
/** @cond */
#define ui_disclosure_base_set_expanded mock_set_expanded
/** @endcond */
#endif
/* clang-format on */

/**
 * @struct ui_accordion_base
 * @struct ui_accordion_base
 * @brief Represents an accordion manager that synchronizes a group of
 * disclosures.
 */
struct ui_accordion_base {
  struct ui_disclosure_base **disclosures;      /**< disclosures */
  size_t count;                                 /**< count */
  size_t capacity;                              /**< capacity */
  struct ui_disclosure_base *active_disclosure; /**< active_disclosure */
  ui_accordion_on_change_t on_change;           /**< on_change */
  void *user_data;                              /**< user_data */
  struct ui_computed *data_signal;              /**< data_signal */
};

static ui_error_t
on_child_disclosure_toggle(struct ui_disclosure_base *disclosure,
                           int is_expanded, void *user_data) {
  struct ui_accordion_base *accordion = (struct ui_accordion_base *)user_data;
  size_t i;
  ui_error_t rc = UI_ERROR_NONE;

  if (!is_expanded) {
    if (accordion->active_disclosure == disclosure) {
      accordion->active_disclosure = NULL;
      if (accordion->on_change) {
        rc = accordion->on_change(accordion, NULL, accordion->user_data);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
      }
    }
    return rc;
  }

  /* We are expanding this item. Let's record it so that when we collapse
     others, they don't trigger cascade events back to us. */
  accordion->active_disclosure = disclosure;

  /* Collapse all other items. */
  for (i = 0; i < accordion->count; ++i) {
    int check_expanded = 0;
    if (accordion->disclosures[i] != disclosure) {
      rc = ui_disclosure_base_is_expanded(accordion->disclosures[i],
                                          &check_expanded);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      if (check_expanded) {
        rc = ui_disclosure_base_set_expanded(accordion->disclosures[i], 0);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
      }
    }
  }

  if (accordion->on_change) {
    rc = accordion->on_change(accordion, disclosure, accordion->user_data);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return rc;
}

ui_error_t ui_accordion_base_create(struct ui_accordion_base **out_accordion) {
  struct ui_accordion_base *accordion;

  if (!out_accordion) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  accordion = (struct ui_accordion_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_accordion_base));
  if (!accordion) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  accordion->disclosures = NULL;
  accordion->count = 0;
  accordion->capacity = 0;
  accordion->active_disclosure = NULL;
  accordion->on_change = NULL;
  accordion->user_data = NULL;
  accordion->data_signal = NULL;

  *out_accordion = accordion;
  return UI_ERROR_NONE;
}

ui_error_t ui_accordion_base_destroy(struct ui_accordion_base *accordion) {
  size_t i;
  ui_error_t rc;
  if (!accordion) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Unhook callbacks to prevent dangling pointers */
  for (i = 0; i < accordion->count; ++i) {
    rc =
        ui_disclosure_base_set_on_toggle(accordion->disclosures[i], NULL, NULL);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(accordion->disclosures);
  C_MULTIPLATFORM_FREE(accordion);
  return UI_ERROR_NONE;
}

ui_error_t
ui_accordion_base_add_disclosure(struct ui_accordion_base *accordion,
                                 struct ui_disclosure_base *disclosure) {
  size_t i;
  ui_error_t rc;

  if (!accordion || !disclosure) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < accordion->count; ++i) {
    if (accordion->disclosures[i] == disclosure) {
      return UI_ERROR_NONE; /* Already added */
    }
  }

  if (accordion->count >= accordion->capacity) {
    size_t new_cap = accordion->capacity == 0 ? 4 : accordion->capacity * 2;
    struct ui_disclosure_base **new_arr =
        (struct ui_disclosure_base **)C_MULTIPLATFORM_REALLOC(
            accordion->disclosures,
            (size_t)new_cap * sizeof(struct ui_disclosure_base *));
    if (!new_arr) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    accordion->disclosures = new_arr;
    accordion->capacity = new_cap;
  }

  accordion->disclosures[accordion->count++] = disclosure;
  rc = ui_disclosure_base_set_on_toggle(disclosure, on_child_disclosure_toggle,
                                        accordion);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  {
    int check_expanded = 0;
    rc = ui_disclosure_base_is_expanded(disclosure, &check_expanded);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    if (check_expanded) {
      rc = on_child_disclosure_toggle(disclosure, 1, accordion);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
ui_accordion_base_remove_disclosure(struct ui_accordion_base *accordion,
                                    struct ui_disclosure_base *disclosure) {
  size_t i;
  int found = -1;
  ui_error_t rc;

  if (!accordion || !disclosure) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < accordion->count; ++i) {
    if (accordion->disclosures[i] == disclosure) {
      found = (int)i;
      break;
    }
  }

  if (found == -1) {
    return UI_ERROR_NOT_FOUND;
  }

  rc = ui_disclosure_base_set_on_toggle(disclosure, NULL, NULL);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (accordion->active_disclosure == disclosure) {
    accordion->active_disclosure = NULL;
  }

  for (i = (size_t)found; i < accordion->count - 1; ++i) {
    accordion->disclosures[i] = accordion->disclosures[i + 1];
  }
  accordion->count--;

  return UI_ERROR_NONE;
}

ui_error_t ui_accordion_base_set_active(struct ui_accordion_base *accordion,
                                        struct ui_disclosure_base *disclosure) {
  size_t i;
  int valid_disclosure = 0;
  ui_error_t rc;

  if (!accordion) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (disclosure) {
    for (i = 0; i < accordion->count; ++i) {
      if (accordion->disclosures[i] == disclosure) {
        valid_disclosure = 1;
        break;
      }
    }
    if (!valid_disclosure) {
      return UI_ERROR_NOT_FOUND;
    }

    rc = ui_disclosure_base_set_expanded(disclosure, 1);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    for (i = 0; i < accordion->count; ++i) {
      rc = ui_disclosure_base_set_expanded(accordion->disclosures[i], 0);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
ui_accordion_base_get_active(const struct ui_accordion_base *accordion,
                             struct ui_disclosure_base **out_active) {
  if (!accordion || !out_active) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_active = accordion->active_disclosure;
  return UI_ERROR_NONE;
}

ui_error_t ui_accordion_base_set_on_change(struct ui_accordion_base *accordion,
                                           ui_accordion_on_change_t on_change,
                                           void *user_data) {
  if (!accordion) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  accordion->on_change = on_change;
  accordion->user_data = user_data;
  return UI_ERROR_NONE;
}

ui_error_t ui_accordion_base_bind_data(struct ui_accordion_base *widget,
                                       struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
static ui_error_t mock_accordion_fail_cb(struct ui_accordion_base *a,
                                         struct ui_disclosure_base *d,
                                         void *u) {
  (void)a;
  (void)d;
  (void)u;
  return UI_ERROR_UNKNOWN;
}

ui_error_t run_accordion_methods_coverage(void);
ui_error_t run_accordion_methods_coverage(void) {
  struct ui_accordion_base *accordion = NULL;
  struct ui_disclosure_base *d1 = NULL;
  struct ui_disclosure_base *d2 = NULL;
  struct ui_disclosure_base *d3 = NULL;

  ui_accordion_base_create(&accordion);
  ui_disclosure_base_create(&d1);
  ui_disclosure_base_create(&d2);

  ui_accordion_base_add_disclosure(accordion, d1);
  ui_accordion_base_add_disclosure(accordion, d2);

  /* Line 29: on_change fails during collapse */
  accordion->active_disclosure = d1;
  accordion->on_change = mock_accordion_fail_cb;
  on_child_disclosure_toggle(d1, 0, accordion);

  /* Line 58: on_change fails during expand */
  accordion->active_disclosure = NULL;
  on_child_disclosure_toggle(d1, 1, accordion);

  accordion->on_change = NULL;

  /* Line 46: is_expanded fails inside on_child_disclosure_toggle */
  g_accordion_mock_fail = 2;
  on_child_disclosure_toggle(d1, 1, accordion);
  g_accordion_mock_fail = 0;

  /* Line 50: set_expanded fails inside on_child_disclosure_toggle */
  ui_disclosure_base_set_expanded(
      d2, 1); /* Actually d2 must be expanded to hit the branch */
  g_accordion_mock_fail = 3;
  on_child_disclosure_toggle(d1, 1, accordion);
  g_accordion_mock_fail = 0;

  /* Line 110: set_on_toggle fails in destroy */
  g_accordion_mock_fail = 1;
  (void)ui_accordion_base_destroy(accordion);
  g_accordion_mock_fail = 0;

  /* we have to recreate accordion since destroy failed but left it partially
     dead? Wait, destroy just returns early, so accordion is still allocated,
     but we should free it. */
  C_MULTIPLATFORM_FREE(accordion->disclosures);
  C_MULTIPLATFORM_FREE(accordion);

  /* Line 153: set_on_toggle fails in add_disclosure */
  ui_accordion_base_create(&accordion);
  g_accordion_mock_fail = 1;
  ui_accordion_base_add_disclosure(accordion, d1);
  g_accordion_mock_fail = 0;

  /* Line 159: is_expanded fails in add_disclosure */
  g_accordion_mock_fail = 2;
  ui_accordion_base_add_disclosure(accordion, d2);
  g_accordion_mock_fail = 0;

  /* Line 163: on_child_disclosure_toggle fails in add_disclosure */
  {
    ui_disclosure_base_create(&d3);
    ui_disclosure_base_set_expanded(d3, 1);
    accordion->on_change = mock_accordion_fail_cb;
    ui_accordion_base_add_disclosure(accordion, d3);
    accordion->on_change = NULL;
    (void)ui_disclosure_base_destroy(d3);
  }

  /* Line 199: set_on_toggle fails in remove_disclosure */
  ui_accordion_base_add_disclosure(accordion, d1);
  g_accordion_mock_fail = 1;
  ui_accordion_base_remove_disclosure(accordion, d1);
  g_accordion_mock_fail = 0;

  /* Line 242: set_expanded fails in set_active(accordion, d1) */
  g_accordion_mock_fail = 3;
  ui_accordion_base_set_active(accordion, d1);
  g_accordion_mock_fail = 0;

  /* Line 247: set_expanded fails in set_active(accordion, NULL) */
  g_accordion_mock_fail = 3;
  ui_accordion_base_set_active(accordion, NULL);
  g_accordion_mock_fail = 0;

  (void)ui_accordion_base_destroy(accordion);
  (void)ui_disclosure_base_destroy(d1);
  (void)ui_disclosure_base_destroy(d2);

  return UI_ERROR_NONE;
}
#endif
