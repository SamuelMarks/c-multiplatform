/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "../include/ui_layout.h"
#include "../include/ui_dom_node.h"
#include "../include/ui_cssom.h"
#include "../src/ui_internal_mem.h"

extern int g_malloc_fail_countdown;
extern int g_malloc_called;

#include "both_tests.h"
#include "test_ui_layout_extra_coverage.c"
#include "test_ui_layout_main_tests.c"
#include "test_ui_layout_bounds_percentage.c"
#include "test_ui_layout_content_invalid.c"
