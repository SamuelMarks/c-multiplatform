# Custom Shaders & Effects

For highly customized visual components or intensive graphical operations, standard CSS rendering might not be enough. The `ui_effects_shaders.h` header allows developers to inject custom GLSL/HLSL (depending on the backend) shaders directly into the UI rendering pipeline.

## How UI Shaders Work

Shaders in this framework are treated as **Effects**. They are applied to a component after its standard layout and painting phases are complete. The compositor takes the component's rendered texture, passes it through your custom fragment shader, and draws the output.

## Writing a Custom Shader

```glsl
// A simple custom fragment shader (GLSL example)
// The framework provides standard uniforms:
uniform float u_time;
uniform vec2 u_resolution;
uniform sampler2D u_content_texture; // The painted UI component

varying vec2 v_uv; // Provided by the framework's vertex shader

void main() {
    // Read the original UI pixels
    vec4 base_color = texture2D(u_content_texture, v_uv);

    // Apply a simple sine-wave color pulse over time
    float pulse = (sin(u_time * 2.0) + 1.0) * 0.5;
    vec3 tinted = mix(base_color.rgb, vec3(1.0, 0.5, 0.0), pulse * 0.3);

    gl_FragColor = vec4(tinted, base_color.a);
}
```

## Applying Shaders via C API

You compile and attach the shader to a specific component.

```c
#include "ui_effects_shaders.h"
#include "ui_component.h"

// 1. Compile the shader program
ui_shader_t* my_pulse_shader = ui_shader_create_from_string(
    engine,
    custom_fragment_shader_source
);

// 2. Create an effect instance
ui_effect_t* effect = ui_effect_create_shader(my_pulse_shader);

// 3. Attach it to a component (e.g., a special promotional banner)
ui_component_add_effect(promo_banner, effect);
```

## Animating Uniforms

To make the shader dynamic, you update its uniforms during the frame rendering loop.

```c
void on_render_frame(ui_engine_t* engine) {
    // Update the time uniform for all instances of this shader
    ui_shader_set_uniform_float(my_pulse_shader, "u_time", engine->time_seconds);
}
```

**Performance Note:** Using complex fragment shaders on very large UI components (like the document root) can consume significant GPU bandwidth. Use them judiciously on smaller, targeted components.
