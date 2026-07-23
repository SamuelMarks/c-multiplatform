# Advanced CSS Engine & W3C Specs

This framework implements a robust, custom CSS engine capable of handling a wide array of W3C specifications. This guide details the supported modules and how to interact with the CSS Object Model (CSSOM).

## Core Topics

*   **[CSSOM & APIs](cssom.md):** Interacting with parsed stylesheets and views programmatically (`ui_cssom.h`, `ui_cssom_api.h`, `ui_cssom_view.h`).
*   **Scrolling & Snapping:** Utilizing CSS scroll anchoring, scroll snapping, and custom scrollbars (`ui_css_scroll_anchoring.h`, `ui_css_scroll_snap.h`, `ui_css_scrollbars.h`).
*   **Specialized CSS Modules:**
    *   **[Spatial Navigation](spatial-navigation.md):** Controlling focus movement using directional keys (`ui_css_spatial_nav.h`).
    *   **Speech & Aural CSS:** Styling for screen readers and speech synthesis (`ui_css_speech.h`).
    *   **Paged Media:** Handling print layouts and generated content (`ui_css_page.h`, `ui_css_gcpm.h`, `ui_css_content.h`).
    *   **Round Displays:** Specific styling for smartwatches and circular screens (`ui_css_round_display.h`).
