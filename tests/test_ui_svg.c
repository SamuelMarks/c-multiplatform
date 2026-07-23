/* clang-format off */
#include "ui_svg.h"
#include "ui_error.h"
#include "../src/ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

static int run_test(const char *name, int (*test_fn)(void)) {
  int result = test_fn();
  if (result == 0) {
    printf("Running %s... PASS\n", name);
  } else {
    printf("Running %s... FAIL\n", name);
  }
  return result;
}

static int test_parse_basic_commands(void) {
  struct ui_svg_path path;
  int failed = 0;

  ACCUM_ERR(failed, ui_svg_path_init(&path));

  ACCUM_ERR(failed, ui_svg_path_parse(&path, "M 10 20 L 30 40 H 50 V 60 Z"));

  failed |= (path.count != 5);

  failed |= (path.commands[0].type != UI_SVG_CMD_MOVE_TO ||
             path.commands[0].data.move_to.p.x != 10.0f ||
             path.commands[0].data.move_to.p.y != 20.0f);

  failed |= (path.commands[1].type != UI_SVG_CMD_LINE_TO ||
             path.commands[1].data.line_to.p.x != 30.0f ||
             path.commands[1].data.line_to.p.y != 40.0f);

  failed |= (path.commands[4].type != UI_SVG_CMD_CLOSE_PATH);

  ui_svg_path_destroy(&path);
  return failed;
}

static int test_parse_implicit_commands(void) {
  struct ui_svg_path path;
  int failed = 0;

  ui_svg_path_init(&path);

  /* M followed by multiple coordinate pairs creates implicit L commands */
  ACCUM_ERR(failed, ui_svg_path_parse(&path, "M 10 20 30 40 50 60"));

  failed |= (path.count != 3);

  failed |= (path.commands[0].type != UI_SVG_CMD_MOVE_TO);
  failed |= (path.commands[1].type != UI_SVG_CMD_LINE_TO ||
             path.commands[1].data.line_to.p.x != 30.0f);
  failed |= (path.commands[2].type != UI_SVG_CMD_LINE_TO ||
             path.commands[2].data.line_to.p.x != 50.0f);

  ui_svg_path_destroy(&path);

  /* m followed by multiple pairs creates implicit l commands */
  ui_svg_path_init(&path);
  ACCUM_ERR(failed, ui_svg_path_parse(&path, "m 10 20 10 10 10 10"));

  failed |= (path.count != 3);
  failed |= (path.commands[0].type != UI_SVG_CMD_MOVE_TO ||
             path.commands[0].data.move_to.p.x != 10.0f);
  failed |= (path.commands[1].type != UI_SVG_CMD_LINE_TO ||
             path.commands[1].data.line_to.p.x != 20.0f);
  failed |= (path.commands[2].type != UI_SVG_CMD_LINE_TO ||
             path.commands[2].data.line_to.p.x != 30.0f);

  ui_svg_path_destroy(&path);
  return failed;
}

static int test_parse_relative_commands(void) {
  struct ui_svg_path path;
  int failed = 0;

  ui_svg_path_init(&path);

  ACCUM_ERR(failed, ui_svg_path_parse(&path, "M 10 20 l 10 10 h 10 v 10"));

  failed |= (path.count != 4);

  failed |= (path.commands[1].data.line_to.p.x != 20.0f ||
             path.commands[1].data.line_to.p.y != 30.0f);

  failed |= (path.commands[2].data.line_to.p.x != 30.0f ||
             path.commands[2].data.line_to.p.y != 30.0f);

  failed |= (path.commands[3].data.line_to.p.x != 30.0f ||
             path.commands[3].data.line_to.p.y != 40.0f);

  ui_svg_path_destroy(&path);
  return failed;
}

static int test_parse_curves(void) {
  struct ui_svg_path path;
  int failed = 0;

  ui_svg_path_init(&path);

  ACCUM_ERR(failed,
            ui_svg_path_parse(&path, "M 0 0 C 10 10 20 10 30 0 S 50 10 60 0 "
                                     "Q 70 10 80 0 T 100 0 A 10 10 0 0 1 110 0 "
                                     "M 0 0 S 10 10 20 20 T 30 30"));

  failed |= (path.count != 9);

  failed |= (path.commands[1].type != UI_SVG_CMD_CUBIC_BEZIER ||
             path.commands[2].type != UI_SVG_CMD_CUBIC_BEZIER ||
             path.commands[3].type != UI_SVG_CMD_QUADRATIC_BEZIER ||
             path.commands[4].type != UI_SVG_CMD_QUADRATIC_BEZIER ||
             path.commands[5].type != UI_SVG_CMD_ARC);

  ui_svg_path_destroy(&path);
  return failed;
}

