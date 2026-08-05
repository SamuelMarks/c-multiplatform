void test_property_grid_extra_err(void) {
  /* To hit out_group=NULL in get_or_create_group, we need to call it. But it's
  static and called via add_property and set_group_collapsed which don't pass
  NULL. So we can't easily hit it without modifying the source. Wait,
  ui_property_grid_base.c is compiled into the test directly? No, it's in
  libui_engine_test.a. */
}
