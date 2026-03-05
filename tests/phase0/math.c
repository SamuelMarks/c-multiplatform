#include "cmpc/cmp_math.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

static void assert_equal(float a, float b, const char* msg) {
    if (fabs(a - b) > 0.001) {
        fprintf(stderr, "FAIL: %s (expected %f, got %f)\n", msg, b, a);
        exit(1);
    }
}

static void test_rect(void) {
    CMPRect a, b, out_rect;
    CMPBool has_int;
    
    a.x = 0; a.y = 0; a.width = 10; a.height = 10;
    b.x = 5; b.y = 5; b.width = 10; b.height = 10;
    
    if (cmp_rect_intersect(&a, &b, &out_rect, &has_int) != CMP_OK) exit(1);
    if (!has_int) exit(1);
    assert_equal(out_rect.x, 5, "intersect x");
    assert_equal(out_rect.y, 5, "intersect y");
    assert_equal(out_rect.width, 5, "intersect w");
    assert_equal(out_rect.height, 5, "intersect h");

    if (cmp_rect_union(&a, &b, &out_rect) != CMP_OK) exit(1);
    assert_equal(out_rect.x, 0, "union x");
    assert_equal(out_rect.y, 0, "union y");
    assert_equal(out_rect.width, 15, "union w");
    assert_equal(out_rect.height, 15, "union h");

    b.x = 20; b.y = 20;
    if (cmp_rect_intersect(&a, &b, &out_rect, &has_int) != CMP_OK) exit(1);
    if (has_int) exit(1);

    /* Test null parameters */
    if (cmp_rect_intersect(NULL, &b, &out_rect, &has_int) == CMP_OK) exit(1);
    if (cmp_rect_union(NULL, &b, &out_rect) == CMP_OK) exit(1);
}

static void test_mat3(void) {
    CMPMat3 ident, trans, scale, rot, prod;
    CMPScalar out_x, out_y;

    if (cmp_mat3_identity(&ident) != CMP_OK) exit(1);
    if (cmp_mat3_translate(10, 20, &trans) != CMP_OK) exit(1);
    if (cmp_mat3_scale(2, 3, &scale) != CMP_OK) exit(1);
    if (cmp_mat3_rotate(3.14159f / 2.0f, &rot) != CMP_OK) exit(1);

    if (cmp_mat3_mul(&trans, &scale, &prod) != CMP_OK) exit(1);

    if (cmp_mat3_transform_point(&ident, 5, 5, &out_x, &out_y) != CMP_OK) exit(1);
    assert_equal(out_x, 5, "ident transform x");
    
    if (cmp_mat3_transform_point(&trans, 5, 5, &out_x, &out_y) != CMP_OK) exit(1);
    assert_equal(out_x, 15, "trans transform x");
    assert_equal(out_y, 25, "trans transform y");

    if (cmp_mat3_transform_point(&scale, 5, 5, &out_x, &out_y) != CMP_OK) exit(1);
    assert_equal(out_x, 10, "scale transform x");
    assert_equal(out_y, 15, "scale transform y");

    if (cmp_mat3_transform_point(&rot, 10, 0, &out_x, &out_y) != CMP_OK) exit(1);
    assert_equal(out_x, 0, "rot transform x");
    assert_equal(out_y, 10, "rot transform y");

    /* Null tests */
    if (cmp_mat3_identity(NULL) == CMP_OK) exit(1);
    if (cmp_mat3_mul(NULL, &scale, &prod) == CMP_OK) exit(1);
    if (cmp_mat3_translate(1, 1, NULL) == CMP_OK) exit(1);
    if (cmp_mat3_scale(1, 1, NULL) == CMP_OK) exit(1);
    if (cmp_mat3_rotate(1, NULL) == CMP_OK) exit(1);
    if (cmp_mat3_transform_point(NULL, 1, 1, &out_x, &out_y) == CMP_OK) exit(1);
}

static void test_contains(void) {
    CMPRect r;
    CMPBool cont;
    r.x = 0; r.y = 0; r.width = 10; r.height = 10;

    if (cmp_rect_contains_point(&r, 5, 5, &cont) != CMP_OK) exit(1);
    if (!cont) exit(1);

    if (cmp_rect_contains_point(&r, -1, 5, &cont) != CMP_OK) exit(1);
    if (cont) exit(1);

    if (cmp_rect_contains_point(NULL, 1, 1, &cont) == CMP_OK) exit(1);
}

static void test_math_edge_cases(void) {
    CMPRect a, b, out_rect;
    CMPBool cont;
    CMPScalar out_x, out_y;

    a.x = 0; a.y = 0; a.width = -1; a.height = 10;
    b.x = 5; b.y = 5; b.width = 10; b.height = 10;
    
    if (cmp_rect_intersect(&a, &b, &out_rect, &cont) != CMP_ERR_RANGE) exit(1);
    if (cmp_rect_union(&a, &b, &out_rect) != CMP_ERR_RANGE) exit(1);
    
    a.width = 10; a.height = -1;
    if (cmp_rect_intersect(&a, &b, &out_rect, &cont) != CMP_ERR_RANGE) exit(1);
    if (cmp_rect_union(&a, &b, &out_rect) != CMP_ERR_RANGE) exit(1);

    a.height = 10; b.width = -1;
    if (cmp_rect_intersect(&a, &b, &out_rect, &cont) != CMP_ERR_RANGE) exit(1);
    if (cmp_rect_union(&a, &b, &out_rect) != CMP_ERR_RANGE) exit(1);

    b.width = 10; b.height = -1;
    if (cmp_rect_intersect(&a, &b, &out_rect, &cont) != CMP_ERR_RANGE) exit(1);
    if (cmp_rect_union(&a, &b, &out_rect) != CMP_ERR_RANGE) exit(1);
    b.height = 10;

    a.width = 10;
    if (cmp_rect_contains_point(&a, 5, 5, NULL) != CMP_ERR_INVALID_ARGUMENT) exit(1);

    a.width = -1;
    if (cmp_rect_contains_point(&a, 5, 5, &cont) != CMP_ERR_RANGE) exit(1);

    a.width = 0; a.height = 10;
    if (cmp_rect_contains_point(&a, 5, 5, &cont) != CMP_OK || cont) exit(1);

    a.width = 10; a.height = 0;
    if (cmp_rect_contains_point(&a, 5, 5, &cont) != CMP_OK || cont) exit(1);

    a.width = 10; a.height = 10;
    if (cmp_rect_contains_point(&a, -1, 5, &cont) != CMP_OK || cont) exit(1);
    if (cmp_rect_contains_point(&a, 5, -1, &cont) != CMP_OK || cont) exit(1);
    if (cmp_rect_contains_point(&a, 11, 5, &cont) != CMP_OK || cont) exit(1);
    if (cmp_rect_contains_point(&a, 5, 11, &cont) != CMP_OK || cont) exit(1);

    if (cmp_mat3_transform_point(NULL, 1, 1, &out_x, NULL) == CMP_OK) exit(1);
    CMPMat3 zmat;
    cmp_mat3_identity(&zmat);
    zmat.m[8] = 0.0f; /* Make tw=0 for x=0, y=0 */
    if (cmp_mat3_transform_point(&zmat, 0, 0, &out_x, &out_y) != CMP_ERR_RANGE) exit(1);
}

int main(void) {
    test_rect();
    test_mat3();
    test_contains();
    test_math_edge_cases();
    printf("math tests passed\n");
    return 0;
}
