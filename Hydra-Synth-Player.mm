#import <Cocoa/Cocoa.h>
#import "./libs/MetalView.h"

class App {
    
    private:
        
        NSWindow *win;
        MetalView *view;
        
        dispatch_source_t timer;
        unsigned int *o0 = nullptr;
        
        CGRect rect = CGRectMake(0,0,1280,720);
        
    public:
        
        App() {
            
            int w = rect.size.width;
            int h = rect.size.height;
            
            this->o0 = new unsigned int[w*h];  
            
            this->win = [[NSWindow alloc] initWithContentRect:rect styleMask:1|1<<2 backing:NSBackingStoreBuffered defer:NO];
            this->view = [[MetalView alloc] 
                initWithFrame:rect 
                :{@"./assets/s0.metallib"}
                :{@"./assets/u0.json"}
            ];
            [this->view mode:0];
            [[this->win contentView] addSubview:this->view];
            
            for(int k=0; k<w*h; k++) this->o0[k] = 0x0;
            
            this->timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,0,0,dispatch_queue_create("ENTER_FRAME",0));
            dispatch_source_set_timer(this->timer,dispatch_time(0,0),(1.0/60)*1000000000,0);
            dispatch_source_set_event_handler(this->timer,^{
                
                int width  = this->rect.size.width;
                int height = this->rect.size.height;
                                        
                [[this->view o0] replaceRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0 withBytes:this->o0 bytesPerRow:width<<2];

                [this->view update:
                ^(id<MTLCommandBuffer> commandBuffer){
                    
                    [[this->view drawableTexture] getBytes:this->o0 bytesPerRow:(width<<2) fromRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0];
                    
                    [this->view cleanup];
                }];
                
                static dispatch_once_t oncePredicate;
                dispatch_once(&oncePredicate,^{
                    dispatch_async(dispatch_get_main_queue(),^{
                        [this->win center];
                        [this->win makeKeyAndOrderFront:nil];
                    });
                });
                
            });
            if(this->timer) dispatch_resume(this->timer);                
        }
        
        ~App() {
            
            if(this->timer){
                dispatch_source_cancel(this->timer);
                this->timer = nullptr;
            }
                
            [this->win setReleasedWhenClosed:NO];
            [this->win close];
            this->win = nil;
        }
};

#pragma mark AppDelegate
@interface AppDelegate:NSObject <NSApplicationDelegate> {
    App *app;
}
@end
@implementation AppDelegate
-(void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    app = new App();
}
-(void)applicationWillTerminate:(NSNotification *)aNotification {
    delete app;
}
@end

int main(int argc, char *argv[]) {
    @autoreleasepool {
        id app = [NSApplication sharedApplication];
        id delegat = [AppDelegate alloc];
        [app setDelegate:delegat];
        [app run];
    }
}