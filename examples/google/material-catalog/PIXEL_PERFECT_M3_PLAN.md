# Pixel-Perfect Material 3 Implementation Plan

The initial Material 3 Catalog implementation successfully laid down the state architecture, routing, theme generation (HCT colors), and general flexbox layout tree. However, the visual rendering was aggressively stubbed out using basic square `FillRect` calls, completely ignoring the microscopic CSS behaviors required for true Material Design.

This course-correction plan mandates implementing "deep" pixel-perfect features across the stack, rather than "wide" shallow component placeholders.

## Phase 1: Foundational Rendering Primitives
- [✓] **Data Structures**: Extend `cmp_ui_node_t` to support `border_radius`, `elevation` (shadow depth), `border_width`, and `border_color`.
- [✓] **Sub-pixel Anti-Aliased Shapes**: Upgrade the core GDI renderer (`cmp_win32_window.c`) to support smooth rounded rectangles using `RoundRect()` or `CreateRoundRectRgn()` (with proper brush painting), circles, and continuous squircles.
- [✓] **Alpha Blending & Compositing**: Ensure the renderer respects `node->opacity` and `node->bg_color` alpha channels for proper overdraw and overlaying state layers.
- [✓] **Elevation & Shadows**: Implement GDI+ or software-fallback 9-patch shadow rendering for Material elevations (1dp to 24dp) using alpha-blended drop shadows.
- [✓] **Typography & VFS Integration**: Hook up true `.ttf` font loading from the Virtual File System (Roboto) rather than relying on system fallback fonts.
- [✓] **SVG Rasterization**: Connect the SVG engine (`cmp_svg_renderer_t`) to the UI image views for scalable, tint-able iconography.

## Phase 2: Pixel-Perfect Components (Deep Implementation)
- [✓] **State Layers**: Implement exact opacity overlays (Hover 8%, Focus 10%, Pressed 10%) correctly composited over the base color using AlphaBlend.
- [✓] **Ripple Effect**: Implement the radial, expanding clipped circle animation for pointer down/up events inside the GDI renderer using `node->properties` for ripple state.
- [✓] **Buttons (Common)**: Pixel-perfect padding, 48x48dp touch targets, correct typography tracking.
- [✓] **Floating Action Buttons (FAB)**: Correct shadow elevation transitions (resting vs. pressed) and corner radius morphing.
- [✓] **Cards**: Elevated, Filled, and Outlined variants with exact border thicknesses and corner radii applied to the actual rendered rects.
- [✓] **Navigation Bar**: Active indicator pills with tweened width/position animations instead of flat backgrounds.
- [✓] **Switches & Checkboxes**: Exact vector path drawing animations for checkmarks, scale-up animations for radio circles.
- [✓] **Text Fields**: Floating labels, exact border cutout metrics, and character limit spacing.

## Phase 3: Integration & Tooling
- [✓] **Catalog Assets**: Actually bundle and load `Roboto-Regular.ttf` and essential SVGs into the VFS.
- [✓] **Visual Regression Goldens**: Lock in the pixel-perfect rendering with `screenshot_generator.exe` to prevent regressions once components reach true visual parity.