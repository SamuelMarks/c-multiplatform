#include "m3/m3_anim.h"
#include "m3/m3_api_env.h"
#include "m3/m3_api_gfx.h"
#include "m3/m3_api_ui.h"
#include "m3/m3_api_ws.h"
#include "m3/m3_arena.h"
#include "m3/m3_backend_ios.h"
#include "m3/m3_backend_null.h"
#include "m3/m3_backend_sdl3.h"
#include "m3/m3_backend_web.h"
#include "m3/m3_camera.h"
#include "m3/m3_color.h"
#include "m3/m3_core.h"
#include "m3/m3_event.h"
#include "m3/m3_layout.h"
#include "m3/m3_log.h"
#include "m3/m3_math.h"
#include "m3/m3_object.h"
#include "m3/m3_predictive.h"
#include "m3/m3_render.h"
#include "m3/m3_router.h"
#include "m3/m3_selection.h"
#include "m3/m3_store.h"
#include "m3/m3_tasks.h"
#include "m3/m3_text.h"
#include "m3/m3_utf8.h"
#include "m3/m3_visuals.h"

int main(void) {
  M3Allocator alloc;
  M3Handle handle;
  M3ObjectHeader obj;
  M3PredictiveBack predictive;
  M3PredictiveBackEvent predictive_event;
  M3Mat3 mat;
  M3Rect rect;
  M3WS ws;
  M3Gfx gfx;
  M3Env env;
  M3Widget widget;
  M3Arena arena;
  M3Utf8Iter utf8_iter;
  M3LogSink sink;
  M3Store store;
  M3Scheme scheme;
  M3LayoutNode layout_node;
  M3AnimController anim_controller;
  M3Router router;
  M3RenderList render_list;
  M3RenderNode render_node;
  M3RenderCmd render_cmd;
  M3EventDispatcher dispatcher;
  M3Tasks tasks;
  M3TasksDefaultConfig tasks_config;
  M3CameraSessionConfig camera_config;
  M3CameraSession camera_session;
  M3Ripple ripple;
  M3Shadow shadow;
  M3TextWidget text_widget;
  M3NullBackendConfig null_config;
  M3NullBackend *null_backend;
  M3SDL3BackendConfig sdl3_config;
  M3SDL3Backend *sdl3_backend;
  M3WebBackendConfig web_config;
  M3WebBackend *web_backend;
  M3IOSBackendConfig ios_config;
  M3IOSBackend *ios_backend;

  M3_UNUSED(&alloc);
  M3_UNUSED(&handle);
  M3_UNUSED(&obj);
  M3_UNUSED(&predictive);
  M3_UNUSED(&predictive_event);
  M3_UNUSED(&mat);
  M3_UNUSED(&rect);
  M3_UNUSED(&ws);
  M3_UNUSED(&gfx);
  M3_UNUSED(&env);
  M3_UNUSED(&widget);
  M3_UNUSED(&arena);
  M3_UNUSED(&utf8_iter);
  M3_UNUSED(&sink);
  M3_UNUSED(&store);
  M3_UNUSED(&scheme);
  M3_UNUSED(&layout_node);
  M3_UNUSED(&anim_controller);
  M3_UNUSED(&router);
  M3_UNUSED(&render_list);
  M3_UNUSED(&render_node);
  M3_UNUSED(&render_cmd);
  M3_UNUSED(&dispatcher);
  M3_UNUSED(&tasks);
  M3_UNUSED(&tasks_config);
  M3_UNUSED(&camera_config);
  M3_UNUSED(&camera_session);
  M3_UNUSED(&ripple);
  M3_UNUSED(&shadow);
  M3_UNUSED(&text_widget);
  M3_UNUSED(&null_config);
  M3_UNUSED(&null_backend);
  M3_UNUSED(&sdl3_config);
  M3_UNUSED(&sdl3_backend);
  M3_UNUSED(&web_config);
  M3_UNUSED(&web_backend);
  M3_UNUSED(&ios_config);
  M3_UNUSED(&ios_backend);

  return 0;
}
