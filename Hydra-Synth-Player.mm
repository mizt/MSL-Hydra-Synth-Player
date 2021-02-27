#define FPS 60.0

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <TargetConditionals.h>
#import <vector>
#import "./libs/MetalView.h"
#import "./libs/Plane.h"
#import "./libs/HydraMetalLayer.h"
#import "./libs/MTLReadPixels.h"

#if TARGET_OS_OSX

#define WIDTH 1280
#define HEIGHT 720

#import "./libs/Menu.h"

class App {
    
    private:
        
        NSWindow *win;
        NSView *view;
        HydraMetalLayer<Plane> *layer;
        
        dispatch_source_t timer;
        
        unsigned int *o0 = nullptr;
        unsigned int *o1 = nullptr;
        unsigned int *o2 = nullptr;
        unsigned int *o3 = nullptr;
        
        unsigned int *s0 = nullptr;
        unsigned int *s1 = nullptr;
        unsigned int *s2 = nullptr;
        unsigned int *s3 = nullptr;
        
        CGRect rect = CGRectMake(0,0,WIDTH,HEIGHT);
        
        dispatch_fd_t fd;
        double timestamp = -1;
        NSString *path[2] = {
            @"MSL-Hydra-Synth/assets/s0.metallib",
            @"MSL-Hydra-Synth/assets/u0.json"
            //[[[NSBundle mainBundle] URLForResource:@"o0" withExtension:@"metallib"] path],
            //[[[NSBundle mainBundle] URLForResource:@"u0" withExtension:@"json"] path]
        };
        NSFileManager *fileManager = [NSFileManager defaultManager];
        
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

        MTLReadPixels *ReadPixels;
        
    public:
                
