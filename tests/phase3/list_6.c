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
    cmp_list_view_update(&list_view);
    
    CMPInputEvent ev;
    memset(&ev, 0, sizeof(ev));
    CMPBool handled;
    
    
    ev.type = CMP_INPUT_GESTURE_FLING;
    ev.data.gesture.velocity_y = -100.0f;
    list_view.widget.vtable->event(&list_view, &ev, &handled);
    
    ev.data.gesture.velocity_y = 100.0f;
    list_view.widget.vtable->event(&list_view, &ev, &handled);
    
    
    ev.type = CMP_INPUT_KEY_DOWN;
    ev.data.key.key_code = 127; 
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
    cmp_grid_view_update(&grid_view);
    
    
    ev.type = CMP_INPUT_GESTURE_FLING;
    ev.data.gesture.velocity_y = -100.0f;
    grid_view.widget.vtable->event(&grid_view, &ev, &handled);
    
    grid_view.widget.vtable->destroy(&grid_view);
    return 0;
}
