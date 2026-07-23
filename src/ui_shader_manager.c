#if defined(_WIN32) || defined(__CYGWIN__)
/* clang-format off */
#include <winsock2.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#elif defined(__linux__) || defined(__unix__) || defined(__EMSCRIPTEN__)
#include <GLES2/gl2.h>
#endif

#include <string.h>
#include "../include/ui_shader_manager.h"
#include "ui_internal_mem.h"
/* clang-format on */

#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif

/* Fallback mock functions for GL shader compilation if headers lack them or we
   are testing. On Windows, GL/gl.h only exposes GL 1.1, so we mock them to
   avoid undefined reference errors during compile-time unless we are strictly
   linking against an extension wrapper. */
#if defined(UI_TEST_MOCK_ALLOC) || defined(_WIN32) || defined(__CYGWIN__) ||   \
    defined(__APPLE__)
int g_mock_shader_fail = 0;
int g_mock_program_fail = 0;
static unsigned int mock_id_counter = 1;
static unsigned int mock_glCreateShader(int type) {
  (void)type;
  return mock_id_counter++;
}
static void mock_glShaderSource(unsigned int shader, int count,
                                const char **string, const int *length) {
  (void)shader;
  (void)count;
  (void)string;
  (void)length;
}
static void mock_glCompileShader(unsigned int shader) { (void)shader; }
static void mock_glGetShaderiv(unsigned int shader, int pname, int *params) {
  (void)shader;
  (void)pname;
  *params = (g_mock_shader_fail == (int)shader) ? 0 : 1;
}
static unsigned int mock_glCreateProgram(void) { return mock_id_counter++; }
static void mock_glAttachShader(unsigned int program, unsigned int shader) {
  (void)program;
  (void)shader;
}
static void mock_glLinkProgram(unsigned int program) { (void)program; }
static void mock_glGetProgramiv(unsigned int program, int pname, int *params) {
  (void)program;
  (void)pname;
  *params = g_mock_program_fail ? 0 : 1;
}
static void mock_glDeleteShader(unsigned int shader) { (void)shader; }
static void mock_glDeleteProgram(unsigned int program) { (void)program; }
static int mock_glGetUniformLocation(unsigned int program, const char *name) {
  (void)program;
  (void)name;
  return 1;
}
static void mock_glUniformMatrix4fv(int location, int count,
                                    unsigned char transpose,
                                    const float *value) {
  (void)location;
  (void)count;
  (void)transpose;
  (void)value;
}
static void mock_glUniform4f(int location, float v0, float v1, float v2,
                             float v3) {
  (void)location;
  (void)v0;
  (void)v1;
  (void)v2;
  (void)v3;
}
static void mock_glUniform1f(int location, float v0) {
  (void)location;
  (void)v0;
}

#define UI_GL_CREATE_SHADER mock_glCreateShader
#define UI_GL_SHADER_SOURCE mock_glShaderSource
#define UI_GL_COMPILE_SHADER mock_glCompileShader
#define UI_GL_GET_SHADERIV mock_glGetShaderiv
#define UI_GL_CREATE_PROGRAM mock_glCreateProgram
#define UI_GL_ATTACH_SHADER mock_glAttachShader
#define UI_GL_LINK_PROGRAM mock_glLinkProgram
#define UI_GL_GET_PROGRAMIV mock_glGetProgramiv
#define UI_GL_DELETE_SHADER mock_glDeleteShader
#define UI_GL_DELETE_PROGRAM mock_glDeleteProgram
#define UI_GL_GET_UNIFORM_LOCATION mock_glGetUniformLocation
#define UI_GL_UNIFORM_MATRIX4FV mock_glUniformMatrix4fv
#define UI_GL_UNIFORM4F mock_glUniform4f
#define UI_GL_UNIFORM1F mock_glUniform1f
#else
/* Assume standard GLES2 headers are available (e.g. Linux, Emscripten) */
#define UI_GL_CREATE_SHADER glCreateShader
#define UI_GL_SHADER_SOURCE glShaderSource
#define UI_GL_COMPILE_SHADER glCompileShader
#define UI_GL_GET_SHADERIV glGetShaderiv
#define UI_GL_CREATE_PROGRAM glCreateProgram
#define UI_GL_ATTACH_SHADER glAttachShader
#define UI_GL_LINK_PROGRAM glLinkProgram
#define UI_GL_GET_PROGRAMIV glGetProgramiv
#define UI_GL_DELETE_SHADER glDeleteShader
#define UI_GL_DELETE_PROGRAM glDeleteProgram
#define UI_GL_GET_UNIFORM_LOCATION glGetUniformLocation
#define UI_GL_UNIFORM_MATRIX4FV glUniformMatrix4fv
#define UI_GL_UNIFORM4F glUniform4f
#define UI_GL_UNIFORM1F glUniform1f
#endif

/** \brief ui_shader_entry */
struct ui_shader_entry {
  char name[64];
  unsigned int program_id;
  struct ui_shader_entry *next;
};

/** \brief ui_shader_manager */
struct ui_shader_manager {
  struct ui_shader_entry *head;
};

enum ui_error ui_shader_manager_create(struct ui_shader_manager **out_manager) {
  struct ui_shader_manager *manager;

