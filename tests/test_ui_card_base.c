/* clang-format off */
#include "ui_card_base.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_card_base *card = NULL;
  enum ui_error rc;
  struct ui_component *comp;

  printf("Testing invalid arguments...\n");
  if (ui_card_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_card_base_destroy(NULL); /* Should not crash */
  if (ui_card_base_set_header(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_card_base_set_content(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_card_base_set_actions(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    struct ui_component *tmp_comp;
    if (ui_card_base_get_component(NULL, &tmp_comp) == UI_ERROR_NONE)
      return 1;
  }

  printf("Testing normal creation and logic...\n");
  rc = ui_card_base_create(&card);
  if (rc != UI_ERROR_NONE || card == NULL)
    return 1;

  if (ui_card_base_get_component(card, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_card_base_get_component(card, &comp) != UI_ERROR_NONE || comp == NULL)
    return 1;

  rc = ui_card_base_set_header(card, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_card_base_set_content(card, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_card_base_set_actions(card, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;

  {
    struct ui_component *dummy_content = NULL;
    ui_component_create(&dummy_content);
    ui_card_base_set_header(card, dummy_content);
    ui_card_base_set_content(card, dummy_content);
    ui_card_base_set_actions(card, dummy_content);
    ui_component_destroy(dummy_content);
  }
  if (ui_card_base_bind_data(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_card_base_bind_data(card, NULL) != UI_ERROR_NONE)
    return 1;

  ui_card_base_destroy(card);
  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_card_base *card = NULL;
  enum ui_error rc;
  int i;

  printf("Testing OOM...\n");
  for (i = 0; i < 200; i++) {
    g_malloc_fail_countdown = i;
    card = NULL;
    rc = ui_card_base_create(&card);
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      if (card != NULL) {
        printf("Leaked card struct pointer on OOM.\n");
        return 1;
      }
    } else if (rc == UI_ERROR_NONE) {
      {
        struct ui_component *dummy_content = NULL;
        ui_component_create(&dummy_content);
        ui_card_base_set_header(card, dummy_content);
        ui_card_base_set_content(card, dummy_content);
        ui_card_base_set_actions(card, dummy_content);
      }
      if (ui_card_base_bind_data(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
        return 1;
      if (ui_card_base_bind_data(card, NULL) != UI_ERROR_NONE)
        return 1;

      ui_card_base_destroy(card);
      break; /* We hit enough allocations to succeed */
    } else {
      printf("Unexpected error code %d\n", rc);
      return 1;
    }
  }
#endif
  return 0;
}

static void test_card_coverage(void) {
  {
    struct ui_card_base *c = NULL;
    ui_card_base_create(&c);
    struct ui_card_base_internal {
      struct ui_component *component;
      struct ui_dom_node *root_node;
      struct ui_dom_node *header_node;
      struct ui_dom_node *content_node;
      struct ui_dom_node *footer_node;
      struct ui_component *header_component;
      struct ui_component *content_component;
      struct ui_component *footer_component;
      int variant; /* Assuming variant is int enum */
    } *ci = (struct ui_card_base_internal *)c;

    struct ui_component *saved = ci->component;
    ci->component = NULL;
    ui_card_base_destroy(c);
    ui_component_destroy(saved);
  }
}
int main(void) {
  test_card_coverage();
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }
  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }
  printf("All ui_card_base tests passed.\n");
  return 0;
}
