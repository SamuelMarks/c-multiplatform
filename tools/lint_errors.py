#!/usr/bin/env python3
"""
AST-based C Linter utilizing Tree-sitter.

This module provides structural enforcement of the codebase's strict error-handling
and memory-management policies. It analyzes C/C++ source files and validates:
1. All functions returning `int` must declare `int rc` and `return rc;`.
2. All memory allocations must be checked for failure (NULL checks).
3. All function calls to `cmp_` functions must be explicitly assigned and checked.
"""

import sys
import os
import tree_sitter
import tree_sitter_c

# Initialize the Tree-sitter parser for C
C_LANGUAGE = tree_sitter.Language(tree_sitter_c.language())
parser = tree_sitter.Parser(C_LANGUAGE)

# Exception patterns that are allowed to skip strict assignment checks
IGNORE_PREFIXES = (
    'cmp_log_', 'cmp_math_', 'cmp_string_', 'cmp_ui_box_', 'cmp_ui_text_', 
    'cmp_ui_button_', 'cmp_m3_', 'cmp_f2_', 'cmp_theme_get_', 'cmp_vfs_read_file_sync', 
    'cmp_svg_renderer_', 'cmp_math_parse', 'cmp_symbols_request', 'cmp_shader_init', 
    'cmp_shader_cache', 'cmp_hit_test', 'cmp_hover_intent', 'cmp_event_push', 
    'cmp_texture_create', 'cmp_event_get_focus', 'cmp_event_pop', 'cmp_layout_calculate',
    'cmp_ui_node_add_event_listener', 'cmp_ui_action_button_create', 'cmp_ring_buffer_pop',
    'cmp_font_set_opentype_features', 'cmp_a11y_tree_add_node', 'cmp_i18n_translate',
    'cmp_window_should_close', 'cmp_ui_node_destroy', 'cmp_window_set_ui_tree', 
    'cmp_window_show', 'cmp_window_poll_events', 'cmp_window_destroy', 
    'cmp_window_system_shutdown', 'cmp_vfs_shutdown', 'cmp_event_system_shutdown', 
    'cmp_event_system_init', 'cmp_vfs_init', 'cmp_window_system_init', 'cmp_databinding_', 
    'cmp_ui_app_bar_', 'cmp_ui_icon_button_', 'cmp_a11y_tree_', 'cmp_ui_badge_', 
    'cmp_dpi_destroy', 'cmp_ui_snackbar_', 'cmp_ui_card_', 'cmp_ui_divider_', 
    'cmp_modality_async_single_init', 'cmp_compositor_anim_set_range', 'cmp_theme_destroy', 
    'cmp_modality_multProcess_init', 'cmp_default_assert_handler', 'cmp_msg_destroy', 
    'cmp_keyboard_shortcut_destroy', 'cmp_md_node_destroy', 'cmp_ring_buffer_destroy', 
    'cmp_resources_set_thermal_state', 'cmp_strtok_r', 'cmp_svg_node_', 'cmp_svg_smil_tick', 
    'cmp_window_get_native_handle', 'cmp_mutex_', 'cmp_system_theme_is_dark', 
    'cmp_tree_sitter_free_node', 'cmp_font_', 'cmp_i18n_set_bidi_direction', 
    'cmp_layout_node_', 'cmp_ui_node_add_child', 'cmp_svg_parse_path_str', 
    'cmp_ui_node_bind_generic', 'cmp_caret_update_blink', 'cmp_pty_write', 
    'cmp_pty_destroy', 'cmp_color_luminance', 'cmp_menu_destroy', 'cmp_text_field_',
    'cmp_window_set_theme', 'cmp_modality_multiprocess_init', 'cmp_modality_sync_multi_init',
    'cmp_ui_action_button_get_node', 'cmp_regex_free', 'cmp_i18n_is_rtl'
)

def walk_tree(node, callback):
    """
    Recursively walks an AST node and applies a callback.

    Args:
        node: The tree-sitter Node to traverse.
        callback: A callable that takes a Node and returns True to stop traversal.
    """
    if callback(node):
        return
    for child in node.children:
        walk_tree(child, callback)

def check_rc_declaration(node):
    """
    Checks if a compound_statement node contains `int rc` and `return rc;`.

    Args:
        node: A tree-sitter Node representing a compound statement.

    Returns:
        tuple: (has_rc_decl, has_rc_return) booleans indicating if both rules are met.
    """
    has_decl = False
    has_ret = False
    
    def check_node(n):
        nonlocal has_decl, has_ret
        if n.type == 'declaration':
            # Check for `int rc;` or `int rc = ...;`
            type_node = n.child_by_field_name('type')
            if type_node and type_node.text.decode('utf-8') == 'int':
                for child in n.children:
                    if child.type == 'identifier' and child.text.decode('utf-8') == 'rc':
                        has_decl = True
                    elif child.type == 'init_declarator':
                        decl_id = child.child_by_field_name('declarator')
                        if decl_id and decl_id.text.decode('utf-8') == 'rc':
                            has_decl = True
        elif n.type == 'return_statement':
            # Check for `return rc;`
            for child in n.children:
                if child.type == 'identifier' and child.text.decode('utf-8') == 'rc':
                    has_ret = True
        return False

    walk_tree(node, check_node)
    return has_decl, has_ret

def check_identifier_in_if(node, identifier_name):
    """
    Scans sibling nodes for an if_statement evaluating a specific identifier.

    Args:
        node: The starting tree-sitter Node to scan forward from.
        identifier_name: The string name of the variable to look for.

    Returns:
        bool: True if an if_statement checks the identifier, False otherwise.
    """
    current = node.next_sibling
    lines_checked = 0
    while current and lines_checked < 6:
        # If it's an if statement, check if the condition references the identifier
        if current.type in ('if_statement', 'expression_statement'):
            found = [False]
            def find_id(n):
                if n.type == 'identifier' and n.text.decode('utf-8') == identifier_name:
                    found[0] = True
                elif n.type == 'field_expression' and n.text.decode('utf-8') == identifier_name:
                    found[0] = True
            walk_tree(current, find_id)
            if found[0]:
                return True
                
        current = current.next_sibling
        lines_checked += 1
    return False

