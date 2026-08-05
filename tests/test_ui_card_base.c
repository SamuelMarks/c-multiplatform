/* clang-format off */
#include "ui_card_base.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_card_mock_fail;

static ui_error_t run_mock_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  int i;
  for (i = 0; i < 5; i++) {
    struct ui_card_base *card = NULL;
    g_card_mock_fail = i;
    (void)ui_card_base_create(&card);
    if (card) {
      (void)ui_card_base_destroy(card);
    }
  }
  g_card_mock_fail = -1;
#endif
  return UI_ERROR_NONE;
}

static ui_error_t run_normal_tests(void) {
  struct ui_card_base *card = NULL;
  ui_error_t rc;
  struct ui_component *comp;

  printf("Testing invalid arguments...\n");
  rc = ui_card_base_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_card_base_destroy(NULL); /* Should not crash */
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_card_base_set_header(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_card_base_set_content(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_card_base_set_actions(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  {
    struct ui_component *tmp_comp;
    rc = ui_card_base_get_component(NULL, &tmp_comp);
    if (rc == UI_ERROR_NONE)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  printf("Testing normal creation and logic...\n");
  rc = ui_card_base_create(&card);
  if (rc != UI_ERROR_NONE || card == NULL)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_card_base_get_component(card, NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  if (ui_card_base_get_component(card, &comp) != UI_ERROR_NONE || comp == NULL)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_card_base_set_header(card, NULL);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_card_base_set_content(card, NULL);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_card_base_set_actions(card, NULL);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  {
    struct ui_component *dummy_content = NULL;
    rc = ui_component_create(&dummy_content);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_card_base_set_header(card, dummy_content);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_card_base_set_content(card, dummy_content);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_card_base_set_actions(card, dummy_content);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_component_destroy(dummy_content);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  rc = ui_card_base_bind_data(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_card_base_bind_data(card, NULL);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_card_base_destroy(card);

  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

static ui_error_t run_oom_test_create_step(int i,
                                           struct ui_card_base **out_card,
                                           int *out_continue, int *out_break) {
  ui_error_t rc;
  g_malloc_fail_countdown = i;
  rc = ui_card_base_create(out_card);
  if (rc == UI_ERROR_OUT_OF_MEMORY) {
    if (*out_card != NULL) {
      printf("Leaked card struct pointer on OOM.\n");
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    }
    *out_continue = 1;
    return UI_ERROR_NONE;
  } else if (rc == UI_ERROR_NONE) {
    g_malloc_fail_countdown = -1;
    {
      struct ui_component *dummy_content = NULL;
      rc = ui_component_create(&dummy_content);
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = ui_card_base_set_header(*out_card, dummy_content);
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = ui_card_base_set_content(*out_card, dummy_content);
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = ui_card_base_set_actions(*out_card, dummy_content);
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = ui_component_destroy(dummy_content);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    rc = ui_card_base_bind_data(NULL, NULL);
    if (rc != UI_ERROR_INVALID_ARGUMENT)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    rc = ui_card_base_bind_data(*out_card, NULL);
    if (rc != UI_ERROR_NONE)
      return rc;

    rc = ui_card_base_destroy(*out_card);
    if (rc != UI_ERROR_NONE)
      return rc;
    *out_break = 1;
    return UI_ERROR_NONE;
  }
  printf("Unexpected error code %d\n", rc);
  return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
}

static ui_error_t run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_card_base *card = NULL;
  ui_error_t rc;
  int i;

  printf("Testing OOM...\n");
  for (i = 0; i < 200; i++) {
    int do_continue = 0, do_break = 0;
    card = NULL;
    rc = run_oom_test_create_step(i, &card, &do_continue, &do_break);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (do_continue)
      continue;
    if (do_break)
      break;
  }
  g_malloc_fail_countdown = -1;
#endif
  return UI_ERROR_NONE;
}

static ui_error_t test_card_coverage(void) {
  ui_error_t rc;
  {
    struct ui_card_base *c = NULL;
    rc = ui_card_base_create(&c);
    if (rc != UI_ERROR_NONE)
      return rc;
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
    rc = ui_card_base_destroy(c);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = ui_component_destroy(saved);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return UI_ERROR_NONE;
}
int main(void) {
  ui_error_t rc;
  rc = test_card_coverage();
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = run_mock_failures();
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = run_normal_tests();
  if (rc != UI_ERROR_NONE) {
    printf("Normal tests failed.\n");
    return 1;
  }
  rc = run_oom_tests();
  if (rc != UI_ERROR_NONE) {
    printf("OOM tests failed.\n");
    return 1;
  }
  printf("All ui_card_base tests passed.\n");
  return 0;
}
