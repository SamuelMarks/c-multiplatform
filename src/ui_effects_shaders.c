/* clang-format off */
#include "../include/ui_effects_shaders.h"
#include "../include/ui_shader_manager.h"
#include <stddef.h>
/* clang-format on */

static const char *blur_vertex_source = "attribute vec4 a_position;\n"
                                        "attribute vec2 a_texcoord;\n"
                                        "varying vec2 v_texcoord;\n"
                                        "void main() {\n"
                                        "    gl_Position = a_position;\n"
                                        "    v_texcoord = a_texcoord;\n"
                                        "}\n";

static const char *blur_h_fragment_source =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture;\n"
    "uniform float u_resolution;\n"
    "uniform float u_radius;\n"
    "void main() {\n"
    "    vec4 sum = vec4(0.0);\n"
    "    float tex_offset = 1.0 / u_resolution;\n"
    "    float step = u_radius * tex_offset;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x - 4.0 * step, "
    "v_texcoord.y)) * 0.0162162162;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x - 3.0 * step, "
    "v_texcoord.y)) * 0.0540540541;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x - 2.0 * step, "
    "v_texcoord.y)) * 0.1216216216;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x - 1.0 * step, "
    "v_texcoord.y)) * 0.1945945946;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x, v_texcoord.y)) * "
    "0.2270270270;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x + 1.0 * step, "
    "v_texcoord.y)) * 0.1945945946;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x + 2.0 * step, "
    "v_texcoord.y)) * 0.1216216216;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x + 3.0 * step, "
    "v_texcoord.y)) * 0.0540540541;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x + 4.0 * step, "
    "v_texcoord.y)) * 0.0162162162;\n"
    "    gl_FragColor = sum;\n"
    "}\n";

static const char *blur_v_fragment_source =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture;\n"
    "uniform float u_resolution;\n"
    "uniform float u_radius;\n"
    "void main() {\n"
    "    vec4 sum = vec4(0.0);\n"
    "    float tex_offset = 1.0 / u_resolution;\n"
    "    float step = u_radius * tex_offset;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x, v_texcoord.y - 4.0 * "
    "step)) * 0.0162162162;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x, v_texcoord.y - 3.0 * "
    "step)) * 0.0540540541;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x, v_texcoord.y - 2.0 * "
    "step)) * 0.1216216216;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x, v_texcoord.y - 1.0 * "
    "step)) * 0.1945945946;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x, v_texcoord.y)) * "
    "0.2270270270;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x, v_texcoord.y + 1.0 * "
    "step)) * 0.1945945946;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x, v_texcoord.y + 2.0 * "
    "step)) * 0.1216216216;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x, v_texcoord.y + 3.0 * "
    "step)) * 0.0540540541;\n"
    "    sum += texture2D(u_texture, vec2(v_texcoord.x, v_texcoord.y + 4.0 * "
    "step)) * 0.0162162162;\n"
    "    gl_FragColor = sum;\n"
    "}\n";

/** \brief ui_error */
enum ui_error
ui_effects_shaders_get_blur_h_program(struct ui_shader_manager *manager,
                                      unsigned int *out_program) {
  if (!manager || !out_program) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_shader_manager_get_program(manager, "blur_h", blur_vertex_source,
                                       blur_h_fragment_source, out_program);
}

/** \brief ui_error */
enum ui_error
ui_effects_shaders_get_blur_v_program(struct ui_shader_manager *manager,
                                      unsigned int *out_program) {
  if (!manager || !out_program) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_shader_manager_get_program(manager, "blur_v", blur_vertex_source,
                                       blur_v_fragment_source, out_program);
}
static const char *blend_vertex_source = "attribute vec4 a_position;\n"
                                         "attribute vec2 a_texcoord;\n"
                                         "varying vec2 v_texcoord;\n"
                                         "void main() {\n"
                                         "    gl_Position = a_position;\n"
                                         "    v_texcoord = a_texcoord;\n"
                                         "}\n";

static const char *blend_fragment_source_multiply =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = src.rgb * dst.rgb;\n"
    "    gl_FragColor = vec4(result, src.a + dst.a - src.a * dst.a);\n"
    "}\n";

static const char *blend_fragment_source_screen =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = src.rgb + dst.rgb - (src.rgb * dst.rgb);\n"
    "    gl_FragColor = vec4(result, src.a + dst.a - src.a * dst.a);\n"
    "}\n";

static const char *blend_fragment_source_overlay =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "float overlay(float s, float d) {\n"
    "    if (d < 0.5) { return 2.0 * s * d; }\n"
    "    return 1.0 - 2.0 * (1.0 - s) * (1.0 - d);\n"
    "}\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = vec3(overlay(src.r, dst.r), overlay(src.g, dst.g), "
    "overlay(src.b, dst.b));\n"
    "    gl_FragColor = vec4(result, src.a + dst.a - src.a * dst.a);\n"
    "}\n";

