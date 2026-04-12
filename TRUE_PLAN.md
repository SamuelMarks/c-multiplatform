# TRUE_PLAN.md: The Path to Absolute Picture Perfection

This exhaustive blueprint outlines the rigorous architectural roadmap to graduate the LibCMPC framework from a functionally correct, GDI-based software fallback into a mathematically pure, hardware-accelerated rendering engine. This represents the ultimate implementation of **Phase 16-20** of the `CSS_STYLE_FEATURES.md` blueprint, achieving sub-pixel, 120fps "Picture Perfection" required for native Google Material 3, Apple Cupertino, and Microsoft Fluent 2 parity.

## Phase 1: Modern Graphics API Abstraction (HAL)
The legacy GDI/software fallback must be replaced with a universal, zero-overhead GPU pipeline.

- [✓] **1. Unified `cmp_gpu_t` Interface**: Implement a low-overhead, stateless abstraction layer bridging Vulkan, Metal, DirectX 12, and WebGL2/GLES3.
- [✓] **2. OS Surface Binding**: Native swapchain instantiation linked to `HWND` (Windows), `NSView/CAMetalLayer` (macOS), `UIView` (iOS), and `xdg_surface` (Wayland).
- [✓] **3. Mailbox Presentation Mode**: Implement double/triple buffered swapchains using FIFO/Mailbox modes to guarantee tear-free, VSync-locked 120fps synchronization.
- [✓] **4. Frame-in-Flight Pipelining**: Manage N-frames in flight (usually 2 or 3) with dedicated command buffers, uniform buffers, and fences to prevent CPU-GPU stalling.
- [✓] **5. Multi-threaded Command Recording**: Allow background worker threads to record drawing commands into secondary command buffers while the main thread resolves the layout tree.
- [✓] **6. Render Pass Graph Builder**: Implement a dependency-resolving graph to optimally sequence render passes, automatic transition of image layouts, and optimal memory barriers.
- [✓] **7. Pipeline State Object (PSO) Caching**: Hash and cache vertex/fragment shader combinations, blend states, and rasterization states to prevent stutter during runtime compilation.
- [✓] **8. Shader Pre-compilation (SPIR-V/MSL)**: Pre-compile and bundle all UI shaders using `glslangValidator` or `dxc` into the binary via the Virtual File System (VFS).

## Phase 2: GPU Memory & Resource Management
Efficient VRAM utilization is critical for high-res assets and dynamic atlases.

- [✓] **9. Slab/Block GPU Allocator**: Implement VMA (Vulkan Memory Allocator) / D3D12MA style memory sub-allocation to prevent VRAM fragmentation.
- [✓] **10. Transient Vertex/Index Buffers (`cmp_vbo_t`)**: A ring-buffer style allocator for streaming UI vertices (Rectangles, Glyphs) to the GPU linearly every frame.
- [✓] **11. Uniform Buffer Objects (`cmp_ubo_t`)**: Fast-path uniform updates to pass projection matrices, time (`dt`), and global UI state without vertex recalculation.
- [✓] **12. Dynamic Texture Atlasing (`cmp_atlas_t`)**: Implement a Skyline/MaxRects bin-packing algorithm to merge distinct icons and glyphs into a single 2048x2048 GPU texture, minimizing descriptor binding swaps.
- [✓] **13. LRU Atlas Eviction**: Algorithm to track texture usage per frame and seamlessly evict/re-pack the atlas when VRAM caps are exceeded.
- [✓] **14. Hardware Texture Compression**: Native decoding and uploading of ASTC, BC7, and ETC2 compressed assets directly to the GPU without CPU-side decompression.

## Phase 3: The Render Graph & Draw Batching
Minimizing draw calls and state changes is the key to mobile battery life and high framerates.

- [✓] **15. Z-Index Spatial Sorting**: Traverse the `cmp_layer_t` compositor tree and perform a stable sort based on Z-index, 3D depth, and painter's algorithm rules.
- [✓] **16. State-Change Minimization**: Sort draw commands by Texture ID, Shader ID, and Blend State to group identical rendering operations.
- [✓] **17. Dynamic Draw Call Batching**: Merge contiguous rectangular primitives and text glyphs into a massive, single `DrawIndexed` command.
- [✓] **18. Frustum & Occlusion Culling**: Fast AABB (Axis-Aligned Bounding Box) intersection tests against the screen viewport to discard completely off-screen nodes before generating vertices.
- [✓] **19. Early-Z Pre-pass**: For opaque UI surfaces, implement depth-buffer writing to allow the GPU to natively discard occluded background fragments before executing costly pixel shaders.
- [✓] **20. Hardware Scissoring & Clipping**: Map CSS `overflow: hidden` and `clip` properties directly to `glScissor` equivalent commands.
- [✓] **21. Stencil Buffer Masking**: For complex, non-rectangular clips (`border-radius` with hidden overflow), utilize the stencil buffer to mask out rasterization mathematically.

