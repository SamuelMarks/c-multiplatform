/* clang-format off */
#ifndef CMP_CSS_SELECTORS_H
#define CMP_CSS_SELECTORS_H

#include <stddef.h>

/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_selectors.h
 * @brief CSS Selectors (Level 3 & 4) Implementation.
 */

/**
 * @brief Opaque type representing a DOM node for CSS selection.
 */
/**
 * @brief Opaque type representing a DOM node for CSS selection.
 */
typedef struct cmp_dom_node cmp_dom_node_t;

/**
 * @brief Form state for input pseudo-classes.
 */
typedef struct cmp_form_state {
  int is_enabled;
  int is_disabled;
  int is_read_only;
  int is_read_write;
  int is_placeholder_shown;
  int is_default;
  int is_checked;
  int is_indeterminate;
  int is_blank;
  int is_valid;
  int is_invalid;
  int is_in_range;
  int is_out_of_range;
  int is_required;
  int is_optional;
  int is_user_valid;
  int is_user_invalid;
  int is_autofill;
} cmp_form_state_t;

/**
 * @brief Virtual table for DOM node property access.
 */
typedef struct cmp_dom_node_vtable {
  /**
   * @brief Get the tag name of a DOM node.
   * @param node The DOM node.
   * @param out_tag Pointer to store the tag name string pointer.
   * @return int 0 on success.
   */
  int (*get_tag)(const cmp_dom_node_t *node, const char **out_tag);

  /**
   * @brief Get the ID of a DOM node.
   * @param node The DOM node.
   * @param out_id Pointer to store the ID string pointer.
   * @return int 0 on success.
   */
  int (*get_id)(const cmp_dom_node_t *node, const char **out_id);

  /**
   * @brief Get the classes string of a DOM node (space-separated).
   * @param node The DOM node.
   * @param out_classes Pointer to store the classes string pointer.
   * @return int 0 on success.
   */
  int (*get_classes)(const cmp_dom_node_t *node, const char **out_classes);

  /**
   * @brief Get an attribute value of a DOM node.
   * @param node The DOM node.
   * @param attr_name The attribute name to look up.
   * @param out_val Pointer to store the attribute value string pointer (NULL if
   * not found).
   * @return int 0 on success.
   */
  int (*get_attribute)(const cmp_dom_node_t *node, const char *attr_name,
                       const char **out_val);

  /**
   * @brief Get the parent node of a DOM node.
   * @param node The DOM node.
   * @param out_parent Pointer to store the parent node pointer (NULL if no
   * parent).
   * @return int 0 on success.
   */
  int (*get_parent)(const cmp_dom_node_t *node,
                    const cmp_dom_node_t **out_parent);

  /**
   * @brief Get the previous sibling node of a DOM node.
   * @param node The DOM node.
   * @param out_prev Pointer to store the previous sibling node pointer (NULL if
   * no previous sibling).
   * @return int 0 on success.
   */
  int (*get_prev_sibling)(const cmp_dom_node_t *node,
                          const cmp_dom_node_t **out_prev);

  /**
   * @brief Get the first child node of a DOM node.
   * @param node The DOM node.
   * @param out_child Pointer to store the first child node pointer (NULL if no
   * children).
   * @return int 0 on success.
   */
  int (*get_first_child)(const cmp_dom_node_t *node,
                         const cmp_dom_node_t **out_child);

  /**
   * @brief Get the next sibling node of a DOM node.
   * @param node The DOM node.
   * @param out_next Pointer to store the next sibling node pointer (NULL if no
   * next sibling).
   * @return int 0 on success.
   */
  int (*get_next_sibling)(const cmp_dom_node_t *node,
                          const cmp_dom_node_t **out_next);

  /**
   * @brief Evaluates whether a cell node belongs to a column that matches the
   * given column selector.
   * @param node The cell DOM node.
   * @param col_sel The column selector to evaluate against candidate columns.
   * @param out_match Pointer to store the result (1 if true, 0 if false).
   * @return int 0 on success.
   */
  int (*match_column_selector)(const cmp_dom_node_t *node,
                               const struct cmp_selector *col_sel,
                               int *out_match);

  /**
   * @brief Determine the directionality of the node for :dir() pseudo-class.
   * @param node The DOM node.
   * @param out_dir Pointer to store "ltr" or "rtl" (or NULL if unknown).
   * @return int 0 on success.
   */
  int (*get_dir)(const cmp_dom_node_t *node, const char **out_dir);

  /**
   * @brief Determine the language of the node for :lang() pseudo-class.
   * @param node The DOM node.
   * @param out_lang Pointer to store the language tag (e.g. "en", "fr-CA") or
   * NULL if unknown.
   * @return int 0 on success.
   */
  int (*get_lang)(const cmp_dom_node_t *node, const char **out_lang);

  /**
   * @brief Determine if the node is a link and its state.
   * @param node The DOM node.
   * @param out_is_any_link 1 if it's any link, 0 otherwise.
   * @param out_is_link 1 if it's an unvisited link, 0 otherwise.
   * @param out_is_visited 1 if it's a visited link, 0 otherwise.
   * @param out_is_local_link 1 if it's a local link, 0 otherwise.
   * @return int 0 on success.
   */
  int (*get_link_state)(const cmp_dom_node_t *node, int *out_is_any_link,
                        int *out_is_link, int *out_is_visited,
                        int *out_is_local_link);

  /**
   * @brief Determine if the node is the target of the document's URL.
   * @param node The DOM node.
   * @param out_is_target 1 if it's the target, 0 otherwise.
   * @return int 0 on success.
   */
  int (*is_target)(const cmp_dom_node_t *node, int *out_is_target);

  /**
   * @brief Determine if the node or its descendants are the target.
   * @param node The DOM node.
   * @param out_is_target_within 1 if it's the target-within, 0 otherwise.
   * @return int 0 on success.
   */
  int (*is_target_within)(const cmp_dom_node_t *node,
                          int *out_is_target_within);

  /**
   /**
    * @brief Determine if the node is a scope element for the :scope
   pseudo-class.
    * @param node The DOM node.
    * @param out_is_scope 1 if it's the scope element, 0 otherwise.
    * @return int 0 on success.
    */
  int (*is_scope)(const cmp_dom_node_t *node, int *out_is_scope);

  /**
   * @brief Determine user action and focus states.
   * @param node The DOM node.
   * @param out_is_hover 1 if hovered, 0 otherwise.
   * @param out_is_active 1 if active, 0 otherwise.
   * @param out_is_focus 1 if focused, 0 otherwise.
   * @param out_is_focus_visible 1 if focus-visible, 0 otherwise.
   * @param out_is_focus_within 1 if focus-within, 0 otherwise.
   * @return int 0 on success.
   */
  int (*get_user_action_state)(const cmp_dom_node_t *node, int *out_is_hover,
                               int *out_is_active, int *out_is_focus,
                               int *out_is_focus_visible,
                               int *out_is_focus_within);

  /**
   * @brief Determine time-dimensional states.
   * @param node The DOM node.
   * @param out_is_current 1 if current, 0 otherwise.
   * @param out_is_past 1 if past, 0 otherwise.
   * @param out_is_future 1 if future, 0 otherwise.
   * @return int 0 on success.
   */
  int (*get_time_state)(const cmp_dom_node_t *node, int *out_is_current,
                        int *out_is_past, int *out_is_future);

  /**
   * @brief Determine input/form states.
   * @param node The DOM node.
   * @param out_state Pointer to cmp_form_state_t to be populated.
   * @return int 0 on success.
   */
  int (*get_form_state)(const cmp_dom_node_t *node,
                        cmp_form_state_t *out_state);

  /**
   * @brief Determine DOM and Shadow DOM states.
   * @param node The DOM node.
   * @param out_is_modal 1 if modal, 0 otherwise.
   * @param out_is_fullscreen 1 if fullscreen, 0 otherwise.
   * @param out_is_pip 1 if picture-in-picture, 0 otherwise.
   * @param out_is_popover_open 1 if popover is open, 0 otherwise.
   * @param out_is_defined 1 if custom element is defined, 0 otherwise.
   * @return int 0 on success.
   */
  int (*get_dom_state)(const cmp_dom_node_t *node, int *out_is_modal,
                       int *out_is_fullscreen, int *out_is_pip,
                       int *out_is_popover_open, int *out_is_defined);

  /**
   /**
    * @brief Get the shadow host of the current shadow root (if any).
    * @param node The DOM node (usually the shadow root itself, or a node inside
   it).
    * @param out_host Pointer to store the host node, or NULL if not in a shadow
   tree.
    * @return int 0 on success.
    */
  int (*get_shadow_host)(const cmp_dom_node_t *node,
                         const cmp_dom_node_t **out_host);

  /**
   * @brief Get media playback and volume state.
   * @param node The DOM node (e.g. video, audio).
   * @param out_is_playing 1 if playing, 0 otherwise.
   * @param out_is_paused 1 if paused, 0 otherwise.
   * @param out_is_muted 1 if muted, 0 otherwise.
   * @param out_is_volume_locked 1 if volume locked, 0 otherwise.
   * @return int 0 on success.
   */
  int (*get_media_state)(const cmp_dom_node_t *node, int *out_is_playing,
                         int *out_is_paused, int *out_is_muted,
                         int *out_is_volume_locked);

  /**
   * @brief Determine if the node is assigned to a slot.
   * @param node The DOM node.
   * @param out_is_slotted 1 if slotted, 0 otherwise.
   * @return int 0 on success.
   */
  int (*is_slotted)(const cmp_dom_node_t *node, int *out_is_slotted);

  /**
   * @brief Determine if the node has a specific part name.
   * @param node The DOM node.
   * @param part_name The part name to check.
   * @param out_has_part 1 if it has the part, 0 otherwise.
   * @return int 0 on success.
   */
  int (*has_part)(const cmp_dom_node_t *node, const char *part_name,
                  int *out_has_part);
} cmp_dom_node_vtable_t;

