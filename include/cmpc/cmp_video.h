#ifndef CMP_VIDEO_H
#define CMP_VIDEO_H

/**
 * @file cmp_video.h
 * @brief Video decoding helpers for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
/* clang-format on */

/**
 * @brief Video decoder configuration.
 */
typedef struct CMPVideoConfig {
  CMPEnv *env; /**< Optional environment backend used for platform decoding. */
  const CMPAllocator *allocator; /**< Allocator used for decoder state (NULL
                                   uses default). */
} CMPVideoConfig;

/**
 * @brief Video decoder state.
 */
typedef struct CMPVideoDecoder {
  CMPVideo video;         /**< Video decoder interface handle. */
  CMPAllocator allocator; /**< Allocator used for decoder state. */
  void *fallback_state;   /**< Internal fallback decoder state. */
  CMPBool ready;          /**< CMP_TRUE when initialized. */
  CMPBool has_backend;    /**< CMP_TRUE when a backend decoder is available. */
  CMPBool opened;         /**< CMP_TRUE when a stream is opened. */
  CMPBool using_fallback; /**< CMP_TRUE when fallback decoder is active. */
} CMPVideoDecoder;

/**
 * @brief Initialize a video configuration with defaults.
 * @param config Configuration to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_video_config_init(CMPVideoConfig *config);

/**
 * @brief Initialize a video open request with defaults.
 * @param request Request structure to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_video_request_init(CMPVideoOpenRequest *request);

/**
 * @brief Initialize a video decoder using the supplied configuration.
 * @param decoder Decoder instance to initialize.
 * @param config Video decoder configuration.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_video_init(CMPVideoDecoder *decoder,
                                    const CMPVideoConfig *config);

/**
 * @brief Shut down a video decoder and release its state.
 * @param decoder Decoder instance to shut down.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_video_shutdown(CMPVideoDecoder *decoder);

/**
 * @brief Open a video stream using backend or fallback decoders.
 * @param decoder Initialized video decoder.
 * @param request Video open request parameters.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_video_open(CMPVideoDecoder *decoder,
                                    const CMPVideoOpenRequest *request);

/**
 * @brief Close a video stream and release its resources.
 * @param decoder Initialized video decoder.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_video_close(CMPVideoDecoder *decoder);

/**
 * @brief Read the next video frame.
 * @param decoder Initialized video decoder.
 * @param out_frame Receives the frame description.
 * @param out_has_frame Receives CMP_TRUE if a frame was read.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_video_read_frame(CMPVideoDecoder *decoder,
                                          CMPVideoFrame *out_frame,
                                          CMPBool *out_has_frame);

/**
 * @brief Helper for test coverage.
 * @param call_count Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_video_test_set_read_u32_fail_after(cmp_u32 call_count);
CMP_API int
    CMP_CALL
    /**
     * @brief Helper for test coverage.
     * @param call_count Parameter for testing.
     * @return Test result.
     */
    cmp_video_test_set_mul_overflow_fail_after(cmp_u32 call_count);
/**
 * @brief Helper for test coverage.
 * @param a Parameter for testing.
 * @param b Parameter for testing.
 * @param out_result Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_video_test_mul_overflow(cmp_usize a, cmp_usize b,
                                                 cmp_usize *out_result);
/**
 * @brief Helper for test coverage.
 * @param data Parameter for testing.
 * @param size Parameter for testing.
 * @param offset Parameter for testing.
 * @param out_val Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_video_test_read_u32_le(const cmp_u8 *data,
                                                cmp_usize size,
                                                cmp_usize offset,
                                                cmp_u32 *out_val);
CMP_API int
    CMP_CALL
    /**
     * @brief Helper for test coverage.
     * @param request Parameter for testing.
     * @return Test result.
     */
    cmp_video_test_fallback_parse(const CMPVideoOpenRequest *request);
/**
 * @brief Helper for test coverage.
 * @param decoder Parameter for testing.
 * @param request Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_video_test_fallback_open(
    CMPVideoDecoder *decoder, const CMPVideoOpenRequest *request);
/**
 * @brief Helper for test coverage.
 * @param decoder Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_video_test_fallback_close(CMPVideoDecoder *decoder);
/**
 * @brief Helper for test coverage.
 * @param decoder Parameter for testing.
 * @param out_frame Parameter for testing.
 * @param out_handled Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_video_test_fallback_read_frame_raw(
    CMPVideoDecoder *decoder, CMPVideoFrame *out_frame, CMPBool *out_handled);
/**
 * @brief Helper for test coverage.
 * @param a Parameter for testing.
 * @param b Parameter for testing.
 * @param c Parameter for testing.
 * @param d Parameter for testing.
 * @param e Parameter for testing.
 * @param f Parameter for testing.
 * @param g Parameter for testing.
 * @param out_frame Parameter for testing.
 * @param out_handled Parameter for testing.
 * @return Test result.
 */
CMP_API int CMP_CALL cmp_video_test_fallback_read_frame_case(
    cmp_u32 a, cmp_u32 b, cmp_u32 c, cmp_u32 d, cmp_u32 e, cmp_u32 f,
    cmp_usize g, CMPVideoFrame *out_frame, CMPBool *out_handled);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_VIDEO_H */
