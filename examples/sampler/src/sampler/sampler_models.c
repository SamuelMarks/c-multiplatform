/**
 * @file sampler_models.c
 * @brief Implementation of catalog models and registry for Compose Material
 * Catalog.
 */

/* clang-format off */
#include "sampler/sampler_models.h"
#include "sampler/sampler_samples.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* clang-format on */

/* URLs defined in AOSP reference */
#define M3_GUIDELINES_URL "https://m3.material.io"
#define M3_DOCS_URL                                                            \
  "https://developer.android.com/reference/kotlin/androidx/compose/material3/" \
  "package-summary"
#define M3_SOURCE_URL                                                          \
  "https://cs.android.com/androidx/platform/frameworks/support/+/"             \
  "androidx-main:compose/material3/material3/"

/* Buttons examples */
static const struct sampler_example g_buttons_examples[] = {
    {1, "Filled Button",
     "Filled button with primary container color and onPrimary label.", 0,
     "Button.kt", sample_filled_button},
    {2, "Elevated Button",
     "Elevated button with surface container low and elevation shadow.", 0,
     "Button.kt", sample_elevated_button},
    {3, "Filled Tonal Button",
     "Filled tonal button with secondary container color.", 0, "Button.kt",
     sample_filled_tonal_button},
    {4, "Outlined Button",
     "Outlined button with outline border and transparent surface.", 0,
     "Button.kt", sample_outlined_button},
    {5, "Text Button",
     "Flat text button with primary colored text and state layer ripple.", 0,
     "Button.kt", sample_text_button},
    {6, "Button with Icon", "Button with leading icon and 8dp spacing.", 0,
     "Button.kt", sample_button_with_icon},
    {7, "Button with Animated Shape",
     "Expressive button morphing between rounded rectangle and pill on press.",
     1, "Button.kt", sample_button_animated_shape}};

/* Button Groups examples */
static const struct sampler_example g_button_groups_examples[] = {
    {1, "Button Group",
     "Connected button group with unified 1dp divider and zero double-line "
     "borders.",
     1, "ButtonGroup.kt", sample_stub_create},
    {2, "Button Group with Custom Item",
     "Group with mixed icons and text labels.", 1, "ButtonGroup.kt",
     sample_stub_create}};

/* Split Buttons examples */
static const struct sampler_example g_split_buttons_examples[] = {
    {1, "Filled Split Button",
     "Main action button + trailing chevron button with dropdown menu.", 1,
     "SplitButton.kt", sample_stub_create},
    {2, "Elevated Split Button",
     "Elevated split button with surface elevation.", 1, "SplitButton.kt",
     sample_stub_create},
    {3, "Extra Large Filled Split Button",
     "Prominent expressive split button with enlarged touch target.", 1,
     "SplitButton.kt", sample_stub_create}};

/* Card examples */
static const struct sampler_example g_card_examples[] = {
    {1, "Card", "Standard filled card with surface variant container.", 0,
     "Card.kt", sample_stub_create},
    {2, "Elevated Card",
     "Elevated card with surface container low and dynamic elevation shadow.",
     0, "Card.kt", sample_stub_create},
    {3, "Outlined Card", "Outlined card with 1dp border outline.", 0, "Card.kt",
     sample_stub_create},
    {4, "Clickable Card",
     "Interactive clickable cards with state layer ripple on press.", 0,
     "Card.kt", sample_stub_create}};

/* Checkboxes examples */
static const struct sampler_example g_checkboxes_examples[] = {
    {1, "Checkbox", "Standard checkbox with animated checkmark vector draw.", 0,
     "Checkbox.kt", sample_stub_create},
    {2, "Checkbox with Text",
     "Checkbox integrated into selectable row with label.", 0, "Checkbox.kt",
     sample_stub_create},
    {3, "Checkbox Rounded Strokes",
     "Expressive checkbox with rounded cap strokes.", 1, "Checkbox.kt",
     sample_stub_create}};

