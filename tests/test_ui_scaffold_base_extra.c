int test_scaffold_base_extra(void) {
  struct ui_scaffold_base *scaffold = NULL;
  ui_scaffold_base_create(NULL);
  ui_scaffold_base_create(&scaffold);
  if (scaffold) {
    ui_scaffold_base_set_top_bar(NULL, NULL);
    ui_scaffold_base_set_top_bar(scaffold, NULL);
    ui_scaffold_base_set_main_content(NULL, NULL);
    ui_scaffold_base_set_main_content(scaffold, NULL);
    ui_scaffold_base_bind_data(NULL, NULL);
    ui_scaffold_base_bind_data(scaffold, NULL);
    (void)ui_component_destroy((struct ui_component *)scaffold);
  }
  return 0;
}

int test_scaffold_base_oom(void) {
  extern int g_malloc_fail_countdown;
  struct ui_scaffold_base *scaffold = NULL;
  int i;
  for (i = 0; i < 20; ++i) {
    g_malloc_fail_countdown = i;
    ui_scaffold_base_create(&scaffold);
  }
  g_malloc_fail_countdown = -1;
  return 0;
}
