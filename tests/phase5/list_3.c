/* clang-format off */
#include "m3/m3_list.h"
#include "test_utils.h"
#include <string.h>
/* clang-format on */

int main(void) {
  M3ListItemStyle style;
  M3ListItem item;
  CMPTextBackend text_backend;

  text_backend.ctx = NULL;
  text_backend.vtable = NULL;

  CMP_TEST_OK(m3_list_item_style_init(&style, M3_LIST_ITEM_VARIANT_1_LINE));
  CMP_TEST_OK(m3_list_item_init(&item, &style, text_backend));

  CMPPaintContext pctx;
  memset(&pctx, 0, sizeof(pctx));
  CMP_TEST_EXPECT(item.widget.vtable->paint(NULL, &pctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(item.widget.vtable->paint(&item, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  item.widget.vtable->destroy(&item);
  return 0;
}
