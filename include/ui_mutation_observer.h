/**
 * @file ui_mutation_observer.h
 * @brief Observer API for tracking DOM tree mutations.
 */

#ifndef UI_MUTATION_OBSERVER_H
#define UI_MUTATION_OBSERVER_H

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Types of DOM mutations.
 */
enum ui_mutation_type {
  UI_MUTATION_TYPE_ATTRIBUTES = 1,    /**< Mutation of attributes */
  UI_MUTATION_TYPE_CHILD_LIST = 2,    /**< Mutation of child node list */
  UI_MUTATION_TYPE_CHARACTER_DATA = 3 /**< Mutation of character data */
};

/**
 * @brief Represents a single mutation record.
 */
struct ui_mutation_record {
  enum ui_mutation_type type;         /**< The type of mutation */
  struct ui_dom_node *target;         /**< The target node that mutated */
  struct ui_dom_node **added_nodes;   /**< Array of added nodes */
  int added_nodes_count;              /**< Number of added nodes */
  struct ui_dom_node **removed_nodes; /**< Array of removed nodes */
  int removed_nodes_count;            /**< Number of removed nodes */
  char *attribute_name; /**< Name of the changed attribute (if applicable) */
  char *old_value;      /**< Old value of attribute or character data */
};

/**
 * @brief Options for a mutation observer.
 */
struct ui_mutation_observer_init {
  int child_list;     /**< Set to 1 to observe child node additions/removals */
  int attributes;     /**< Set to 1 to observe attribute mutations */
  int character_data; /**< Set to 1 to observe text content changes */
  int subtree; /**< Set to 1 to observe the entire subtree rooted at the target
                */
  int attribute_old_value; /**< Set to 1 to record the prior value of an
                              attribute */
};

/**
 * @brief Represents a mutation observer instance.
 */
struct ui_mutation_observer;

/**
 * @brief Callback function type for mutation events.
 * @param observer The observer instance.
 * @param records Array of mutation records.
 * @param record_count Number of records in the array.
 * @param user_data Opaque pointer passed during creation.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_mutation_observer_cb_t)(
    struct ui_mutation_observer *observer,
    const struct ui_mutation_record *records, int record_count,
    void *user_data);

/**
 * @brief Creates a new mutation observer.
 * @param callback The callback to invoke when mutations occur.
 * @param user_data Opaque pointer passed to the callback.
 * @param out_observer Pointer to receive the new observer instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_mutation_observer_create(ui_mutation_observer_cb_t callback, void *user_data,
                            struct ui_mutation_observer **out_observer);

/**
 * @brief Destroys a mutation observer.
 * @param observer The observer to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_mutation_observer_destroy(struct ui_mutation_observer *observer);

/**
 * @brief Starts observing a target element.
 * @param observer The observer instance.
 * @param target The DOM node to observe.
 * @param options The initialization options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_mutation_observer_observe(struct ui_mutation_observer *observer,
                             struct ui_dom_node *target,
                             const struct ui_mutation_observer_init *options);

/**
 * @brief Disconnects the observer, stopping observation of all targets and
 * clearing the queue.
 * @param observer The observer instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_mutation_observer_disconnect(struct ui_mutation_observer *observer);

/**
 * @brief Internal notification hook when a child is added or removed.
 * @param target The parent node.
 * @param added The node added (or NULL).
 * @param removed The node removed (or NULL).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_mutation_observer_notify_child_list(struct ui_dom_node *target,
                                                  struct ui_dom_node *added,
                                                  struct ui_dom_node *removed);

/**
 * @brief Internal notification hook when an attribute is modified.
 * @param target The target node.
 * @param name The name of the attribute.
 * @param old_value The previous value (or NULL).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_mutation_observer_notify_attribute(struct ui_dom_node *target,
                                                 const char *name,
                                                 const char *old_value);

/**
 * @brief Internal notification hook when text content is modified.
 * @param target The target node.
 * @param old_value The previous text content (or NULL).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_mutation_observer_notify_character_data(struct ui_dom_node *target,
                                           const char *old_value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_MUTATION_OBSERVER_H */