## Phase 4: Advanced Vector & Shape Tessellation
GDI `RoundRect` is flawed. We must move to mathematical SDFs and GPU tessellation.

- [✓] **22. Fragment Shader Rounded Rects**: Implement an SDF-based fragment shader calculating rounded corners using `length()` and `step()`, bypassing high-poly geometry for perfect anti-aliasing at any scale.
- [✓] **23. Continuous Curves (Apple Squircles)**: Implement mathematical evaluation for G2/G3 continuous corner radii (`corner-shape: squircle`) natively in the fragment shader.
- [✓] **24. GPU Vector Tessellator (`cmp_svg_renderer_t`)**: Replace `PolyPolygon` with a CPU-side ear-clipping or libtess2 style triangulator for complex SVGs.
- [✓] **25. Bezier Curve Subdivision**: Adaptive subdivision of quadratic and cubic bezier curves based on screen-space derivatives (zooming in generates more geometry dynamically).
- [✓] **26. Stroke Expansion Algorithm**: Tessellate `stroke-width`, handling complex `stroke-linejoin` (miter, bevel, round) and `stroke-linecap` geometry generation.
- [✓] **27. Even-Odd Fill Winding Rules**: Implement complex, self-intersecting polygon fills using GPU stencil-then-cover techniques (like Pathfinder or NanoVG).
- [✓] **28. Multi-Sample Anti-Aliasing (MSAA)**: Enable 4x or 8x MSAA natively on the swapchain render target to smooth the edges of SVG triangles.
- [✓] **29. Morphing Vector Paths**: Interpolate vertices between two SVG paths with identical command lengths for seamless icon state transitions (e.g., Hamburger to Close button).

## Phase 5: Pixel-Perfect Shadows & Compositing
Executing the defining microscopic details of modern design languages.

- [✓] **30. True Gaussian Shadows (`cmp_shadow_9patch_t`)**: Replace the stacked AlphaBlend trick with a true multi-pass Separable Gaussian Blur convolution shader.
- [✓] **31. Cached Shadow Atlases**: Render complex drop shadows once to an off-screen texture map, caching them in the atlas until the source element resizes or mutates.
- [✓] **32. Real-time Backdrop Filtering (Acrylic/Mica)**: Render the background UI into an off-screen framebuffer, apply a dual-pass Kawase blur shader, and sample it for the foreground element to achieve Apple's "Frosted Glass".
- [✓] **33. Edge-Mirrored Convolutions**: Prevent dark/transparent halos around screen edges during backdrop blurs by reflecting edge pixels outward before executing the blur shader.
- [✓] **34. CSS Mix-Blend Modes**: Map `multiply`, `screen`, `overlay`, and `color-dodge` to fixed-function GPU blend states (or advanced programmable blending where extensions allow).
- [✓] **35. Isolate Blending Contexts**: Enforce `isolation: isolate` bounds via off-screen render passes so internal blend modes do not bleed into the global application background.
- [✓] **36. Image Alpha Masking (`mask-image`)**: Apply secondary grayscale/alpha texture maps in the fragment shader to mathematically clip content (used heavily in Cupertino).
- [✓] **37. SVG Filter Primitives (`feColorMatrix` / `feDisplacementMap`)**: Support node-based image processing for complex fluid, gooey, or color-shifting UI effects.

## Phase 6: World-Class Typography (HarfBuzz + FreeType + SDF)
`TextOutA` is completely inadequate for modern i18n typography.

- [✓] **38. FreeType Glyph Rasterization**: Integrate FreeType to parse `.ttf`/`.otf` files directly from memory (`cmp_vfs_read_file_sync`), extracting precise bearing, advance, and kerning metrics.
- [✓] **39. HarfBuzz Text Shaping**: Translate UTF-8 strings into specific glyph indices and absolute layout offsets, correctly handling complex ligatures (e.g., "fi", "ffi").
- [✓] **40. Arabic/Indic Shaping Rules**: Guarantee flawless cursive joining (Arabic) and consonant conjunct reordering (Devanagari, Tamil) via HarfBuzz integration.
- [✓] **41. Bidirectional Text (Bidi) Run Splitting**: Process Unicode strings through the UBA (Unicode Bidirectional Algorithm) to split LTR/RTL blocks before passing them to the shaper.
- [✓] **42. Signed Distance Field (SDF) Font Rendering**: Upgrade FreeType rasterization to generate SDFs instead of bitmaps, allowing text to scale infinitely without pixelation.
- [✓] **43. Multi-Channel SDF (MSDF)**: Implement MSDF texture generation to preserve razor-sharp internal and external corners on complex typography at massive scales.
- [✓] **44. Sub-pixel LCD Text Rendering (ClearType)**: Exploit physical RGB monitor sub-pixels via dual-source blending or shader-based color fringe correction to triple horizontal typographic resolution.
- [✓] **45. Color Fonts & Emojis**: Support extraction and rendering of `CBDT`, `COLR`, and `sbix` font tables for native OS emoji rendering.
- [✓] **46. Variable Fonts Axis Interpolation**: Support OpenType variable axes (weight, width, slant, optical size), dynamically updating glyph outlines prior to rasterization.

