/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "../include/ui_dom_node.h"
#include "../src/ui_internal_mem.h"
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_malloc_called;

int main(void) {
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child1 = NULL;
  struct ui_dom_node *child2 = NULL;
  struct ui_dom_node *text_node = NULL;
  enum ui_error rc;
  const char *val;

  printf("Starting test_ui_dom_node...\n");

  /* Test 1: Basic Node Creation and Property Setting */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (rc != UI_ERROR_NONE || !root) {
    printf("Failed to create root node\n");
    return 1;
  }

  rc = ui_dom_node_set_tag_name(root, "div");
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set tag name\n");
    return 1;
  }

  rc = ui_dom_node_set_tag_name(root, "span");
  if (rc != UI_ERROR_NONE) {
    printf("Failed to replace tag name\n");
    return 1;
  }

  if (strcmp(root->tag_name, "span") != 0) {
    printf("Tag name mismatch\n");
    return 1;
  }

  /* Test 2: Attributes */
  rc = ui_dom_node_set_attribute(root, "id", "main");
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set attribute\n");
    return 1;
  }

  rc = ui_dom_node_get_attribute(root, "id", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "main") != 0) {
    printf("Failed to get attribute\n");
    return 1;
  }

  rc = ui_dom_node_set_attribute(root, "class", "container");
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set second attribute\n");
    return 1;
  }

  /* Update existing attribute */
  rc = ui_dom_node_set_attribute(root, "id", "new-main");
  if (rc != UI_ERROR_NONE) {
    printf("Failed to update attribute\n");
    return 1;
  }

  rc = ui_dom_node_get_attribute(root, "id", &val);
  if (rc != UI_ERROR_NONE || strcmp(val, "new-main") != 0) {
    printf("Failed to get updated attribute\n");
    return 1;
  }

  /* Update existing attribute mock failure */
  g_malloc_fail_countdown = 0;
  rc = ui_dom_node_set_attribute(root, "id", "newer-main");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on update attribute\n");
    return 1;
  }

  /* Test Remove Attribute */
  rc = ui_dom_node_remove_attribute(root, "class");
  if (rc != UI_ERROR_NONE) {
    printf("Failed to remove attribute\n");
    return 1;
  }

  rc = ui_dom_node_get_attribute(root, "class", &val);
  if (rc != UI_ERROR_NOT_FOUND) {
    printf("Attribute should be removed\n");
    return 1;
  }

  /* Remove head attribute (id) */
  rc = ui_dom_node_remove_attribute(root, "id");
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Add three attributes to test middle and tail removal */
  ui_dom_node_set_attribute(root, "first", "1");
  ui_dom_node_set_attribute(root, "second", "2");
  ui_dom_node_set_attribute(root, "third", "3");

  rc = ui_dom_node_remove_attribute(root, "second"); /* middle */
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_dom_node_remove_attribute(root, "third"); /* tail */
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Remove non-existent attribute */
  rc = ui_dom_node_remove_attribute(root, "non-existent");
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test 3: Tree Building */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create child1\n");
    return 1;
  }
  ui_dom_node_set_tag_name(child1, "span");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create child2\n");
    return 1;
  }
  ui_dom_node_set_tag_name(child2, "a");

  rc = ui_dom_node_append_child(root, child1);
  if (rc != UI_ERROR_NONE || root->first_child != child1 ||
      root->last_child != child1) {
    printf("Failed to append child1\n");
    return 1;
  }

  rc = ui_dom_node_append_child(root, child2);
  if (rc != UI_ERROR_NONE || root->last_child != child2 ||
      child1->next_sibling != child2) {
    printf("Failed to append child2\n");
    return 1;
  }

  /* Append child that already has parent */
  if (ui_dom_node_append_child(root, child2) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT when child has parent\n");
    return 1;
  }

  /* Test 4: Text Nodes */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create text node\n");
    return 1;
  }

  rc = ui_dom_node_set_text_content(text_node, "Hello World");
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set text content\n");
    return 1;
  }

  /* Test replacing text content */
  rc = ui_dom_node_set_text_content(text_node, "Hello World!");
  if (rc != UI_ERROR_NONE)
    return 1;

  if (strcmp(text_node->text_content, "Hello World!") != 0) {
    printf("Text content mismatch\n");
    return 1;
  }

  ui_dom_node_append_child(child1, text_node);

  /* Test remove child */
  if (ui_dom_node_remove_child(NULL, child1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_remove_child(root, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_remove_child(child2, child1) != UI_ERROR_INVALID_ARGUMENT)
    return 1; /* wrong parent */

  /* Re-append for next tests */
  {
    struct ui_dom_node *child3;
    ui_dom_node_append_child(root, child1);
    ui_dom_node_append_child(root, child2);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child3);
    ui_dom_node_append_child(root, child3);

    /* Currently list is: child1 -> child2 -> child3 */

    /* Remove from middle to hit child->previous_sibling branch */
    rc = ui_dom_node_remove_child(root, child2);
    if (rc != UI_ERROR_NONE || root->first_child != child1 ||
        root->last_child != child3) {
      printf("Failed to remove child2\n");
      return 1;
    }

    /* Remove from beginning */
    rc = ui_dom_node_remove_child(root, child1);
    if (rc != UI_ERROR_NONE || root->first_child != child3) {
      return 1;
    }

    /* Remove last remaining */
    rc = ui_dom_node_remove_child(root, child3);
    if (rc != UI_ERROR_NONE || root->first_child != NULL ||
        root->last_child != NULL) {
      printf("Failed to remove child3\n");
      return 1;
    }

    ui_dom_node_destroy(child1);
    ui_dom_node_destroy(child2);
    ui_dom_node_destroy(child3);
  }

  /* Re-append for cleanup */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
  ui_dom_node_append_child(root, child1);
  ui_dom_node_append_child(root, child2);

  /* Test Event Listeners */
  if (ui_dom_node_add_event_listener(NULL, UI_EVENT_CLICK,
                                     (ui_event_handler_t)1,
                                     NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_add_event_listener(root, UI_EVENT_CLICK, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_dom_node_remove_event_listener(NULL, UI_EVENT_CLICK,
                                        (ui_event_handler_t)1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_remove_event_listener(root, UI_EVENT_CLICK, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_dom_node_add_event_listener(root, UI_EVENT_CLICK,
                                      (ui_event_handler_t)1, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_dom_node_add_event_listener(root, UI_EVENT_KEY_DOWN,
                                      (ui_event_handler_t)2, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_dom_node_add_event_listener(root, UI_EVENT_MOUSE_DOWN,
                                      (ui_event_handler_t)3, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_dom_node_remove_event_listener(
          root, UI_EVENT_KEY_UP, (ui_event_handler_t)1) != UI_ERROR_NOT_FOUND)
    return 1;
  if (ui_dom_node_remove_event_listener(
          root, UI_EVENT_CLICK, (ui_event_handler_t)2) != UI_ERROR_NOT_FOUND)
    return 1;

  /* Remove middle */
  rc = ui_dom_node_remove_event_listener(root, UI_EVENT_KEY_DOWN,
                                         (ui_event_handler_t)2);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Add duplicate */
  ui_dom_node_add_event_listener(root, UI_EVENT_MOUSE_DOWN,
                                 (ui_event_handler_t)3, NULL);

  /* Remove head */
  rc = ui_dom_node_remove_event_listener(root, UI_EVENT_MOUSE_DOWN,
                                         (ui_event_handler_t)3);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Remove tail */
  rc = ui_dom_node_remove_event_listener(root, UI_EVENT_CLICK,
                                         (ui_event_handler_t)1);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Mock alloc failure for event listener */
  g_malloc_fail_countdown = 0;
  rc = ui_dom_node_add_event_listener(root, UI_EVENT_MOUSE_DOWN,
                                      (ui_event_handler_t)3, NULL);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  /* Let the remaining listener be destroyed by ui_dom_node_destroy */

  /* Clean up the whole tree */
  ui_dom_node_destroy(root);
  ui_dom_node_destroy(NULL);

  /* Test 5: Error Percolation and Null Checks */
  if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on null out_node\n");
    return 1;
  }

  if (ui_dom_node_append_child(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on null parent\n");
    return 1;
  }
  if (ui_dom_node_append_child(root, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  if (ui_dom_node_set_attribute(root, NULL, "val") !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Expected INVALID_ARGUMENT on null attribute name\n");
    return 1;
  }
  if (ui_dom_node_set_attribute(root, "a", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_set_attribute(NULL, "a", "val") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_get_attribute(NULL, "a", &val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_get_attribute(root, NULL, &val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_get_attribute(root, "a", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_remove_attribute(NULL, "a") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_remove_attribute(root, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_set_tag_name(NULL, "div") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_set_tag_name(root, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_set_text_content(NULL, "text") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_set_text_content(root, "text") != UI_ERROR_INVALID_ARGUMENT)
    return 1; /* root is element, not text */

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
  if (ui_dom_node_set_tag_name(text_node, "div") != UI_ERROR_INVALID_ARGUMENT)
    return 1; /* text is not element */
  if (ui_dom_node_set_attribute(text_node, "id", "a") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_get_attribute(text_node, "id", &val) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_remove_attribute(text_node, "id") !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dom_node_set_text_content(text_node, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_dom_node_destroy(text_node);

  /* Comment node text content test */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_COMMENT, &text_node);
  if (ui_dom_node_set_text_content(text_node, "comment") != UI_ERROR_NONE)
    return 1;
  ui_dom_node_destroy(text_node);

  ui_dom_node_destroy(root);

  /* Test 6: Mock Malloc Failures */
  g_malloc_fail_countdown = 0;
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  g_malloc_fail_countdown = -1; /* reset */
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on create\n");
    return 1;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  g_malloc_fail_countdown = 0;
  rc = ui_dom_node_set_tag_name(root, "div");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on set_tag_name\n");
    return 1;
  }

  /* Attribute creation mock failure - fail on first strdup */
  g_malloc_fail_countdown = 0;
  rc = ui_dom_node_set_attribute(root, "class", "box");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on set_attribute (1)\n");
    return 1;
  }

  /* Attribute creation mock failure - fail on second strdup */
  g_malloc_fail_countdown = 1;
  rc = ui_dom_node_set_attribute(root, "class", "box");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on set_attribute (2)\n");
    return 1;
  }

  /* Attribute creation mock failure - fail on struct alloc */
  g_malloc_fail_countdown = 2;
  rc = ui_dom_node_set_attribute(root, "class", "box");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OUT_OF_MEMORY on set_attribute (3)\n");
    return 1;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
  g_malloc_fail_countdown = 0;
  rc = ui_dom_node_set_text_content(text_node, "text");
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  ui_dom_node_destroy(text_node);

  ui_dom_node_destroy(root);

  printf("All dom node tests passed.\n");
  return 0;
}