/**
 * @brief Base DOM node structure.
 */
struct cmp_dom_node {
  const cmp_dom_node_vtable_t *vtable;
};

/**
 * @brief Abstract CSS Selector interface.
 */
typedef struct cmp_selector cmp_selector_t;

/**
 * @brief Match callback for an abstract CSS Selector.
 * @param sel The selector instance.
 * @param node The DOM node to evaluate against.
 * @param out_match Pointer to store the result (1 for match, 0 for no match).
 * @return int 0 on success, non-zero on failure.
 */
typedef int (*cmp_selector_match_cb_t)(const cmp_selector_t *sel,
                                       const cmp_dom_node_t *node,
                                       int *out_match);

struct cmp_selector {
  cmp_selector_match_cb_t match;
};

/**
 * @brief Universal selector struct. Matches any element.
 */
typedef struct cmp_sel_universal {
  cmp_selector_t base; /**< Base interface */
} cmp_sel_universal_t;

/**
 * @brief Tag selector struct. Matches specific HTML tags.
 */
typedef struct cmp_sel_tag {
  cmp_selector_t base;  /**< Base interface */
  const char *tag_name; /**< The tag name to match (e.g., "div") */
} cmp_sel_tag_t;

/**
 * @brief Class selector struct. Matches specific CSS classes.
 */
typedef struct cmp_sel_class {
  cmp_selector_t base;    /**< Base interface */
  const char *class_name; /**< The class name to match (e.g., "container") */
} cmp_sel_class_t;

/**
 * @brief ID selector struct. Matches a specific element ID.
 */
typedef struct cmp_sel_id {
  cmp_selector_t base; /**< Base interface */
  const char *id_name; /**< The ID name to match (e.g., "main-header") */
} cmp_sel_id_t;

