# DOM, Components & Event System

At the heart of the framework is a lightweight Document Object Model (DOM) and a reactive event system that powers everything else.

## Core Topics

*   **[The Component Lifecycle](component-lifecycle.md):** Understanding how components are created, mounted, updated, and destroyed (`ui_component.h`).
*   **DOM Nodes:** The low-level representation of UI elements in memory (`ui_dom_node.h`).
*   **Event Propagation:** How events capture, target, and bubble through the tree, and how to handle them efficiently (`ui_event.h`).
*   **Execution Context:** Managing thread-local state and the main rendering loop (`ui_execution_context.h`, `ui_engine.h`).
*   **[Modifiers](modifiers.md):** Applying reusable behaviors and styling adjustments to existing components without subclassing (`ui_modifier.h`).
