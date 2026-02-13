#include "cmpc/cmp_anim.h"
#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_api_ws.h"
#include "cmpc/cmp_arena.h"
#include "cmpc/cmp_backend_ios.h"
#include "cmpc/cmp_backend_null.h"
#include "cmpc/cmp_backend_sdl3.h"
#include "cmpc/cmp_backend_web.h"
#include "cmpc/cmp_camera.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_datetime.h"
#include "cmpc/cmp_event.h"
#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_log.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_object.h"
#include "cmpc/cmp_predictive.h"
#include "cmpc/cmp_render.h"
#include "cmpc/cmp_router.h"
#include "cmpc/cmp_store.h"
#include "cmpc/cmp_tasks.h"
#include "cmpc/cmp_text.h"
#include "cmpc/cmp_utf8.h"
#include "cmpc/cmp_visuals.h"
#include "m3/m3_color.h"
#include "m3/m3_selection.h"

int main(void) {
  CMPAllocator alloc;
  CMPHandle handle;
  CMPObjectHeader obj;
  CMPPredictiveBack predictive;
  CMPPredictiveBackEvent predictive_event;
  CMPMat3 mat;
  CMPRect rect;
  CMPWS ws;
  CMPGfx gfx;
  CMPEnv env;
  CMPWidget widget;
  CMPArena arena;
  CMPUtf8Iter utf8_iter;
  CMPLogSink sink;
  CMPStore store;
  M3Scheme scheme;
  CMPLayoutNode layout_node;
  CMPAnimController anim_controller;
  CMPRouter router;
  CMPRenderList render_list;
  CMPRenderNode render_node;
  CMPRenderCmd render_cmd;
  CMPEventDispatcher dispatcher;
  CMPTasks tasks;
  CMPTasksDefaultConfig tasks_config;
  CMPCameraSessionConfig camera_config;
  CMPCameraSession camera_session;
  CMPRipple ripple;
  CMPShadow shadow;
  CMPTextWidget text_widget;
  CMPNullBackendConfig null_config;
  CMPNullBackend *null_backend;
  CMPSDL3BackendConfig sdl3_config;
  CMPSDL3Backend *sdl3_backend;
  CMPWebBackendConfig web_config;
  CMPWebBackend *web_backend;
  CMPIOSBackendConfig ios_config;
  CMPIOSBackend *ios_backend;

  CMP_UNUSED(&alloc);
  CMP_UNUSED(&handle);
  CMP_UNUSED(&obj);
  CMP_UNUSED(&predictive);
  CMP_UNUSED(&predictive_event);
  CMP_UNUSED(&mat);
  CMP_UNUSED(&rect);
  CMP_UNUSED(&ws);
  CMP_UNUSED(&gfx);
  CMP_UNUSED(&env);
  CMP_UNUSED(&widget);
  CMP_UNUSED(&arena);
  CMP_UNUSED(&utf8_iter);
  CMP_UNUSED(&sink);
  CMP_UNUSED(&store);
  CMP_UNUSED(&scheme);
  CMP_UNUSED(&layout_node);
  CMP_UNUSED(&anim_controller);
  CMP_UNUSED(&router);
  CMP_UNUSED(&render_list);
  CMP_UNUSED(&render_node);
  CMP_UNUSED(&render_cmd);
  CMP_UNUSED(&dispatcher);
  CMP_UNUSED(&tasks);
  CMP_UNUSED(&tasks_config);
  CMP_UNUSED(&camera_config);
  CMP_UNUSED(&camera_session);
  CMP_UNUSED(&ripple);
  CMP_UNUSED(&shadow);
  CMP_UNUSED(&text_widget);
  CMP_UNUSED(&null_config);
  CMP_UNUSED(&null_backend);
  CMP_UNUSED(&sdl3_config);
  CMP_UNUSED(&sdl3_backend);
  CMP_UNUSED(&web_config);
  CMP_UNUSED(&web_backend);
  CMP_UNUSED(&ios_config);
  CMP_UNUSED(&ios_backend);

  return 0;
}
