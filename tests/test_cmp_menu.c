/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_menu_presentation(void) {
  cmp_menu_t *ctx = NULL;
  int blur, lift, arrow;

  /* Context Menu */
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_create(&ctx, CMP_MENU_PRESENTATION_CONTEXT));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_menu_resolve_presentation(ctx, &blur, &lift, &arrow));
  ASSERT_EQ(1, blur);
  ASSERT_EQ(1, lift);
  ASSERT_EQ(0, arrow);
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_destroy(ctx));

  /* Popover Menu */
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_create(&ctx, CMP_MENU_PRESENTATION_POPOVER));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_menu_resolve_presentation(ctx, &blur, &lift, &arrow));
  ASSERT_EQ(0, blur);
  ASSERT_EQ(0, lift);
  ASSERT_EQ(1, arrow);
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_destroy(ctx));

  PASS();
}

TEST test_menu_item_visuals(void) {
  cmp_menu_item_t *item = NULL;
  int is_red;
  const char *icon = NULL;

  /* Destructive */
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_item_create(&item, "Delete", "trash",
                                              CMP_MENU_ITEM_ROLE_DESTRUCTIVE));
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_item_resolve_visuals(item, &is_red, &icon));
  ASSERT_EQ(1, is_red);
  ASSERT_EQ(NULL, icon);

  /* State Changes */
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_item_set_state(item, CMP_MENU_ITEM_STATE_ON));
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_item_resolve_visuals(item, &is_red, &icon));
  ASSERT_STR_EQ("checkmark", icon);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_menu_item_set_state(item, CMP_MENU_ITEM_STATE_MIXED));
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_item_resolve_visuals(item, &is_red, &icon));
  ASSERT_STR_EQ("minus", icon);

  /* Manual cleanup since we aren't attaching to a menu here to avoid leaking */
  /* cmp_menu_destroy normally handles children */
  /* We just hack it by making a dummy menu */
  {
    cmp_menu_t *m = NULL;
    cmp_menu_create(&m, CMP_MENU_PRESENTATION_ACTION_SHEET);
    cmp_menu_add_item(m, item);
    cmp_menu_destroy(m);
  }

  PASS();
}

TEST test_nested_menus(void) {
  cmp_menu_t *root = NULL;
  cmp_menu_t *sub = NULL;
  cmp_menu_item_t *sub_item = NULL;
  cmp_menu_item_t *def_item = NULL;
  int is_red;
  const char *icon = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_menu_create(&root, CMP_MENU_PRESENTATION_CONTEXT));
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_create(&sub, CMP_MENU_PRESENTATION_CONTEXT));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_menu_item_create_submenu(&sub_item, "Options", "gear", sub));
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_add_item(root, sub_item));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_menu_item_resolve_visuals(sub_item, &is_red, &icon));
  ASSERT_STR_EQ("chevron.right", icon);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_menu_item_create_deferred(&def_item, "Loading..."));
  ASSERT_EQ(CMP_SUCCESS, cmp_menu_add_item(root, def_item));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_menu_item_resolve_visuals(def_item, &is_red, &icon));
  ASSERT_STR_EQ("spinner", icon);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_menu_destroy(root)); /* Destroys all children including sub */
  PASS();
}

TEST test_null_args(void) {
  cmp_menu_t *m = NULL;
  cmp_menu_item_t *i = NULL;
  int r;
  const char *c;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_menu_create(NULL, CMP_MENU_PRESENTATION_CONTEXT));
  cmp_menu_create(&m, CMP_MENU_PRESENTATION_CONTEXT);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_menu_item_create(NULL, "a", "b", CMP_MENU_ITEM_ROLE_STANDARD));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_menu_item_create(&i, NULL, "b", CMP_MENU_ITEM_ROLE_STANDARD));
  cmp_menu_item_create(&i, "a", "b", CMP_MENU_ITEM_ROLE_STANDARD);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_menu_add_item(NULL, i));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_menu_add_item(m, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_menu_item_create_submenu(NULL, "a", "b", m));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_menu_item_create_submenu(&i, "a", "b", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_menu_item_set_state(NULL, CMP_MENU_ITEM_STATE_ON));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_menu_resolve_presentation(NULL, &r, &r, &r));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_menu_resolve_presentation(m, NULL, &r, &r));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_menu_resolve_presentation(m, &r, NULL, &r));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_menu_resolve_presentation(m, &r, &r, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_menu_item_resolve_visuals(NULL, &r, &c));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_menu_item_resolve_visuals(i, NULL, &c));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_menu_item_resolve_visuals(i, &r, NULL));

  cmp_menu_add_item(m, i);
  cmp_menu_destroy(m);
  PASS();
}

SUITE(menu_suite) {
  RUN_TEST(test_menu_presentation);
  RUN_TEST(test_menu_item_visuals);
  RUN_TEST(test_nested_menus);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(menu_suite);
  GREATEST_MAIN_END();
}
