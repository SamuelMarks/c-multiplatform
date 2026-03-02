#include "m3/m3_list.h"
#include "test_utils.h"

static int test_on_press(void *ctx, M3ListItem *item) {
  int *called = (int *)ctx;
  *called = 1;
  return CMP_OK;
}

int main(void) {
  M3ListItemStyle style;
  M3ListItem item;
  CMPTextBackend text_backend;
  int called = 0;

  CMP_TEST_EXPECT(m3_list_item_style_init(NULL, M3_LIST_ITEM_VARIANT_1_LINE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_list_item_style_init(&style, 999),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_list_item_style_init(&style, M3_LIST_ITEM_VARIANT_1_LINE));
  CMP_TEST_EXPECT(style.min_height == 56.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);
  CMP_TEST_OK(m3_list_item_style_init(&style, M3_LIST_ITEM_VARIANT_2_LINE));
  CMP_TEST_EXPECT(style.min_height == 72.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);
  CMP_TEST_OK(m3_list_item_style_init(&style, M3_LIST_ITEM_VARIANT_3_LINE));
  CMP_TEST_EXPECT(style.min_height == 88.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  text_backend.ctx = NULL;
  text_backend.vtable = NULL;

  CMP_TEST_EXPECT(m3_list_item_init(NULL, &style, text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_list_item_init(&item, NULL, text_backend),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_list_item_init(&item, &style, text_backend));

  CMP_TEST_EXPECT(m3_list_item_set_headline(NULL, "Headline"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_list_item_set_headline(&item, "Headline"));

  CMP_TEST_EXPECT(m3_list_item_set_supporting(NULL, "Supporting"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_list_item_set_supporting(&item, "Supporting"));

  CMP_TEST_EXPECT(m3_list_item_set_trailing(NULL, "Trailing"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_list_item_set_trailing(&item, "Trailing"));

  CMP_TEST_EXPECT(m3_list_item_set_widgets(NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_list_item_set_widgets(&item, NULL, NULL));

  CMP_TEST_EXPECT(m3_list_item_set_on_press(NULL, test_on_press, &called),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_list_item_set_on_press(&item, test_on_press, &called));

  CMP_TEST_OK(m3_list_test_helper());

  {
    CMPSize size;
    CMPRect bounds = {0, 0, 100, 100};
    CMPInputEvent event = {0};
    CMPBool handled;
    CMPSemantics semantics;
    CMPMeasureSpec m100 = {1, 100.0f};

    CMP_TEST_EXPECT(item.widget.vtable->measure(NULL, m100, m100, &size),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(item.widget.vtable->measure(&item, m100, m100, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(item.widget.vtable->measure(&item, m100, m100, &size));

    CMP_TEST_EXPECT(item.widget.vtable->layout(NULL, bounds),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(item.widget.vtable->layout(&item, bounds));

    CMP_TEST_EXPECT(item.widget.vtable->event(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(item.widget.vtable->event(&item, NULL, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(item.widget.vtable->event(&item, &event, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.x = 10;
    event.data.pointer.y = 10;
    CMP_TEST_OK(item.widget.vtable->event(&item, &event, &handled));

    event.type = CMP_INPUT_POINTER_UP;
    CMP_TEST_OK(item.widget.vtable->event(&item, &event, &handled));
    CMP_TEST_EXPECT(called, 1);

    CMP_TEST_EXPECT(item.widget.vtable->get_semantics(NULL, &semantics),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(item.widget.vtable->get_semantics(&item, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(item.widget.vtable->get_semantics(&item, &semantics));

    CMP_TEST_EXPECT(item.widget.vtable->destroy(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(item.widget.vtable->destroy(&item));
  }

  return 0;
}