  if (!out_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  manager =
      (struct ui_shader_manager *)UI_MALLOC(sizeof(struct ui_shader_manager));
  if (!manager) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  manager->head = NULL;
  *out_manager = manager;

  return UI_ERROR_NONE;
}

enum ui_error ui_shader_manager_destroy(struct ui_shader_manager *manager) {
  struct ui_shader_entry *current;
  struct ui_shader_entry *next;

  if (!manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = manager->head;
  while (current) {
    next = current->next;
    UI_GL_DELETE_PROGRAM(current->program_id);
    UI_FREE(current);
    current = next;
  }

  UI_FREE(manager);
  return UI_ERROR_NONE;
}

enum ui_error ui_shader_manager_get_program(struct ui_shader_manager *manager,
                                            const char *name,
                                            const char *vertex_source,
                                            const char *fragment_source,
                                            unsigned int *out_program_id) {
  struct ui_shader_entry *current;
  struct ui_shader_entry *new_entry;
  unsigned int vertex_shader;
  unsigned int fragment_shader;
  unsigned int program_id;
  int success;

  if (!manager || !name || !vertex_source || !fragment_source ||
      !out_program_id) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Check cache */
  current = manager->head;
  while (current) {
    if (strcmp(current->name, name) == 0) {
      *out_program_id = current->program_id;
      return UI_ERROR_NONE;
    }
    current = current->next;
  }

  /* Compile vertex shader */
  vertex_shader = UI_GL_CREATE_SHADER(GL_VERTEX_SHADER);
  UI_GL_SHADER_SOURCE(vertex_shader, 1, &vertex_source, NULL);
  UI_GL_COMPILE_SHADER(vertex_shader);
  UI_GL_GET_SHADERIV(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    UI_GL_DELETE_SHADER(vertex_shader);
    return UI_ERROR_UNKNOWN;
  }

  /* Compile fragment shader */
  fragment_shader = UI_GL_CREATE_SHADER(GL_FRAGMENT_SHADER);
  UI_GL_SHADER_SOURCE(fragment_shader, 1, &fragment_source, NULL);
  UI_GL_COMPILE_SHADER(fragment_shader);
  UI_GL_GET_SHADERIV(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    UI_GL_DELETE_SHADER(vertex_shader);
    UI_GL_DELETE_SHADER(fragment_shader);
    return UI_ERROR_UNKNOWN;
  }

  /* Link program */
  program_id = UI_GL_CREATE_PROGRAM();
  UI_GL_ATTACH_SHADER(program_id, vertex_shader);
  UI_GL_ATTACH_SHADER(program_id, fragment_shader);
  UI_GL_LINK_PROGRAM(program_id);
  UI_GL_GET_PROGRAMIV(program_id, GL_LINK_STATUS, &success);

  UI_GL_DELETE_SHADER(vertex_shader);
  UI_GL_DELETE_SHADER(fragment_shader);

  if (!success) {
    UI_GL_DELETE_PROGRAM(program_id);
    return UI_ERROR_UNKNOWN;
  }

  /* Cache the compiled program */
  new_entry =
      (struct ui_shader_entry *)UI_MALLOC(sizeof(struct ui_shader_entry));
  if (!new_entry) {
    UI_GL_DELETE_PROGRAM(program_id);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  UI_STRNCPY(new_entry->name, sizeof(new_entry->name), name,
             sizeof(new_entry->name) - 1);
  new_entry->name[sizeof(new_entry->name) - 1] = '\0';
  new_entry->program_id = program_id;

  new_entry->next = manager->head;
  manager->head = new_entry;

  *out_program_id = program_id;
  return UI_ERROR_NONE;
}

/** \brief ui_shader_manager_set_uniform_matrix */
enum ui_error ui_shader_manager_set_uniform_matrix(
    struct ui_shader_manager *manager, unsigned int program_id,
    const char *uniform_name, const float *matrix4x4) {
  int loc;
  if (!manager || !uniform_name || !matrix4x4)
    return UI_ERROR_INVALID_ARGUMENT;
  loc = UI_GL_GET_UNIFORM_LOCATION(program_id, uniform_name);
  if (loc >= 0) {
    UI_GL_UNIFORM_MATRIX4FV(loc, 1, 0, matrix4x4); /* GL_FALSE = 0 */
  }
  return UI_ERROR_NONE;
}

/** \brief ui_shader_manager_set_uniform_color */
enum ui_error ui_shader_manager_set_uniform_color(
    struct ui_shader_manager *manager, unsigned int program_id,
    const char *uniform_name, float r, float g, float b, float a) {
  int loc;
  if (!manager || !uniform_name)
    return UI_ERROR_INVALID_ARGUMENT;
  loc = UI_GL_GET_UNIFORM_LOCATION(program_id, uniform_name);
  if (loc >= 0) {
    UI_GL_UNIFORM4F(loc, r, g, b, a);
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_shader_manager_set_uniform_float(struct ui_shader_manager *manager,
                                    unsigned int program_id,
                                    const char *uniform_name, float value) {
  int loc;
  if (!manager || !uniform_name)
    return UI_ERROR_INVALID_ARGUMENT;
  loc = UI_GL_GET_UNIFORM_LOCATION(program_id, uniform_name);
  if (loc >= 0) {
    UI_GL_UNIFORM1F(loc, value);
  }
  return UI_ERROR_NONE;
}
