#ifndef M3_API_ENV_H
#define M3_API_ENV_H

/**
 * @file m3_api_env.h
 * @brief Environment abstraction ABI for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_math.h"
#include "m3_object.h"

/**
 * @brief File information metadata.
 */
typedef struct M3FileInfo {
  m3_u32 size_bytes; /**< File size in bytes. */
  m3_u32 flags;      /**< Backend-specific file flags. */
} M3FileInfo;

/**
 * @brief IO virtual table.
 */
typedef struct M3IOVTable {
  /**
   * @brief Read a file into a caller-provided buffer.
   * @param io IO backend instance.
   * @param utf8_path File path in UTF-8.
   * @param buffer Destination buffer.
   * @param buffer_size Size of the destination buffer.
   * @param out_read Receives number of bytes read.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *read_file)(void *io, const char *utf8_path, void *buffer,
                          m3_usize buffer_size, m3_usize *out_read);
  /**
   * @brief Read a file by allocating a new buffer.
   * @param io IO backend instance.
   * @param utf8_path File path in UTF-8.
   * @param allocator Allocator used for the buffer.
   * @param out_data Receives the allocated buffer.
   * @param out_size Receives the buffer size in bytes.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *read_file_alloc)(void *io, const char *utf8_path,
                                const M3Allocator *allocator, void **out_data,
                                m3_usize *out_size);
  /**
   * @brief Write a buffer to a file.
   * @param io IO backend instance.
   * @param utf8_path File path in UTF-8.
   * @param data Source data buffer.
   * @param size Size of the data buffer.
   * @param overwrite M3_TRUE to overwrite existing files.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *write_file)(void *io, const char *utf8_path, const void *data,
                           m3_usize size, M3Bool overwrite);
  /**
   * @brief Test whether a file exists.
   * @param io IO backend instance.
   * @param utf8_path File path in UTF-8.
   * @param out_exists Receives M3_TRUE if the file exists.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *file_exists)(void *io, const char *utf8_path,
                            M3Bool *out_exists);
  /**
   * @brief Delete a file.
   * @param io IO backend instance.
   * @param utf8_path File path in UTF-8.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *delete_file)(void *io, const char *utf8_path);
  /**
   * @brief Retrieve file metadata.
   * @param io IO backend instance.
   * @param utf8_path File path in UTF-8.
   * @param out_info Receives file information.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *stat_file)(void *io, const char *utf8_path,
                          M3FileInfo *out_info);
} M3IOVTable;

/**
 * @brief IO interface.
 */
typedef struct M3IO {
  void *ctx;                /**< IO backend context pointer. */
  const M3IOVTable *vtable; /**< IO virtual table. */
} M3IO;

/** @brief Accelerometer sensor type. */
#define M3_SENSOR_ACCELEROMETER 1
/** @brief Gyroscope sensor type. */
#define M3_SENSOR_GYROSCOPE 2
/** @brief Magnetometer sensor type. */
#define M3_SENSOR_MAGNETOMETER 3
/** @brief GPS sensor type. */
#define M3_SENSOR_GPS 4

/**
 * @brief Sensor reading.
 */
typedef struct M3SensorReading {
  m3_u32 type;        /**< Sensor type (M3_SENSOR_*). */
  m3_u32 time_ms;     /**< Timestamp in milliseconds. */
  M3Scalar values[4]; /**< Sensor values. */
  m3_u32 value_count; /**< Number of valid entries in values. */
} M3SensorReading;

/**
 * @brief Sensors virtual table.
 */
typedef struct M3SensorsVTable {
  /**
   * @brief Test whether a sensor type is available.
   * @param sensors Sensors backend instance.
   * @param type Sensor type (M3_SENSOR_*).
   * @param out_available Receives M3_TRUE if available.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *is_available)(void *sensors, m3_u32 type, M3Bool *out_available);
  /**
   * @brief Start producing readings for a sensor.
   * @param sensors Sensors backend instance.
   * @param type Sensor type (M3_SENSOR_*).
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *start)(void *sensors, m3_u32 type);
  /**
   * @brief Stop producing readings for a sensor.
   * @param sensors Sensors backend instance.
   * @param type Sensor type (M3_SENSOR_*).
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *stop)(void *sensors, m3_u32 type);
  /**
   * @brief Read the latest sensor data.
   * @param sensors Sensors backend instance.
   * @param type Sensor type (M3_SENSOR_*).
   * @param out_reading Receives the sensor reading.
   * @param out_has_reading Receives M3_TRUE if data was read.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *read)(void *sensors, m3_u32 type, M3SensorReading *out_reading,
                     M3Bool *out_has_reading);
} M3SensorsVTable;

/**
 * @brief Sensors interface.
 */
typedef struct M3Sensors {
  void *ctx;                     /**< Sensors backend context pointer. */
  const M3SensorsVTable *vtable; /**< Sensors virtual table. */
} M3Sensors;

/** @brief Any camera frame format (backend chooses). */
#define M3_CAMERA_FORMAT_ANY 0
/** @brief RGBA8 camera frame format. */
#define M3_CAMERA_FORMAT_RGBA8 1
/** @brief BGRA8 camera frame format. */
#define M3_CAMERA_FORMAT_BGRA8 2
/** @brief NV12 camera frame format. */
#define M3_CAMERA_FORMAT_NV12 3

/** @brief Camera facing preference unspecified. */
#define M3_CAMERA_FACING_UNSPECIFIED 0
/** @brief Front-facing camera preference. */
#define M3_CAMERA_FACING_FRONT 1
/** @brief Back-facing camera preference. */
#define M3_CAMERA_FACING_BACK 2
/** @brief External-facing camera preference. */
#define M3_CAMERA_FACING_EXTERNAL 3

/** @brief Default camera identifier. */
#define M3_CAMERA_ID_DEFAULT ((m3_u32) ~(m3_u32)0)

/**
 * @brief Camera frame description.
 */
typedef struct M3CameraFrame {
  m3_u32 format;    /**< Pixel format (M3_CAMERA_FORMAT_*). */
  m3_u32 width;     /**< Frame width in pixels. */
  m3_u32 height;    /**< Frame height in pixels. */
  const void *data; /**< Pointer to frame data. */
  m3_usize size;    /**< Size of frame data in bytes. */
} M3CameraFrame;

/**
 * @brief Camera configuration.
 */
typedef struct M3CameraConfig {
  m3_u32 camera_id; /**< Backend-specific camera identifier
                       (M3_CAMERA_ID_DEFAULT for automatic selection). */
  m3_u32 facing;    /**< Requested camera facing (M3_CAMERA_FACING_*). */
  m3_u32 width; /**< Requested frame width in pixels (0 for backend default). */
  m3_u32
      height; /**< Requested frame height in pixels (0 for backend default). */
  m3_u32 format; /**< Requested pixel format (M3_CAMERA_FORMAT_*). */
} M3CameraConfig;

/**
 * @brief Camera virtual table.
 */
typedef struct M3CameraVTable {
  /**
   * @brief Open a camera device.
   * @param camera Camera backend instance.
   * @param camera_id Backend-specific camera identifier.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *open)(void *camera, m3_u32 camera_id);
  /**
   * @brief Open a camera device with configuration options.
   * @param camera Camera backend instance.
   * @param config Camera configuration.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *open_with_config)(void *camera, const M3CameraConfig *config);
  /**
   * @brief Close the camera device.
   * @param camera Camera backend instance.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *close)(void *camera);
  /**
   * @brief Start streaming camera frames.
   * @param camera Camera backend instance.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *start)(void *camera);
  /**
   * @brief Stop streaming camera frames.
   * @param camera Camera backend instance.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *stop)(void *camera);
  /**
   * @brief Read the latest camera frame.
   * @param camera Camera backend instance.
   * @param out_frame Receives the camera frame description.
   * @param out_has_frame Receives M3_TRUE if a frame was read.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *read_frame)(void *camera, M3CameraFrame *out_frame,
                           M3Bool *out_has_frame);
} M3CameraVTable;

/**
 * @brief Camera interface.
 */
typedef struct M3Camera {
  void *ctx;                    /**< Camera backend context pointer. */
  const M3CameraVTable *vtable; /**< Camera virtual table. */
} M3Camera;

/**
 * @brief Network request description.
 */
typedef struct M3NetworkRequest {
  const char *method;  /**< HTTP method name. */
  const char *url;     /**< Request URL. */
  const char *headers; /**< Raw header string, backend-defined format. */
  const void *body;    /**< Request body bytes. */
  m3_usize body_size;  /**< Request body size in bytes. */
  m3_u32 timeout_ms;   /**< Timeout in milliseconds. */
} M3NetworkRequest;

/**
 * @brief Network response description.
 */
typedef struct M3NetworkResponse {
  m3_u32 status_code; /**< HTTP status code. */
  const void *body;   /**< Response body bytes. */
  m3_usize body_size; /**< Response body size in bytes. */
} M3NetworkResponse;

/**
 * @brief Network virtual table.
 */
typedef struct M3NetworkVTable {
  /**
   * @brief Execute a network request.
   * @param net Network backend instance.
   * @param request Request parameters.
   * @param allocator Allocator for response body storage.
   * @param out_response Receives the response data.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *request)(void *net, const M3NetworkRequest *request,
                        const M3Allocator *allocator,
                        M3NetworkResponse *out_response);
  /**
   * @brief Release resources in a response.
   * @param net Network backend instance.
   * @param allocator Allocator used for the response body.
   * @param response Response to free.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *free_response)(void *net, const M3Allocator *allocator,
                              M3NetworkResponse *response);
} M3NetworkVTable;

/**
 * @brief Network interface.
 */
typedef struct M3Network {
  void *ctx;                     /**< Network backend context pointer. */
  const M3NetworkVTable *vtable; /**< Network virtual table. */
} M3Network;

/**
 * @brief Task function signature.
 * @param user User data pointer.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3TaskFn)(void *user);

/**
 * @brief Thread entry function signature.
 * @param user User data pointer.
 * @return M3_OK on success or a failure code.
 */
typedef int(M3_CALL *M3ThreadFn)(void *user);

/**
 * @brief Tasking virtual table.
 */
typedef struct M3TasksVTable {
  /**
   * @brief Create a new thread.
   * @param tasks Task backend instance.
   * @param entry Thread entry function.
   * @param user User data pointer.
   * @param out_thread Receives the thread handle.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *thread_create)(void *tasks, M3ThreadFn entry, void *user,
                              M3Handle *out_thread);
  /**
   * @brief Join a thread.
   * @param tasks Task backend instance.
   * @param thread Thread handle to join.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *thread_join)(void *tasks, M3Handle thread);
  /**
   * @brief Create a mutex.
   * @param tasks Task backend instance.
   * @param out_mutex Receives the mutex handle.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *mutex_create)(void *tasks, M3Handle *out_mutex);
  /**
   * @brief Destroy a mutex.
   * @param tasks Task backend instance.
   * @param mutex Mutex handle to destroy.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *mutex_destroy)(void *tasks, M3Handle mutex);
  /**
   * @brief Lock a mutex.
   * @param tasks Task backend instance.
   * @param mutex Mutex handle to lock.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *mutex_lock)(void *tasks, M3Handle mutex);
  /**
   * @brief Unlock a mutex.
   * @param tasks Task backend instance.
   * @param mutex Mutex handle to unlock.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *mutex_unlock)(void *tasks, M3Handle mutex);
  /**
   * @brief Sleep for a number of milliseconds.
   * @param tasks Task backend instance.
   * @param ms Milliseconds to sleep.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *sleep_ms)(void *tasks, m3_u32 ms);
  /**
   * @brief Post a task to the default task queue.
   * @param tasks Task backend instance.
   * @param fn Task function to execute.
   * @param user User data pointer.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *task_post)(void *tasks, M3TaskFn fn, void *user);
  /**
   * @brief Post a task to run after a delay.
   * @param tasks Task backend instance.
   * @param fn Task function to execute.
   * @param user User data pointer.
   * @param delay_ms Delay in milliseconds.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *task_post_delayed)(void *tasks, M3TaskFn fn, void *user,
                                  m3_u32 delay_ms);
} M3TasksVTable;

/**
 * @brief Tasking interface.
 */
typedef struct M3Tasks {
  void *ctx;                   /**< Task backend context pointer. */
  const M3TasksVTable *vtable; /**< Task virtual table. */
} M3Tasks;

/**
 * @brief Environment virtual table.
 */
typedef struct M3EnvVTable {
  /**
   * @brief Retrieve the IO interface.
   * @param env Environment backend instance.
   * @param out_io Receives the IO interface.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *get_io)(void *env, M3IO *out_io);
  /**
   * @brief Retrieve the sensors interface.
   * @param env Environment backend instance.
   * @param out_sensors Receives the sensors interface.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *get_sensors)(void *env, M3Sensors *out_sensors);
  /**
   * @brief Retrieve the camera interface.
   * @param env Environment backend instance.
   * @param out_camera Receives the camera interface.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *get_camera)(void *env, M3Camera *out_camera);
  /**
   * @brief Retrieve the network interface.
   * @param env Environment backend instance.
   * @param out_network Receives the network interface.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *get_network)(void *env, M3Network *out_network);
  /**
   * @brief Retrieve the tasking interface.
   * @param env Environment backend instance.
   * @param out_tasks Receives the tasking interface.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *get_tasks)(void *env, M3Tasks *out_tasks);
  /**
   * @brief Get the current time in milliseconds.
   * @param env Environment backend instance.
   * @param out_time_ms Receives the time in milliseconds.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *get_time_ms)(void *env, m3_u32 *out_time_ms);
} M3EnvVTable;

/**
 * @brief Environment interface.
 */
typedef struct M3Env {
  void *ctx;                 /**< Environment backend context pointer. */
  const M3EnvVTable *vtable; /**< Environment virtual table. */
} M3Env;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_API_ENV_H */
