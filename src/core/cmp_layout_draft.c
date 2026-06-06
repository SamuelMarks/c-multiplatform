/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_layout_node_create(cmp_layout_node_t **out_node) { /* same */ }
int cmp_layout_node_destroy(cmp_layout_node_t *node) { /* same */ }
int cmp_layout_node_add_child(cmp_layout_node_t *parent,
                              cmp_layout_node_t *child) { /* same */ }

static void translate_descendants(cmp_layout_node_t *node,
                                  float dx) { /* same */ }
static void apply_rtl_mirroring(cmp_layout_node_t *node) { /* same */ }

/* ... */