/* Dialogs examples */
static const struct sampler_example g_dialogs_examples[] = {
    {1, "Alert Dialog",
     "Standard alert dialog with title, body, confirm button, and dismiss "
     "button.",
     0, "AlertDialog.kt", sample_stub_create},
    {2, "Alert Dialog with Icon",
     "Prominent top centered icon with alert dialog layout.", 0,
     "AlertDialog.kt", sample_stub_create},
    {3, "Basic Alert Dialog",
     "Custom content surface with freeform layout slots.", 0, "AlertDialog.kt",
     sample_stub_create}};

/* Floating Action Buttons examples */
static const struct sampler_example g_fabs_examples[] = {
    {1, "Floating Action Button",
     "Standard 56x56dp FAB with icon and elevation.", 0,
     "FloatingActionButton.kt", sample_stub_create},
    {2, "Small FAB", "Compact 40x40dp FAB.", 0, "FloatingActionButton.kt",
     sample_stub_create},
    {3, "Large FAB", "Expressive 96x96dp Large FAB with 36dp icon.", 1,
     "FloatingActionButton.kt", sample_stub_create},
    {4, "Animated FAB", "FAB with shape-morphing geometry on state transition.",
     1, "FloatingActionButton.kt", sample_stub_create}};

/* Extended FAB examples */
static const struct sampler_example g_extended_fabs_examples[] = {
    {1, "Extended FAB", "Extended FAB with icon and text label.", 0,
     "FloatingActionButton.kt", sample_stub_create},
    {2, "Animated Extended FAB",
     "Extended FAB collapsing to icon-only FAB on list scroll.", 0,
     "FloatingActionButton.kt", sample_stub_create}};

/* Navigation Bar examples */
static const struct sampler_example g_navigation_bar_examples[] = {
    {1, "Navigation Bar",
     "Bottom navigation bar with 3 to 5 destinations and active pill "
     "indicator.",
     0, "NavigationBar.kt", sample_stub_create}};

/* Navigation Drawer examples */
static const struct sampler_example g_navigation_drawer_examples[] = {
    {1, "Modal Navigation Drawer",
     "Slide-out drawer with dark scrim overlay and drawer header.", 0,
     "NavigationDrawer.kt", sample_stub_create},
    {2, "Dismissible Navigation Drawer", "Side drawer pushing main content.", 0,
     "NavigationDrawer.kt", sample_stub_create},
    {3, "Permanent Navigation Drawer", "Persistent desktop side drawer.", 0,
     "NavigationDrawer.kt", sample_stub_create}};

/* Navigation Rail examples */
static const struct sampler_example g_navigation_rail_examples[] = {
    {1, "Navigation Rail",
     "Vertical navigation rail for tablet and desktop screens with top FAB.", 0,
     "NavigationRail.kt", sample_stub_create},
    {2, "Dismissible Modal Wide Navigation Rail",
     "Wide expandable rail with labels and secondary actions.", 1,
     "NavigationRail.kt", sample_stub_create}};

/* Generic single sample for other components */
static const struct sampler_example g_generic_adaptive_examples[] = {
    {1, "List Detail Pane Scaffold",
     "Two-pane list-detail layout adapting to window resizing.", 0,
     "ThreePaneScaffold.kt", sample_stub_create}};
static const struct sampler_example g_generic_badge_examples[] = {
    {1, "Badge",
     "Badges with icon only or dynamic text/number notification count.", 0,
     "Badge.kt", sample_stub_create}};
static const struct sampler_example g_generic_bottom_app_bar_examples[] = {
    {1, "Bottom App Bar",
     "Bottom app bar displaying navigation and key actions at the bottom of "
     "screens.",
     0, "AppBar.kt", sample_stub_create}};
static const struct sampler_example g_generic_bottom_sheet_examples[] = {
    {1, "Bottom Sheet",
     "Surfaces containing supplementary content anchored to the bottom of the "
     "screen.",
     0, "ModalBottomSheet.kt", sample_stub_create}};
static const struct sampler_example g_generic_carousel_examples[] = {
    {1, "Carousel",
     "Stylized lists providing unique viewing for large imagery.", 1,
     "Carousel.kt", sample_stub_create}};
