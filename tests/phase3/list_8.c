#include "cmpc/cmp_list.h"
#include "test_utils.h"

int main(void) {
    CMPListView list_view;
    CMPListStyle list_style;
    cmp_list_style_init(&list_style);
    cmp_list_view_init(&list_view, &list_style, NULL, 10, 0);
    
    
    list_view.bounds.height = -100.0f;
    CMP_TEST_EXPECT(cmp_list_view_set_style(&list_view, &list_style), CMP_ERR_RANGE);
    CMP_TEST_EXPECT(cmp_list_view_set_scroll(&list_view, 10.0f), CMP_ERR_RANGE);

    list_view.bounds.height = 100.0f;

    CMPGridView grid_view;
    CMPGridStyle grid_style;
    cmp_grid_style_init(&grid_style);
    cmp_grid_view_init(&grid_view, &grid_style, NULL, 10, 0);

    
    grid_view.bounds.height = -100.0f;
    CMP_TEST_EXPECT(cmp_grid_view_set_style(&grid_view, &grid_style), CMP_ERR_RANGE);
    CMP_TEST_EXPECT(cmp_grid_view_set_scroll(&grid_view, 10.0f), CMP_ERR_RANGE);
    
    return CMP_OK;
}
