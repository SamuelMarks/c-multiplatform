/* clang-format off */
#ifndef CMP_INK_RIPPLE_H
#define CMP_INK_RIPPLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cmp_ink_ripple cmp_ink_ripple_t;

int cmp_ink_ripple_create(cmp_ink_ripple_t **out_ripple);
int cmp_ink_ripple_destroy(cmp_ink_ripple_t *ripple);
int cmp_ink_ripple_update(cmp_ink_ripple_t *ripple, float dt_ms);
int cmp_ink_ripple_trigger(cmp_ink_ripple_t *ripple, float start_x, float start_y);

#ifdef __cplusplus
}
#endif
#endif /* CMP_INK_RIPPLE_H */
/* clang-format on */
