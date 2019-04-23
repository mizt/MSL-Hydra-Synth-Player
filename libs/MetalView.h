#import <MetalKit/MetalKit.h>
#import <vector>
#import <JavascriptCore/JavascriptCore.h>

@interface MetalView:NSView
-(id)initWithFrame:(CGRect)frame :(std::vector<NSString *>)shaders;
-(id)initWithFrame:(CGRect)frame :(std::vector<NSString *>)shaders :(std::vector<NSString *>)uniforms;
-(void)update:(void (^)(id<MTLCommandBuffer>))onComplete;
-(bool)reloadShader:(dispatch_data_t)data :(unsigned int)index;
-(void)mode:(unsigned int)n;
-(id<MTLTexture>)o0;
-(id<MTLTexture>)o1;
-(id<MTLTexture>)o2;
-(id<MTLTexture>)o3;
-(id<MTLTexture>)drawableTexture;
-(void)cleanup;
@end