/**
 * @brief Attribute selector matching operators.
 */
typedef enum cmp_sel_attr_op {
  CMP_SEL_ATTR_OP_EXISTS,     /**< [attr] */
  CMP_SEL_ATTR_OP_EQUALS,     /**< [attr=val] */
  CMP_SEL_ATTR_OP_PREFIX,     /**< [attr^=val] */
  CMP_SEL_ATTR_OP_SUFFIX,     /**< [attr$=val] */
  CMP_SEL_ATTR_OP_CONTAINS,   /**< [attr*=val] */
  CMP_SEL_ATTR_OP_WHITESPACE, /**< [attr~=val] */
  CMP_SEL_ATTR_OP_DASH        /**< [attr|=val] */
} cmp_sel_attr_op_t;

/**
 * @brief Attribute selector modifier.
 */
typedef enum cmp_sel_attr_mod {
  CMP_SEL_ATTR_MOD_NONE,   /**< No modifier */
  CMP_SEL_ATTR_MOD_NOCASE, /**< i modifier (case-insensitive) */
  CMP_SEL_ATTR_MOD_CASE    /**< s modifier (case-sensitive) */
} cmp_sel_attr_mod_t;

/**
 * @brief Attribute selector struct.
 */
typedef struct cmp_sel_attr {
  cmp_selector_t base;    /**< Base interface */
  const char *attr_name;  /**< Attribute name to match */
  const char *attr_value; /**< Attribute value to match against (can be NULL if
                             op is EXISTS) */
  cmp_sel_attr_op_t op;   /**< Matching operator */
  cmp_sel_attr_mod_t modifier; /**< Case-sensitivity modifier */
} cmp_sel_attr_t;

/**
 * @brief Descendant combinator (space).
 */
typedef struct cmp_sel_descendant {
  cmp_selector_t base;         /**< Base selector interface */
  const cmp_selector_t *left;  /**< Ancestor selector */
  const cmp_selector_t *right; /**< Descendant selector */
} cmp_sel_descendant_t;

/**
 * @brief Child combinator (>).
 */
typedef struct cmp_sel_child {
  cmp_selector_t base;         /**< Base selector interface */
  const cmp_selector_t *left;  /**< Parent selector */
  const cmp_selector_t *right; /**< Child selector */
} cmp_sel_child_t;

/**
 * @brief Next-sibling combinator (+).
 */
typedef struct cmp_sel_next_sibling {
  cmp_selector_t base;         /**< Base selector interface */
  const cmp_selector_t *left;  /**< Previous sibling selector */
  const cmp_selector_t *right; /**< Next sibling selector */
} cmp_sel_next_sibling_t;

/**
 * @brief Subsequent-sibling combinator (~).
 */
typedef struct cmp_sel_subsequent_sibling {
  cmp_selector_t base;         /**< Base selector interface */
  const cmp_selector_t *left;  /**< Preceding sibling selector */
  const cmp_selector_t *right; /**< Following sibling selector */
} cmp_sel_subsequent_sibling_t;

/**
 * @brief Column combinator (||).
 */
typedef struct cmp_sel_column {
  cmp_selector_t base;         /**< Base selector interface */
  const cmp_selector_t *left;  /**< Column selector */
  const cmp_selector_t *right; /**< Cell selector */
} cmp_sel_column_t;

/**
 * @brief Evaluate an abstract selector against a DOM node.
 *
 * @param sel The selector instance.
 * @param node The DOM node to evaluate against.
 * @param out_match Pointer to store the match result.
 * @return int 0 on success, non-zero on failure.
 */
int cmp_selector_match(const cmp_selector_t *sel, const cmp_dom_node_t *node,
                       int *out_match);

/**
 * @brief Matches a universal selector against a DOM node.
 *
 * @param sel The universal selector.
 * @param node The DOM node to test.
 * @param out_match Pointer to an integer to store the match result (1 for true,
 * 0 for false).
 * @return int 0 on success, non-zero on failure.
 */