static int test_parse_implicit_signs(void) {
  struct ui_svg_path path;
  int failed = 0;

  ui_svg_path_init(&path);

  ACCUM_ERR(failed, ui_svg_path_parse(&path, "M100-200L49.5-199.5"));

  failed |= (path.count != 2);
  failed |= (path.commands[0].data.move_to.p.x != 100.0f ||
             path.commands[0].data.move_to.p.y != -200.0f);
  failed |= (path.commands[1].data.line_to.p.x != 49.5f ||
             path.commands[1].data.line_to.p.y != -199.5f);

  ui_svg_path_destroy(&path);
  return failed;
}

static int test_parse_trailing_whitespace(void) {
  struct ui_svg_path path;
  int failed = 0;
  ui_svg_path_init(&path);
  ACCUM_ERR(failed, ui_svg_path_parse(&path, "M 0 0   "));
  ui_svg_path_destroy(&path);
  return failed;
}

static int test_fine_flattening(void) {
  struct ui_svg_path path;
  struct ui_svg_flattened_path flat;
  int failed = 0;

  ui_svg_path_init(&path);
  ui_svg_flattened_path_init(&flat);

  ACCUM_ERR(failed, ui_svg_path_parse(
                        &path, "M 0 0 C 0 100 100 100 100 0 Q 50 100 0 0"));

  /* Very fine tolerance to force recursive subdivisions */
  ACCUM_ERR(failed, ui_svg_path_flatten(&flat, &path, 0.001f));

  ui_svg_flattened_path_destroy(&flat);
  ui_svg_path_destroy(&path);
  return failed;
}

static int test_flattening(void) {
  struct ui_svg_path path;
  struct ui_svg_flattened_path flat;
  int failed = 0;

  ui_svg_path_init(&path);
  ui_svg_flattened_path_init(&flat);

  ACCUM_ERR(failed,
            ui_svg_path_parse(
                &path,
                "M 0 0 C 100 0 100 100 0 100 Z M 10 10 Q 20 20 30 30 T 40 40"));

  ACCUM_ERR(failed, ui_svg_path_flatten(&flat, &path, 1.0f));

  failed |= (flat.count != 2);
  if (!failed) {
    failed |= (flat.subpaths[0].count < 4);
    failed |= (flat.subpaths[0].closed != 1);
  }

  ui_svg_flattened_path_destroy(&flat);
  ui_svg_path_destroy(&path);
  return failed;
}

static int test_tessellate(void) {
  struct ui_svg_path path;
  struct ui_svg_flattened_path flat;
  struct ui_svg_geometry geom;
  int failed = 0;

  ui_svg_path_init(&path);
  ui_svg_flattened_path_init(&flat);
  ui_svg_geometry_init(&geom);

  ACCUM_ERR(failed, ui_svg_path_parse(&path, "M 0 0 L 10 0 L 10 10 L 0 10 Z"));

  ACCUM_ERR(failed, ui_svg_path_flatten(&flat, &path, 1.0f));
  ACCUM_ERR(failed, ui_svg_tessellate_fill(&geom, &flat));

  ui_svg_geometry_destroy(&geom);
  ui_svg_flattened_path_destroy(&flat);
  ui_svg_path_destroy(&path);

  return failed;
}

static int run_parse_failures(void) {
  struct ui_svg_path path;
  int failed = 0;
  ui_svg_path_init(&path);

  failed |= (ui_svg_path_parse(&path, "X 10") != UI_ERROR_PARSE_FAILED);
  failed |= (ui_svg_path_parse(&path, "M 10") != UI_ERROR_PARSE_FAILED);
  failed |= (ui_svg_path_parse(&path, "10 10") != UI_ERROR_PARSE_FAILED);
  failed |= (ui_svg_path_parse(&path, "M 10 A x") != UI_ERROR_PARSE_FAILED);
  failed |=
      (ui_svg_path_parse(&path, "M 0 0 W 10 10") != UI_ERROR_PARSE_FAILED);

  ui_svg_path_destroy(&path);
  return failed;
}