static const struct sampler_example g_generic_chips_examples[] = {
    {1, "Chips",
     "Chips allowing users to enter information, filter, or trigger actions.",
     0, "Chip.kt", sample_stub_create}};
static const struct sampler_example g_generic_date_pickers_examples[] = {
    {1, "Date Picker",
     "Date pickers letting users select a date or range of dates.", 0,
     "DatePicker.kt", sample_stub_create}};
static const struct sampler_example g_generic_fab_menu_examples[] = {
    {1, "FAB Menu",
     "Speed dial FAB menu displaying additional key actions on click of a FAB.",
     1, "FloatingActionButtonMenu.kt", sample_stub_create}};
static const struct sampler_example g_generic_floating_toolbars_examples[] = {
    {1, "Floating Toolbar",
     "Floating toolbar displaying key actions above content.", 1,
     "FloatingToolbar.kt", sample_stub_create}};
static const struct sampler_example g_generic_icon_buttons_examples[] = {
    {1, "Icon Buttons",
     "Icon buttons allowing users to take actions with a single tap.", 0,
     "IconButton.kt", sample_stub_create}};
static const struct sampler_example g_generic_lists_examples[] = {
    {1, "Lists", "Lists are continuous, vertical indexes of text or images.", 0,
     "ListItem.kt", sample_stub_create}};
static const struct sampler_example g_generic_loading_indicators_examples[] = {
    {1, "Loading Indicators",
     "Loading indicators expressing wait time or process progress.", 1,
     "LoadingIndicator.kt", sample_stub_create}};
static const struct sampler_example g_generic_menus_examples[] = {
    {1, "Menus", "Menus displaying choices on temporary surfaces.", 0,
     "Menu.kt", sample_stub_create}};
static const struct sampler_example g_generic_nav_suite_examples[] = {
    {1, "Navigation Suite Scaffold",
     "Adaptive navigation suite choosing between bar, rail, and drawer.", 0,
     "NavigationSuiteScaffold.kt", sample_stub_create}};
static const struct sampler_example g_generic_progress_examples[] = {
    {1, "Progress Indicators", "Linear and circular progress indicators.", 0,
     "ProgressIndicator.kt", sample_stub_create}};
static const struct sampler_example g_generic_pull_refresh_examples[] = {
    {1, "Pull to Refresh",
     "Swipe gesture at beginning of lists to refresh content.", 0,
     "PullToRefresh.kt", sample_stub_create}};
static const struct sampler_example g_generic_radio_buttons_examples[] = {
    {1, "Radio Buttons",
     "Radio buttons allowing users to select one option from a set.", 0,
     "RadioButton.kt", sample_stub_create}};
static const struct sampler_example g_generic_scroll_field_examples[] = {
    {1, "Scroll Field",
     "Scroll field allowing user to select a value like time or numbers.", 1,
     "ScrollField.kt", sample_stub_create}};
static const struct sampler_example g_generic_search_bars_examples[] = {
    {1, "Search Bars",
     "Search bars allowing users to enter a keyword or phrase.", 0,
     "SearchBar.kt", sample_stub_create}};
static const struct sampler_example g_generic_segmented_buttons_examples[] = {
    {1, "Segmented Button",
     "Segmented buttons helping people select options or sort elements.", 0,
     "SegmentedButton.kt", sample_stub_create}};
static const struct sampler_example g_generic_sliders_examples[] = {
    {1, "Sliders",
     "Sliders allowing selections from a continuous or discrete range of "
     "values.",
     0, "Slider.kt", sample_stub_create}};
static const struct sampler_example g_generic_snackbars_examples[] = {
    {1, "Snackbars",
     "Brief messages about app processes at the bottom of the screen.", 0,
     "Snackbar.kt", sample_stub_create}};
static const struct sampler_example g_generic_switches_examples[] = {
    {1, "Switches", "Switches toggle the state of a single setting on or off.",
     0, "Switch.kt", sample_stub_create}};
static const struct sampler_example g_generic_tabs_examples[] = {
    {1, "Tabs", "Tabs organize content across different screens and data sets.",
     0, "Tab.kt", sample_stub_create}};
