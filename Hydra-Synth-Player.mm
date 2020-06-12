#define WIDTH 1280
#define HEIGHT 720
#define FPS 60.0

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>
#import <vector>
#import "./libs/HydraMetalLayer.h"

class App {
    
    private:
        
        NSWindow *win;
        NSView *view;
        HydraMetalLayer *layer;
        
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
            @"./MSL-Hydra-Synth/assets/o0.metallib",
            @"./MSL-Hydra-Synth/assets/u0.json"
            //[[[NSBundle mainBundle] URLForResource:@"o0" withExtension:@"metallib"] path],
            //[[[NSBundle mainBundle] URLForResource:@"u0" withExtension:@"json"] path]
        };
        NSFileManager *fileManager = [NSFileManager defaultManager];
        
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

        
    public:
                
        App() {
            
            int w = rect.size.width;
            int h = rect.size.height;
            
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

            this->layer = new HydraMetalLayer();
            this->layer->init(rect.size.width,rect.size.height,
                {path[0]},
                {path[1]}
            );
            
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
                        
                        [this->layer->drawableTexture() getBytes:this->o0 bytesPerRow:(width<<2) fromRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0];
                        
                        for(int i=0; i<height; i++) {
                            for(int j=0; j<width; j++) {
                                unsigned int argb = this->o0[i*width+j];
                                this->o0[i*width+j] = (0xFF00FF00&argb)|((argb>>16)&0xFF)|((argb&0xFF)<<16);
                            }
                        }
                        
                        this->layer->cleanup();
                    });
                    
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
        }
        
        ~App() {
            
            
            if(this->timer){
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