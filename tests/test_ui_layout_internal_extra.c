void test_ui_layout_internal_extra(void) {
  struct ui_dom_node *root;
  struct ui_layout_node *out_node = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);

  /* Line 2593: create_layout_node(!out_node) */
  create_layout_node(root, NULL, 0, NULL);

  /* Line 1169: ui_layout_node_update_from_style(!node->computed_style) */
  create_layout_node(root, NULL, 0, &out_node);
  ui_layout_node_update_from_style(out_node);

  /* Line 2707: create_layout_node disp_rc != NONE */
  g_mock_prop_name = "display";
  g_mock_prop_val = "none";
  g_mock_prop_rc =
      UI_ERROR_INVALID_ARGUMENT; /* NOT_FOUND is skipped, INVALID_ARGUMENT
                                    triggers disp_rc */
  create_layout_node(root, (struct ui_css_computed_style *)1, 0, &out_node);

  /* Line 2865: ui_layout_box_model_calculate(NULL) */
  ui_layout_box_model_calculate(NULL, 0.0f, 0.0f);
}