static int run_stroke_test(void) {
  struct ui_svg_geometry geom;
  struct ui_svg_flattened_path f;
  struct ui_svg_subpath sp;
  struct ui_svg_point pts[2] = {{0, 0}, {10, 10}};

  ui_svg_geometry_init(&geom);
  ui_svg_flattened_path_init(&f);
  sp.points = pts;
  sp.count = 2;
  sp.capacity = 2;
  sp.closed = 0;
  f.subpaths = &sp;
  f.count = 1;
  f.capacity = 1;

  ui_svg_tessellate_stroke(&geom, &f, 2.0f);

  /* Trigger degenerate stroke */
  struct ui_svg_point dpts[2] = {{0, 0}, {0, 0}};
  sp.points = dpts;
  ui_svg_tessellate_stroke(&geom, &f, 2.0f);

  f.subpaths = NULL;
  ui_svg_geometry_destroy(&geom);
  return 0;
}

static int run_cw_tests(void) {
  struct ui_svg_geometry geom;
  struct ui_svg_flattened_path f;
  struct ui_svg_subpath sp;
  /* Complex concave CW polygon 2 to force ear false positive branch */
  struct ui_svg_point cw_pts2[8] = {{0, 0},  {0, -10}, {5, -5}, {10, -10},
                                    {10, 0}, {5, 5},   {2, 5},  {2, 0}};

  ui_svg_geometry_init(&geom);
  ui_svg_flattened_path_init(&f);
  sp.points = cw_pts2;
  sp.count = 8;
  sp.capacity = 8;
  sp.closed = 1;
  f.subpaths = &sp;
  f.count = 1;
  f.capacity = 1;

  ui_svg_tessellate_fill(&geom, &f);

  f.subpaths = NULL;
  ui_svg_geometry_destroy(&geom);

  /* Another star with inside points to hit is_ear = 0 */
  struct ui_svg_point star2[6] = {{0, 20},  {20, 20}, {10, 0},
                                  {10, 10}, {5, 5},   {15, 5}};
  sp.points = star2;
  sp.count = 6;
  sp.capacity = 6;
  ui_svg_geometry_init(&geom);
  f.subpaths = &sp;
  ui_svg_tessellate_fill(&geom, &f);
  f.subpaths = NULL;
  ui_svg_geometry_destroy(&geom);

  /* Run a larger star to test is_ear logic */
  struct ui_svg_point star_pts[10] = {{50, 0},  {61, 35}, {98, 35}, {68, 57},
                                      {79, 91}, {50, 70}, {21, 91}, {32, 57},
                                      {2, 35},  {39, 35}};
  sp.points = star_pts;
  sp.count = 10;
  sp.capacity = 10;

  ui_svg_geometry_init(&geom);
  f.subpaths = &sp;
  ui_svg_tessellate_fill(&geom, &f);

  f.subpaths = NULL;
  ui_svg_geometry_destroy(&geom);

  return 0;
}

static int run_tessellate_oom_branches(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_svg_geometry geom;
  struct ui_svg_flattened_path f;
  struct ui_svg_subpath sp;
  struct ui_svg_point pts[4] = {{0, 0}, {0, 10}, {10, 10}, {10, 0}};
  int i;

  ui_svg_geometry_init(&geom);
  ui_svg_flattened_path_init(&f);
  sp.points = pts;
  sp.count = 4;
  sp.capacity = 4;
  sp.closed = 1;
  f.subpaths = &sp;
  f.count = 1;
  f.capacity = 1;

  for (i = 0; i < 400; i++) {
    ui_svg_geometry_destroy(&geom);
    ui_svg_geometry_init(&geom);

    int oom_idx = i % 10;
    int force_realloc_at = (i / 10) % 15;

    g_malloc_fail_countdown = -1;
    geom.vertex_capacity = 64;
    geom.vertex_count = 64;
    geom.vertices =
        (struct ui_svg_point *)UI_MALLOC(64 * sizeof(struct ui_svg_point));

    geom.index_capacity = 128;
    geom.index_count = 128;
    geom.indices = (ui_uint32 *)UI_MALLOC(128 * sizeof(ui_uint32));

    if (force_realloc_at < 6) {
      geom.vertex_count = geom.vertex_capacity - force_realloc_at;
    } else {
      geom.index_count = geom.index_capacity - (force_realloc_at - 6);
    }

    g_malloc_fail_countdown = oom_idx;
    ui_svg_tessellate_fill(&geom, &f);
  }
  g_malloc_fail_countdown = -1;

  f.subpaths = NULL;
  ui_svg_geometry_destroy(&geom);
#endif
  return 0;
}

