#include <cmpc/cmp_backend_cocoa.h>
#include <app/demo_app.h>
#import <Cocoa/Cocoa.h>

static double get_time(void) {
    return [[NSDate date] timeIntervalSince1970];
}

int main(int argc, char *argv[]) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        CMPCocoaBackendConfig config;
        cmp_cocoa_backend_config_init(&config);
        
        CMPCocoaBackend *backend = NULL;
        if (cmp_cocoa_backend_create(&config, &backend) != CMP_OK) {
            fprintf(stderr, "Failed to create Cocoa backend\n");
            return 1;
        }

        CMPWS ws;
        CMPGfx gfx;
        CMPEnv env;
        
        cmp_cocoa_backend_get_ws(backend, &ws);
        cmp_cocoa_backend_get_gfx(backend, &gfx);
        cmp_cocoa_backend_get_env(backend, &env);

        CMPHandle window;
        CMPWSWindowConfig wcfg = {
            .width = 400,
            .height = 700,
            .utf8_title = "CMPC Demo (Cocoa)",
            .flags = CMP_WS_WINDOW_RESIZABLE
        };
        
        ws.vtable->create_window(ws.ctx, &wcfg, &window);
        ws.vtable->show_window(ws.ctx, window);
        [NSApp activateIgnoringOtherApps:YES];

        CMPAllocator alloc;
        cmp_get_default_allocator(&alloc);
        
        DemoApp *app = NULL;
        demo_app_create(&alloc, &app);
        // Important: Init resources here to load fonts/images
        demo_app_init_resources(app, &gfx, &env);

        double last = get_time();
        int running = 1;

        while (running) {
            NSEvent *ev;
            while ((ev = [NSApp nextEventMatchingMask:NSEventMaskAny 
                                            untilDate:[NSDate distantPast] 
                                               inMode:NSDefaultRunLoopMode 
                                              dequeue:YES])) {
                [NSApp sendEvent:ev];
            }
            [NSApp updateWindows];

            CMPInputEvent evt; 
            CMPBool has;
            while(1) {
                ws.vtable->poll_event(ws.ctx, &evt, &has);
                if(!has) break;
                
                if(evt.type == CMP_INPUT_WINDOW_CLOSE) running = 0;
                
                CMPBool h;
                demo_app_handle_event(app, &evt, &h);
            }

            double now = get_time();
            demo_app_update(app, now - last);
            last = now;

            int w = 0, h = 0;
            CMPScalar s = 1.0f;
            ws.vtable->get_window_size(ws.ctx, window, &w, &h);
            ws.vtable->get_window_dpi_scale(ws.ctx, window, &s);
            
            demo_app_render(app, &gfx, window, w, h, (float)s);

            // Cap frame rate roughly
            [NSThread sleepForTimeInterval:0.016];
        }
        
        demo_app_destroy(app);
        ws.vtable->destroy_window(ws.ctx, window);
        cmp_cocoa_backend_destroy(backend);
    }
    return 0;
}