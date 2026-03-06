# Material 3 (M3) Compatibility & Implementation Status

**Purpose of this document:**
This file serves as the definitive map of the Material 3 implementation within this codebase. Because this library groups related components to optimize structural overhead and C89 compilation, some widgets are not split into standalone files (e.g., Sliders are in `m3_progress.c`, Snackbars are in `m3_dialogs.c`, Segmented Buttons are in `m3_tabs.c`). 

**Future LLMs / Contributors:** Please consult this map *before* concluding a widget is missing or creating redundant files.

## Global Systems & Foundations

| Feature | Status | Location | Notes / Remaining Work |
| :--- | :---: | :--- | :--- |
| **Color System (Material You)** | 🟢 | `m3_color.h` | HCT color science, tonal palettes, and standard/expressive schemes are complete. Includes Content-Based Dynamic Color (`m3_color_extract_seed_from_image`), OS-Native Dynamic Color (`m3_scheme_generate_system`), and M3 Surface Container Roles (e.g., `surface_container_highest`). Also supports Dynamic Contrast Tones (Standard, Medium, High). |
| **Typography & Fonts** | 🟢 | `m3_typography.h` | Base and Expressive type scales are implemented. |
| **Shape System** | 🟢 | `m3_shape.h` | Standard M3 shapes and expressive fluid morphing radii (`m3_shape_morph`) are complete. |
| **Motion & Easing** | 🟢 | `m3_motion.h`<br>`cmp_anim.h` | Core easing curves (Emphasized, Standard, etc.) and shared axis transitions exist. Expressive motion curves fully wired to UI events via default component styles. Prefers Reduced Motion globally implemented via `cmp_anim_set_reduced_motion`. Advanced motion such as Container Transform (`m3_motion_container_transform`) and Predictive Back Motion (`m3_motion_predictive_back`) are complete. |
| **Adaptive Layouts** | 🟢 | `m3_adaptive.h` | Window size classes exist. Deep hinge/posture widget integration for foldables (bottom sheets) implemented. |
| **Accessibility (a11y)** | 🟢 | `m3_*` / `cmp_a11y.h` | Internal `CMPSemantics` tree nodes are fully populated. Full OS structural hooks bridged for Web (ARIA), Windows (MSAA), and macOS (NSAccessibility VoiceOver). |
| **Internationalisation (i18n)** | 🟢 | `m3_*` / `cmp_i18n.h` | Basic RTL structural mirroring exists (Tabs, Scaffold). BiDi shaping (complex text layout) is fully integrated into the rendering pipeline (`cmp_text_draw_utf8_gfx`) across all text-bearing M3 components. |

## Material 3 Widget Catalog

| Component | Status | File Location | Notes / Remaining Work |
| :--- | :---: | :--- | :--- |
| **App Bars (Top / Bottom)** | 🟢 | `m3_app_bar.h` | Fully implemented, including scroll behaviors. |
| **Badges** | 🟢 | `m3_badge.h` | Both small dots and large label variants implemented. |
| **Buttons (Standard, FAB, Icon)** | 🟢 | `m3_button.h` | Includes Elevated, Filled, Tonal, Outlined, Text, FAB, and Icon buttons. |
| **Cards** | 🟢 | `m3_card.h` | Elevated, Filled, and Outlined variants included. |
| **Carousel** | 🟢 | `m3_carousel.h` | Fully implemented. Includes advanced Standard, Hero, and Multi-browse variants. |
| **Chips** | 🟢 | `m3_chip.h` | Assist, Filter, Input, and Suggestion chips. |
| **Canonical Layouts** | 🟢 | `m3_adaptive.h` | List-Detail, Feed, and Supporting Pane layouts for large screens. |
| **Data Tables** | 🟢 | `m3_data_table.h` | Includes headers, row parsing, text alignment, sorting directions. |
| **Date Pickers** | 🟢 | `m3_date_picker.h` | Modal, docked, and range variants supported. |
| **Dialogs (Basic, Full-Screen)** | 🟢 | `m3_dialogs.h` | Standard dialogs and full-screen layouts. |
| **Dividers** | 🟢 | `m3_divider.h` | Fully implemented. |
| **Lists** | 🟢 | `m3_list.h` | 1-line, 2-line, 3-line list items. |
| **Menus** | 🟢 | `m3_menu.h` | Dropdown and cascading menus. |
| **Navigation (Bar, Drawer, Rail)** | 🟢 | `m3_navigation.h` | All 3 core navigation structures are fully implemented. |
| **Progress Indicators** | 🟢 | `m3_progress.h` | Linear and Circular variants. |
| **Pull to Refresh** | 🟢 | `m3_pull_to_refresh.h` | Fully implemented. |
| **Scaffold** | 🟢 | `m3_scaffold.h` | Page structure including FAB and Snackbar wiring. |
| **Search** | 🟢 | `m3_search.h` | Search Bar and Search View components. |
| **Segmented Buttons** | 🟢 | `m3_tabs.h` | Single-select and Multi-select variants. *(Note: Located in tabs header)* |
| **Selection Controls** | 🟢 | `m3_selection.h` | Checkboxes, Radio Buttons, and Switches. |
| **Sheets (Bottom, Side)** | 🟢 | `m3_sheet.h` | Standard, modal, and detached sheets. |
| **Sliders** | 🟢 | `m3_progress.h` | Continuous, Discrete, Range sliders, and Expressive Media Sliders. *(Note: Located in progress header)* |
| **Snackbars** | 🟢 | `m3_dialogs.h` | Transient message bars with actions. *(Note: Located in dialogs header)* |
| **Steppers** | 🟢 | `m3_stepper.h` | Multi-step flow indicators. |
| **Tabs** | 🟢 | `m3_tabs.h` | Primary and Secondary tabs. |
| **Text Fields** | 🟢 | `m3_text_field.h` | Filled and Outlined variants with state management. Includes native touch selection handles and text highlighting. |
| **Time Pickers** | 🟢 | `m3_time_picker.h` | Fully implemented. |
| **Tooltips** | 🟢 | `m3_tooltip.h` | Plain and Rich tooltips. |

## Path to "Complete" Material 3
100% COMPLETE. The framework provides full technical parity across standard, adaptive, and expressive schemas, including comprehensive deep hooks for screen readers and right-to-left complex text shaping. The addition of Dynamic Contrast Tones, Data Tables, and Steppers ensures strict conformance to the newest Material 3 standards. The final Expressive updates (Advanced Carousels, Expressive Media Sliders, Canonical Layouts, and Predictive Back Motion) bring the library to absolute state-of-the-art parity.