static int run_tessellate_stroke_oom_branches(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_svg_geometry geom;
  struct ui_svg_flattened_path f;
  struct ui_svg_subpath sp;
  struct ui_svg_point pts[2] = {{0, 0}, {10, 10}};
  int i;

  ui_svg_geometry_init(&geom);
  ui_svg_flattened_path_init(&f);
  sp.points = pts;
  sp.count = 2;
  sp.capacity = 2;
  sp.closed = 0;
  f.subpaths = &sp;
  f.count = 1;
  f.capacity = 1;

  for (i = 0; i < 400; i++) {
    ui_svg_geometry_destroy(&geom);
    ui_svg_geometry_init(&geom);

    int oom_idx = i % 10;
    int force_realloc_at = (i / 10) % 10; /* 0 to 9 */

    g_malloc_fail_countdown = -1;
    geom.vertex_capacity = 64;
    geom.vertex_count = 64;
    geom.vertices =
        (struct ui_svg_point *)UI_MALLOC(64 * sizeof(struct ui_svg_point));

    geom.index_capacity = 128;
    geom.index_count = 128;
    geom.indices = (ui_uint32 *)UI_MALLOC(128 * sizeof(ui_uint32));

    /* Adjust counts so the Nth append triggers reallocation */
    if (force_realloc_at < 4) {
      geom.vertex_count = geom.vertex_capacity - force_realloc_at;
    } else {
      geom.index_count = geom.index_capacity - (force_realloc_at - 4);
    }

    g_malloc_fail_countdown = oom_idx;
    ui_svg_tessellate_stroke(&geom, &f, 2.0f);
  }
  g_malloc_fail_countdown = -1;

  f.subpaths = NULL;
  ui_svg_geometry_destroy(&geom);
#endif
  return 0;
}

