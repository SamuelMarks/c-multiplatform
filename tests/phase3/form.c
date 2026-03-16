/* clang-format off */
#include "cmpc/cmp_form.h"
#include "test_utils.h"
#include <string.h>

#ifndef CMP_TESTING
#define CMP_TESTING
#endif
/* clang-format on */

static int CMP_CALL mock_measure(void *widget, CMPMeasureSpec width,
                                 CMPMeasureSpec height, CMPSize *out_size) {
  (void)widget;
  (void)width;
  (void)height;
  out_size->width = 100;
  out_size->height = 100;
  return CMP_OK;
}

static int CMP_CALL mock_layout(void *widget, CMPRect bounds) {
  (void)widget;
  (void)bounds;
  return CMP_OK;
}

static int CMP_CALL mock_paint(void *widget, CMPPaintContext *ctx) {
  (void)widget;
  (void)ctx;
  return CMP_OK;
}

static int CMP_CALL mock_event(void *widget, const CMPInputEvent *event,
                               CMPBool *out_handled) {
  printf("  mock_event called\n");
  fflush(stdout);
  (void)widget;
  (void)event;
  *out_handled = CMP_TRUE;
  return CMP_OK;
}

static int CMP_CALL mock_get_semantics(void *widget,
                                       CMPSemantics *out_semantics) {
  (void)widget;
  out_semantics->role = CMP_SEMANTIC_TEXT;
  out_semantics->flags = CMP_SEMANTIC_FLAG_FOCUSABLE;
  return CMP_OK;
}

static int CMP_CALL on_submit(void *ctx, CMPForm *form) {
  CMPBool *submitted = (CMPBool *)ctx;
  (void)form;
  *submitted = CMP_TRUE;
  return CMP_OK;
}

static int test_form_init(void) {
  CMPEventDispatcher dispatcher;
  CMPWidget root;
  CMPForm form;

  memset(&dispatcher, 0, sizeof(dispatcher));
  CMP_TEST_EXPECT(cmp_form_init(NULL, &dispatcher, &root),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_form_init(&form, NULL, &root), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_form_init(&form, &dispatcher, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  cmp_event_dispatcher_init(&dispatcher);
  CMP_TEST_OK(cmp_form_init(&form, &dispatcher, &root));

  return 0;
}

static int test_form_lifecycle(void) {
  CMPEventDispatcher dispatcher;
  CMPWidget root, inputs[2], submit_btn;
  CMPWidgetVTable vtable;
  CMPForm form;
  CMPWidget *inputs_ptr[2];
  CMPBool submitted = CMP_FALSE;

  memset(&dispatcher, 0, sizeof(dispatcher));
  CMP_TEST_OK(cmp_event_dispatcher_init(&dispatcher));

  memset(&root, 0, sizeof(root));
  memset(&inputs[0], 0, sizeof(inputs[0]));
  memset(&inputs[1], 0, sizeof(inputs[1]));
  memset(&submit_btn, 0, sizeof(submit_btn));
  memset(&vtable, 0, sizeof(vtable));

  vtable.measure = mock_measure;
  vtable.layout = mock_layout;
  vtable.paint = mock_paint;
  vtable.event = mock_event;
  vtable.get_semantics = mock_get_semantics;

  root.vtable = &vtable;
  inputs[0].vtable = &vtable;
  inputs[0].flags = CMP_WIDGET_FLAG_FOCUSABLE;
  inputs[1].vtable = &vtable;
  inputs[1].flags = CMP_WIDGET_FLAG_FOCUSABLE;
  submit_btn.vtable = &vtable;
  submit_btn.flags = CMP_WIDGET_FLAG_FOCUSABLE;

  inputs_ptr[0] = &inputs[0];
  inputs_ptr[1] = &inputs[1];

  cmp_form_init(&form, &dispatcher, &root);

  /* Test bad inputs */
  CMP_TEST_EXPECT(cmp_form_set_inputs(NULL, inputs_ptr, 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_form_set_submit(NULL, &submit_btn, on_submit, &submitted),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_form_set_submitting(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Test setting state */
  CMP_TEST_OK(cmp_form_set_inputs(&form, inputs_ptr, 2));
  CMP_TEST_OK(cmp_form_set_submit(&form, &submit_btn, on_submit, &submitted));

  /* Test vtable forwards */
  {
    CMPSize size;
    CMPRect bounds = {0, 0, 10, 10};
    CMPSemantics semantics;

    form.widget.vtable->measure(&form, (CMPMeasureSpec){0, 0},
                                (CMPMeasureSpec){0, 0}, &size);
    CMP_TEST_ASSERT(100 == size.width);

    CMP_TEST_OK(form.widget.vtable->layout(&form, bounds));
    CMP_TEST_OK(form.widget.vtable->paint(&form, NULL));

    memset(&semantics, 0, sizeof(semantics));
    CMP_TEST_OK(form.widget.vtable->get_semantics(&form, &semantics));
    CMP_TEST_ASSERT(CMP_SEMANTIC_TEXT == semantics.role);
  }

  /* Test Tab Event Navigation */
  cmp_event_dispatcher_set_focus(&dispatcher, &inputs[0]);

  {
    CMPInputEvent evt;
    CMPBool handled = CMP_FALSE;
    memset(&evt, 0, sizeof(evt));
    evt.type = CMP_INPUT_KEY_DOWN;
    evt.data.key.key_code = 9u; /* Tab */
    evt.modifiers = 0;

    form.widget.vtable->event(&form, &evt, &handled);
    CMP_TEST_ASSERT(handled);

    /* Focus should now be inputs[1] */
    {
      CMPWidget *curr = NULL;
      cmp_event_dispatcher_get_focus(&dispatcher, &curr);
      CMP_TEST_ASSERT(&inputs[1] == curr);
    }
  }

  /* Test Shift+Tab Navigation */
  {
    CMPInputEvent evt;
    CMPBool handled = CMP_FALSE;
    memset(&evt, 0, sizeof(evt));
    evt.type = CMP_INPUT_KEY_DOWN;
    evt.data.key.key_code = 9u; /* Tab */
    evt.modifiers = CMP_MOD_SHIFT;

    form.widget.vtable->event(&form, &evt, &handled);
    CMP_TEST_ASSERT(handled);

    {
      CMPWidget *curr = NULL;
      cmp_event_dispatcher_get_focus(&dispatcher, &curr);
      CMP_TEST_ASSERT(&inputs[0] == curr);
    }
  }

  /* Test Enter Event (Submit) */
  {
    CMPInputEvent evt;
    CMPBool handled = CMP_FALSE;
    memset(&evt, 0, sizeof(evt));
    evt.type = CMP_INPUT_KEY_DOWN;
    evt.data.key.key_code = 13u; /* Enter */
    evt.modifiers = 0;

    form.widget.vtable->event(&form, &evt, &handled);
    CMP_TEST_ASSERT(handled);
    CMP_TEST_ASSERT(submitted);
  }

  /* Test Submitting state ignores events */
  cmp_form_set_submitting(&form, CMP_TRUE);
  {
    CMPInputEvent evt;
    CMPBool handled = CMP_FALSE;
    memset(&evt, 0, sizeof(evt));
    evt.type = CMP_INPUT_POINTER_DOWN;

    form.widget.vtable->event(&form, &evt, &handled);
    CMP_TEST_ASSERT(handled); /* Should be suppressed and marked handled */
  }

  if (form.widget.vtable->destroy) {
    form.widget.vtable->destroy(&form);
  }

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_form_init();
  fails += test_form_lifecycle();
  return fails;
}
