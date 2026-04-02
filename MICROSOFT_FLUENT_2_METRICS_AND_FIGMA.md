# Microsoft Fluent 2 Design Alignment & Specifications

## Figma Mapping
The C-Multiplatform Fluent 2 implementation strictly adheres to the official Microsoft Fluent 2 Figma libraries.

*   **Fluent 2 Web:** [Figma Community Link](https://www.figma.com/community/file/1128362694691459461/Microsoft-Fluent-2-Web)
*   **Fluent 2 iOS:** [Figma Community Link](https://www.figma.com/community/file/1128362791484831610/Microsoft-Fluent-2-iOS)
*   **Fluent 2 Windows:** [Figma Community Link](https://www.figma.com/community/file/1159947337440812328/Microsoft-Fluent-2-Windows)

All components (`cmp_f2_button`, `cmp_f2_checkbox`, etc.) refer to these master files for their layout dimensions, color mapping, and interactive states.

## Metric Transparency

To achieve 1-to-1 visual fidelity with the Figma designs, the following mathematical conversions are strictly enforced in the engine:

### Typography and Letter Spacing
Fluent 2 Figma uses precise letter tracking ratios. The C-Multiplatform `cmp_f2_theme_t` translates these directly to `letter_spacing` points (em equivalents) via the shaping engine (`cmp_shaper_t`).
*   **Display:** -0.5px mapping directly to `-0.00735em` for perfect font kerning at 68px.
*   **Caption:** +0.4px mapping directly to `+0.04em` for improved legibility at 10px.

### Corner Radii
*   Figma `Corner Radius: 4` -> `cmp_f2_theme->radius_medium = 4.0f`
*   Figma `Corner Radius: Pill` -> `cmp_f2_theme->radius_circular = 9999.0f`

### Shadows
The 6 elevation levels (2, 4, 8, 16, 28, 64) in Figma use a composite of an Ambient shadow and a Directional shadow. This translates into `cmp_f2_shadow_t`:
*   `shadow_2`: `y1: 1px, blur1: 2px, y2: 0px, blur2: 2px`
*   `shadow_64`: `y1: 32px, blur1: 64px, y2: 0px, blur2: 16px`
The alpha channels match the precise `0x1A000000` (10%) and `0x24000000` (14%) defined in the design specs.

### Color Calculations
The 100-step token generator (`cmp_f2_theme_generate`) linearly interpolates in standard RGB for base tones, ensuring exact matches to the standard palette. Dark mode semantic tokens invert lightness while maintaining perceptual contrast thresholds above WCAG 4.5:1.
