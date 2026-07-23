/* clang-format off */
#include "ui_theme_provider.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

enum ui_error
ui_theme_provider_create(struct ui_arena *arena,
                         struct ui_design_token_dict *tokens,
                         struct ui_theme_provider **out_provider) {
  void *ptr;
  enum ui_error err;

  if (!arena || !tokens || !out_provider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_theme_provider), 8, &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  *out_provider = (struct ui_theme_provider *)ptr;
  (*out_provider)->tokens = tokens;

  /* Initialize the underlying component */
  /* Since we don't have ui_component_init, we can't fully initialize it if
   * ui_component_create allocates. Wait, ui_component_create takes a double
   * pointer and allocates it itself! Actually, let's just leave base zeroed for
   * now or use ui_component APIs. If ui_theme_provider is just a wrapper, maybe
   * it doesn't need to inherit ui_component in C, but the struct has it.
   */
  memset(&(*out_provider)->base, 0, sizeof(struct ui_component));

  return UI_ERROR_NONE;
}

enum ui_error ui_theme_provider_mount(struct ui_theme_provider *provider,
                                      struct ui_dom_node *node) {
  char ptr_str[64];

  if (!provider || !node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  sprintf_s(ptr_str, sizeof(ptr_str), "%p", (void *)provider->tokens);
#else
  sprintf(ptr_str, "%p", (void *)provider->tokens);
#endif

  return ui_dom_node_set_attribute(node, "__ui_theme_ptr", ptr_str);
}

enum ui_error ui_theme_provider_get(struct ui_dom_node *node,
                                    struct ui_design_token_dict **out_tokens) {
  struct ui_dom_node *current = node;
  const char *attr_val;
  enum ui_error err;

  if (!node || !out_tokens) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  while (current) {
    err = ui_dom_node_get_attribute(current, "__ui_theme_ptr", &attr_val);
    if (err == UI_ERROR_NONE && attr_val) {
      /* Parse pointer */
      void *ptr = NULL;
      /* Using sscanf safely */
#if defined(_MSC_VER)
      if (sscanf_s(attr_val, "%p", &ptr) == 1) {
#else
      if (sscanf(attr_val, "%p", &ptr) == 1) {
#endif
        *out_tokens = (struct ui_design_token_dict *)ptr;
        return UI_ERROR_NONE;
      }
    }
    current = current->parent;
  }

  return UI_ERROR_NOT_FOUND;
}
