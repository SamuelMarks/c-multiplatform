Example ideas
=============

This plan outlines the feature gaps between the current state of **LibCMPC** (core APIs through Phase 6 are implemented; backend coverage varies) and the requirements to port high-fidelity sample apps. LibCMPC currently ships the **Material 3** (`m3`) library; **Apple Cupertino** and **Microsoft Fluent** are planned.

Sample app sources (roadmap)
----------------------------

- Android Compose Samples (Material 3)
- Apple SwiftUI / Cupertino sample apps (HIG samples)
- Microsoft Fluent UI sample gallery/apps

The goal is to copy/port these example apps to validate design-system parity and stress-test the core APIs.

---

The primary gaps for Material 3 sample parity are in **Multimedia** (image/audio), **Advanced Graphics** (gradients/shaders), and **Rich Text**.

### 1. Multimedia & Assets Subsytem
*Essential for almost all samples (JetNews, Jetsnack, Jetchat, Jetcaster).*

| Feature | Targeted Sample | Description | Affected Core File | Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Image Decoding** | JetNews, Jetsnack | LibCMPC includes fallback decoders for PPM (P6) and raw RGBA, but lacks PNG/JPEG/WebP and image metadata parsing. Add a real codec layer (stb_image or platform codecs). | `include/cmpc/cmp_image.h` | **Medium** |
| **Async Image Loader** | Jetcaster, Jetchat | A helper to fetch an image via `CMPNetwork`, decode it on a worker thread (`CMPTasks`), and upload to texture on the main thread. | `include/cmpc/cmp_assets.h` (New) | **Medium** |
| **Audio Playback** | Jetcaster | LibCMPC can decode WAV (PCM16) but has no audio output interface yet. Add playback/output (`CMPAudioOutput`) across backends. | `include/cmpc/cmp_api_env.h` | **High** |
| **SVG/Vector Parsing** | JetLagged | `cmp_icon` handles simple path icons, but complex vector scenes need a richer parser/renderer. | `include/cmpc/cmp_vector.h` (New) | **High** |

### 2. Graphics & Rendering Enhancements
*Required for the polished look of Jetsnack and JetLagged.*

| Feature | Targeted Sample | Description | Affected Core File | Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Gradient Shaders** | Jetsnack | The current `draw_rect` only accepts a flat `CMPColor`. Add a `CMPBrush`/`CMPPaint` API for linear/radial gradients. | `include/cmpc/cmp_api_gfx.h` | **High** |
| **Blend Modes** | Jetchat | Sophisticated UI often requires blend modes beyond simple alpha blending. | `include/cmpc/cmp_api_gfx.h` | **Medium** |
| **Blur Effects** | Jetsnack | The "frosted glass" effect or blurring background content. | `include/cmpc/cmp_api_gfx.h` | **High** |
| **Path Effects** | JetLagged | Dashed lines and corner rounding on arbitrary paths (for graph lines). | `include/cmpc/cmp_path.h` | **Medium** |

### 3. Advanced Layout & Scrolling
*Required for Jetsnack (coordinator layouts) and Reply (adaptive layouts).*

| Feature | Targeted Sample | Description | Affected Core File | Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Staggered Grid** | Jetsnack | `cmp_list` supports uniform grids. Jetsnack uses a masonry/waterfall layout. | `include/cmpc/cmp_list.h` | **Medium** |
| **Collapsing Toolbar**| Jetsnack | Logic to coordinate a `CMPScrollParent` that consumes scroll deltas to shrink a top bar before scrolling the list content. | `include/m3/m3_app_bar.h` | **Medium** |
| **Window Insets** | Jetchat, Reply | Handling IME (keyboard) appearance sliding up the content, and status bar padding. Scaffold has `safe_area`, but needs dynamic OS inset updates. | `include/cmpc/cmp_api_ws.h` | **Medium** |
| **Shared Element** | Jetsnack | Transitioning a widget from list geometry to detail geometry smoothly. | `include/cmpc/cmp_anim.h` | **Very High** |

### 4. Rich Text & Input
*Required for JetNews (article reading) and Jetchat.*

| Feature | Targeted Sample | Description | Affected Core File | Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Rich Text (Spans)** | JetNews | `CMPTextStyle` applies to the whole string. Mixed bold/italic/link spans need a richer text layout model. | `include/cmpc/cmp_text.h` | **High** |
| **Downloadable Fonts**| Jetchat | Helper to fetch fonts via network and register them with the text backend at runtime. | `include/cmpc/cmp_text.h` | **Medium** |
| **IME Integration** | Jetchat | Soft-keyboard control (show/hide, input type selection, emoji support). Current `CMPInputEvent` handles text, but not keyboard control. | `include/cmpc/cmp_api_ws.h` | **High** |

### 5. Algorithmic Features
*Specific logic requirements.*

| Feature | Targeted Sample | Description | Affected Core File | Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Palette Generation**| Jetcaster | Extract a color palette from cover art to theme the UI. Needs a quantization algorithm feeding `m3_color`. | `include/m3/m3_color.h` | **Medium** |
| **Diff/DiffUtil** | All (Lists) | When `CMPStore` updates a list of items, calculate diffs to animate insertions/removals in `CMPListView`. | `include/cmpc/cmp_list.h` | **Medium** |

### Execution Order

To reach parity with the sample apps, the recommended implementation order is:

1.  **GFX Upgrade (Priority):** Add gradients and real image decoding.
2.  **Rich Text:** Essential for JetNews and realistic app content.
3.  **Advanced Layouts:** Staggered grids and collapsing mechanics.
4.  **Audio + IME:** Audio output and keyboard control.
5.  **Design Systems + Samples:** Cupertino + Fluent widgets, then port the official sample apps for each design system.