        App() {
            
            int w = rect.size.width;
            int h = rect.size.height;
            
            this->ReadPixels = new MTLReadPixels(w,h);
            
            this->o0 = new unsigned int[w*h]; 
            /*
            this->o1 = new unsigned int[w*h]; 
            this->o2 = new unsigned int[w*h]; 
            this->o3 = new unsigned int[w*h]; 
            */
            
            this->s0 = new unsigned int[w*h]; 
            /*
            this->s1 = new unsigned int[w*h];  
            this->s2 = new unsigned int[w*h];  
            this->s3 = new unsigned int[w*h];  
            */
            
            this->win = [[NSWindow alloc] initWithContentRect:rect styleMask:1|1<<2 backing:NSBackingStoreBuffered defer:NO];
            this->view = [[NSView alloc] initWithFrame:rect];
            [this->view setWantsLayer:YES];
            
            
            this->layer = new HydraMetalLayer<Plane>();
            this->layer->init(rect.size.width,rect.size.height,
                {path[0]},
                {path[1]}
                ,false
            );
            
            Menu::$()->on(^(id me,IMenuItem *item){
                if(item) {
                    MenuType type = item->type();
                    if(type==MenuType::TEXT||type==MenuType::BUTTON) {
                        if(item->isParent()) {
                            
                        }
                        else {
                            if(item->eq(@"Quit")) {
                                [NSApp terminate:nil];
                            }
                            else {
                                NSLog(@"%@",item->name());
                            }
                        }
                    }
                    else if(type==MenuType::SLIDER||type==MenuType::RADIOBUTTON||type==MenuType::CHECKBOX) {
                        NSLog(@"%@,%f",item->name(),item->value());
                    }
                }
            })
            //->addItem(@"slider",MenuType::SLIDER,@"{'min':0.0,'max':1.0,'value':0.5,'label':false}")
            //->hr()
            ->addItem(@"Quit",MenuType::TEXT,@"{'key':''}");
            
            if(this->layer->isInit()) {
            
                this->view.layer = this->layer->layer();
                
                [[this->win contentView] addSubview:this->view];
                
                for(int k=0; k<w*h; k++) {
                    this->o0[k] = 0xFF000000; // ABGR 
                    this->s0[k] = 0xFFFF0000; // ABGR 
                }
                
                this->timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,0,0,dispatch_queue_create("ENTER_FRAME",0));
                dispatch_source_set_timer(this->timer,dispatch_time(0,0),(1.0/FPS)*1000000000,0);
                dispatch_source_set_event_handler(this->timer,^{
                    
                    //NSLog(@"%@",NSStringFromRect([this->win frame]));
                    this->layer->frame([this->win frame]);
                    
                    int width  = this->rect.size.width;
                    int height = this->rect.size.height;
                                                                
                    [this->layer->o0() replaceRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0 withBytes:this->o0 bytesPerRow:width<<2];
                    /*
                    [this->layer->o1() replaceRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0 withBytes:this->o0 bytesPerRow:width<<2];
                    [this->layer->o2() replaceRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0 withBytes:this->o0 bytesPerRow:width<<2];
                    [this->layer->o3() replaceRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0 withBytes:this->o0 bytesPerRow:width<<2];
                    */
                    [this->layer->s0() replaceRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0 withBytes:this->s0 bytesPerRow:width<<2];
                    /*
                    [this->layer->s1() replaceRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0 withBytes:this->s0 bytesPerRow:width<<2];
                    [this->layer->s2() replaceRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0 withBytes:this->s0 bytesPerRow:width<<2];
                    [this->layer->s3() replaceRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0 withBytes:this->s0 bytesPerRow:width<<2];
                    */
                    
                     if([this->fileManager fileExistsAtPath:this->path[0]]) { //}&&[this->fileManager fileExistsAtPath:this->path[1]]) {
                                                
                        double date = [[[this->fileManager attributesOfItemAtPath:this->path[0] error:nil] objectForKey:NSFileModificationDate] timeIntervalSince1970];
                        
                        if(this->timestamp==-1) { // initalize
                             this->timestamp = date;
                        } 
                        else if(this->timestamp!=date) { // &&this->timestamp[1]!=date[1]) {
                                                    
                            this->timestamp = date;
                                                    
                            NSError *error = nil;
                            NSDictionary *attributes[2] = {
                                [this->fileManager attributesOfItemAtPath:this->path[0] error:&error],
                                [this->fileManager attributesOfItemAtPath:this->path[1] error:&error]
                            };
                            
                            if(!error) {
                                long size[2] = {
                                    [[attributes[0] objectForKey:NSFileSize] integerValue],
                                    [[attributes[1] objectForKey:NSFileSize] integerValue]
                                };
                                
                                if(size[0]>0&&size[1]>0) {
                                    this->fd = open([this->path[0] UTF8String],O_RDONLY);
                                    dispatch_read(fd,size[0],dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0),^(dispatch_data_t d,int e) {
                                        
                                        this->layer->reloadShader(0,d,this->path[1]);
                                        close(this->fd);                                    
                                        dispatch_semaphore_signal(this->semaphore);

                                    });
                                    dispatch_semaphore_wait(this->semaphore,DISPATCH_TIME_FOREVER);
                                                                        
                                }
                            }
                        }
                    }
                                        
                    this->layer->update(^(id<MTLCommandBuffer> commandBuffer) {
                        
                        this->ReadPixels->setDrawableTexture(this->layer->drawableTexture());
                        unsigned int *bytes = this->ReadPixels->bytes();
                        
                        for(int i=0; i<height; i++) {
                            for(int j=0; j<width; j++) {
                                unsigned int argb = bytes[i*width+j];
                                this->o0[i*width+j] = (0xFF00FF00&argb)|((argb>>16)&0xFF)|((argb&0xFF)<<16);
                            }
                        }
                        
                        this->layer->cleanup();
                    });
                    
                    static dispatch_once_t oncePredicate;
                    dispatch_once(&oncePredicate,^{
                        dispatch_async(dispatch_get_main_queue(),^{
                            CGRect screen = [[NSScreen mainScreen] frame];
                            CGRect rect = [this->win frame];
                            CGRect center = CGRectMake(
                                (screen.size.width-rect.size.width)*0.5,
                                (screen.size.height-(rect.size.height))*0.5,
                                rect.size.width,rect.size.height
                            );
                            [this->win setFrame:center display:YES];
                            [this->win makeKeyAndOrderFront:nil];
                        });
                    });
                    
                });
                if(this->timer) dispatch_resume(this->timer);      
            }          
        }
        
        ~App() {
            
            
            if(this->timer) {
                dispatch_source_cancel(this->timer);
                this->timer = nullptr;
            }
                
            delete[] this->o0;
            delete[] this->s0;
                
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

#else

class App {
  
    private:
        
        UIWindow *window;
        UIViewController *controller;
        MetalView *view;
    
        HydraMetalLayer<Plane> *layer;
    
        dispatch_source_t timer;
             
        unsigned int *o0 = nullptr;
         
        CGRect rect;
    
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

    
    public:
        
        App() {
                        
            this->rect = CGRectMake(0,0,(int)([[UIScreen mainScreen] bounds].size.width),(int)([[UIScreen mainScreen] bounds].size.height));
            
            NSLog(@"%f,%f",this->rect.size.width,this->rect.size.height);
            
            this->o0 = new unsigned int[(int)this->rect.size.width*(int)this->rect.size.height];
            
            this->window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
            this->controller = [[UIViewController alloc] init];
            
            this->view = [[MetalView alloc] initWithFrame:this->rect];
            this->view.multipleTouchEnabled=NO;
            this->view.exclusiveTouch=YES;
            
            NSBundle *bundle = [NSBundle mainBundle];
            
            this->layer = new HydraMetalLayer<Plane>((CAMetalLayer *)this->view.layer);
            this->layer->init(rect.size.width,rect.size.height,
                {[bundle pathForResource:@"s0" ofType:@"metallib"]},
                {[bundle pathForResource:@"u0" ofType:@"json"]},
                false
            );
                        
            this->controller.view = this->view;
            [this->window setRootViewController:controller];
            [this->window makeKeyAndVisible];
            
            this->timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,0,0,dispatch_queue_create("ENTER_FRAME",0));
            dispatch_source_set_timer(this->timer,dispatch_time(0,0),(1.0/FPS)*1000000000,0);
            dispatch_source_set_event_handler(this->timer,^{
                
                int width  = this->rect.size.width;
                int height = this->rect.size.height;
                                
                [this->layer->o0() replaceRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0 withBytes:this->o0 bytesPerRow:width<<2];
                
                this->layer->update(^(id<MTLCommandBuffer> commandBuffer) {
                    
                    [this->layer->drawableTexture() getBytes:this->o0 bytesPerRow:(width<<2) fromRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0];
                    
                    this->layer->cleanup();
                });
            
            });
            if(this->timer) dispatch_resume(this->timer);

        }
    
        ~App() {
        
            if(this->timer){
                dispatch_source_cancel(this->timer);
                this->timer = nullptr;
            }
                           
            if(this->o0) delete[] this->o0;
            
        }
    
};


@interface AppDelegate:UIResponder<UIApplicationDelegate> {
    App *app;
}
@end

@implementation AppDelegate

-(BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    app = new App();
    return YES;
}

@end

int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc,argv,nil,@"AppDelegate");
    }
}

#endif
