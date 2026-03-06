#ifndef M3_MOTION_H
#define M3_MOTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_predictive.h"
#include "cmpc/cmp_anim.h"
#include "cmpc/cmp_math.h"

#define M3_SHARED_AXIS_X 0
#define M3_SHARED_AXIS_Y 1
#define M3_SHARED_AXIS_Z 2

#define M3_SHARED_AXIS_SLIDE_DISTANCE 30.0f

/**
 * @brief Represents the result of a motion calculation.
 */
typedef struct M3MotionResult {
  CMPScalar opacity; /**< The computed opacity. */
  CMPScalar scale_x; /**< The computed horizontal scale. */
  CMPScalar scale_y; /**< The computed vertical scale. */
  CMPScalar offset_x; /**< The computed horizontal offset. */
  CMPScalar offset_y; /**< The computed vertical offset. */
} M3MotionResult;

CMP_API int CMP_CALL m3_motion_shared_axis(cmp_u32 axis, CMPBool forward,
                                           CMPScalar fraction,
                                           M3MotionResult *out_entering,
                                           M3MotionResult *out_exiting);

CMP_API int CMP_CALL m3_motion_fade_through(CMPScalar fraction,
                                            M3MotionResult *out_entering,
                                            M3MotionResult *out_exiting);

/** 
 * @brief Calculate the predictive back motion transformation. 
 * @param event The predictive back event. 
 * @param start_bounds The original bounds of the container. 
 * @param out_bounds Receives the morphed bounds. 
 * @param out_corner_radius Receives the computed corner radius. 
 * @param out_opacity Receives the computed opacity. 
 * @return CMP_OK on success or a failure code. 
 */ 
CMP_API int CMP_CALL m3_motion_predictive_back(const CMPPredictiveBackEvent *event,
                                               CMPRect start_bounds,
                                               CMPRect *out_bounds,
                                               CMPScalar *out_corner_radius,
                                               CMPScalar *out_opacity);

CMP_API int CMP_CALL m3_motion_container_transform(CMPRect start_bounds,
                                                   CMPRect end_bounds,
                                                   CMPScalar fraction,
                                                   CMPRect *out_bounds,
                                                   M3MotionResult *out_entering,
                                                   M3MotionResult *out_exiting);

#ifdef __cplusplus
}
#endif

#endif /* M3_MOTION_H */
