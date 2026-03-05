#include <string.h>
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
    
    CMPInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = CMP_INPUT_POINTER_SCROLL;
    ev.data.pointer.scroll_x = 0;
    ev.data.pointer.scroll_y = 10;
    
    CMPBool handled;
    list_view.widget.vtable->event(&list_view, &ev, &handled);
    
    list_style.orientation = CMP_LIST_ORIENTATION_HORIZONTAL;
    cmp_list_view_set_style(&list_view, &list_style);
    ev.data.pointer.scroll_x = 10;
    ev.data.pointer.scroll_y = 0;
    list_view.widget.vtable->event(&list_view, &ev, &handled);
    
    ev.data.pointer.scroll_x = 1000;
    list_view.widget.vtable->event(&list_view, &ev, &handled);
    
    list_view.widget.vtable->destroy(&list_view);
    
    CMPGridView grid_view;
    CMPGridStyle grid_style;
    cmp_grid_style_init(&grid_style);
    cmp_grid_view_init(&grid_view, &grid_style, NULL, 10, 0);
    cmp_grid_view_set_bind(&grid_view, bind_fn_impl, &binds);
    cmp_grid_view_reserve(&grid_view, 10);
    grid_view.bounds.width = 100.0f;
    grid_view.bounds.height = 100.0f;
    
    ev.data.pointer.scroll_x = 0;
    ev.data.pointer.scroll_y = 10;
    grid_view.widget.vtable->event(&grid_view, &ev, &handled);
    
    grid_style.scroll_axis = CMP_GRID_SCROLL_HORIZONTAL;
    cmp_grid_view_set_style(&grid_view, &grid_style);
    ev.data.pointer.scroll_x = 10;
    ev.data.pointer.scroll_y = 0;
    grid_view.widget.vtable->event(&grid_view, &ev, &handled);
    
    grid_view.widget.vtable->destroy(&grid_view);
    return 0;
}