static const char *blend_fragment_source_darken =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = min(src.rgb, dst.rgb);\n"
    "    gl_FragColor = vec4(result, src.a + dst.a - src.a * dst.a);\n"
    "}\n";

static const char *blend_fragment_source_lighten =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = max(src.rgb, dst.rgb);\n"
    "    gl_FragColor = vec4(result, src.a + dst.a - src.a * dst.a);\n"
    "}\n";

static const char *blend_fragment_source_color_dodge =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "float color_dodge(float s, float d) {\n"
    "    if (d == 0.0) return 0.0;\n"
    "    if (s == 1.0) return 1.0;\n"
    "    return min(1.0, d / (1.0 - s));\n"
    "}\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = vec3(color_dodge(src.r, dst.r), color_dodge(src.g, "
    "dst.g), color_dodge(src.b, dst.b));\n"
    "    gl_FragColor = vec4(result, src.a + dst.a - src.a * dst.a);\n"
    "}\n";

static const char *blend_fragment_source_color_burn =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "float color_burn(float s, float d) {\n"
    "    if (d == 1.0) return 1.0;\n"
    "    if (s == 0.0) return 0.0;\n"
    "    return 1.0 - min(1.0, (1.0 - d) / s);\n"
    "}\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = vec3(color_burn(src.r, dst.r), color_burn(src.g, "
    "dst.g), color_burn(src.b, dst.b));\n"
    "    gl_FragColor = vec4(result, src.a + dst.a - src.a * dst.a);\n"
    "}\n";

static const char *blend_fragment_source_hard_light =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "float hard_light(float s, float d) {\n"
    "    if (s < 0.5) { return 2.0 * s * d; }\n"
    "    return 1.0 - 2.0 * (1.0 - s) * (1.0 - d);\n"
    "}\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = vec3(hard_light(src.r, dst.r), hard_light(src.g, "
    "dst.g), hard_light(src.b, dst.b));\n"
    "    gl_FragColor = vec4(result, src.a + dst.a - src.a * dst.a);\n"
    "}\n";

static const char *blend_fragment_source_soft_light =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "float soft_light(float s, float d) {\n"
    "    if (s <= 0.5) { return d - (1.0 - 2.0 * s) * d * (1.0 - d); }\n"
    "    float d_cap = (d <= 0.25) ? ((16.0 * d - 12.0) * d + 4.0) * d : "
    "sqrt(d);\n"
    "    return d + (2.0 * s - 1.0) * (d_cap - d);\n"
    "}\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = vec3(soft_light(src.r, dst.r), soft_light(src.g, "
    "dst.g), soft_light(src.b, dst.b));\n"
    "    gl_FragColor = vec4(result, src.a + dst.a - src.a * dst.a);\n"
    "}\n";

static const char *blend_fragment_source_difference =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = abs(dst.rgb - src.rgb);\n"
    "    gl_FragColor = vec4(result, src.a + dst.a - src.a * dst.a);\n"
    "}\n";

static const char *blend_fragment_source_exclusion =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = dst.rgb + src.rgb - 2.0 * dst.rgb * src.rgb;\n"
    "    gl_FragColor = vec4(result, src.a + dst.a - src.a * dst.a);\n"
    "}\n";

/* Advanced blend modes (hue, saturation, color, luminosity) require color space
 * conversions (RGB <-> HSL) */
/* We will map them to a simple fallback for now (normal blend) since full
 * RGB<->HSL in shader is complex. */
static const char *blend_fragment_source_normal =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture_src;\n"
    "uniform sampler2D u_texture_dst;\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture_src, v_texcoord);\n"
    "    vec4 dst = texture2D(u_texture_dst, v_texcoord);\n"
    "    vec3 result = src.rgb * src.a + dst.rgb * dst.a * (1.0 - src.a);\n"
    "    gl_FragColor = vec4(result, src.a + dst.a * (1.0 - src.a));\n"
    "}\n";

