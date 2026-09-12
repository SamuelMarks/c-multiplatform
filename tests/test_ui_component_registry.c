/* clang-format off */
#include "ui_component_registry.h"
#include "ui_dom_node.h"
#include "ui_control_value_accessor.h"
#include "ui_button_base.h"
#include "ui_event.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_mock_strcpy_fail;

static int g_custom_factory_called = 0;
static ui_error_t custom_factory(void **out_instance,
                                 struct ui_dom_node **out_dom_node) {
  g_custom_factory_called = 1;
  *out_instance = (void *)0x1234;
  *out_dom_node = NULL;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_event_cb(void *user_data) {
  int *flag = (int *)user_data;
  if (flag) {
    *flag = 1;
  }
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_component_registry *registry = NULL;
  const struct ui_component_vtable *vtable = NULL;
  struct ui_component_vtable custom_vt;
  struct ui_control_value_accessor cva;
  struct ui_dom_node *child_node = NULL;
  struct ui_dom_node *node = NULL;
  void *inst = NULL;
  ui_error_t rc;
  int event_flag = 0;
  int i;

  /* 1. NULL and Invalid Arguments */
  rc = ui_component_registry_create(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_component_registry_destroy(NULL);
  assert(rc == UI_ERROR_NONE);

  rc = ui_component_registry_register(NULL, "test", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_component_registry_create(&registry);
  assert(rc == UI_ERROR_NONE);

  rc = ui_component_registry_register(registry, NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  memset(&custom_vt, 0, sizeof(custom_vt));
  custom_vt.factory = custom_factory;
  rc = ui_component_registry_register(registry, "test", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_component_registry_lookup(NULL, "test", &vtable);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_component_registry_lookup(registry, NULL, &vtable);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_component_registry_lookup(registry, "test", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_component_registry_register_defaults(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_component_registry_get_default(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* 2. OOM & Error branches in create, register, register_defaults */
  g_malloc_fail_countdown = 0;
  rc = ui_component_registry_create(&registry);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  rc = ui_component_registry_create(&registry);
  assert(rc == UI_ERROR_NONE);

  /* OOM on entry */
  g_malloc_fail_countdown = 0;
  rc = ui_component_registry_register(registry, "oom_test", &custom_vt);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* OOM on type_name */
  g_malloc_fail_countdown = 1;
  rc = ui_component_registry_register(registry, "oom_test", &custom_vt);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* UI_STRCPY failure */
  g_mock_strcpy_fail = 1;
  rc = ui_component_registry_register(registry, "strcpy_fail", &custom_vt);
  g_mock_strcpy_fail = 0;
  assert(rc == UI_ERROR_UNKNOWN);

  /* Loop countdown over register_defaults to hit all error returns */
  for (i = 0; i < 40; i++) {
    struct ui_component_registry *temp_reg = NULL;
    rc = ui_component_registry_create(&temp_reg);
    if (rc == UI_ERROR_NONE) {
      g_malloc_fail_countdown = i;
      rc = ui_component_registry_register_defaults(temp_reg);
      g_malloc_fail_countdown = -1;
      ui_component_registry_destroy(temp_reg);
    }
  }

  rc = ui_component_registry_destroy(registry);
  assert(rc == UI_ERROR_NONE);

  /* 3. Normal create and register_defaults */
  rc = ui_component_registry_create(&registry);
  assert(rc == UI_ERROR_NONE);

  rc = ui_component_registry_register_defaults(registry);
  assert(rc == UI_ERROR_NONE);

  /* 4. Widget: ui_button_base */
  rc = ui_component_registry_lookup(registry, "ui_button_base", &vtable);
  assert(rc == UI_ERROR_NONE && vtable != NULL);

  /* OOM in button factory */
  g_malloc_fail_countdown = 0;
  rc = vtable->factory(&inst, &node);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);

  /* Normal button factory */
  rc = vtable->factory(&inst, &node);
  assert(rc == UI_ERROR_NONE && inst != NULL && node != NULL);

  /* set_prop invalid arguments */
  rc = vtable->set_prop(NULL, "text", "Click Me");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->set_prop(inst, NULL, "Click Me");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->set_prop(inst, "text", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* set_prop valid and invalid values */
  rc = vtable->set_prop(inst, "text", "Click Me");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "disabled", "true");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "disabled", "invalid_bool");
  assert(rc != UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "unknown_prop", "val");
  assert(rc == UI_ERROR_NONE);

  /* attach_event invalid arguments */
  rc = vtable->attach_event(NULL, "on_click", dummy_event_cb, &event_flag);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->attach_event(inst, NULL, dummy_event_cb, &event_flag);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->attach_event(inst, "on_click", NULL, &event_flag);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->attach_event(inst, "unknown_event", dummy_event_cb, &event_flag);
  assert(rc == UI_ERROR_NONE);

  /* attach_event OOM */
  g_malloc_fail_countdown = 0;
  rc = vtable->attach_event(inst, "on_click", dummy_event_cb, &event_flag);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* attach_event success and click trigger */
  rc = vtable->set_prop(inst, "disabled", "false");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->attach_event(inst, "on_click", dummy_event_cb, &event_flag);
  assert(rc == UI_ERROR_NONE);
  {
    struct ui_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = UI_EVENT_MOUSE_DOWN;
    ev.event_data.mouse.button = 0;
    ev.event_data.mouse.x = 10;
    ev.event_data.mouse.y = 10;
    rc =
        ui_button_base_process_event((struct ui_button_base *)inst, &ev, 100.0);
    assert(rc == UI_ERROR_NONE);

    ev.type = UI_EVENT_MOUSE_UP;
    ev.event_data.mouse.button = 0;
    ev.event_data.mouse.x = 10;
    ev.event_data.mouse.y = 10;
    rc =
        ui_button_base_process_event((struct ui_button_base *)inst, &ev, 150.0);
    assert(rc == UI_ERROR_NONE);
    assert(event_flag == 1);
  }

  rc = vtable->destroy(inst);
  assert(rc == UI_ERROR_NONE);

  /* destroy button without user_data */
  rc = vtable->factory(&inst, &node);
  assert(rc == UI_ERROR_NONE);
  rc = vtable->destroy(inst);
  assert(rc == UI_ERROR_NONE);
  rc = vtable->destroy(NULL);
  assert(rc == UI_ERROR_NONE);

  /* 5. Widget: ui_input_base */
  rc = ui_component_registry_lookup(registry, "ui_input_base", &vtable);
  assert(rc == UI_ERROR_NONE);

  g_malloc_fail_countdown = 0;
  rc = vtable->factory(&inst, &node);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);

  rc = vtable->factory(&inst, &node);
  assert(rc == UI_ERROR_NONE);

  rc = vtable->set_prop(NULL, "placeholder", "Enter name");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->set_prop(inst, NULL, "Enter name");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->set_prop(inst, "placeholder", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = vtable->set_prop(inst, "placeholder", "Enter name");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "type", "text");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "disabled", "false");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "disabled", "invalid_bool");
  assert(rc != UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "text", "initial");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "value", "updated");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "unknown", "val");
  assert(rc == UI_ERROR_NONE);

  rc = vtable->get_cva(inst, &cva);
  assert(rc == UI_ERROR_NONE);
  rc = vtable->destroy(inst);
  assert(rc == UI_ERROR_NONE);

  /* 6. Widget: ui_card_base */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child_node);
  assert(rc == UI_ERROR_NONE);

  rc = ui_component_registry_lookup(registry, "ui_card_base", &vtable);
  assert(rc == UI_ERROR_NONE);

  g_malloc_fail_countdown = 0;
  rc = vtable->factory(&inst, &node);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);

  rc = vtable->factory(&inst, &node);
  assert(rc == UI_ERROR_NONE);

  rc = vtable->set_prop(NULL, "title", "Title");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->set_prop(inst, NULL, "Title");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->set_prop(inst, "title", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = vtable->set_prop(inst, "title", "Card Title");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "subtitle", "Card Subtitle");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "unknown", "val");
  assert(rc == UI_ERROR_NONE);

  rc = vtable->append_child(NULL, child_node);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->append_child(inst, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->append_child(inst, child_node);
  assert(rc == UI_ERROR_NONE);
  rc = vtable->destroy(inst);
  assert(rc == UI_ERROR_NONE);

  /* 7. Widget: ui_checkbox_base */
  rc = ui_component_registry_lookup(registry, "ui_checkbox_base", &vtable);
  assert(rc == UI_ERROR_NONE);

  g_malloc_fail_countdown = 0;
  rc = vtable->factory(&inst, &node);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);

  rc = vtable->factory(&inst, &node);
  assert(rc == UI_ERROR_NONE);

  rc = vtable->set_prop(NULL, "label", "Accept");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->set_prop(inst, NULL, "Accept");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->set_prop(inst, "label", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = vtable->set_prop(inst, "label", "Accept Terms");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "checked", "true");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "checked", "invalid_bool");
  assert(rc != UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "unknown", "val");
  assert(rc == UI_ERROR_NONE);

  rc = vtable->get_cva(inst, &cva);
  assert(rc == UI_ERROR_NONE);
  rc = vtable->destroy(inst);
  assert(rc == UI_ERROR_NONE);

  /* 8. Widget: ui_label_base */
  rc = ui_component_registry_lookup(registry, "ui_label_base", &vtable);
  assert(rc == UI_ERROR_NONE);

  g_malloc_fail_countdown = 0;
  rc = vtable->factory(&inst, &node);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);

  rc = vtable->factory(&inst, &node);
  assert(rc == UI_ERROR_NONE);

  rc = vtable->set_prop(NULL, "text", "Label");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->set_prop(inst, NULL, "Label");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->set_prop(inst, "text", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = vtable->set_prop(inst, "text", "Username:");
  assert(rc == UI_ERROR_NONE);
  rc = vtable->set_prop(inst, "unknown", "val");
  assert(rc == UI_ERROR_NONE);

  rc = vtable->destroy(inst);
  assert(rc == UI_ERROR_NONE);

  /* 9. Widget: ui_slider_base */
  rc = ui_component_registry_lookup(registry, "ui_slider_base", &vtable);
  assert(rc == UI_ERROR_NONE);

  g_malloc_fail_countdown = 0;
  rc = vtable->factory(&inst, &node);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);

  rc = vtable->factory(&inst, &node);
  assert(rc == UI_ERROR_NONE);

  rc = vtable->get_cva(NULL, &cva);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = vtable->get_cva(inst, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  g_malloc_fail_countdown = 0;
  rc = vtable->get_cva(inst, &cva);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  rc = vtable->get_cva(inst, &cva);
  assert(rc == UI_ERROR_NONE);
  rc = vtable->destroy(inst);
  assert(rc == UI_ERROR_NONE);

  /* 10. Layout primitives */
  {
    const char *layouts[] = {"row", "column", "grid", "container"};
    size_t li;
    for (li = 0; li < 4; li++) {
      rc = ui_component_registry_lookup(registry, layouts[li], &vtable);
      assert(rc == UI_ERROR_NONE);

      /* OOM in layout factory: node create fail */
      g_malloc_fail_countdown = 0;
      rc = vtable->factory(&inst, &node);
      g_malloc_fail_countdown = -1;
      assert(rc != UI_ERROR_NONE);

      /* OOM in layout factory: set_tag_name fail */
      g_malloc_fail_countdown = 1;
      rc = vtable->factory(&inst, &node);
      g_malloc_fail_countdown = -1;
      assert(rc != UI_ERROR_NONE);

      /* OOM in layout factory: set_attribute fail */
      g_malloc_fail_countdown = 2;
      rc = vtable->factory(&inst, &node);
      g_malloc_fail_countdown = -1;
      assert(rc != UI_ERROR_NONE);

      rc = vtable->factory(&inst, &node);
      assert(rc == UI_ERROR_NONE && inst != NULL);

      rc = vtable->set_prop(NULL, "class", "box");
      assert(rc == UI_ERROR_INVALID_ARGUMENT);
      rc = vtable->set_prop(inst, NULL, "box");
      assert(rc == UI_ERROR_INVALID_ARGUMENT);
      rc = vtable->set_prop(inst, "class", NULL);
      assert(rc == UI_ERROR_INVALID_ARGUMENT);
      rc = vtable->set_prop(inst, "class", "my-box");
      assert(rc == UI_ERROR_NONE);

      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child_node);
      assert(rc == UI_ERROR_NONE);
      rc = vtable->append_child(NULL, child_node);
      assert(rc == UI_ERROR_INVALID_ARGUMENT);
      rc = vtable->append_child(inst, NULL);
      assert(rc == UI_ERROR_INVALID_ARGUMENT);
      rc = vtable->append_child(inst, child_node);
      assert(rc == UI_ERROR_NONE);

      rc = vtable->destroy(inst);
      assert(rc == UI_ERROR_NONE);
    }
  }

  /* 11. Custom registration, update duplicate, and not found */
  memset(&custom_vt, 0, sizeof(custom_vt));
  custom_vt.factory = custom_factory;
  rc = ui_component_registry_register(registry, "custom_widget", &custom_vt);
  assert(rc == UI_ERROR_NONE);

  rc = ui_component_registry_lookup(registry, "custom_widget", &vtable);
  assert(rc == UI_ERROR_NONE);
  assert(vtable->factory == custom_factory);

  /* Update duplicate */
  rc = ui_component_registry_register(registry, "custom_widget", &custom_vt);
  assert(rc == UI_ERROR_NONE);

  rc = ui_component_registry_lookup(registry, "nonexistent", &vtable);
  assert(rc == UI_ERROR_NOT_FOUND);

  rc = ui_component_registry_destroy(registry);
  assert(rc == UI_ERROR_NONE);

  /* 12. Global Singleton get_default & shutdown_default */
  /* Shutdown when already NULL */
  rc = ui_component_registry_shutdown_default();
  assert(rc == UI_ERROR_NONE);

  /* OOM on create in get_default */
  g_malloc_fail_countdown = 0;
  rc = ui_component_registry_get_default(&registry);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* OOM on register_defaults in get_default */
  g_malloc_fail_countdown = 1;
  rc = ui_component_registry_get_default(&registry);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* Success in get_default */
  rc = ui_component_registry_get_default(&registry);
  assert(rc == UI_ERROR_NONE);
  assert(registry != NULL);

  /* Second call returns cached default instance */
  {
    struct ui_component_registry *second_reg = NULL;
    rc = ui_component_registry_get_default(&second_reg);
    assert(rc == UI_ERROR_NONE);
    assert(second_reg == registry);
  }

  rc = ui_component_registry_shutdown_default();
  assert(rc == UI_ERROR_NONE);

  printf("test_ui_component_registry passed\n");
  return 0;
}