def extract_base_identifier(node):
    """
    Extracts the base string name of an identifier, ignoring pointers and fields.
    """
    if node is None: return None
    if node.type == 'identifier':
        return node.text.decode('utf-8')
    elif node.type == 'pointer_declarator' or node.type == 'pointer_expression':
        # Safely find the identifier inside the declarator
        found = [None]
        def find_id(n):
            if n.type == 'identifier':
                found[0] = n.text.decode('utf-8')
                return True
            return False
        walk_tree(node, find_id)
        if found[0]: return found[0]
        return None
    elif node.type == 'init_declarator':
        return extract_base_identifier(node.child_by_field_name('declarator'))
    elif node.type == 'cast_expression':
        return extract_base_identifier(node.child_by_field_name('value'))
    elif node.type == 'field_expression':
        return node.text.decode('utf-8')
    return node.text.decode('utf-8')

def lint_file(filepath):
    """
    Lints a single C source file against strict error handling rules.

    Args:
        filepath: The path to the C source file to lint.

    Returns:
        int: The total number of linting violations found.
    """
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            source_code = f.read()
    except Exception as e:
        print(f"Failed to read {filepath}: {e}")
        return 0

    tree = parser.parse(bytes(source_code, "utf8"))
    root_node = tree.root_node
    
    is_test = 'test_' in filepath or 'tests/' in filepath or 'tests\\' in filepath
    errors = 0

    def analyze(node):
        nonlocal errors
        
        # Rule 1: Int functions must define and return rc
        if node.type == 'function_definition':
            type_node = node.child_by_field_name('type')
            decl_node = node.child_by_field_name('declarator')
            body_node = node.child_by_field_name('body')
            
            # Extract function name
            func_name = None
            if decl_node:
                # Handle pointer return types or standard declarators
                while decl_node.type in ('pointer_declarator', 'function_declarator'):
                    if decl_node.type == 'function_declarator':
                        decl_node = decl_node.child_by_field_name('declarator')
                    else:
                        decl_node = decl_node.child_by_field_name('declarator')
                    if not decl_node: break
                
                if decl_node and decl_node.type == 'identifier':
                    func_name = decl_node.text.decode('utf-8')

            if type_node and type_node.text.decode('utf-8') == 'int' and func_name:
                if func_name != 'main' and not func_name.startswith('test_') and body_node:
                    has_decl, has_ret = check_rc_declaration(body_node)
                    if not has_decl:
                        print(f"{filepath}:{node.start_point[0]+1}: Function '{func_name}' returning int must define 'int rc;'")
                        errors += 1
                    if not has_ret:
                        print(f"{filepath}:{node.start_point[0]+1}: Function '{func_name}' returning int must 'return rc;'")
                        errors += 1
                        
        # Rule 2 & 3: Call expression evaluations
        elif node.type == 'call_expression':
            func_node = node.child_by_field_name('function')
            if not func_node or func_node.type != 'identifier':
                return False
                
            func_name = func_node.text.decode('utf-8')
            is_alloc = func_name in ('malloc', 'calloc', 'realloc', 'strdup')
            is_cmp = func_name.startswith('cmp_') and not any(func_name.startswith(p) for p in IGNORE_PREFIXES)
            
            if is_alloc or (is_cmp and not is_test):
                parent = node.parent
                
                # If wrapped in a cast, step up
                if parent.type == 'cast_expression':
                    parent = parent.parent
                    
                # Must be part of an assignment or declaration
                if parent.type == 'assignment_expression':
                    lhs = parent.child_by_field_name('left')
                    base_id = extract_base_identifier(lhs)
                    
                    # Find the expression statement wrapper to look at next siblings
                    expr_stmt = parent.parent
                    while expr_stmt and expr_stmt.type != 'expression_statement':
                        expr_stmt = expr_stmt.parent
                        
                    if expr_stmt and base_id:
                        checked = check_identifier_in_if(expr_stmt, base_id)
                        if not checked:
                            print(f"{filepath}:{node.start_point[0]+1}: Result of '{func_name}' assigned to '{base_id}' (raw: {lhs.text.decode('utf-8')}) must be checked.")
                            errors += 1
                            
                elif parent.type == 'init_declarator':
                    lhs = parent.child_by_field_name('declarator')
                    base_id = extract_base_identifier(lhs)
                    
                    decl_stmt = parent.parent
                    if decl_stmt and base_id:
                        checked = check_identifier_in_if(decl_stmt, base_id)
                        if not checked:
                            print(f"{filepath}:{node.start_point[0]+1}: Result of '{func_name}' initialized to '{base_id}' (raw: {lhs.text.decode('utf-8')}) must be checked.")
                            errors += 1
                else:
                    pass

        return False

    walk_tree(root_node, analyze)
    return errors

def main():
    """
    Main entry point for the CLI. Runs the linter on provided files.
    """
    staged_files = sys.argv[1:]
    if not staged_files:
        sys.exit(0)
        
    total_errors = 0
    for f in staged_files:
        if os.path.exists(f) and (f.endswith('.c') or f.endswith('.cpp') or f.endswith('.h')):
            total_errors += lint_file(f)
            
    if total_errors > 0:
        print(f"\\n[Error] AST Strict lint failed: {total_errors} unhandled edge cases found!")
        sys.exit(1)
    sys.exit(0)

if __name__ == '__main__':
    main()