/** \brief ui_error */
enum ui_error
ui_effects_shaders_get_blend_program(struct ui_shader_manager *manager,
                                     enum ui_css_blend_mode mode,
                                     unsigned int *out_program) {
  const char *frag_source;
  const char *program_name;

  if (!manager || !out_program) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  switch (mode) {
  case UI_CSS_BLEND_MODE_MULTIPLY:
    frag_source = blend_fragment_source_multiply;
    program_name = "blend_multiply";
    break;
  case UI_CSS_BLEND_MODE_SCREEN:
    frag_source = blend_fragment_source_screen;
    program_name = "blend_screen";
    break;
  case UI_CSS_BLEND_MODE_OVERLAY:
    frag_source = blend_fragment_source_overlay;
    program_name = "blend_overlay";
    break;
  case UI_CSS_BLEND_MODE_DARKEN:
    frag_source = blend_fragment_source_darken;
    program_name = "blend_darken";
    break;
  case UI_CSS_BLEND_MODE_LIGHTEN:
    frag_source = blend_fragment_source_lighten;
    program_name = "blend_lighten";
    break;
  case UI_CSS_BLEND_MODE_COLOR_DODGE:
    frag_source = blend_fragment_source_color_dodge;
    program_name = "blend_color_dodge";
    break;
  case UI_CSS_BLEND_MODE_COLOR_BURN:
    frag_source = blend_fragment_source_color_burn;
    program_name = "blend_color_burn";
    break;
  case UI_CSS_BLEND_MODE_HARD_LIGHT:
    frag_source = blend_fragment_source_hard_light;
    program_name = "blend_hard_light";
    break;
  case UI_CSS_BLEND_MODE_SOFT_LIGHT:
    frag_source = blend_fragment_source_soft_light;
    program_name = "blend_soft_light";
    break;
  case UI_CSS_BLEND_MODE_DIFFERENCE:
    frag_source = blend_fragment_source_difference;
    program_name = "blend_difference";
    break;
  case UI_CSS_BLEND_MODE_EXCLUSION:
    frag_source = blend_fragment_source_exclusion;
    program_name = "blend_exclusion";
    break;
  case UI_CSS_BLEND_MODE_NORMAL:
  case UI_CSS_BLEND_MODE_HUE:
  case UI_CSS_BLEND_MODE_SATURATION:
  case UI_CSS_BLEND_MODE_COLOR:
  case UI_CSS_BLEND_MODE_LUMINOSITY:
  default:
    frag_source = blend_fragment_source_normal;
    program_name = "blend_normal";
    break;
  }

  return ui_shader_manager_get_program(
      manager, program_name, blend_vertex_source, frag_source, out_program);
}

static const char *filter_fragment_source_invert =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture;\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture, v_texcoord);\n"
    "    gl_FragColor = vec4(1.0 - src.rgb, src.a);\n"
    "}\n";

static const char *filter_fragment_source_grayscale =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture;\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture, v_texcoord);\n"
    "    float gray = dot(src.rgb, vec3(0.299, 0.587, 0.114));\n"
    "    gl_FragColor = vec4(vec3(gray), src.a);\n"
    "}\n";

static const char *filter_fragment_source_sepia =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture;\n"
    "void main() {\n"
    "    vec4 src = texture2D(u_texture, v_texcoord);\n"
    "    float r = dot(src.rgb, vec3(0.393, 0.769, 0.189));\n"
    "    float g = dot(src.rgb, vec3(0.349, 0.686, 0.168));\n"
    "    float b = dot(src.rgb, vec3(0.272, 0.534, 0.131));\n"
    "    gl_FragColor = vec4(r, g, b, src.a);\n"
    "}\n";

/** \brief ui_error */
enum ui_error
ui_effects_shaders_get_color_filter_program(struct ui_shader_manager *manager,
                                            enum ui_css_filter_type type,
                                            unsigned int *out_program) {
  const char *frag_source = NULL;
  const char *name = NULL;

  if (!manager || !out_program)
    return UI_ERROR_INVALID_ARGUMENT;

  if (type == UI_CSS_FILTER_INVERT) {
    frag_source = filter_fragment_source_invert;
    name = "filter_invert";
  } else if (type == UI_CSS_FILTER_GRAYSCALE) {
    frag_source = filter_fragment_source_grayscale;
    name = "filter_grayscale";
  } else if (type == UI_CSS_FILTER_SEPIA) {
    frag_source = filter_fragment_source_sepia;
    name = "filter_sepia";
  } else {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_shader_manager_get_program(manager, name, blur_vertex_source,
                                       frag_source, out_program);
}

static const char *drop_shadow_fragment_source =
    "precision mediump float;\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture;\n"
    "uniform vec4 u_color;\n"
    "void main() {\n"
    "    float alpha = texture2D(u_texture, v_texcoord).a;\n"
    "    gl_FragColor = u_color * alpha;\n"
    "}\n";

/** \brief ui_error */
enum ui_error
ui_effects_shaders_get_drop_shadow_program(struct ui_shader_manager *manager,
                                           unsigned int *out_program) {
  if (!manager || !out_program)
    return UI_ERROR_INVALID_ARGUMENT;
  return ui_shader_manager_get_program(
      manager, "drop_shadow", blur_vertex_source, drop_shadow_fragment_source,
      out_program);
}