int cmp_sel_universal_match(const cmp_sel_universal_t *sel,
                            const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a tag selector against a DOM node.
 *
 * @param sel The tag selector.
 * @param node The DOM node to test.
 * @param out_match Pointer to an integer to store the match result (1 for true,
 * 0 for false).
 * @return int 0 on success, non-zero on failure.
 */
int cmp_sel_tag_match(const cmp_sel_tag_t *sel, const cmp_dom_node_t *node,
                      int *out_match);

/**
 * @brief Matches a class selector against a DOM node.
 *
 * @param sel The class selector.
 * @param node The DOM node to test.
 * @param out_match Pointer to an integer to store the match result (1 for true,
 * 0 for false).
 * @return int 0 on success, non-zero on failure.
 */
int cmp_sel_class_match(const cmp_sel_class_t *sel, const cmp_dom_node_t *node,
                        int *out_match);

/**
 * @brief Matches an ID selector against a DOM node.
 *
 * @param sel The ID selector.
 * @param node The DOM node to test.
 * @param out_match Pointer to an integer to store the match result (1 for true,
 * 0 for false).
 * @return int 0 on success, non-zero on failure.
 */
int cmp_sel_id_match(const cmp_sel_id_t *sel, const cmp_dom_node_t *node,
                     int *out_match);

/**
 * @brief Matches an attribute selector against a DOM node.
 *
 * @param sel The attribute selector.
 * @param node The DOM node to test.
 * @param out_match Pointer to an integer to store the match result (1 for true,
 * 0 for false).
 * @return int 0 on success, non-zero on failure.
 */
int cmp_sel_attr_match(const cmp_sel_attr_t *sel, const cmp_dom_node_t *node,
                       int *out_match);

/**
 * @brief Matches a descendant combinator.
 * @param sel The descendant combinator.
 * @param node The DOM node to test.
 * @param out_match Pointer to store the match result.
 * @return int 0 on success.
 */
int cmp_sel_descendant_match(const cmp_sel_descendant_t *sel,
                             const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a child combinator.
 * @param sel The child combinator.
 * @param node The DOM node to test.
 * @param out_match Pointer to store the match result.
 * @return int 0 on success.
 */
int cmp_sel_child_match(const cmp_sel_child_t *sel, const cmp_dom_node_t *node,
                        int *out_match);

/**
 * @brief Matches a next-sibling combinator.
 * @param sel The next-sibling combinator.
 * @param node The DOM node to test.
 * @param out_match Pointer to store the match result.
 * @return int 0 on success.
 */
int cmp_sel_next_sibling_match(const cmp_sel_next_sibling_t *sel,
                               const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a subsequent-sibling combinator.
 * @param sel The subsequent-sibling combinator.
 * @param node The DOM node to test.
 * @param out_match Pointer to store the match result.
 * @return int 0 on success.
 */
int cmp_sel_subsequent_sibling_match(const cmp_sel_subsequent_sibling_t *sel,
                                     const cmp_dom_node_t *node,
                                     int *out_match);

/**
 * @brief Matches a column combinator.
 * @param sel The column combinator.
 * @param node The DOM node to test.
 * @param out_match Pointer to store the match result.
 * @return int 0 on success.
 */
int cmp_sel_column_match(const cmp_sel_column_t *sel,
                         const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Logical pseudo-class :is()
 */
typedef struct cmp_sel_is {
  cmp_selector_t base;              /**< Base interface */
  const cmp_selector_t **selectors; /**< Array of selector pointers */
  size_t selector_count;            /**< Number of selectors */
} cmp_sel_is_t;

/**
 * @brief Logical pseudo-class :where()
 */
typedef struct cmp_sel_where {
  cmp_selector_t base;              /**< Base interface */
  const cmp_selector_t **selectors; /**< Array of selector pointers */
  size_t selector_count;            /**< Number of selectors */
} cmp_sel_where_t;

/**
 * @brief Logical pseudo-class :has()
 */
typedef struct cmp_sel_has {
  cmp_selector_t base;              /**< Base interface */
  const cmp_selector_t **selectors; /**< Array of relative selector pointers */
  size_t selector_count;            /**< Number of selectors */
} cmp_sel_has_t;

/**
 * @brief Logical pseudo-class :not()
 */
typedef struct cmp_sel_not {
  cmp_selector_t base;              /**< Base interface */
  const cmp_selector_t **selectors; /**< Array of selector pointers */
  size_t selector_count;            /**< Number of selectors */
} cmp_sel_not_t;

/**
 * @brief Matches an :is() pseudo-class.
 * @param sel The :is() selector.
 * @param node The DOM node to test.
 * @param out_match Pointer to store the match result.
 * @return int 0 on success.
 */
int cmp_sel_is_match(const cmp_sel_is_t *sel, const cmp_dom_node_t *node,
                     int *out_match);

/**
 * @brief Matches a :where() pseudo-class.
 * @param sel The :where() selector.
 * @param node The DOM node to test.
 * @param out_match Pointer to store the match result.
 * @return int 0 on success.
 */
int cmp_sel_where_match(const cmp_sel_where_t *sel, const cmp_dom_node_t *node,
                        int *out_match);

/**
 * @brief Matches a :has() pseudo-class.
 * @param sel The :has() selector.
 * @param node The DOM node to test.
 * @param out_match Pointer to store the match result.
 * @return int 0 on success.
 */
int cmp_sel_has_match(const cmp_sel_has_t *sel, const cmp_dom_node_t *node,
                      int *out_match);

/**
 * @brief Matches a :not() pseudo-class.
 * @param sel The :not() selector.
 * @param node The DOM node to test.
 * @param out_match Pointer to store the match result.
 * @return int 0 on success.
 */
int cmp_sel_not_match(const cmp_sel_not_t *sel, const cmp_dom_node_t *node,
                      int *out_match);

/**
 * @brief Represents the An+B pattern used in nth-child and similar
 * pseudo-classes.
 */
typedef struct cmp_an_plus_b {
  int a;
  int b;
} cmp_an_plus_b_t;

/**
 * @brief Tree-structural pseudo-class `:empty`
 */
typedef struct cmp_pseudo_empty {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_empty_t;

/**
 * @brief Tree-structural pseudo-class `:root`
 */
typedef struct cmp_pseudo_root {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_root_t;

/**
 * @brief Tree-structural pseudo-class `:nth-child(An+B)`
 */
typedef struct cmp_pseudo_nth_child {
  cmp_selector_t base;               /**< Base interface */
  cmp_an_plus_b_t pattern;           /**< An+B pattern */
  const cmp_selector_t *of_selector; /**< Optional 'of <selector>' part */
} cmp_pseudo_nth_child_t;

/**
 * @brief Tree-structural pseudo-class `:nth-last-child(An+B)`
 */
typedef struct cmp_pseudo_nth_last_child {
  cmp_selector_t base;               /**< Base interface */
  cmp_an_plus_b_t pattern;           /**< An+B pattern */
  const cmp_selector_t *of_selector; /**< Optional 'of <selector>' part */
} cmp_pseudo_nth_last_child_t;

/**
 * @brief Tree-structural pseudo-class `:first-child`
 */
typedef struct cmp_pseudo_first_child {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_first_child_t;

/**
 * @brief Tree-structural pseudo-class `:last-child`
 */
typedef struct cmp_pseudo_last_child {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_last_child_t;

/**
 * @brief Tree-structural pseudo-class `:only-child`
 */
typedef struct cmp_pseudo_only_child {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_only_child_t;

/**
 * @brief Tree-structural pseudo-class `:nth-of-type(An+B)`
 */
typedef struct cmp_pseudo_nth_of_type {
  cmp_selector_t base;     /**< Base interface */
  cmp_an_plus_b_t pattern; /**< An+B pattern */
} cmp_pseudo_nth_of_type_t;

/**
 * @brief Tree-structural pseudo-class `:nth-last-of-type(An+B)`
 */
typedef struct cmp_pseudo_nth_last_of_type {
  cmp_selector_t base;     /**< Base interface */
  cmp_an_plus_b_t pattern; /**< An+B pattern */
} cmp_pseudo_nth_last_of_type_t;

/**
 * @brief Tree-structural pseudo-class `:first-of-type`
 */
typedef struct cmp_pseudo_first_of_type {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_first_of_type_t;

/**
 * @brief Tree-structural pseudo-class `:last-of-type`
 */
typedef struct cmp_pseudo_last_of_type {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_last_of_type_t;

/**
 * @brief Tree-structural pseudo-class `:only-of-type`
 */
typedef struct cmp_pseudo_only_of_type {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_only_of_type_t;

/**
 * @brief Matches an :empty pseudo-class.
 */
int cmp_pseudo_empty_match(const cmp_pseudo_empty_t *sel,
                           const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :root pseudo-class.
 */
int cmp_pseudo_root_match(const cmp_pseudo_root_t *sel,
                          const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches an :nth-child() pseudo-class.
 */
int cmp_pseudo_nth_child_match(const cmp_pseudo_nth_child_t *sel,
                               const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches an :nth-last-child() pseudo-class.
 */
int cmp_pseudo_nth_last_child_match(const cmp_pseudo_nth_last_child_t *sel,
                                    const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :first-child pseudo-class.
 */
int cmp_pseudo_first_child_match(const cmp_pseudo_first_child_t *sel,
                                 const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :last-child pseudo-class.
 */
int cmp_pseudo_last_child_match(const cmp_pseudo_last_child_t *sel,
                                const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches an :only-child pseudo-class.
 */
int cmp_pseudo_only_child_match(const cmp_pseudo_only_child_t *sel,
                                const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches an :nth-of-type() pseudo-class.
 */
int cmp_pseudo_nth_of_type_match(const cmp_pseudo_nth_of_type_t *sel,
                                 const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches an :nth-last-of-type() pseudo-class.
 */
int cmp_pseudo_nth_last_of_type_match(const cmp_pseudo_nth_last_of_type_t *sel,
                                      const cmp_dom_node_t *node,
                                      int *out_match);

/**
 * @brief Matches a :first-of-type pseudo-class.
 */
int cmp_pseudo_first_of_type_match(const cmp_pseudo_first_of_type_t *sel,
                                   const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :last-of-type pseudo-class.
 */
int cmp_pseudo_last_of_type_match(const cmp_pseudo_last_of_type_t *sel,
                                  const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches an :only-of-type pseudo-class.
 */
int cmp_pseudo_only_of_type_match(const cmp_pseudo_only_of_type_t *sel,
                                  const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Linguistic pseudo-class `:dir()`
 */
typedef struct cmp_pseudo_dir {
  cmp_selector_t base; /**< Base interface */
  const char *dir;     /**< "ltr" or "rtl" */
} cmp_pseudo_dir_t;

/**
 * @brief Linguistic pseudo-class `:lang()`
 */
typedef struct cmp_pseudo_lang {
  cmp_selector_t base; /**< Base interface */
  const char **langs;  /**< Array of language strings */
  size_t lang_count;   /**< Number of languages */
} cmp_pseudo_lang_t;

/**
 * @brief Document pseudo-class `:any-link`
 */
typedef struct cmp_pseudo_any_link {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_any_link_t;

/**
 * @brief Document pseudo-class `:link`
 */
typedef struct cmp_pseudo_link {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_link_t;

/**
 * @brief Document pseudo-class `:visited`
 */
typedef struct cmp_pseudo_visited {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_visited_t;

/**
 * @brief Document pseudo-class `:local-link`
 */
typedef struct cmp_pseudo_local_link {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_local_link_t;

/**
 * @brief Document pseudo-class `:target`
 */
typedef struct cmp_pseudo_target {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_target_t;

/**
 * @brief Document pseudo-class `:target-within`
 */
typedef struct cmp_pseudo_target_within {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_target_within_t;

/**
 * @brief Document pseudo-class `:scope`
 */
typedef struct cmp_pseudo_scope {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_scope_t;

/**
 * @brief Matches a :dir() pseudo-class.
 */
int cmp_pseudo_dir_match(const cmp_pseudo_dir_t *sel,
                         const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :lang() pseudo-class.
 */
int cmp_pseudo_lang_match(const cmp_pseudo_lang_t *sel,
                          const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches an :any-link pseudo-class.
 */
int cmp_pseudo_any_link_match(const cmp_pseudo_any_link_t *sel,
                              const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :link pseudo-class.
 */
int cmp_pseudo_link_match(const cmp_pseudo_link_t *sel,
                          const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :visited pseudo-class.
 */
int cmp_pseudo_visited_match(const cmp_pseudo_visited_t *sel,
                             const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :local-link pseudo-class.
 */
int cmp_pseudo_local_link_match(const cmp_pseudo_local_link_t *sel,
                                const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :target pseudo-class.
 */
int cmp_pseudo_target_match(const cmp_pseudo_target_t *sel,
                            const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :target-within pseudo-class.
 */
int cmp_pseudo_target_within_match(const cmp_pseudo_target_within_t *sel,
                                   const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :scope pseudo-class.
 */
int cmp_pseudo_scope_match(const cmp_pseudo_scope_t *sel,
                           const cmp_dom_node_t *node, int *out_match);

/**
 * @brief User action pseudo-class `:hover`
 */
typedef struct cmp_pseudo_hover {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_hover_t;

/**
 * @brief User action pseudo-class `:active`
 */
typedef struct cmp_pseudo_active {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_active_t;

/**
 * @brief User action pseudo-class `:focus`
 */
typedef struct cmp_pseudo_focus {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_focus_t;

/**
 * @brief User action pseudo-class `:focus-visible`
 */
typedef struct cmp_pseudo_focus_visible {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_focus_visible_t;

/**
 * @brief User action pseudo-class `:focus-within`
 */
typedef struct cmp_pseudo_focus_within {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_focus_within_t;

/**
 * @brief Time-dimensional pseudo-class `:current`
 */
typedef struct cmp_pseudo_current {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_current_t;

/**
 * @brief Time-dimensional pseudo-class `:past`
 */
typedef struct cmp_pseudo_past {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_past_t;

/**
 * @brief Time-dimensional pseudo-class `:future`
 */
typedef struct cmp_pseudo_future {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_future_t;

/**
 * @brief Matches a :hover pseudo-class.
 */
int cmp_pseudo_hover_match(const cmp_pseudo_hover_t *sel,
                           const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches an :active pseudo-class.
 */
int cmp_pseudo_active_match(const cmp_pseudo_active_t *sel,
                            const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :focus pseudo-class.
 */
int cmp_pseudo_focus_match(const cmp_pseudo_focus_t *sel,
                           const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :focus-visible pseudo-class.
 */
int cmp_pseudo_focus_visible_match(const cmp_pseudo_focus_visible_t *sel,
                                   const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :focus-within pseudo-class.
 */
int cmp_pseudo_focus_within_match(const cmp_pseudo_focus_within_t *sel,
                                  const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :current pseudo-class.
 */
int cmp_pseudo_current_match(const cmp_pseudo_current_t *sel,
                             const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :past pseudo-class.
 */
int cmp_pseudo_past_match(const cmp_pseudo_past_t *sel,
                          const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :future pseudo-class.
 */
int cmp_pseudo_future_match(const cmp_pseudo_future_t *sel,
                            const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Input pseudo-class `:enabled` or `:disabled`
 */
typedef struct cmp_pseudo_enabled_disabled {
  cmp_selector_t base; /**< Base interface */
  int is_disabled;     /**< 1 for :disabled, 0 for :enabled */
} cmp_pseudo_enabled_disabled_t;

/**
 * @brief Input pseudo-class `:read-only` or `:read-write`
 */
typedef struct cmp_pseudo_read_only_write {
  cmp_selector_t base; /**< Base interface */
  int is_read_write;   /**< 1 for :read-write, 0 for :read-only */
} cmp_pseudo_read_only_write_t;

/**
 * @brief Input pseudo-class `:placeholder-shown`
 */
typedef struct cmp_pseudo_placeholder_shown {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_placeholder_shown_t;

/**
 * @brief Input pseudo-class `:default`
 */
typedef struct cmp_pseudo_default {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_default_t;

/**
 * @brief Input pseudo-class `:checked`
 */
typedef struct cmp_pseudo_checked {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_checked_t;

/**
 * @brief Input pseudo-class `:indeterminate`
 */
typedef struct cmp_pseudo_indeterminate {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_indeterminate_t;

/**
 * @brief Input pseudo-class `:blank`
 */
typedef struct cmp_pseudo_blank {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_blank_t;

/**
 * @brief Input pseudo-class `:valid` or `:invalid`
 */
typedef struct cmp_pseudo_valid_invalid {
  cmp_selector_t base; /**< Base interface */
  int is_invalid;      /**< 1 for :invalid, 0 for :valid */
} cmp_pseudo_valid_invalid_t;

/**
 * @brief Input pseudo-class `:in-range` or `:out-of-range`
 */
typedef struct cmp_pseudo_range {
  cmp_selector_t base; /**< Base interface */
  int is_out_of_range; /**< 1 for :out-of-range, 0 for :in-range */
} cmp_pseudo_range_t;

/**
 * @brief Input pseudo-class `:required` or `:optional`
 */
typedef struct cmp_pseudo_required_optional {
  cmp_selector_t base; /**< Base interface */
  int is_optional;     /**< 1 for :optional, 0 for :required */
} cmp_pseudo_required_optional_t;

/**
 * @brief Input pseudo-class `:user-valid` or `:user-invalid`
 */
typedef struct cmp_pseudo_user_validity {
  cmp_selector_t base; /**< Base interface */
  int is_invalid;      /**< 1 for :user-invalid, 0 for :user-valid */
} cmp_pseudo_user_validity_t;

/**
 * @brief Input pseudo-class `:autofill`
 */
typedef struct cmp_pseudo_autofill {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_autofill_t;

/**
 * @brief Matches an :enabled or :disabled pseudo-class.
 */
int cmp_pseudo_enabled_disabled_match(const cmp_pseudo_enabled_disabled_t *sel,
                                      const cmp_dom_node_t *node,
                                      int *out_match);

/**
 * @brief Matches a :read-only or :read-write pseudo-class.
 */
int cmp_pseudo_read_only_write_match(const cmp_pseudo_read_only_write_t *sel,
                                     const cmp_dom_node_t *node,
                                     int *out_match);

/**
 * @brief Matches a :placeholder-shown pseudo-class.
 */
int cmp_pseudo_placeholder_shown_match(
    const cmp_pseudo_placeholder_shown_t *sel, const cmp_dom_node_t *node,
    int *out_match);

/**
 * @brief Matches a :default pseudo-class.
 */
int cmp_pseudo_default_match(const cmp_pseudo_default_t *sel,
                             const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :checked pseudo-class.
 */
int cmp_pseudo_checked_match(const cmp_pseudo_checked_t *sel,
                             const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches an :indeterminate pseudo-class.
 */
int cmp_pseudo_indeterminate_match(const cmp_pseudo_indeterminate_t *sel,
                                   const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :blank pseudo-class.
 */
int cmp_pseudo_blank_match(const cmp_pseudo_blank_t *sel,
                           const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :valid or :invalid pseudo-class.
 */
int cmp_pseudo_valid_invalid_match(const cmp_pseudo_valid_invalid_t *sel,
                                   const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches an :in-range or :out-of-range pseudo-class.
 */
int cmp_pseudo_range_match(const cmp_pseudo_range_t *sel,
                           const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :required or :optional pseudo-class.
 */
int cmp_pseudo_required_optional_match(
    const cmp_pseudo_required_optional_t *sel, const cmp_dom_node_t *node,
    int *out_match);

/**
 * @brief Matches a :user-valid or :user-invalid pseudo-class.
 */
int cmp_pseudo_user_validity_match(const cmp_pseudo_user_validity_t *sel,
                                   const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches an :autofill pseudo-class.
 */
int cmp_pseudo_autofill_match(const cmp_pseudo_autofill_t *sel,
                              const cmp_dom_node_t *node, int *out_match);

/**
 * @brief DOM pseudo-class `:host`
 */
typedef struct cmp_pseudo_host {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_host_t;

/**
 * @brief DOM pseudo-class `:host(selector)`
 */
typedef struct cmp_pseudo_host_func {
  cmp_selector_t base;            /**< Base interface */
  const cmp_selector_t *selector; /**< Inner selector */
} cmp_pseudo_host_func_t;

/**
 * @brief DOM pseudo-class `:host-context(selector)`
 */
typedef struct cmp_pseudo_host_context {
  cmp_selector_t base;            /**< Base interface */
  const cmp_selector_t *selector; /**< Inner selector */
} cmp_pseudo_host_context_t;

/**
 * @brief DOM pseudo-class `:modal`
 */
typedef struct cmp_pseudo_modal {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_modal_t;

/**
 * @brief DOM pseudo-class `:fullscreen`
 */
typedef struct cmp_pseudo_fullscreen {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_fullscreen_t;

/**
 * @brief DOM pseudo-class `:picture-in-picture`
 */
typedef struct cmp_pseudo_pip {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_pip_t;

/**
 * @brief DOM pseudo-class `:popover-open`
 */
typedef struct cmp_pseudo_popover_open {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_popover_open_t;

/**
 * @brief DOM pseudo-class `:defined`
 */
typedef struct cmp_pseudo_defined {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_defined_t;

/**
 * @brief Matches a :host pseudo-class.
 */
int cmp_pseudo_host_match(const cmp_pseudo_host_t *sel,
                          const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :host(selector) pseudo-class.
 */
int cmp_pseudo_host_func_match(const cmp_pseudo_host_func_t *sel,
                               const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :host-context(selector) pseudo-class.
 */
int cmp_pseudo_host_context_match(const cmp_pseudo_host_context_t *sel,
                                  const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :modal pseudo-class.
 */
int cmp_pseudo_modal_match(const cmp_pseudo_modal_t *sel,
                           const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :fullscreen pseudo-class.
 */
int cmp_pseudo_fullscreen_match(const cmp_pseudo_fullscreen_t *sel,
                                const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :picture-in-picture pseudo-class.
 */
int cmp_pseudo_pip_match(const cmp_pseudo_pip_t *sel,
                         const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :popover-open pseudo-class.
 */
int cmp_pseudo_popover_open_match(const cmp_pseudo_popover_open_t *sel,
                                  const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :defined pseudo-class.
 */
int cmp_pseudo_defined_match(const cmp_pseudo_defined_t *sel,
                             const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Media pseudo-class `:playing` and `:paused`
 */
typedef struct cmp_pseudo_media_state {
  cmp_selector_t base; /**< Base interface */
  int is_paused;       /**< 1 for :paused, 0 for :playing */
} cmp_pseudo_media_state_t;

/**
 * @brief Media pseudo-class `:muted` and `:volume-locked`
 */
typedef struct cmp_pseudo_media_volume {
  cmp_selector_t base;  /**< Base interface */
  int is_volume_locked; /**< 1 for :volume-locked, 0 for :muted */
} cmp_pseudo_media_volume_t;

/**
 * @brief Matches a :playing or :paused pseudo-class.
 */
int cmp_pseudo_media_state_match(const cmp_pseudo_media_state_t *sel,
                                 const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a :muted or :volume-locked pseudo-class.
 */
int cmp_pseudo_media_volume_match(const cmp_pseudo_media_volume_t *sel,
                                  const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Pseudo-element ::before, ::after
 */
typedef struct cmp_pseudo_elem_before_after {
  cmp_selector_t base; /**< Base interface */
  int is_after;        /**< 1 for ::after, 0 for ::before */
} cmp_pseudo_elem_before_after_t;

/**
 * @brief Pseudo-element ::first-line, ::first-letter
 */
typedef struct cmp_pseudo_elem_first_line_letter {
  cmp_selector_t base; /**< Base interface */
  int is_first_letter; /**< 1 for ::first-letter, 0 for ::first-line */
} cmp_pseudo_elem_first_line_letter_t;

/**
 * @brief Pseudo-element ::selection
 */
typedef struct cmp_pseudo_elem_selection {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_elem_selection_t;

/**
 * @brief Pseudo-element ::target-text
 */
typedef struct cmp_pseudo_elem_target_text {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_elem_target_text_t;

/**
 * @brief Pseudo-element ::spelling-error, ::grammar-error
 */
typedef struct cmp_pseudo_elem_spelling_grammar {
  cmp_selector_t base;  /**< Base interface */
  int is_grammar_error; /**< 1 for ::grammar-error, 0 for ::spelling-error */
} cmp_pseudo_elem_spelling_grammar_t;

/**
 * @brief Pseudo-element ::marker
 */
typedef struct cmp_pseudo_elem_marker {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_elem_marker_t;

/**
 * @brief Pseudo-element ::placeholder
 */
typedef struct cmp_pseudo_elem_placeholder {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_elem_placeholder_t;

/**
 * @brief Pseudo-element ::backdrop
 */
typedef struct cmp_pseudo_elem_backdrop {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_elem_backdrop_t;

/**
 * @brief Pseudo-element ::file-selector-button
 */
typedef struct cmp_pseudo_elem_file_selector {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_elem_file_selector_t;

/**
 * @brief Pseudo-element ::cue
 */
typedef struct cmp_pseudo_elem_cue {
  cmp_selector_t base;            /**< Base interface */
  const cmp_selector_t *selector; /**< Optional inner selector */
} cmp_pseudo_elem_cue_t;

/**
 * @brief Pseudo-element ::slotted()
 */
typedef struct cmp_pseudo_elem_slotted {
  cmp_selector_t base;            /**< Base interface */
  const cmp_selector_t *selector; /**< Inner selector */
} cmp_pseudo_elem_slotted_t;

/**
 * @brief Pseudo-element ::part()
 */
typedef struct cmp_pseudo_elem_part {
  cmp_selector_t base; /**< Base interface */
  const char **parts;  /**< Array of part names */
  size_t part_count;   /**< Number of parts */
} cmp_pseudo_elem_part_t;

/**
 * @brief Pseudo-element ::view-transition
 */
typedef struct cmp_pseudo_elem_view_transition {
  cmp_selector_t base; /**< Base interface */
} cmp_pseudo_elem_view_transition_t;

/**
 * @brief Pseudo-element ::view-transition-group()
 */
typedef struct cmp_pseudo_elem_vt_group {
  cmp_selector_t base; /**< Base interface */
  const char *name;    /**< Name of the transition group */
} cmp_pseudo_elem_vt_group_t;

/**
 * @brief Pseudo-element ::view-transition-image-pair()
 */
typedef struct cmp_pseudo_elem_vt_image_pair {
  cmp_selector_t base; /**< Base interface */
  const char *name;    /**< Name of the transition group */
} cmp_pseudo_elem_vt_image_pair_t;

/**
 * @brief Pseudo-element ::view-transition-old(), ::view-transition-new()
 */
typedef struct cmp_pseudo_elem_vt_old_new {
  cmp_selector_t base; /**< Base interface */
  const char *name;    /**< Name of the transition group */
  int is_new; /**< 1 for ::view-transition-new, 0 for ::view-transition-old */
} cmp_pseudo_elem_vt_old_new_t;

/**
 * @brief Matches a ::before or ::after pseudo-element.
 */
int cmp_pseudo_elem_before_after_match(
    const cmp_pseudo_elem_before_after_t *sel, const cmp_dom_node_t *node,
    int *out_match);

/**
 * @brief Matches a ::first-line or ::first-letter pseudo-element.
 */
int cmp_pseudo_elem_first_line_letter_match(
    const cmp_pseudo_elem_first_line_letter_t *sel, const cmp_dom_node_t *node,
    int *out_match);

/**
 * @brief Matches a ::selection pseudo-element.
 */
int cmp_pseudo_elem_selection_match(const cmp_pseudo_elem_selection_t *sel,
                                    const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a ::target-text pseudo-element.
 */
int cmp_pseudo_elem_target_text_match(const cmp_pseudo_elem_target_text_t *sel,
                                      const cmp_dom_node_t *node,
                                      int *out_match);

/**
 * @brief Matches a ::spelling-error or ::grammar-error pseudo-element.
 */
int cmp_pseudo_elem_spelling_grammar_match(
    const cmp_pseudo_elem_spelling_grammar_t *sel, const cmp_dom_node_t *node,
    int *out_match);

/**
 * @brief Matches a ::marker pseudo-element.
 */
int cmp_pseudo_elem_marker_match(const cmp_pseudo_elem_marker_t *sel,
                                 const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a ::placeholder pseudo-element.
 */
int cmp_pseudo_elem_placeholder_match(const cmp_pseudo_elem_placeholder_t *sel,
                                      const cmp_dom_node_t *node,
                                      int *out_match);

/**
 * @brief Matches a ::backdrop pseudo-element.
 */
int cmp_pseudo_elem_backdrop_match(const cmp_pseudo_elem_backdrop_t *sel,
                                   const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a ::file-selector-button pseudo-element.
 */
int cmp_pseudo_elem_file_selector_match(
    const cmp_pseudo_elem_file_selector_t *sel, const cmp_dom_node_t *node,
    int *out_match);

/**
 * @brief Matches a ::cue pseudo-element.
 */
int cmp_pseudo_elem_cue_match(const cmp_pseudo_elem_cue_t *sel,
                              const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a ::slotted() pseudo-element.
 */
int cmp_pseudo_elem_slotted_match(const cmp_pseudo_elem_slotted_t *sel,
                                  const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a ::part() pseudo-element.
 */
int cmp_pseudo_elem_part_match(const cmp_pseudo_elem_part_t *sel,
                               const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a ::view-transition pseudo-element.
 */
int cmp_pseudo_elem_view_transition_match(
    const cmp_pseudo_elem_view_transition_t *sel, const cmp_dom_node_t *node,
    int *out_match);

/**
 * @brief Matches a ::view-transition-group() pseudo-element.
 */
int cmp_pseudo_elem_vt_group_match(const cmp_pseudo_elem_vt_group_t *sel,
                                   const cmp_dom_node_t *node, int *out_match);

/**
 * @brief Matches a ::view-transition-image-pair() pseudo-element.
 */
int cmp_pseudo_elem_vt_image_pair_match(
    const cmp_pseudo_elem_vt_image_pair_t *sel, const cmp_dom_node_t *node,
    int *out_match);

/**
 * @brief Matches a ::view-transition-old() or ::view-transition-new()
 * pseudo-element.
 */
int cmp_pseudo_elem_vt_old_new_match(const cmp_pseudo_elem_vt_old_new_t *sel,
                                     const cmp_dom_node_t *node,
                                     int *out_match);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_SELECTORS_H */
