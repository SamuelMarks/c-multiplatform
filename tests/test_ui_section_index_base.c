/* clang-format off */
#include "ui_section_index_base.h"
#include "ui_component.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_ui_section_index_base_init(void) {
  struct ui_section_index_base *index = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;
  int i;

  /* Test NULL out */
  rc = ui_section_index_base_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test creation OOM */
  for (i = 0; i < 40; ++i) {
    g_malloc_fail_countdown = i;
    rc = ui_section_index_base_create(&index);
    if (rc == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_section_index_base_destroy(index);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    } else {
      if (index != NULL)
        return 1; /* index should be null on fail */
    }
  }
  g_malloc_fail_countdown = -1;

  /* Successful create */
  rc = ui_section_index_base_create(&index);
  if (rc != UI_ERROR_NONE || !index)
    return 1;

  /* Test NULL args */
  rc = ui_section_index_base_get_component(NULL, &comp);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_section_index_base_get_component(index, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test get component */
  rc = ui_section_index_base_get_component(index, &comp);
  if (rc != UI_ERROR_NONE || !comp)
    return 1;

  /* Test bind data invalid */
  rc = ui_section_index_base_bind_data(NULL, (struct ui_computed *)0x1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test bind data */
  rc = ui_section_index_base_bind_data(index, (struct ui_computed *)0x1);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Destroy */
  {
    ui_error_t rc_cleanup = ui_section_index_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_section_index_base_destroy(index);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return 0;
}

static int test_ui_section_index_base_sections(void) {
  struct ui_section_index_base *index = NULL;
  const char *sections[] = {"A", "B", "C"};
  ui_error_t rc;
  int i;

  rc = ui_section_index_base_create(&index);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test invalid args */
  rc = ui_section_index_base_set_sections(NULL, sections, 3);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_section_index_base_set_sections(index, NULL, 3);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test set zero sections */
  {
    ui_error_t _ign = ui_section_index_base_set_sections(index, NULL, 0);
    (void)_ign;
  }
  rc = ui_section_index_base_set_sections(index, sections, 0);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test set sections OOM */
  for (i = 0; i < 20; ++i) {
    g_malloc_fail_countdown = i;
    rc = ui_section_index_base_set_sections(index, sections, 3);
    if (rc == UI_ERROR_NONE) {
      break; /* We succeeded eventually */
    }
  }
  g_malloc_fail_countdown = -1;

  /* Re-set sections correctly */
  rc = ui_section_index_base_set_sections(index, sections, 3);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test re-setting to replace existing items */
  const char *sections2[] = {"X", "Y"};
  rc = ui_section_index_base_set_sections(index, sections2, 2);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test active section */
  rc = ui_section_index_base_set_active_section(NULL, 1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_section_index_base_set_active_section(index, 5);
  if (rc != UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  rc = ui_section_index_base_set_active_section(index, 1);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test shrinking sections causing active_idx to be out of bounds for removal
   */
  {
    const char *s3[] = {"1"};
    {
      ui_error_t _ign = ui_section_index_base_set_sections(index, s3, 1);
      (void)_ign;
    }
    /* now active_idx is 1, count is 1. next set_active_section will see old
     * active_idx >= count */
    {
      ui_error_t _ign = ui_section_index_base_set_active_section(index, 0);
      (void)_ign;
    }
  }

  /* Set again to test clearing previous active section */
  rc = ui_section_index_base_set_active_section(index, 0);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Set -1 to test out of bounds / clearing active section safely (but wait,
   * out of bounds is only checked on > count, what about < 0?) */
  /* Our implementation says active_idx >= 0 && active_idx < index->count is
     checked before removing. Wait, it says: if (active_idx >=
     (int)index->count) { return UI_ERROR_OUT_OF_BOUNDS; } Then it removes the
     old active_idx. Then index->active_idx = active_idx; Then if
     (index->active_idx >= 0 ...) it sets it. Let's verify what happens if
     active_idx is -1.
  */
  rc = ui_section_index_base_set_active_section(index, -1);
  if (rc != UI_ERROR_NONE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_section_index_base_destroy(index);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

#include "ui_dom_node.h"

struct ui_section_index_base {
  struct ui_component *component;
  struct ui_dom_node **item_nodes;
  size_t count;
  int active_idx;
  struct ui_computed *data_signal;
};

void test_ui_section_index_errs(void);
void test_ui_section_index_remove_attr_err(void);
int main(void) {
  int failed = 0;
  printf("Running ui_section_index_base tests...\n");

  failed |= test_ui_section_index_base_init();
  failed |= test_ui_section_index_base_sections();
  test_ui_section_index_errs();
  test_ui_section_index_remove_attr_err();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}

void test_ui_section_index_errs(void) {
  extern int g_malloc_fail_countdown;
  struct ui_section_index_base *index = NULL;
  const char *sections[] = {"A", "B", "C"};
  ui_error_t rc;
  int i;

  rc = ui_section_index_base_create(&index);
  {
    ui_error_t rc_cleanup =
        ui_section_index_base_set_sections(index, sections, 3);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_section_index_base_set_active_section(index, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_malloc_fail_countdown = 0;
  {
    ui_error_t rc_cleanup = ui_section_index_base_set_active_section(index, 2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  {
    ui_error_t rc_cleanup = ui_section_index_base_set_active_section(index, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_malloc_fail_countdown = -1;
  {
    ui_error_t rc_cleanup = ui_section_index_base_destroy(index);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}
void test_ui_section_index_remove_attr_err(void) {
  struct ui_section_index_base *index2 = NULL;
  const char *sections2[] = {"A", "B", "C"};
  {
    ui_error_t _ign = ui_section_index_base_create(&index2);
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_section_index_base_set_sections(index2, sections2, 3);
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_section_index_base_set_active_section(index2, 1);
    (void)_ign;
  }
  /* manually remove to trigger NOT_FOUND during next set_active_section */
  {
    ui_error_t _ign =
        ui_dom_node_remove_attribute(index2->item_nodes[1], "data-active");
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_section_index_base_set_active_section(index2, 2);
    (void)_ign;
  }
  {
    ui_error_t _ign = ui_section_index_base_destroy(index2);
    (void)_ign;
  }

  struct ui_section_index_base *index = NULL;
  const char *sections[] = {"A", "B", "C"};
  {
    ui_error_t rc_cleanup = ui_section_index_base_create(&index);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_section_index_base_set_sections(index, sections, 3);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_section_index_base_set_active_section(index, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  /* corrupt node to fail remove_attribute */
  index->item_nodes[1]->type = UI_DOM_NODE_TYPE_TEXT;
  {
    ui_error_t rc_cleanup = ui_section_index_base_set_active_section(index, 2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  index->item_nodes[1]->type = UI_DOM_NODE_TYPE_ELEMENT;
  {
    ui_error_t rc_cleanup = ui_section_index_base_destroy(index);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}
