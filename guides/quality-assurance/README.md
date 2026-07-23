# Testing & Quality Assurance

Robust user interfaces require rigorous testing. This guide covers the strategies and tools available within the framework for validating your UI components and application flows.

## Core Topics

*   **Unit Testing Components:** Isolating UI components and testing their state transitions and event emissions.
*   **[Headless Integration Testing](headless-testing.md):** Using `ui_e2e_headless.h` to run fast, simulated rendering pipelines without a physical display server.
*   **[Headful E2E Testing](headful-testing.md):** Using `ui_e2e_headful.h` for complete, pixel-perfect validation on actual OS compositors.
*   **Visual Regression:** Strategies for snapshotting and comparing UI states.
*   **Performance Benchmarking:** Identifying rendering bottlenecks and layout thrashing.