static int run_targeted_flatten_oom(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_svg_path path;
  struct ui_svg_flattened_path flat;
  int i;

  /* Target implicit subpath */
  ui_svg_path_init(&path);
  ui_svg_path_parse(&path, "L 10 10");
  for (i = 0; i < 5; i++) {
    ui_svg_flattened_path_init(&flat);
    g_malloc_fail_countdown = i;
    ui_svg_path_flatten(&flat, &path, 1.0f);
    g_malloc_fail_countdown = -1;
    ui_svg_flattened_path_destroy(&flat);
  }
  ui_svg_path_destroy(&path);

  /* Target cubic */
  ui_svg_path_init(&path);
  ui_svg_path_parse(&path, "M 0 0 C 0 100 100 100 100 0");
  for (i = 0; i < 10; i++) {
    ui_svg_flattened_path_init(&flat);
    g_malloc_fail_countdown = i;
    ui_svg_path_flatten(&flat, &path, 0.01f);
    g_malloc_fail_countdown = -1;
    ui_svg_flattened_path_destroy(&flat);
  }
  ui_svg_path_destroy(&path);

  /* Target quadratic */
  ui_svg_path_init(&path);
  ui_svg_path_parse(&path, "M 0 0 Q 50 100 100 0");
  for (i = 0; i < 10; i++) {
    ui_svg_flattened_path_init(&flat);
    g_malloc_fail_countdown = i;
    ui_svg_path_flatten(&flat, &path, 0.01f);
    g_malloc_fail_countdown = -1;
    ui_svg_flattened_path_destroy(&flat);
  }
  ui_svg_path_destroy(&path);

  /* Target arc */
  ui_svg_path_init(&path);
  ui_svg_path_parse(
      &path, "M 0 0 L 1 1 L 2 2 L 3 3 L 4 4 L 5 5 L 6 6 L 7 7 L 8 8 L 9 9 L 10 "
             "10 L 11 11 L 12 12 L 13 13 L 14 14 L 15 15 A 10 10 0 0 0 20 20");
  for (i = 0; i < 10; i++) {
    ui_svg_flattened_path_init(&flat);
    g_malloc_fail_countdown = i;
    ui_svg_path_flatten(&flat, &path, 0.01f);
    g_malloc_fail_countdown = -1;
    ui_svg_flattened_path_destroy(&flat);
  }
  ui_svg_path_destroy(&path);

  /* Target close path */
  ui_svg_path_init(&path);
  ui_svg_path_parse(&path,
                    "M 0 0 L 1 1 L 2 2 L 3 3 L 4 4 L 5 5 L 6 6 L 7 7 L 8 8 L 9 "
                    "9 L 10 10 L 11 11 L 12 12 L 13 13 L 14 14 L 15 15 Z");
  for (i = 0; i < 10; i++) {
    ui_svg_flattened_path_init(&flat);
    g_malloc_fail_countdown = i;
    ui_svg_path_flatten(&flat, &path, 1.0f);
    g_malloc_fail_countdown = -1;
    ui_svg_flattened_path_destroy(&flat);
  }
  ui_svg_path_destroy(&path);

#endif
  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_svg_path path;
  struct ui_svg_flattened_path flat;
  struct ui_svg_geometry geom;
  int failed = 0;
  int temp_failed;

  ui_svg_path_init(&path);
  ui_svg_flattened_path_init(&flat);
  ui_svg_geometry_init(&geom);

  /* path parsing capacity OOM */
  g_malloc_fail_countdown = 0;
  failed |= (ui_svg_path_parse(&path, "M 0 0") != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_svg_path_parse(&path,
                    "M 0 0 L 1 1 L 2 2 L 3 3 L 4 4 L 5 5 L 6 6 L 7 7 L 8 8 L 9 "
                    "9 L 10 10 L 11 11 L 12 12 L 13 13 L 14 14 L 15 15 L 16 16 "
                    "C 0 100 100 100 100 0 C 0 100 100 100 100 0 "
                    "Q 50 100 0 0 Q 50 100 0 0 "
                    "A 10 10 0 0 0 10 10 "
                    "Z");

  /* Flatten OOM exhaustive */
  int i;
  for (i = 0; i < 300; i++) {
    ui_svg_flattened_path_destroy(&flat);
    ui_svg_flattened_path_init(&flat);
    g_malloc_fail_countdown = i;
    ui_svg_path_flatten(&flat, &path, 0.1f);
  }
  g_malloc_fail_countdown = -1;

  ui_svg_flattened_path_destroy(&flat);
  ui_svg_flattened_path_init(&flat);
  ui_svg_path_flatten(&flat, &path, 1.0f);

  /* Need a shape with ears to test vertex append OOM */
  ui_svg_path_destroy(&path);
  ui_svg_flattened_path_destroy(&flat);
  ui_svg_path_init(&path);
  ui_svg_flattened_path_init(&flat);
  ui_svg_path_parse(&path, "M 0 0 L 10 0 L 10 10 Z");
  ui_svg_path_flatten(&flat, &path, 1.0f);

  /* Tessellate Fill OOM (linked array) */
  ui_svg_geometry_destroy(&geom);
  ui_svg_geometry_init(&geom);
  g_malloc_fail_countdown = 0;
  failed |= (ui_svg_tessellate_fill(&geom, &flat) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_svg_geometry_destroy(&geom);
  ui_svg_geometry_init(&geom);
  g_malloc_fail_countdown = 1;
  failed |= (ui_svg_tessellate_fill(&geom, &flat) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_svg_geometry_destroy(&geom);
  ui_svg_geometry_init(&geom);
  g_malloc_fail_countdown = 2; /* index append */
  failed |= (ui_svg_tessellate_fill(&geom, &flat) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
  /* Tessellate stroke OOM */
  ui_svg_geometry_destroy(&geom);
  ui_svg_geometry_init(&geom);
  g_malloc_fail_countdown = 0;
  failed |=
      (ui_svg_tessellate_stroke(&geom, &flat, 2.0f) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_svg_geometry_destroy(&geom);
  ui_svg_geometry_init(&geom);
  g_malloc_fail_countdown = 1;
  failed |=
      (ui_svg_tessellate_stroke(&geom, &flat, 2.0f) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
  ui_svg_geometry_destroy(&geom);
  ui_svg_flattened_path_destroy(&flat);
  ui_svg_path_destroy(&path);
  return failed;
#else
  return 0;
#endif
}

static int run_edge_cases(void) {
  struct ui_svg_geometry geom;
  struct ui_svg_flattened_path f;
  struct ui_svg_subpath sp;
  int failed = 0;

  /* NULL Checks */
  failed |= (ui_svg_path_init(NULL) != UI_ERROR_INVALID_ARGUMENT);
  ui_svg_path_destroy(NULL);
  failed |= (ui_svg_path_parse(NULL, "M 0 0") != UI_ERROR_INVALID_ARGUMENT);

  struct ui_svg_path p;
  ui_svg_path_init(&p);
  failed |= (ui_svg_path_parse(&p, NULL) != UI_ERROR_INVALID_ARGUMENT);

  failed |= (ui_svg_flattened_path_init(NULL) != UI_ERROR_INVALID_ARGUMENT);
  ui_svg_flattened_path_destroy(NULL);
  failed |= (ui_svg_path_flatten(NULL, &p, 1.0f) != UI_ERROR_INVALID_ARGUMENT);
  ui_svg_flattened_path_init(&f);
  failed |= (ui_svg_path_flatten(&f, NULL, 1.0f) != UI_ERROR_INVALID_ARGUMENT);

  failed |= (ui_svg_geometry_init(NULL) != UI_ERROR_INVALID_ARGUMENT);
  ui_svg_geometry_destroy(NULL);
  ui_svg_geometry_init(&geom);

  failed |= (ui_svg_tessellate_fill(NULL, &f) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_svg_tessellate_fill(&geom, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_svg_tessellate_stroke(NULL, &f, 1.0f) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_svg_tessellate_stroke(&geom, NULL, 1.0f) !=
             UI_ERROR_INVALID_ARGUMENT);

  /* Edge Case: Degen Arc rx=ry=0 */
  ui_svg_path_parse(&p, "M 0 0 A 0 0 0 0 0 10 10 Z");
  ui_svg_path_flatten(&f, &p, 1.0f);
  ui_svg_tessellate_fill(&geom, &f);
  ui_svg_tessellate_stroke(&geom, &f, 2.0f);

  /* Edge Case: Collinear */
  ui_svg_path_destroy(&p);
  ui_svg_flattened_path_destroy(&f);
  ui_svg_geometry_destroy(&geom);
  ui_svg_path_init(&p);
  ui_svg_flattened_path_init(&f);
  ui_svg_geometry_init(&geom);
  ui_svg_path_parse(&p, "M 0 0 L 0 0 L 0 0 Z");
  ui_svg_path_flatten(&f, &p, 1.0f);
  ui_svg_tessellate_fill(&geom, &f);
  ui_svg_tessellate_stroke(&geom, &f, 2.0f);

  /* Edge Case: Rad Check Arc */
  ui_svg_path_destroy(&p);
  ui_svg_flattened_path_destroy(&f);
  ui_svg_path_init(&p);
  ui_svg_flattened_path_init(&f);
  ui_svg_path_parse(&p, "M 0 0 A 10 10 0 0 0 10 10 A 10 10 0 0 1 15 15 A 10 "
                        "10 0 1 0 10 20 A 10 10 0 1 1 0 20 A 10 10 0 1 1 10 10 "
                        "A 10 10 0 0 0 100 100");
  ui_svg_path_flatten(&f, &p, 1.0f);

  /* Path starting without M */
  ui_svg_path_destroy(&p);
  ui_svg_flattened_path_destroy(&f);
  ui_svg_path_init(&p);
  ui_svg_flattened_path_init(&f);
  ui_svg_path_parse(&p, "L 10 10 Z");
  ui_svg_path_flatten(&f, &p, 1.0f);

  ui_svg_path_destroy(&p);
  ui_svg_flattened_path_destroy(&f);
  ui_svg_geometry_destroy(&geom);
  return failed;
}

int main(void) {
  int failed = 0;
  failed += run_test("test_parse_basic_commands", test_parse_basic_commands);
  failed +=
      run_test("test_parse_implicit_commands", test_parse_implicit_commands);
  failed +=
      run_test("test_parse_relative_commands", test_parse_relative_commands);
  failed += run_test("test_parse_curves", test_parse_curves);
  failed += run_test("test_parse_implicit_signs", test_parse_implicit_signs);
  failed += run_test("test_parse_trailing_whitespace",
                     test_parse_trailing_whitespace);
  failed += run_test("test_fine_flattening", test_fine_flattening);
  failed += run_test("test_flattening", test_flattening);
  failed += run_test("test_tessellate", test_tessellate);
  failed += run_test("run_parse_failures", run_parse_failures);
  failed += run_test("run_stroke_test", run_stroke_test);
  failed += run_test("run_cw_tests", run_cw_tests);
  failed += run_test("run_tessellate_stroke_oom_branches",
                     run_tessellate_stroke_oom_branches);
  failed +=
      run_test("run_tessellate_oom_branches", run_tessellate_oom_branches);
  failed += run_test("run_targeted_flatten_oom", run_targeted_flatten_oom);
  failed += run_test("run_edge_cases", run_edge_cases);
  failed += run_test("run_oom_tests", run_oom_tests);
  return failed == 0 ? 0 : 1;
}
