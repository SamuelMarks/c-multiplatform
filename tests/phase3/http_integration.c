/* clang-format off */
#include "cmpc/cmp_http_integration.h"
#include "cmpc/cmp_text_field.h"
#include "test_utils.h"
#include <string.h>

#ifndef CMP_TESTING
#define CMP_TESTING
#endif
/* clang-format on */

static int test_http_integration_init(void) {
  CMPHttpIntegration integration;
  CMPEventDispatcher dispatcher;

  CMP_TEST_EXPECT(cmp_http_integration_init(NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_http_integration_init(&integration, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_http_integration_init(NULL, &dispatcher),
                  CMP_ERR_INVALID_ARGUMENT);

  cmp_event_dispatcher_init(&dispatcher);
  CMP_TEST_OK(cmp_http_integration_init(&integration, &dispatcher));

  cmp_http_integration_shutdown(&integration);
  return 0;
}

static int test_http_integration_lifecycle(void) {
  CMPHttpIntegration integration;
  CMPEventDispatcher dispatcher;

  cmp_event_dispatcher_init(&dispatcher);
  cmp_http_integration_init(&integration, &dispatcher);

  /* Bad arguments */
  CMP_TEST_EXPECT(cmp_http_integration_tick(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_http_integration_shutdown(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Valid tick */
  CMP_TEST_OK(cmp_http_integration_tick(&integration));

  /* Shutdown */
  CMP_TEST_OK(cmp_http_integration_shutdown(&integration));

  return 0;
}

static int dummy_http_on_done(void *ctx, CMPHttpForm *form, int status_code,
                              const void *body, cmp_usize body_size) {
  CMPBool *called = (CMPBool *)ctx;
  (void)form;
  (void)status_code;
  (void)body;
  (void)body_size;
  *called = CMP_TRUE;
  return CMP_OK;
}

static int mock_measure(void *widget, CMPMeasureSpec width,
                        CMPMeasureSpec height, CMPSize *out_size) {
  (void)widget;
  (void)width;
  (void)height;
  out_size->width = 10;
  out_size->height = 10;
  return CMP_OK;
}

static int mock_layout(void *widget, CMPRect bounds) {
  (void)widget;
  (void)bounds;
  return CMP_OK;
}

static int test_http_form(void) {
  CMPHttpIntegration integration;
  CMPEventDispatcher dispatcher;
  CMPWidget root;
  CMPWidgetVTable vtable;
  CMPHttpForm form;
  CMPBool called = CMP_FALSE;

  memset(&root, 0, sizeof(root));
  memset(&vtable, 0, sizeof(vtable));
  vtable.measure = mock_measure;
  vtable.layout = mock_layout;
  root.vtable = &vtable;

  cmp_event_dispatcher_init(&dispatcher);
  cmp_http_integration_init(&integration, &dispatcher);

  CMP_TEST_EXPECT(cmp_http_form_init(NULL, &integration, &dispatcher, &root,
                                     "POST", "/api"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_http_form_init(&form, NULL, &dispatcher, &root, "POST", "/api"),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_http_form_init(&form, &integration, &dispatcher, &root,
                                 "POST", "/api"));

  CMP_TEST_EXPECT(cmp_http_form_set_on_done(NULL, dummy_http_on_done, &called),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_http_form_set_on_done(&form, dummy_http_on_done, &called));

  CMP_TEST_EXPECT(cmp_http_form_tick(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_http_form_tick(&form));

  /* test submit by invoking form's submit directly or via enter event */
  cmp_form_submit(&form.form);
  CMP_TEST_OK(cmp_http_form_tick(&form));

  /* Submit again to hit the early return inside the internal submit */
  cmp_form_submit(&form.form);

  CMP_TEST_EXPECT(cmp_http_form_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_http_form_shutdown(&form));

  cmp_http_integration_shutdown(&integration);
  return 0;
}

static int test_http_form_other_methods(void) {
  CMPHttpIntegration integration;
  CMPEventDispatcher dispatcher;
  CMPWidget root;
  CMPWidgetVTable vtable;
  CMPHttpForm form_put;
  CMPHttpForm form_get;
  CMPHttpForm form_default;
  CMPTextField tf;
  CMPTextBackend tb;
  CMPTextFieldStyle ts;
  CMPWidget *inputs[1];
  char text[] = "hello";

  memset(&root, 0, sizeof(root));
  memset(&vtable, 0, sizeof(vtable));
  vtable.measure = mock_measure;
  vtable.layout = mock_layout;
  root.vtable = &vtable;

  cmp_event_dispatcher_init(&dispatcher);
  cmp_http_integration_init(&integration, &dispatcher);

  tb.ctx = NULL;
  tb.vtable = NULL;
  cmp_text_field_style_init(&ts);
  cmp_text_field_init(&tf, &tb, &ts, NULL, text, 5);
  cmp_text_field_set_label(&tf, "myfield", 7);
  inputs[0] = &tf.widget;

  CMP_TEST_OK(cmp_http_form_init(&form_put, &integration, &dispatcher, &root,
                                 "PUT", "/api"));
  cmp_form_set_inputs(&form_put.form, inputs, 1);
  cmp_form_submit(&form_put.form);
  cmp_http_form_shutdown(&form_put);

  CMP_TEST_OK(cmp_http_form_init(&form_get, &integration, &dispatcher, &root,
                                 "GET", "/api"));
  cmp_form_set_inputs(&form_get.form, inputs, 1);
  cmp_form_submit(&form_get.form);
  cmp_http_form_shutdown(&form_get);

  CMP_TEST_OK(cmp_http_form_init(&form_default, &integration, &dispatcher,
                                 &root, NULL, NULL));
  cmp_form_submit(&form_default.form);
  cmp_http_form_shutdown(&form_default);

  if (tf.widget.vtable->destroy) {
    tf.widget.vtable->destroy(&tf.widget);
  }

  cmp_http_integration_shutdown(&integration);
  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_http_integration_init();
  fails += test_http_integration_lifecycle();
  fails += test_http_form();
  fails += test_http_form_other_methods();
  return fails;
}