static const struct sampler_example g_generic_text_fields_examples[] = {
    {1, "Text Fields",
     "Filled and outlined text fields letting users enter and edit text.", 0,
     "TextField.kt", sample_stub_create}};
static const struct sampler_example g_generic_time_pickers_examples[] = {
    {1, "Time Picker", "Interactive circular clock dial and input time picker.",
     0, "TimePicker.kt", sample_stub_create}};
static const struct sampler_example g_generic_toggle_buttons_examples[] = {
    {1, "Toggle Buttons", "Selectable button that animates on press.", 1,
     "ToggleButton.kt", sample_stub_create}};
static const struct sampler_example g_generic_tooltips_examples[] = {
    {1, "Tooltips", "Tooltips calling user attention to an anchor component.",
     0, "Tooltip.kt", sample_stub_create}};
static const struct sampler_example g_generic_top_app_bar_examples[] = {
    {1, "Top App Bar",
     "Top app bars displaying information and actions at the top of a screen.",
     0, "AppBar.kt", sample_stub_create}};
static const struct sampler_example g_generic_typography_examples[] = {
    {1, "Typography",
     "Material Design type scale contrasting styles supporting product "
     "content.",
     0, "Typography.kt", sample_stub_create}};

/* The full 41 Components in exact alphabetical order matching Compose catalog
 */
