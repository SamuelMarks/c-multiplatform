Example ideas
=============


Actually why not be super boring, and copycat
- https://github.com/android/compose-samples

It's all Apache-2.0 licensed afterall…

---

This plan outlines the feature gaps between the current state of **LibM3C** (Phase 6 complete) and the requirements to implement the **Android Compose Samples**.

The primary gaps are in **Multimedia** (Image decoding/Audio), **Advanced Graphics** (Gradients/Shaders), and **Rich Text**.

### 1. Multimedia & Assets Subsytem
*Essential for almost all samples (JetNews, Jetsnack, Jetchat, Jetcaster).*

| Feature | Targeted Sample | Description | Affected Core File | Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Image Decoding** | JetNews, Jetsnack | LibM3C can render textures (`m3_api_gfx`), but cannot decode JPG/PNG/WebP from disk/network to raw bytes. Needs a `m3_image` module (wrapping `stb_image` or platform codecs). | `include/m3/m3_image.h` (New) | **Medium** |
| **Async Image Loader** | Jetcaster, Jetchat | A helper to fetch an image via `M3Network`, decode it on a worker thread (`M3Tasks`), and upload to texture on the main thread. | `include/m3/m3_assets.h` (New) | **Medium** |
| **Audio Playback** | Jetcaster | Jetcaster is a podcast app. LibM3C needs an audio output interface (`M3Audio`) implemented across backends. | `include/m3/m3_api_env.h` | **High** |
| **SVG/Vector Parsing** | JetLagged | While `m3_icon` handles simple SVG paths, JetLagged requires complex dynamic graphs. A dedicated Vector Drawable loader/renderer is needed beyond simple icons. | `include/m3/m3_vector.h` (New) | **High** |

### 2. Graphics & Rendering Enhancements
*Required for the polished look of Jetsnack and JetLagged.*

| Feature | Targeted Sample | Description | Affected Core File | Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Gradient Shaders** | Jetsnack | The current `draw_rect` only accepts a flat `M3Color`. Jetsnack relies heavily on gradients for the "snack" cards. The GFX ABI needs a `M3Brush` or `M3Paint` struct supporting Linear/Radial gradients. | `include/m3/m3_api_gfx.h` | **High** |
| **Blend Modes** | Jetchat | Sophisticated UI often requires specific blend modes (e.g., overlay, screen) beyond simple alpha blending for images and shapes. | `include/m3/m3_api_gfx.h` | **Medium** |
| **Blur Effects** | Jetsnack | The "frosted glass" effect or blurring background content. | `include/m3/m3_api_gfx.h` | **High** |
| **Path Effects** | JetLagged | Dashed lines and corner rounding on arbitrary paths (for graph lines). | `include/m3/m3_path.h` | **Medium** |

### 3. Advanced Layout & Scrolling
*Required for Jetsnack (coordinator layouts) and Reply (adaptive layouts).*

| Feature | Targeted Sample | Description | Affected Core File | Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Staggered Grid** | Jetsnack | `m3_list.h` supports uniform Grids. Jetsnack uses a masonry/waterfall layout. | `include/m3/m3_list.h` | **Medium** |
| **Collapsing Toolbar**| Jetsnack | Logic to coordinate a `M3ScrollParent` that consumes scroll deltas to shrink a top bar before scrolling the list content. Logic exists in `m3_scroll`, but a widget implementation is needed. | `include/m3/m3_app_bar.h` | **Medium** |
| **Window Insets** | Jetchat, Reply | Handling IME (Keyboard) appearance sliding up the content, and status bar padding. LibM3C has `safe_area` in Scaffold, but needs dynamic listening to OS Inset changes. | `include/m3/m3_api_ws.h` | **Medium** |
| **Shared Element** | Jetsnack | One of the hardest features: transitioning a widget from a List geometry to a Detail page geometry smoothly. | `include/m3/m3_anim.h` | **Very High** |

### 4. Rich Text & Input
*Required for JetNews (article reading) and Jetchat.*

| Feature | Targeted Sample | Description | Affected Core File | Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Rich Text (Spans)** | JetNews | Currently `M3TextStyle` applies to the whole string. JetNews articles require bold, italic, and links mixed within a single paragraph. | `include/m3/m3_text.h` | **High** |
| **Downloadable Fonts**| Jetchat | Helper to fetch fonts via network and register them with the text backend at runtime. | `include/m3/m3_text.h` | **Medium** |
| **IME Integration** | Jetchat | Advanced soft-keyboard control (show/hide, input type selection, emoji support). Current `M3InputEvent` handles text, but not control flow of the keyboard itself. | `include/m3/m3_api_ws.h` | **High** |

### 5. Algorithmic Features
*Specific logic requirements.*

| Feature | Targeted Sample | Description | Affected Core File | Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Palette Generation**| Jetcaster | Jetcaster extracts the color palette from the podcast cover art to theme the UI. Requires an implementation of the quantization algorithm (e.g., Wu or K-Means) to feed into `m3_color`. | `include/m3/m3_color.h` | **Medium** |
| **Diff/DiffUtil** | All (Lists) | When `M3Store` updates a list of items, an algorithm (Myers Diff) is needed to animate insertions/removals in the `M3ListView` rather than reloading the whole list. | `include/m3/m3_list.h` | **Medium** |

### Execution Order

To reach parity with the Compose samples, the recommended implementation order is:

1.  **GFX Upgrade (Priority):** Add Gradients and Image Decoding. Without this, the UI will look flat and empty compared to the samples.
2.  **Rich Text:** Essential for JetNews and any realistic app content.
3.  **Advanced Layouts:** Staggered Grids and Collapsing mechanics.
4.  **Audio/Platform inputs:** Audio playback and Keyboard control.
