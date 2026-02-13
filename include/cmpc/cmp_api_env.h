#ifndef CMP_API_ENV_H
#define CMP_API_ENV_H

/**
 * @file cmp_api_env.h
 * @brief Environment abstraction ABI for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_math.h"
#include "cmpc/cmp_object.h"

/**
 * @brief File information metadata.
 */
typedef struct CMPFileInfo {
  cmp_u32 size_bytes; /**< File size in bytes. */
  cmp_u32 flags;      /**< Backend-specific file flags. */
} CMPFileInfo;

/**
 * @brief IO virtual table.
 */
typedef struct CMPIOVTable {
/**
 * @brief Read a file into a caller-provided buffer.
 * @param io IO backend instance.
 * @param utf8_path File path in UTF-8.
 * @param buffer Destination buffer.
 * @param buffer_size Size of the destination buffer.
 * @param out_read Receives number of bytes read.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int read_file(void *io, const char *utf8_path, void *buffer,
                cmp_usize buffer_size, cmp_usize *out_read);
#else
  int(CMP_CALL *read_file)(void *io, const char *utf8_path, void *buffer,
                           cmp_usize buffer_size, cmp_usize *out_read);
#endif
/**
 * @brief Read a file by allocating a new buffer.
 * @param io IO backend instance.
 * @param utf8_path File path in UTF-8.
 * @param allocator Allocator used for the buffer.
 * @param out_data Receives the allocated buffer.
 * @param out_size Receives the buffer size in bytes.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int read_file_alloc(void *io, const char *utf8_path,
                      const CMPAllocator *allocator, void **out_data,
                      cmp_usize *out_size);
#else
  int(CMP_CALL *read_file_alloc)(void *io, const char *utf8_path,
                                 const CMPAllocator *allocator, void **out_data,
                                 cmp_usize *out_size);
#endif
/**
 * @brief Write a buffer to a file.
 * @param io IO backend instance.
 * @param utf8_path File path in UTF-8.
 * @param data Source data buffer.
 * @param size Size of the data buffer.
 * @param overwrite CMP_TRUE to overwrite existing files.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int write_file(void *io, const char *utf8_path, const void *data,
                 cmp_usize size, CMPBool overwrite);
#else
  int(CMP_CALL *write_file)(void *io, const char *utf8_path, const void *data,
                            cmp_usize size, CMPBool overwrite);
#endif
/**
 * @brief Test whether a file exists.
 * @param io IO backend instance.
 * @param utf8_path File path in UTF-8.
 * @param out_exists Receives CMP_TRUE if the file exists.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int file_exists(void *io, const char *utf8_path, CMPBool *out_exists);
#else
  int(CMP_CALL *file_exists)(void *io, const char *utf8_path,
                             CMPBool *out_exists);
#endif
/**
 * @brief Delete a file.
 * @param io IO backend instance.
 * @param utf8_path File path in UTF-8.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int delete_file(void *io, const char *utf8_path);
#else
  int(CMP_CALL *delete_file)(void *io, const char *utf8_path);
#endif
/**
 * @brief Retrieve file metadata.
 * @param io IO backend instance.
 * @param utf8_path File path in UTF-8.
 * @param out_info Receives file information.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int stat_file(void *io, const char *utf8_path, CMPFileInfo *out_info);
#else
  int(CMP_CALL *stat_file)(void *io, const char *utf8_path,
                           CMPFileInfo *out_info);
#endif
} CMPIOVTable;

/**
 * @brief IO interface.
 */
typedef struct CMPIO {
  void *ctx;                 /**< IO backend context pointer. */
  const CMPIOVTable *vtable; /**< IO virtual table. */
} CMPIO;

/** @brief Accelerometer sensor type. */
#define CMP_SENSOR_ACCELEROMETER 1
/** @brief Gyroscope sensor type. */
#define CMP_SENSOR_GYROSCOPE 2
/** @brief Magnetometer sensor type. */
#define CMP_SENSOR_MAGNETOMETER 3
/** @brief GPS sensor type. */
#define CMP_SENSOR_GPS 4

/**
 * @brief Sensor reading.
 */
typedef struct CMPSensorReading {
  cmp_u32 type;        /**< Sensor type (CMP_SENSOR_*). */
  cmp_u32 time_ms;     /**< Timestamp in milliseconds. */
  CMPScalar values[4]; /**< Sensor values. */
  cmp_u32 value_count; /**< Number of valid entries in values. */
} CMPSensorReading;

/**
 * @brief Sensors virtual table.
 */
typedef struct CMPSensorsVTable {
/**
 * @brief Test whether a sensor type is available.
 * @param sensors Sensors backend instance.
 * @param type Sensor type (CMP_SENSOR_*).
 * @param out_available Receives CMP_TRUE if available.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int is_available(void *sensors, cmp_u32 type, CMPBool *out_available);
#else
  int(CMP_CALL *is_available)(void *sensors, cmp_u32 type,
                              CMPBool *out_available);
#endif
/**
 * @brief Start producing readings for a sensor.
 * @param sensors Sensors backend instance.
 * @param type Sensor type (CMP_SENSOR_*).
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int start(void *sensors, cmp_u32 type);
#else
  int(CMP_CALL *start)(void *sensors, cmp_u32 type);
#endif
/**
 * @brief Stop producing readings for a sensor.
 * @param sensors Sensors backend instance.
 * @param type Sensor type (CMP_SENSOR_*).
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int stop(void *sensors, cmp_u32 type);
#else
  int(CMP_CALL *stop)(void *sensors, cmp_u32 type);
#endif
/**
 * @brief Read the latest sensor data.
 * @param sensors Sensors backend instance.
 * @param type Sensor type (CMP_SENSOR_*).
 * @param out_reading Receives the sensor reading.
 * @param out_has_reading Receives CMP_TRUE if data was read.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int read(void *sensors, cmp_u32 type, CMPSensorReading *out_reading,
           CMPBool *out_has_reading);
#else
  int(CMP_CALL *read)(void *sensors, cmp_u32 type,
                      CMPSensorReading *out_reading, CMPBool *out_has_reading);
#endif
} CMPSensorsVTable;

/**
 * @brief Sensors interface.
 */
typedef struct CMPSensors {
  void *ctx;                      /**< Sensors backend context pointer. */
  const CMPSensorsVTable *vtable; /**< Sensors virtual table. */
} CMPSensors;

/** @brief Any camera frame format (backend chooses). */
#define CMP_CAMERA_FORMAT_ANY 0
/** @brief RGBA8 camera frame format. */
#define CMP_CAMERA_FORMAT_RGBA8 1
/** @brief BGRA8 camera frame format. */
#define CMP_CAMERA_FORMAT_BGRA8 2
/** @brief NV12 camera frame format. */
#define CMP_CAMERA_FORMAT_NV12 3

/** @brief Camera facing preference unspecified. */
#define CMP_CAMERA_FACING_UNSPECIFIED 0
/** @brief Front-facing camera preference. */
#define CMP_CAMERA_FACING_FRONT 1
/** @brief Back-facing camera preference. */
#define CMP_CAMERA_FACING_BACK 2
/** @brief External-facing camera preference. */
#define CMP_CAMERA_FACING_EXTERNAL 3

/** @brief Default camera identifier. */
#define CMP_CAMERA_ID_DEFAULT ((cmp_u32) ~(cmp_u32)0)

/**
 * @brief Camera frame description.
 */
typedef struct CMPCameraFrame {
  cmp_u32 format;   /**< Pixel format (CMP_CAMERA_FORMAT_*). */
  cmp_u32 width;    /**< Frame width in pixels. */
  cmp_u32 height;   /**< Frame height in pixels. */
  const void *data; /**< Pointer to frame data. */
  cmp_usize size;   /**< Size of frame data in bytes. */
} CMPCameraFrame;

/**
 * @brief Camera configuration.
 */
typedef struct CMPCameraConfig {
  cmp_u32 camera_id; /**< Backend-specific camera identifier
                       (CMP_CAMERA_ID_DEFAULT for automatic selection). */
  cmp_u32 facing;    /**< Requested camera facing (CMP_CAMERA_FACING_*). */
  cmp_u32
      width; /**< Requested frame width in pixels (0 for backend default). */
  cmp_u32
      height; /**< Requested frame height in pixels (0 for backend default). */
  cmp_u32 format; /**< Requested pixel format (CMP_CAMERA_FORMAT_*). */
} CMPCameraConfig;

/**
 * @brief Open a camera device.
 * @param camera Camera backend instance.
 * @param camera_id Backend-specific camera identifier.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPCameraOpenFn)(void *camera, cmp_u32 camera_id);

/**
 * @brief Open a camera device with configuration options.
 * @param camera Camera backend instance.
 * @param config Camera configuration.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPCameraOpenWithConfigFn)(void *camera,
                                                 const CMPCameraConfig *config);

/**
 * @brief Close the camera device.
 * @param camera Camera backend instance.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPCameraCloseFn)(void *camera);

/**
 * @brief Start streaming camera frames.
 * @param camera Camera backend instance.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPCameraStartFn)(void *camera);

/**
 * @brief Stop streaming camera frames.
 * @param camera Camera backend instance.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPCameraStopFn)(void *camera);

/**
 * @brief Read the latest camera frame.
 * @param camera Camera backend instance.
 * @param out_frame Receives the camera frame description.
 * @param out_has_frame Receives CMP_TRUE if a frame was read.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPCameraReadFrameFn)(void *camera,
                                            CMPCameraFrame *out_frame,
                                            CMPBool *out_has_frame);

/**
 * @brief Camera virtual table.
 */
typedef struct CMPCameraVTable {
  /** @brief Open a camera device. */
  CMPCameraOpenFn open;
  /** @brief Open a camera device with configuration options. */
  CMPCameraOpenWithConfigFn open_with_config;
  /** @brief Close the camera device. */
  CMPCameraCloseFn close;
  /** @brief Start streaming camera frames. */
  CMPCameraStartFn start;
  /** @brief Stop streaming camera frames. */
  CMPCameraStopFn stop;
  /** @brief Read the latest camera frame. */
  CMPCameraReadFrameFn read_frame;
} CMPCameraVTable;

/**
 * @brief Camera interface.
 */
typedef struct CMPCamera {
  void *ctx;                     /**< Camera backend context pointer. */
  const CMPCameraVTable *vtable; /**< Camera virtual table. */
} CMPCamera;

/** @brief Any image format (backend chooses). */
#define CMP_IMAGE_FORMAT_ANY 0
/** @brief RGBA8 image format. */
#define CMP_IMAGE_FORMAT_RGBA8 1
/** @brief BGRA8 image format. */
#define CMP_IMAGE_FORMAT_BGRA8 2
/** @brief Alpha-only 8-bit image format. */
#define CMP_IMAGE_FORMAT_A8 3

/** @brief Auto-detect image encoding. */
#define CMP_IMAGE_ENCODING_AUTO 0
/** @brief PPM (P6) binary image encoding. */
#define CMP_IMAGE_ENCODING_PPM 1
/** @brief Raw RGBA8 image encoding (width/height required). */
#define CMP_IMAGE_ENCODING_RAW_RGBA8 2

/** @brief Image data originated from backend decoder. */
#define CMP_IMAGE_DATA_FLAG_BACKEND 1u

/**
 * @brief Image decode request.
 */
typedef struct CMPImageDecodeRequest {
  const char *utf8_path; /**< Optional file path for backend decode. */
  const void *data;      /**< Encoded image bytes. */
  cmp_usize size;        /**< Size of encoded data in bytes. */
  cmp_u32 encoding;      /**< Encoding hint (CMP_IMAGE_ENCODING_*). */
  cmp_u32 format;        /**< Desired output format (CMP_IMAGE_FORMAT_*). */
  cmp_u32 width;         /**< Width for raw encodings (pixels). */
  cmp_u32 height;        /**< Height for raw encodings (pixels). */
  cmp_u32 stride;        /**< Row stride for raw encodings (bytes). */
} CMPImageDecodeRequest;

/**
 * @brief Decoded image data.
 */
typedef struct CMPImageData {
  cmp_u32 format;   /**< Pixel format (CMP_IMAGE_FORMAT_*). */
  cmp_u32 width;    /**< Image width in pixels. */
  cmp_u32 height;   /**< Image height in pixels. */
  cmp_u32 stride;   /**< Row stride in bytes (0 means tightly packed). */
  const void *data; /**< Pixel data pointer. */
  cmp_usize size;   /**< Pixel data size in bytes. */
  cmp_u32 flags;    /**< Data flags (CMP_IMAGE_DATA_FLAG_*). */
} CMPImageData;

/**
 * @brief Image decoder virtual table.
 */
typedef struct CMPImageVTable {
/**
 * @brief Decode an image from memory or file.
 * @param image Image backend instance.
 * @param request Decode request parameters.
 * @param allocator Allocator for decoded pixel storage.
 * @param out_image Receives decoded image data.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int decode(void *image, const CMPImageDecodeRequest *request,
             const CMPAllocator *allocator, CMPImageData *out_image);
#else
  int(CMP_CALL *decode)(void *image, const CMPImageDecodeRequest *request,
                        const CMPAllocator *allocator, CMPImageData *out_image);
#endif
/**
 * @brief Release decoded image data.
 * @param image Image backend instance.
 * @param allocator Allocator used for decoded data.
 * @param image_data Image data to free.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int free_image(void *image, const CMPAllocator *allocator,
                 CMPImageData *image_data);
#else
  int(CMP_CALL *free_image)(void *image, const CMPAllocator *allocator,
                            CMPImageData *image_data);
#endif
} CMPImageVTable;

/**
 * @brief Image decoder interface.
 */
typedef struct CMPImage {
  void *ctx;                    /**< Image backend context pointer. */
  const CMPImageVTable *vtable; /**< Image virtual table. */
} CMPImage;

/** @brief Any video format (backend chooses). */
#define CMP_VIDEO_FORMAT_ANY 0
/** @brief RGBA8 video format. */
#define CMP_VIDEO_FORMAT_RGBA8 1
/** @brief BGRA8 video format. */
#define CMP_VIDEO_FORMAT_BGRA8 2
/** @brief Alpha-only 8-bit video format. */
#define CMP_VIDEO_FORMAT_A8 3

/** @brief Auto-detect video encoding. */
#define CMP_VIDEO_ENCODING_AUTO 0
/** @brief LibCMPC raw video container encoding. */
#define CMP_VIDEO_ENCODING_M3V0 1

/**
 * @brief Video open request.
 */
typedef struct CMPVideoOpenRequest {
  const char *utf8_path; /**< Optional file path for backend decode. */
  const void *data;      /**< Encoded video bytes. */
  cmp_usize size;        /**< Size of encoded data in bytes. */
  cmp_u32 encoding;      /**< Encoding hint (CMP_VIDEO_ENCODING_*). */
  cmp_u32 format;        /**< Desired output format (CMP_VIDEO_FORMAT_*). */
} CMPVideoOpenRequest;

/**
 * @brief Video frame description.
 */
typedef struct CMPVideoFrame {
  cmp_u32 format;       /**< Pixel format (CMP_VIDEO_FORMAT_*). */
  cmp_u32 width;        /**< Frame width in pixels. */
  cmp_u32 height;       /**< Frame height in pixels. */
  const void *data;     /**< Frame data pointer. */
  cmp_usize size;       /**< Frame data size in bytes. */
  cmp_u32 timestamp_ms; /**< Frame timestamp in milliseconds. */
} CMPVideoFrame;

/**
 * @brief Video decoder virtual table.
 */
typedef struct CMPVideoVTable {
/**
 * @brief Open a video stream.
 * @param video Video backend instance.
 * @param request Video open request.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int open(void *video, const CMPVideoOpenRequest *request);
#else
  int(CMP_CALL *open)(void *video, const CMPVideoOpenRequest *request);
#endif
/**
 * @brief Close a video stream.
 * @param video Video backend instance.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int close(void *video);
#else
  int(CMP_CALL *close)(void *video);
#endif
/**
 * @brief Read the next video frame.
 * @param video Video backend instance.
 * @param out_frame Receives the frame description.
 * @param out_has_frame Receives CMP_TRUE if a frame was read.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int read_frame(void *video, CMPVideoFrame *out_frame, CMPBool *out_has_frame);
#else
  int(CMP_CALL *read_frame)(void *video, CMPVideoFrame *out_frame,
                            CMPBool *out_has_frame);
#endif
} CMPVideoVTable;

/**
 * @brief Video decoder interface.
 */
typedef struct CMPVideo {
  void *ctx;                    /**< Video backend context pointer. */
  const CMPVideoVTable *vtable; /**< Video virtual table. */
} CMPVideo;

/** @brief Auto-detect audio encoding. */
#define CMP_AUDIO_ENCODING_AUTO 0
/** @brief WAV (RIFF) PCM audio encoding. */
#define CMP_AUDIO_ENCODING_WAV 1

/** @brief Signed 16-bit PCM audio format. */
#define CMP_AUDIO_FORMAT_S16 1
/** @brief 32-bit float PCM audio format. */
#define CMP_AUDIO_FORMAT_F32 2

/** @brief Audio data originated from backend decoder. */
#define CMP_AUDIO_DATA_FLAG_BACKEND 1u

/**
 * @brief Audio decode request.
 */
typedef struct CMPAudioDecodeRequest {
  const char *utf8_path; /**< Optional file path for backend decode. */
  const void *data;      /**< Encoded audio bytes. */
  cmp_usize size;        /**< Size of encoded data in bytes. */
  cmp_u32 encoding;      /**< Encoding hint (CMP_AUDIO_ENCODING_*). */
} CMPAudioDecodeRequest;

/**
 * @brief Decoded audio data.
 */
typedef struct CMPAudioData {
  cmp_u32 format;      /**< Sample format (CMP_AUDIO_FORMAT_*). */
  cmp_u32 channels;    /**< Number of channels. */
  cmp_u32 sample_rate; /**< Sample rate in Hz. */
  cmp_usize frames;    /**< Number of audio frames. */
  const void *data;    /**< PCM data pointer. */
  cmp_usize size;      /**< PCM data size in bytes. */
  cmp_u32 flags;       /**< Data flags (CMP_AUDIO_DATA_FLAG_*). */
} CMPAudioData;

/**
 * @brief Audio decode function signature.
 * @param audio_ctx Audio backend instance.
 * @param request Decode request parameters.
 * @param allocator Allocator for decoded PCM storage.
 * @param out_audio Receives decoded audio data.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPAudioDecodeFn)(void *audio_ctx,
                                        const CMPAudioDecodeRequest *request,
                                        const CMPAllocator *allocator,
                                        CMPAudioData *out_audio);

/**
 * @brief Audio release function signature.
 * @param audio_ctx Audio backend instance.
 * @param allocator Allocator used for decoded data.
 * @param audio_data Audio data to free.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPAudioFreeFn)(void *audio_ctx,
                                      const CMPAllocator *allocator,
                                      CMPAudioData *audio_data);

/**
 * @brief Audio decoder virtual table.
 */
typedef struct CMPAudioVTable {
  /**
   * @brief Decode audio from memory or file.
   */
  CMPAudioDecodeFn decode;
  /**
   * @brief Release decoded audio data.
   */
  CMPAudioFreeFn free_audio;
} CMPAudioVTable;

/**
 * @brief Audio decoder interface.
 */
typedef struct CMPAudio {
  void *ctx;                    /**< Audio backend context pointer. */
  const CMPAudioVTable *vtable; /**< Audio virtual table. */
} CMPAudio;

/**
 * @brief Network request description.
 */
typedef struct CMPNetworkRequest {
  const char *method;  /**< HTTP method name. */
  const char *url;     /**< Request URL. */
  const char *headers; /**< Raw header string, backend-defined format. */
  const void *body;    /**< Request body bytes. */
  cmp_usize body_size; /**< Request body size in bytes. */
  cmp_u32 timeout_ms;  /**< Timeout in milliseconds. */
} CMPNetworkRequest;

/**
 * @brief Network response description.
 */
typedef struct CMPNetworkResponse {
  cmp_u32 status_code; /**< HTTP status code. */
  const void *body;    /**< Response body bytes. */
  cmp_usize body_size; /**< Response body size in bytes. */
} CMPNetworkResponse;

/**
 * @brief Network virtual table.
 */
typedef struct CMPNetworkVTable {
/**
 * @brief Execute a network request.
 * @param net Network backend instance.
 * @param request Request parameters.
 * @param allocator Allocator for response body storage.
 * @param out_response Receives the response data.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int request(void *net, const CMPNetworkRequest *request,
              const CMPAllocator *allocator, CMPNetworkResponse *out_response);
#else
  int(CMP_CALL *request)(void *net, const CMPNetworkRequest *request,
                         const CMPAllocator *allocator,
                         CMPNetworkResponse *out_response);
#endif
/**
 * @brief Release resources in a response.
 * @param net Network backend instance.
 * @param allocator Allocator used for the response body.
 * @param response Response to free.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int free_response(void *net, const CMPAllocator *allocator,
                    CMPNetworkResponse *response);
#else
  int(CMP_CALL *free_response)(void *net, const CMPAllocator *allocator,
                               CMPNetworkResponse *response);
#endif
} CMPNetworkVTable;

/**
 * @brief Network interface.
 */
typedef struct CMPNetwork {
  void *ctx;                      /**< Network backend context pointer. */
  const CMPNetworkVTable *vtable; /**< Network virtual table. */
} CMPNetwork;

/**
 * @brief Task function signature.
 * @param user User data pointer.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPTaskFn)(void *user);

/**
 * @brief Thread entry function signature.
 * @param user User data pointer.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPThreadFn)(void *user);

/**
 * @brief Tasking virtual table.
 */
typedef struct CMPTasksVTable {
/**
 * @brief Create a new thread.
 * @param tasks Task backend instance.
 * @param entry Thread entry function.
 * @param user User data pointer.
 * @param out_thread Receives the thread handle.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int thread_create(void *tasks, CMPThreadFn entry, void *user,
                    CMPHandle *out_thread);
#else
  int(CMP_CALL *thread_create)(void *tasks, CMPThreadFn entry, void *user,
                               CMPHandle *out_thread);
#endif
/**
 * @brief Join a thread.
 * @param tasks Task backend instance.
 * @param thread Thread handle to join.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int thread_join(void *tasks, CMPHandle thread);
#else
  int(CMP_CALL *thread_join)(void *tasks, CMPHandle thread);
#endif
/**
 * @brief Create a mutex.
 * @param tasks Task backend instance.
 * @param out_mutex Receives the mutex handle.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int mutex_create(void *tasks, CMPHandle *out_mutex);
#else
  int(CMP_CALL *mutex_create)(void *tasks, CMPHandle *out_mutex);
#endif
/**
 * @brief Destroy a mutex.
 * @param tasks Task backend instance.
 * @param mutex Mutex handle to destroy.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int mutex_destroy(void *tasks, CMPHandle mutex);
#else
  int(CMP_CALL *mutex_destroy)(void *tasks, CMPHandle mutex);
#endif
/**
 * @brief Lock a mutex.
 * @param tasks Task backend instance.
 * @param mutex Mutex handle to lock.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int mutex_lock(void *tasks, CMPHandle mutex);
#else
  int(CMP_CALL *mutex_lock)(void *tasks, CMPHandle mutex);
#endif
/**
 * @brief Unlock a mutex.
 * @param tasks Task backend instance.
 * @param mutex Mutex handle to unlock.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int mutex_unlock(void *tasks, CMPHandle mutex);
#else
  int(CMP_CALL *mutex_unlock)(void *tasks, CMPHandle mutex);
#endif
/**
 * @brief Sleep for a number of milliseconds.
 * @param tasks Task backend instance.
 * @param ms Milliseconds to sleep.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int sleep_ms(void *tasks, cmp_u32 ms);
#else
  int(CMP_CALL *sleep_ms)(void *tasks, cmp_u32 ms);
#endif
/**
 * @brief Post a task to the default task queue.
 * @param tasks Task backend instance.
 * @param fn Task function to execute.
 * @param user User data pointer.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int task_post(void *tasks, CMPTaskFn fn, void *user);
#else
  int(CMP_CALL *task_post)(void *tasks, CMPTaskFn fn, void *user);
#endif
/**
 * @brief Post a task to run after a delay.
 * @param tasks Task backend instance.
 * @param fn Task function to execute.
 * @param user User data pointer.
 * @param delay_ms Delay in milliseconds.
 * @return CMP_OK on success or a failure code.
 */
#ifdef CMP_DOXYGEN
  int task_post_delayed(void *tasks, CMPTaskFn fn, void *user,
                        cmp_u32 delay_ms);
#else
  int(CMP_CALL *task_post_delayed)(void *tasks, CMPTaskFn fn, void *user,
                                   cmp_u32 delay_ms);
#endif
} CMPTasksVTable;

/**
 * @brief Tasking interface.
 */
typedef struct CMPTasks {
  void *ctx;                    /**< Task backend context pointer. */
  const CMPTasksVTable *vtable; /**< Task virtual table. */
} CMPTasks;

/**
 * @brief Retrieve the IO interface.
 * @param env Environment backend instance.
 * @param out_io Receives the IO interface.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPEnvGetIOFn)(void *env, CMPIO *out_io);

/**
 * @brief Retrieve the sensors interface.
 * @param env Environment backend instance.
 * @param out_sensors Receives the sensors interface.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPEnvGetSensorsFn)(void *env, CMPSensors *out_sensors);

/**
 * @brief Retrieve the camera interface.
 * @param env Environment backend instance.
 * @param out_camera Receives the camera interface.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPEnvGetCameraFn)(void *env, CMPCamera *out_camera);

/**
 * @brief Retrieve the image decoder interface.
 * @param env Environment backend instance.
 * @param out_image Receives the image decoder interface.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPEnvGetImageFn)(void *env, CMPImage *out_image);

/**
 * @brief Retrieve the video decoder interface.
 * @param env Environment backend instance.
 * @param out_video Receives the video decoder interface.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPEnvGetVideoFn)(void *env, CMPVideo *out_video);

/**
 * @brief Retrieve the audio decoder interface.
 * @param env Environment backend instance.
 * @param out_audio Receives the audio decoder interface.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPEnvGetAudioFn)(void *env, CMPAudio *out_audio);

/**
 * @brief Retrieve the network interface.
 * @param env Environment backend instance.
 * @param out_network Receives the network interface.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPEnvGetNetworkFn)(void *env, CMPNetwork *out_network);

/**
 * @brief Retrieve the tasking interface.
 * @param env Environment backend instance.
 * @param out_tasks Receives the tasking interface.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPEnvGetTasksFn)(void *env, CMPTasks *out_tasks);

/**
 * @brief Get the current time in milliseconds.
 * @param env Environment backend instance.
 * @param out_time_ms Receives the time in milliseconds.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPEnvGetTimeMsFn)(void *env, cmp_u32 *out_time_ms);

/**
 * @brief Environment virtual table.
 */
typedef struct CMPEnvVTable {
  /** @brief Retrieve the IO interface. */
  CMPEnvGetIOFn get_io;
  /** @brief Retrieve the sensors interface. */
  CMPEnvGetSensorsFn get_sensors;
  /** @brief Retrieve the camera interface. */
  CMPEnvGetCameraFn get_camera;
  /** @brief Retrieve the image decoder interface. */
  CMPEnvGetImageFn get_image;
  /** @brief Retrieve the video decoder interface. */
  CMPEnvGetVideoFn get_video;
  /** @brief Retrieve the audio decoder interface. */
  CMPEnvGetAudioFn get_audio;
  /** @brief Retrieve the network interface. */
  CMPEnvGetNetworkFn get_network;
  /** @brief Retrieve the tasking interface. */
  CMPEnvGetTasksFn get_tasks;
  /** @brief Get the current time in milliseconds. */
  CMPEnvGetTimeMsFn get_time_ms;
} CMPEnvVTable;

/**
 * @brief Environment interface.
 */
typedef struct CMPEnv {
  void *ctx;                  /**< Environment backend context pointer. */
  const CMPEnvVTable *vtable; /**< Environment virtual table. */
} CMPEnv;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_API_ENV_H */
