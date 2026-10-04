/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "android_native_app_glue.h"

/* Minimal stub implementation for MSVC Wine CI tests to link against if needed.
   In a real NDK build, the system's android_native_app_glue.c is compiled
   instead. */

#ifdef UI_TEST_MOCK_ANDROID
int8_t android_app_read_cmd(struct android_app *android_app) { return 0; }
void android_app_pre_exec_cmd(struct android_app *android_app, int8_t cmd) {}
void android_app_post_exec_cmd(struct android_app *android_app, int8_t cmd) {}
#endif
