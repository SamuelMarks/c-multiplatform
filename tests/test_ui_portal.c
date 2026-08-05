/* clang-format off */
#include "../include/ui_portal.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_portal_lifecycle(void) {
  struct ui_dom_node *physical_target = NULL;
  struct ui_dom_node *content_node = NULL;
  struct ui_dom_node *out_content = NULL;
  struct ui_portal *portal = NULL;
  ui_error_t rc;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &physical_target);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_portal_create(&portal, physical_target);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content_node);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_portal_set_content(portal, content_node);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_portal_get_content(portal, &out_content);
  if (rc != UI_ERROR_NONE || out_content != content_node)
    return 1;

  (void)ui_portal_destroy(portal);
  (void)ui_dom_node_destroy(physical_target);

  return 0;
}

static int test_portal_replacement(void) {
  struct ui_dom_node *physical_target = NULL;
  struct ui_dom_node *content1 = NULL;
  struct ui_dom_node *content2 = NULL;
  struct ui_portal *portal = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &physical_target);
  ui_portal_create(&portal, physical_target);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content1);
  ui_portal_set_content(portal, content1);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content2);
  ui_portal_set_content(portal, content2); /* Should destroy content1 */

  ui_portal_destroy(portal); /* Should destroy content2 */
  (void)ui_dom_node_destroy(physical_target);

  return 0;
}

static int test_portal_nulls(void) {
  struct ui_portal *portal = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_dom_node *out_node = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

  if (ui_portal_create(NULL, node) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_portal_create(&portal, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  (void)ui_portal_destroy(NULL);

  ui_portal_create(&portal, node);

  if (ui_portal_set_content(NULL, node) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_portal_set_content(portal, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_portal_get_content(NULL, &out_node) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_portal_get_content(portal, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  (void)ui_portal_destroy(portal);
  (void)ui_dom_node_destroy(node);

  return 0;
}

static int test_portal_oom(void) {
  struct ui_dom_node *node = NULL;
  struct ui_portal *portal = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

  g_malloc_fail_countdown = 0;
  if (ui_portal_create(&portal, node) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  (void)ui_dom_node_destroy(node);
  return 0;
}

static int test_portal_content_moved(void) {
  struct ui_portal *portal = NULL;
  struct ui_dom_node *target = NULL;
  struct ui_dom_node *content = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &target);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content);

  ui_portal_create(&portal, target);
  ui_portal_set_content(portal, content);

  /* Move out manually (detach) */
  ui_dom_node_remove_child(target, content);

  /* Set new content, old content is already detached */
  struct ui_dom_node *content2 = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content2);
  ui_portal_set_content(portal, content2);

  /* Move out manually again (detach) */
  ui_dom_node_remove_child(target, content2);

  /* Destroy, content2 is already detached */
  (void)ui_portal_destroy(portal);

  (void)ui_dom_node_destroy(target);

  return 0;
}
static int test_portal_append_fail(void) {
  struct ui_portal *portal = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_dom_node *content = NULL;
  struct ui_dom_node *dummy_parent = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &content);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dummy_parent);

  ui_portal_create(&portal, node);

  /* Make content un-appendable by giving it a parent */
  ui_dom_node_append_child(dummy_parent, content);

  if (ui_portal_set_content(portal, content) == UI_ERROR_NONE)
    return 1;

  (void)ui_portal_destroy(portal);
  (void)ui_dom_node_destroy(node);
  ui_dom_node_destroy(dummy_parent); /* also destroys content */

  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_portal tests...\n");

  failed |= test_portal_lifecycle();
  failed |= test_portal_replacement();
  failed |= test_portal_nulls();
  failed |= test_portal_oom();
  failed |= test_portal_append_fail();
  failed |= test_portal_content_moved();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
