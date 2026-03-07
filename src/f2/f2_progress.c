#include "f2/f2_progress.h"
#include "cmpc/cmp_core.h"
#include <string.h>

CMP_API int CMP_CALL f2_linear_progress_style_init(F2LinearProgressStyle *style) {
    if (!style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(style, 0, sizeof(*style));
    style->height = F2_LINEAR_PROGRESS_DEFAULT_HEIGHT;
    style->track_color = (CMPColor){0.9f, 0.9f, 0.9f, 1.0f};
    style->fill_color = (CMPColor){0.0f, 0.36f, 0.77f, 1.0f};
    return CMP_OK;
}

CMP_API int CMP_CALL f2_linear_progress_init(F2LinearProgress *progress,
                                             const F2LinearProgressStyle *style,
                                             CMPScalar value) {
    if (!progress || !style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(progress, 0, sizeof(*progress));
    progress->style = *style;
    progress->value = value;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_linear_progress_set_value(F2LinearProgress *progress, CMPScalar value) {
    if (!progress) return CMP_ERR_INVALID_ARGUMENT;
    progress->value = value;
    return CMP_OK;
}


CMP_API int CMP_CALL f2_circular_progress_style_init(F2CircularProgressStyle *style) {
    if (!style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(style, 0, sizeof(*style));
    style->size = F2_CIRCULAR_PROGRESS_DEFAULT_SIZE;
    style->thickness = F2_CIRCULAR_PROGRESS_DEFAULT_THICKNESS;
    style->track_color = (CMPColor){0.0f, 0.0f, 0.0f, 0.0f}; /* Fluent circular usually has no track */
    style->fill_color = (CMPColor){0.0f, 0.36f, 0.77f, 1.0f};
    return CMP_OK;
}

CMP_API int CMP_CALL f2_circular_progress_init(F2CircularProgress *progress,
                                               const F2CircularProgressStyle *style,
                                               CMPScalar value) {
    if (!progress || !style) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(progress, 0, sizeof(*progress));
    progress->style = *style;
    progress->value = value;
    return CMP_OK;
}

CMP_API int CMP_CALL f2_circular_progress_set_value(F2CircularProgress *progress, CMPScalar value) {
    if (!progress) return CMP_ERR_INVALID_ARGUMENT;
    progress->value = value;
    return CMP_OK;
}
