# Overlays & Portals

Managing UI elements that break out of the normal document flow—like modals, tooltips, and context menus—requires a robust overlay system.

## Core Topics

*   **[Overlay Director](overlay-director.md):** Orchestrating the stacking order (z-index) and lifecycle of floating elements (`ui_overlay_director.h`).
*   **Portals:** Rendering children into a different part of the DOM/View tree than their parent (`ui_portal.h`).
*   **Dialogs & Popovers:** Best practices for implementing accessible modal dialogs and lightweight popovers (`ui_dialog_base.h`, `ui_popover_base.h`).
*   **Context Menus:** Creating right-click menus that are context-aware and bounds-checked (`ui_context_menu_base.h`).
