#include <cmpc/cmp_backend_ios.h>
#include "demo_app.h"
#import <UIKit/UIKit.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@property (strong) CADisplayLink *link;
@end

@implementation AppDelegate {
    CMPIOSBackend *_backend;
    CMPWS *_ws;
    CMPGfx *_gfx;
    CMPEnv *_env;
    CMPHandle _wh;
    DemoApp *_app;
}

- (BOOL)application:(UIApplication *)app didFinishLaunchingWithOptions:(NSDictionary *)opt {
    CMPIOSBackendConfig cfg;
    cmp_ios_backend_config_init(&cfg);
    cmp_ios_backend_create(&cfg, &_backend);
    
    cmp_ios_backend_get_ws(_backend, &_ws);
    cmp_ios_backend_get_gfx(_backend, &_gfx);
    cmp_ios_backend_get_env(_backend, &_env);

    CMPWSWindowConfig wcfg = {0};
    _ws->vtable->create_window(_ws->ctx, &wcfg, &_wh);
    _ws->vtable->show_window(_ws->ctx, _wh);

    CMPAllocator alloc;
    cmp_get_default_allocator(&alloc);
    demo_app_create(&alloc, &_app);
    demo_app_init_resources(_app, _gfx, _env);

    self.link = [CADisplayLink displayLinkWithTarget:self selector:@selector(tick:)];
    [self.link addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    return YES;
}

- (void)tick:(CADisplayLink *)link {
    CMPInputEvent evt; 
    CMPBool has;
    // Process input
    while (1) {
        _ws->vtable->poll_event(_ws->ctx, &evt, &has);
        if (!has) break;
        CMPBool h;
        demo_app_handle_event(_app, &evt, &h);
    }
    
    demo_app_update(_app, link.duration);
    
    int w, h; 
    CMPScalar s;
    _ws->vtable->get_window_size(_ws->ctx, _wh, &w, &h);
    _ws->vtable->get_window_dpi_scale(_ws->ctx, _wh, &s);
    demo_app_render(_app, _gfx, _wh, w, h, (float)s);
}

@end

int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}