static const struct sampler_component
    g_all_components[SAMPLER_TOTAL_COMPONENTS] = {
        {1, "Adaptive",
         "Adaptive scaffolds providing automatic layout adjustment on "
         "different window sizes.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_adaptive_examples, 1},
        {2, "Badge",
         "A badge can contain dynamic information, such as notification "
         "counts.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_badge_examples, 1},
        {3, "Bottom App Bar",
         "Displays navigation and key actions at the bottom of mobile screens.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_bottom_app_bar_examples, 1},
        {4, "Bottom Sheet",
         "Surfaces containing supplementary content, anchored to the bottom of "
         "the screen.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_bottom_sheet_examples, 1},
        {5, "Buttons",
         "Buttons help people initiate actions, from sending an email, to "
         "sharing a document.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1,
         g_buttons_examples,
         sizeof(g_buttons_examples) / sizeof(g_buttons_examples[0])},
        {6, "Button Groups",
         "Button groups is a container for material components that adds "
         "animation on press.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1,
         g_button_groups_examples,
         sizeof(g_button_groups_examples) /
             sizeof(g_button_groups_examples[0])},
        {7, "Card",
         "Cards contain content and actions that relate information about a "
         "subject.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_card_examples, sizeof(g_card_examples) / sizeof(g_card_examples[0])},
        {8, "Carousel",
         "Stylized versions of lists that provide unique viewing for large "
         "imagery.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1,
         g_generic_carousel_examples, 1},
        {9, "Checkboxes",
         "Checkboxes allow the user to select one or more items from a set.", 1,
         1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1,
         g_checkboxes_examples,
         sizeof(g_checkboxes_examples) / sizeof(g_checkboxes_examples[0])},
        {10, "Chips",
         "Chips allow users to enter information, make selections, filter, or "
         "trigger actions.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_chips_examples, 1},
        {11, "Date Pickers",
         "Date pickers let users select a date or range of dates.", 1, 1,
         M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_date_pickers_examples, 1},
        {12, "Dialogs",
         "Dialogs provide important prompts requiring an action or "
         "communicating information.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_dialogs_examples,
         sizeof(g_dialogs_examples) / sizeof(g_dialogs_examples[0])},
        {13, "Extended FAB",
         "Extended FABs help people take primary actions and include a text "
         "label.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_extended_fabs_examples,
         sizeof(g_extended_fabs_examples) /
             sizeof(g_extended_fabs_examples[0])},
        {14, "Floating Action Buttons",
         "The FAB represents the most important action on a screen.", 1, 1,
         M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1, g_fabs_examples,
         sizeof(g_fabs_examples) / sizeof(g_fabs_examples[0])},
        {15, "FAB Menu",
         "The FAB Menu displays additional key actions on click of a FAB.", 1,
         1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1,
         g_generic_fab_menu_examples, 1},
        {16, "Floating Toolbars",
         "A floating toolbar displays key actions above the content.", 1, 1,
         M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1,
         g_generic_floating_toolbars_examples, 1},
        {17, "Icon Buttons",
         "Icon buttons allow users to take actions and make choices with a "
         "single tap.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_icon_buttons_examples, 1},
        {18, "Lists",
         "Lists are continuous, vertical indexes of text or images.", 1, 1,
         M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_lists_examples, 1},
        {19, "Loading Indicators",
         "Loading indicators express unspecified wait time or display progress "
         "length.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1,
         g_generic_loading_indicators_examples, 1},
        {20, "Menus", "Menus display a list of choices on temporary surfaces.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_menus_examples, 1},
        {21, "Navigation Bar",
         "Navigation bars offer a convenient way to switch between primary "
         "destinations.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_navigation_bar_examples,
         sizeof(g_navigation_bar_examples) /
             sizeof(g_navigation_bar_examples[0])},
        {22, "Navigation Drawer",
         "Navigation drawers provide ergonomic access to destinations in an "
         "app.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_navigation_drawer_examples,
         sizeof(g_navigation_drawer_examples) /
             sizeof(g_navigation_drawer_examples[0])},
        {23, "Navigation Rail",
         "Navigation rails provide access to destinations on tablet and "
         "desktop screens.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1,
         g_navigation_rail_examples,
         sizeof(g_navigation_rail_examples) /
             sizeof(g_navigation_rail_examples[0])},
        {24, "Navigation Suite Scaffold",
         "Wraps content and places adequate navigation component according to "
         "window size.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_nav_suite_examples, 1},
        {25, "Progress Indicators",
         "Progress indicators express an unspecified wait time or display "
         "process length.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_progress_examples, 1},
        {26, "Pull-to-Refresh Indicator",
         "Swipe gesture available at the beginning of lists where recent "
         "content appears.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_pull_refresh_examples, 1},
        {27, "Radio Buttons",
         "Radio buttons allow the user to select one option from a set.", 1, 1,
         M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_radio_buttons_examples, 1},
        {28, "Scroll Field",
         "Scroll field allows the user to select a value like time or numbers.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1,
         g_generic_scroll_field_examples, 1},
        {29, "Search Bars",
         "Search bars allow users to enter a keyword or phrase and get "
         "relevant information.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_search_bars_examples, 1},
        {30, "Segmented Buttons",
         "Segmented buttons help people select options, switch views, or sort "
         "elements.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_segmented_buttons_examples, 1},
        {31, "Sliders",
         "Sliders allow users to make selections from a range of values.", 1, 1,
         M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_sliders_examples, 1},
        {32, "Snackbars",
         "Snackbars provide brief messages about app processes at the bottom "
         "of the screen.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_snackbars_examples, 1},
        {33, "Split Buttons",
         "Split buttons let user perform additional actions besides the main "
         "action.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1,
         g_split_buttons_examples,
         sizeof(g_split_buttons_examples) /
             sizeof(g_split_buttons_examples[0])},
        {34, "Switches",
         "Switches toggle the state of a single setting on or off.", 1, 1,
         M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_switches_examples, 1},
        {35, "Tabs",
         "Tabs organize content across different screens, data sets, and other "
         "interactions.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_tabs_examples, 1},
        {36, "Text Fields", "Text fields let users enter and edit text.", 1, 1,
         M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_text_fields_examples, 1},
        {37, "Time Pickers",
         "Time picker allows the user to choose time of day.", 1, 1,
         M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_time_pickers_examples, 1},
        {38, "Toggle Buttons",
         "Toggle buttons provide a selectable button that animates on press.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 1,
         g_generic_toggle_buttons_examples, 1},
        {39, "Tooltips", "Tooltips call user attention to an anchor component.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_tooltips_examples, 1},
        {40, "Top App Bar",
         "Top app bars display information and actions at the top of a screen.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_top_app_bar_examples, 1},
        {41, "Typography",
         "The Material Design type scale includes a range of contrasting "
         "styles.",
         1, 1, M3_GUIDELINES_URL, M3_DOCS_URL, M3_SOURCE_URL, 0,
         g_generic_typography_examples, 1}};

static const struct sampler_component
    *g_component_ptrs[SAMPLER_TOTAL_COMPONENTS];
static int g_ptrs_initialized = 0;

static void sampler_init_ptrs_if_needed(void) {
  size_t i;
  if (!g_ptrs_initialized) {
    for (i = 0; i < SAMPLER_TOTAL_COMPONENTS; ++i) {
      g_component_ptrs[i] = &g_all_components[i];
    }
    g_ptrs_initialized = 1;
  }
}

static int sampler_case_insensitive_contains(const char *haystack,
                                             const char *needle) {
  size_t needle_len;
  size_t haystack_len;
  size_t i;
  size_t j;

  if (needle == NULL || needle[0] == '\0') {
    return 1;
  }
  if (haystack == NULL) {
    return 0;
  }

  needle_len = strlen(needle);
  haystack_len = strlen(haystack);
  if (needle_len > haystack_len) {
    return 0;
  }

  for (i = 0; i <= haystack_len - needle_len; ++i) {
    int match = 1;
    for (j = 0; j < needle_len; ++j) {
      char h = (char)tolower((unsigned char)haystack[i + j]);
      char n = (char)tolower((unsigned char)needle[j]);
      if (h != n) {
        match = 0;
        break;
      }
    }
    if (match) {
      return 1;
    }
  }

  return 0;
}

sampler_error_t sampler_catalog_get_components(
    const struct sampler_component *const **out_components, size_t *out_count) {
  if (out_components == NULL || out_count == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  sampler_init_ptrs_if_needed();
  *out_components = g_component_ptrs;
  *out_count = SAMPLER_TOTAL_COMPONENTS;
  return SAMPLER_SUCCESS;
}

sampler_error_t sampler_catalog_find_component_by_id(
    int id, const struct sampler_component **out_component) {
  size_t i;

  if (out_component == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  for (i = 0; i < SAMPLER_TOTAL_COMPONENTS; ++i) {
    if (g_all_components[i].id == id) {
      *out_component = &g_all_components[i];
      return SAMPLER_SUCCESS;
    }
  }

  return SAMPLER_ERROR_ROUTE_NOT_FOUND;
}

sampler_error_t sampler_catalog_find_component_by_name(
    const char *name, const struct sampler_component **out_component) {
  size_t i;

  if (name == NULL || out_component == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  for (i = 0; i < SAMPLER_TOTAL_COMPONENTS; ++i) {
    size_t len_a = strlen(g_all_components[i].name);
    size_t len_b = strlen(name);
    if (len_a == len_b) {
      size_t j;
      int match = 1;
      for (j = 0; j < len_a; ++j) {
        if (tolower((unsigned char)g_all_components[i].name[j]) !=
            tolower((unsigned char)name[j])) {
          match = 0;
          break;
        }
      }
      if (match) {
        *out_component = &g_all_components[i];
        return SAMPLER_SUCCESS;
      }
    }
  }

  return SAMPLER_ERROR_ROUTE_NOT_FOUND;
}

sampler_error_t
sampler_catalog_filter(const char *search_query, int show_only_expressive,
                       const struct sampler_component **out_components,
                       size_t max_results, size_t *out_count) {
  size_t i;
  size_t count = 0;

  if (out_components == NULL || out_count == NULL) {
    return SAMPLER_ERROR_NULL_POINTER;
  }

  for (i = 0; i < SAMPLER_TOTAL_COMPONENTS; ++i) {
    const struct sampler_component *comp = &g_all_components[i];
    int matches_expressive;
    int matches_search;

    matches_expressive = !show_only_expressive || comp->has_expressive_examples;
    matches_search =
        (search_query == NULL || search_query[0] == '\0') ||
        sampler_case_insensitive_contains(comp->name, search_query) ||
        sampler_case_insensitive_contains(comp->description, search_query);

    if (matches_expressive && matches_search) {
      if (count < max_results) {
        out_components[count] = comp;
        count++;
      }
    }
  }

  *out_count = count;
  return SAMPLER_SUCCESS;
}
