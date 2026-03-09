#include "cmpc/cmp_list.h"
#include "test_utils.h"

static int bind_fn_impl(void *ctx, CMPListSlot *slot, cmp_usize index) {
  if (ctx) {
    int *called = (int *)ctx;
    (*called)++;
  }
  if (slot) {
    slot->index = index;
  }
  return CMP_OK;
}

int main(void) {
  CMPListView list_view;
  CMPListStyle list_style;
  cmp_list_style_init(&list_style);
  cmp_list_view_init(&list_view, &list_style, NULL, 10, 0);
  int binds = 0;
  cmp_list_view_set_bind(&list_view, bind_fn_impl, &binds);
  cmp_list_view_reserve(&list_view, 10);
  list_view.bounds.width = 100.0f;
  list_view.bounds.height = 100.0f;
  cmp_list_view_update(&list_view);

  CMPGridView grid_view;
  CMPGridStyle grid_style;
  cmp_grid_style_init(&grid_style);
  cmp_grid_view_init(&grid_view, &grid_style, NULL, 10, 0);
  cmp_grid_view_set_bind(&grid_view, bind_fn_impl, &binds);
  cmp_grid_view_reserve(&grid_view, 10);
  grid_view.bounds.width = 100.0f;
  grid_view.bounds.height = 100.0f;

  CMPMeasureSpec mw = {CMP_MEASURE_EXACTLY, 100.0f};
  CMPMeasureSpec mh = {CMP_MEASURE_EXACTLY, 50.0f};
  CMPSize size;
  grid_view.widget.vtable->measure(&grid_view, mw, mh, &size);

  mw.mode = CMP_MEASURE_AT_MOST;
  mw.size = 1.0f;
  grid_view.widget.vtable->measure(&grid_view, mw, mh, &size);

  mh.mode = CMP_MEASURE_AT_MOST;
  mh.size = 1.0f;
  grid_view.widget.vtable->measure(&grid_view, mw, mh, &size);

  mw.mode = CMP_MEASURE_UNSPECIFIED;
  mh.mode = CMP_MEASURE_UNSPECIFIED;
  grid_view.widget.vtable->measure(&grid_view, mw, mh, &size);

  list_view.widget.vtable->measure(&list_view, mw, mh, &size);
  mw.mode = CMP_MEASURE_AT_MOST;
  mw.size = 1.0f;
  list_view.widget.vtable->measure(&list_view, mw, mh, &size);
  mh.mode = CMP_MEASURE_AT_MOST;
  mh.size = 1.0f;
  list_view.widget.vtable->measure(&list_view, mw, mh, &size);

  grid_view.widget.vtable->layout(&grid_view, grid_view.bounds);

  list_view.widget.vtable->destroy(&list_view);
  grid_view.widget.vtable->destroy(&grid_view);
  return 0;
}
