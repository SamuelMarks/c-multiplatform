/* clang-format off */
#include "cmpc/cmp_form.h"
#include <stddef.h>
/* clang-format on */

static int cmp_form_measure(void *widget, CMPMeasureSpec width,
                            CMPMeasureSpec height, CMPSize *out_size) {
  CMPForm *form = (CMPForm *)widget;
  if (form->root_widget && form->root_widget->vtable &&
      form->root_widget->vtable->measure) {
    return form->root_widget->vtable->measure(form->root_widget, width, height,
                                              out_size);
  }
  out_size->width = 0;
  out_size->height = 0;
  return CMP_OK;
}

static int cmp_form_layout(void *widget, CMPRect bounds) {
  CMPForm *form = (CMPForm *)widget;
  form->bounds = bounds;
  if (form->root_widget && form->root_widget->vtable &&
      form->root_widget->vtable->layout) {
    return form->root_widget->vtable->layout(form->root_widget, bounds);
  }
  return CMP_OK;
}

static int cmp_form_paint(void *widget, CMPPaintContext *ctx) {
  CMPForm *form = (CMPForm *)widget;
  if (form->root_widget && form->root_widget->vtable &&
      form->root_widget->vtable->paint) {
    return form->root_widget->vtable->paint(form->root_widget, ctx);
  }
  return CMP_OK;
}

static int cmp_form_event(void *widget, const CMPInputEvent *event,
                          CMPBool *out_handled) {
  CMPForm *form = (CMPForm *)widget;

  if (form->is_submitting) {
    /* Suppress interactions while submitting */
    if (event->type == CMP_INPUT_POINTER_DOWN ||
        event->type == CMP_INPUT_KEY_DOWN || event->type == CMP_INPUT_TEXT) {
      *out_handled = CMP_TRUE;
      return CMP_OK;
    }
  }

  if (event->type == CMP_INPUT_KEY_DOWN && form->dispatcher) {
    cmp_u32 key = event->data.key.key_code;
    CMPBool is_shift = (event->modifiers & CMP_MOD_SHIFT) != 0;

    /* 9u is Tab */
    if (key == 9u && form->input_count > 0) {
      CMPWidget *curr = NULL;
      cmp_usize i, focused_index = (cmp_usize)-1;

      cmp_event_dispatcher_get_focus(form->dispatcher, &curr);

      /* Find focused widget index */
      for (i = 0; i < form->input_count; ++i) {
        if (form->inputs[i] == curr) {
          focused_index = i;
          break;
        }
      }

      if (focused_index != (cmp_usize)-1) {
        /* Navigate */
        CMPWidget *next = NULL;
        if (is_shift) {
          next = form->inputs[(focused_index + form->input_count - 1) %
                              form->input_count];
        } else {
          next = form->inputs[(focused_index + 1) % form->input_count];
        }

        cmp_event_dispatcher_set_focus(form->dispatcher, next);

        *out_handled = CMP_TRUE;
        return CMP_OK;
      }
    }

    /* 13u is Enter */
    if (key == 13u && form->on_submit) {
      CMPWidget *curr = NULL;
      cmp_event_dispatcher_get_focus(form->dispatcher, &curr);

      /* Only submit if focus is within our inputs or submit button,
         or if we just want to submit globally. Assuming globally for now. */
      form->on_submit(form->user_data, form);
      *out_handled = CMP_TRUE;
      return CMP_OK;
    }
  }

  /* Forward to root widget if not handled */
  if (form->root_widget && form->root_widget->vtable &&
      form->root_widget->vtable->event) {
    return form->root_widget->vtable->event(form->root_widget, event,
                                            out_handled);
  }

  return CMP_OK;
}

static int cmp_form_get_semantics(void *widget, CMPSemantics *out_semantics) {
  CMPForm *form = (CMPForm *)widget;

  if (form->root_widget && form->root_widget->vtable &&
      form->root_widget->vtable->get_semantics) {
    return form->root_widget->vtable->get_semantics(form->root_widget,
                                                    out_semantics);
  }

  out_semantics->role = CMP_SEMANTIC_CONTAINER;
  return CMP_OK;
}

static int cmp_form_destroy(void *widget) {
  (void)widget;
  return CMP_OK;
}

static const CMPWidgetVTable g_cmp_form_vtable = {
    cmp_form_measure, cmp_form_layout,        cmp_form_paint,
    cmp_form_event,   cmp_form_get_semantics, cmp_form_destroy};

CMP_API int CMP_CALL cmp_form_init(CMPForm *form,
                                   CMPEventDispatcher *dispatcher,
                                   CMPWidget *root_widget) {
  if (!form || !dispatcher || !root_widget) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  form->widget.vtable = &g_cmp_form_vtable;
  form->widget.flags = 0;
  form->dispatcher = dispatcher;
  form->root_widget = root_widget;
  form->inputs = NULL;
  form->input_count = 0;
  form->submit_btn = NULL;
  form->on_submit = NULL;
  form->user_data = NULL;
  form->is_submitting = CMP_FALSE;

  return CMP_OK;
}

CMP_API int CMP_CALL cmp_form_set_inputs(CMPForm *form, CMPWidget **inputs,
                                         cmp_usize input_count) {
  if (!form) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  form->inputs = inputs;
  form->input_count = input_count;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_form_set_submit(CMPForm *form, CMPWidget *submit_btn,
                                         CMPFormSubmitFn on_submit,
                                         void *user_data) {
  if (!form) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  form->submit_btn = submit_btn;
  form->on_submit = on_submit;
  form->user_data = user_data;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_form_set_submitting(CMPForm *form,
                                             CMPBool is_submitting) {
  if (!form) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  form->is_submitting = is_submitting;
  return CMP_OK;
}