## Phase 7: Image Processing & Color Science
Ensuring mathematically accurate color reproduction across different displays.

- [?] **47. Linear sRGB Pipeline**: Guarantee texture sampling and alpha blending occur in linear color space, applying a final gamma-correction pass (sRGB OETF) to prevent "muddy" gradients.
- [?] **48. Perceptual Color Spaces (OKLCH, LAB)**: Implement internal color math in OKLCH to allow for perceptually uniform color tweening and dynamic Material 3 tonal palette generation.
- [?] **49. ICC Profile Parsing**: Read embedded color profiles from JPEG/PNG assets and dynamically construct 3D LUTs (Look-Up Tables) or shader matrices to color-match the target monitor's gamut.
- [?] **50. Wide Color Gamut (Display P3)**: Output directly to Display P3 swapchains on compatible Apple and HDR displays, expanding the maximum visible saturation.
- [?] **51. Hardware Video Decoding (NV12 -> RGB)**: Support zero-copy mapping of hardware video decoding buffers (e.g., VAAPI, DXVA, VideoToolbox) directly to GPU textures, using a YUV/NV12-to-RGB conversion shader.
- [?] **52. High-Quality Image Downsampling (Mipmaps)**: Automatically generate anisotropic filtering mipmaps for loaded images to prevent moiré patterns when scaled down within the UI.

## Phase 8: Decoupled Compositor & Kinematics
Animations must bypass the CPU Layout phase entirely to prevent UI jank.

- [?] **53. Asynchronous Compositor Thread**: Decouple the renderer from the layout engine. The compositor thread operates at exactly 120hz, taking immutable snapshots of the UI Layer Tree.
- [?] **54. Hardware-Accelerated Scrolling (`cmp_scroll_ctx_t`)**: Scrolling a `ListView` updates a `scroll_offset` uniform matrix in the compositor. The CPU does not recalculate layout or vertices during a scroll.
- [?] **55. Asynchronous Pan/Zoom (APZ)**: Handle multi-touch pinch-to-zoom purely on the compositor thread via affine transformation matrices before the main thread receives the gesture.
- [?] **56. Spring Physics Overscroll (Rubber-banding)**: Execute damped harmonic oscillator formulas directly inside the compositor loop to calculate elastic bounce-back effects independently of main thread lag.
- [?] **57. CSS 3D Transforms (`perspective`, `rotateX`)**: Apply true 4x4 matrix mathematics in the vertex shader, projecting UI elements into a 3D frustum.
- [?] **58. Backface Visibility Culling**: Implement `backface-visibility: hidden` natively via GPU backface culling (`glCullFace`) during card-flip transitions.
- [?] **59. Compositor-Only Transitions (`cmp_transition_t`)**: Morph opacity, translation, and scale properties in real-time on the GPU via uniform interpolation.
- [?] **60. Sub-frame VSync Delta (`cmp_dt_t`)**: Query exact OS display link hardware timings to synchronize animation frame progressions to the literal nanosecond of the monitor refresh.

## Phase 9: Design System Specific Advanced Visuals
Executing the defining microscopic details of modern design languages.

- [?] **61. Fluent 2: Reveal Highlight**: Pass mouse coordinates to button fragment shaders to render proximity-based volumetric radial lighting over borders and backgrounds.
- [?] **62. Fluent 2: Acrylic Noise Maps**: Multiply a static, high-frequency monochrome noise texture over backdrop-filtered surfaces to break up banding and replicate the "Acrylic" material.
- [?] **63. Cupertino: True Vibrancy**: Combine `backdrop-filter` with intense saturation boosts and `color-dodge` blend modes to replicate exact iOS/macOS translucent visual hierarchies.
- [?] **64. Material 3: Advanced Ink Ripples**: Calculate radial expanding ripple origins on the GPU. Apply eased opacity curves and noise-based distortion to replicate physical ink spreading.
- [?] **65. View Morphing (Shared Element Transitions)**: Capture the rendered texture of an outgoing view, capture the incoming view, and execute a cross-faded affine warp in the compositor to seamlessly merge states (e.g., Image Thumbnail -> Fullscreen Hero).
- [?] **66. Lottie/Rive Native Playback**: Map complex vector animation formats directly to the new GPU Tessellation pipeline, allowing fully hardware-accelerated icon micro-interactions (e.g., a heart icon filling up).

