#import "MetalLayer.h"
#import <JavascriptCore/JavascriptCore.h>
#import <objc/runtime.h>
#import <objc/message.h>
#import <utility>
#import <string>
#import <vector>

#define OFFSET_UNIFORM 11

namespace Plane {
	static const float textureCoordinateData[6][2] = {
		{ 0.f, 0.f },
		{ 1.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 1.f }
	};
}

@interface NSSlider (HydraSynth) 
@property (nonatomic) int mode;
@property (nonatomic) int index;
@end

@implementation NSSlider (HydraSynth)

-(int)mode { return [objc_getAssociatedObject(self, (const void *)@"mode") intValue]; }
-(void)setMode:(int)mode { objc_setAssociatedObject(self,(const void *)@"mode", [NSNumber numberWithInt:mode],OBJC_ASSOCIATION_RETAIN_NONATOMIC); }

-(int)index { return [objc_getAssociatedObject(self, (const void *)@"index") intValue]; }
-(void)setIndex:(int)index { objc_setAssociatedObject(self,(const void *)@"index", [NSNumber numberWithInt:index],OBJC_ASSOCIATION_RETAIN_NONATOMIC); }

@end

#pragma mark TextField
@interface TextField:NSTextField {
}
@end
@implementation TextField
-(bool)isDark {
	id style = [[[NSUserDefaults standardUserDefaults] persistentDomainForName:NSGlobalDomain] objectForKey:@"AppleInterfaceStyle"];
	return (style&&[style isKindOfClass:[NSString class]]&&[style caseInsensitiveCompare:@"dark"]==NSOrderedSame)?true:false;}
-(id)initWithFrame:(NSRect)frame {
	self = [super initWithFrame:frame];
	if(self) {
		[self setEditable:NO];
		[self setSelectable:NO];
		[self setBordered:NO];
		[self setDrawsBackground:NO];
		self.textColor = [self isDark]?[NSColor whiteColor]:[NSColor blackColor];
		[self setFont:[NSFont systemFontOfSize:14]];
	}
	return self;
}
-(void)text:(NSString *)text {
	[self setStringValue:text];
	[self sizeToFit];
}
@end

#pragma mark SliderMenuItem
@interface SliderMenuItem:NSMenuItem {
	NSView *view;
	NSSlider *slider;
	TextField *textField;
	BOOL isLabel;
}@end
@implementation SliderMenuItem
-(bool)isDark {
	id style = [[[NSUserDefaults standardUserDefaults] persistentDomainForName:NSGlobalDomain] objectForKey:@"AppleInterfaceStyle"];
	return (style&&[style isKindOfClass:[NSString class]]&&[style caseInsensitiveCompare:@"dark"]==NSOrderedSame)?true:false;
}
-(double)doubleValue { return [slider doubleValue]; }
-(void)doubleValue:(double)value {
	if([slider minValue]<=value&&value<=[slider maxValue]) {
		[slider setFloatValue:value];
	}
}
-(void)text:(NSString *)name {
	[textField text:name];
	[view setFrame:NSMakeRect(0,0,19+[slider bounds].size.width+5+[textField bounds].size.width+19,16+8+8)];
}
-(void)on {
	[slider setEnabled:YES];
	[textField setTextColor:[self isDark]?[NSColor whiteColor]:[NSColor blackColor]];
}
-(void)off {
	[slider setEnabled:NO];
	[textField setTextColor:[self isDark]?[NSColor lightGrayColor]:[NSColor darkGrayColor]];
}
-(id)init:(NSString *)key :(double)min :(double)max :(double)value :(bool)label :(int)mode :(int)index {
	self = [super init];
	if(self) {
		
		view = [[NSView alloc] init];
		
		slider = [[NSSlider alloc] initWithFrame:CGRectMake(19,8,160,16)];
		[slider setMode:mode];
		[slider setIndex:index];
		[slider setMinValue:min];
		[slider setMaxValue:max];
		[slider setFloatValue:value];
		[view addSubview:slider];
		[slider setTarget:slider];
		[slider setIdentifier:key];
		[slider setAction:NSSelectorFromString(@"onUpdate")];
		
		isLabel = label;
		
		if(isLabel) {
			
			NSString *label = [key componentsSeparatedByString:@"_"][0];
			
			textField = [[TextField alloc] initWithFrame:NSMakeRect(19+[slider bounds].size.width+5,8,128,19)];
			[textField text:label];
			[view setFrame:NSMakeRect(0,0,19+[slider bounds].size.width+5+[textField bounds].size.width+19,16+8+8)];
			[view addSubview:textField];
			[self setView:view];
		}
		else {
			[view setFrame:NSMakeRect(0,0,19+[slider bounds].size.width+19,16+8+8)];
			[view addSubview:textField];
			[self setView:view];
		}
	}
	return self;
}
@end


class Slider {
	
	protected:
		
		NSMenuItem  *menuItem;
		NSString  *menuName;
		
	public:
				
		Slider(NSString *name, double min, double max, double value, bool label, int mode, int index) {
			menuName = name;
			menuItem = [[SliderMenuItem alloc] init:name :min :max :value :label :mode :index];
		}
		~Slider() {
			//NSLog(@"~Slider");
			menuItem = nil;
		}
		
		NSMenuItem *item() { return menuItem; }
		NSString *name() { return menuName; }
		bool eq(NSString *name) { return ([menuName compare:name]==NSOrderedSame)?true:false; }
		
		double value() { return [(SliderMenuItem *)(menuItem) doubleValue]; }
		void on() { [(SliderMenuItem *)(menuItem) on]; }
		void off() { [(SliderMenuItem *)(menuItem) off]; }
};



enum HydraUniformType {
	DoubleType = 0,
	FunctionType,
	SliderType
};


class HydraMetalLayer : public MetalLayer {
	
	private:
		
		NSStatusItem *statusItem;
		NSMenu *menu;
		std::vector<std::pair<NSString*,Slider *>> items;
		
		JSContext *context; 
		
		id<MTLBuffer> _texcoordBuffer;
		
		id<MTLBuffer> _timeBuffer;
		id<MTLBuffer> _resolutionBuffer;
		id<MTLBuffer> _mouseBuffer;
		
		id<MTLTexture> _o0;
		id<MTLTexture> _o1;
		id<MTLTexture> _o2;
		id<MTLTexture> _o3;
		
		id<MTLTexture> _s0;
		id<MTLTexture> _s1;
		id<MTLTexture> _s2;
		id<MTLTexture> _s3;
		
		std::vector<id> _params;
		std::vector<NSString *> _uniformsPath;
		std::vector<NSMutableArray *> _uniformsData;
		
		std::vector<std::vector<HydraUniformType>> _uniformsType;
		
		std::vector<id<MTLBuffer>> _argumentEncoderBuffer;


		double _starttime;
		
	public:
		
		id<MTLTexture> o0() { return this->_o0; }
		id<MTLTexture> o1() { return this->_o1;  }
		id<MTLTexture> o2() { return this->_o2;  }
		id<MTLTexture> o3() { return this->_o3;  }
		
		id<MTLTexture> s0() { return this->_s0;  }
		id<MTLTexture> s1() { return this->_s1;  }
		id<MTLTexture> s2() { return this->_s2;  }
		id<MTLTexture> s3() { return this->_s3;  }
		
		void set(int index) {
			
			this->_argumentEncoderBuffer.push_back([this->_device newBufferWithLength:sizeof(float)*[this->_argumentEncoder[index] encodedLength] options:MTLResourceOptionCPUCacheModeDefault]);

			[this->_argumentEncoder[index] setArgumentBuffer:this->_argumentEncoderBuffer[index] offset:0];
			[this->_argumentEncoder[index] setBuffer:this->_timeBuffer offset:0 atIndex:0];
			[this->_argumentEncoder[index] setBuffer:this->_resolutionBuffer offset:0 atIndex:1];
			[this->_argumentEncoder[index] setBuffer:this->_mouseBuffer offset:0 atIndex:2];
			[this->_argumentEncoder[index] setTexture:this->_o0 atIndex:3];
			[this->_argumentEncoder[index] setTexture:this->_o1 atIndex:4];
			[this->_argumentEncoder[index] setTexture:this->_o2 atIndex:5];
			[this->_argumentEncoder[index] setTexture:this->_o3 atIndex:6];
			[this->_argumentEncoder[index] setTexture:this->_s0 atIndex:7];
			[this->_argumentEncoder[index] setTexture:this->_s1 atIndex:8];
			[this->_argumentEncoder[index] setTexture:this->_s2 atIndex:9];	
			[this->_argumentEncoder[index] setTexture:this->_s3 atIndex:10];
				
			NSString *path = this->_uniformsPath.size()?this->_uniformsPath[index]:@"./default.json";
			NSString *json= [[NSString alloc] initWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
			NSData *jsonData = [json dataUsingEncoding:NSUnicodeStringEncoding];
			NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingAllowFragments error:nil];
			NSMutableArray *list = [NSMutableArray array];
						
			for(id key in [dict keyEnumerator]) {
				[list addObject:key];
			}
			list = (NSMutableArray *)[(NSArray *)list sortedArrayUsingComparator:^NSComparisonResult(NSString *s1,NSString *s2) {
				int n1 = [[s1 componentsSeparatedByString:@"_"][1] intValue];
				int n2 = [[s2 componentsSeparatedByString:@"_"][1] intValue];
				if(n1<n2) return (NSComparisonResult)NSOrderedAscending;
				else if(n1>n2) return (NSComparisonResult)NSOrderedDescending;
				else return (NSComparisonResult)NSOrderedSame;
			}];
			
			// remove Menu
			
			for(int k=0; k<this->items.size(); k++) {
				delete this->items[k].second;
				this->items[k].second = nullptr;
			}
			
			this->items.clear();	
			[this->menu removeAllItems];
			
			
			for(int k=0; k<[list count]; k++) {
				
				
				if([this->_uniformsData[index] count]<=k) { // add
					[this->_uniformsData[index] addObject:dict[list[k]]];
				}
				else { // overwrite
					this->_uniformsData[index][k] = dict[list[k]];
				}
				
				if(this->_params.size()<=k) {
					this->_params.push_back((id)[_device newBufferWithLength:sizeof(float) options:MTLResourceOptionCPUCacheModeDefault]);
					[this->_argumentEncoder[index] setBuffer:(id<MTLBuffer>)this->_params[k] offset:0 atIndex:OFFSET_UNIFORM+k];
				}
				else {
					this->_params[k] = (id)[_device newBufferWithLength:sizeof(float) options:MTLResourceOptionCPUCacheModeDefault];
					[this->_argumentEncoder[index] setBuffer:(id<MTLBuffer>)this->_params[k] offset:0 atIndex:OFFSET_UNIFORM+k];
				}
			}
						
			for(int k=0; k<[list count]; k++) {

				__block HydraUniformType type = HydraUniformType::DoubleType;
				
				if([[_uniformsData[index][k] className] compare:@"__NSCFString"]==NSOrderedSame) {
					
					NSString *js = this->_uniformsData[index][k];
					
					NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:@"slider\\(.+,.+,.+\\)" options:NSRegularExpressionCaseInsensitive error:nil];
							
					__block bool isMatch = false;
							
					[regex enumerateMatchesInString:js options:0 range:NSMakeRange(0,[js length]) usingBlock:^(NSTextCheckingResult *match, NSMatchingFlags flags, BOOL *stop){
											
						NSArray *arr = [[[[js stringByReplacingOccurrencesOfString:@" " withString:@""]
							stringByReplacingOccurrencesOfString:@"slider(" withString:@""]
								stringByReplacingOccurrencesOfString:@")" withString:@""] 
									componentsSeparatedByString:@","];

						if([arr count]==3) {
							
							type = HydraUniformType::SliderType;	
							//NSLog(@"%@, %f, [%f-%f]",list[k],[arr[0] doubleValue],[arr[1] doubleValue],[arr[2] doubleValue]);
							
							this->items.push_back(std::pair<NSString *,Slider *>(
								list[k],
								new Slider(list[k],[arr[1] doubleValue],[arr[2] doubleValue],[arr[0] doubleValue],YES,index,k)
							));
							
							float *tmp = (float *)[(id<MTLBuffer>)this->_params[k] contents];
							tmp[0] = [arr[0] doubleValue];

							[this->menu addItem:(this->items[this->items.size()-1].second)->item()];
								
							
						}
						else { // error
							//tmp[0] = 0;
							NSLog(@"slider() argument is incorrect");
							
							if(this->_uniformsType[index][k]==HydraUniformType::DoubleType) {
								float *tmp = (float *)[(id<MTLBuffer>)this->_params[k] contents];
								tmp[0] = 0;
							}
						}
								
						isMatch = true;
										
					}];
						
					if(isMatch==false) type = HydraUniformType::FunctionType;
				}
				else {
					
					float *tmp = (float *)[(id<MTLBuffer>)this->_params[k] contents];
					tmp[0] = [_uniformsData[index][k] doubleValue];
					
				}
				
				if(this->_uniformsType[index].size()<=k) { // add
					this->_uniformsType[index].push_back(type);
				}
				else { // overwrite
					this->_uniformsType[index][k] = type;
				}
				
			}
					
		}
		
		bool reloadShader(unsigned int index, dispatch_data_t data, NSString *uniform=@"u0.json") {
			this->_uniformsPath[index] = uniform;
			bool ret = MetalLayer::reloadShader(data,index);
			this->set(index);
			return ret;
		}
		
		bool setup() {
			
			class_addMethod([NSSlider class],NSSelectorFromString(@"onUpdate"),imp_implementationWithBlock(^(id me) {
				
				NSString *key = ((NSSlider *)me).identifier;
								
				float *tmp = (float *)[(id<MTLBuffer>)this->_params[[((NSSlider *)me) index]] contents];
				tmp[0] = [me doubleValue];
				
			}),"v@*");
						
			this->statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
			[this->statusItem.button setEnabled:YES];
			[this->statusItem.button setTitle:@"↓"];
						  
			this->menu = [[NSMenu alloc] init];            
			this->statusItem.menu = this->menu;
			
			this->context = [JSContext new];
			
			this->_starttime = CFAbsoluteTimeGetCurrent();
			
			MTLTextureDescriptor *texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:this->_width height:this->_height mipmapped:NO];
			if(!texDesc) return false;
			
			
			this->_texcoordBuffer = [_device newBufferWithBytes:Plane::textureCoordinateData length:6*sizeof(float)*2 options:MTLResourceOptionCPUCacheModeDefault];
			if(!_texcoordBuffer) return nil;
		
			this->_timeBuffer = [this->_device newBufferWithLength:sizeof(float) options:MTLResourceOptionCPUCacheModeDefault];
			if(!this->_timeBuffer) return false;
			
			this->_resolutionBuffer = [this->_device newBufferWithLength:sizeof(float)*2 options:MTLResourceOptionCPUCacheModeDefault];
			if(!this->_resolutionBuffer) return false;
			
			float *resolutionBuffer = (float *)[this->_resolutionBuffer contents];
			resolutionBuffer[0] = this->_width;
			resolutionBuffer[1] = this->_height;
			
			[this->context evaluateScript:[NSString stringWithFormat:@"resolution={x:%f,y:%f};",resolutionBuffer[0],resolutionBuffer[1]]]; 
			
			this->_mouseBuffer = [this->_device newBufferWithLength:sizeof(float)*2 options:MTLResourceOptionCPUCacheModeDefault];
			if(!this->_mouseBuffer) return false;
			
			this->_o0 = [this->_device newTextureWithDescriptor:texDesc];
			if(!this->_o0)  return false;
			
			this->_o1 = [this->_device newTextureWithDescriptor:texDesc];
			if(!this->_o1)  return false;
			
			this->_o2 = [this->_device newTextureWithDescriptor:texDesc];
			if(!this->_o2)  return false;
			
			this->_o3 = [this->_device newTextureWithDescriptor:texDesc];
			if(!this->_o3)  return false;
			
			this->_s0 = [this->_device newTextureWithDescriptor:texDesc];
			if(!this->_s0)  return false;
			
			this->_s1 = [this->_device newTextureWithDescriptor:texDesc];
			if(!this->_s1)  return false;
		
			this->_s2 = [this->_device newTextureWithDescriptor:texDesc];
			if(!this->_s2)  return false;
		
			this->_s3 = [this->_device newTextureWithDescriptor:texDesc];
			if(!this->_s3)  return false;
		
			if(MetalLayer::setup()==false) return false;
						
			for(int k=0; k<this->_library.size(); k++) {
				
				this->_uniformsData.push_back([NSMutableArray array]);
				
				std::vector<HydraUniformType> v;
				this->_uniformsType.push_back(v);
				this->set(k);
			}
									
			return true;
		} 
		
		bool init(int width,int height,std::vector<NSString *> shaders={@"defalt.metallib"},std::vector<NSString *> uniforms={@"u0.json"}, bool isGetBytes=false) {
			for(int k=0; k<uniforms.size(); k++) this->_uniformsPath.push_back(uniforms[k]);
			
			return MetalLayer::init(width,height,shaders,isGetBytes);
		}
			
		id<MTLCommandBuffer> setupCommandBuffer(int mode) {
						
			id<MTLCommandBuffer> commandBuffer = [this->_commandQueue commandBuffer];
			
			float *timeBuffer = (float *)[this->_timeBuffer contents];
			timeBuffer[0] = CFAbsoluteTimeGetCurrent()-this->_starttime;
			
			float *mouseBuffer = (float *)[this->_mouseBuffer contents];
			
			double x = _frame.origin.x;
			double y = _frame.origin.y;
			double w = _frame.size.width;
			double h = _frame.size.height;
			
			NSPoint mouseLoc = [NSEvent mouseLocation];
			mouseBuffer[0] = (mouseLoc.x-x);
			mouseBuffer[1] = (mouseLoc.y-y);
						
			[context evaluateScript:[NSString stringWithFormat:@"time=%f;",timeBuffer[0]]]; 
			[context evaluateScript:[NSString stringWithFormat:@"mouse={x:%f,y:%f};",mouseBuffer[0],mouseBuffer[1]]]; 
			
			for(int k=0; k< [this->_uniformsData[mode] count]; k++) {				
				HydraUniformType type = this->_uniformsType[mode][k];
				if(type==HydraUniformType::FunctionType) {
					float *tmp = (float *)[(id<MTLBuffer>)this->_params[k] contents];
					tmp[0] = [[context evaluateScript:[NSString stringWithFormat:@"(%@)();",this->_uniformsData[mode][k]]] toDouble];
				}
			}
			
			MTLRenderPassColorAttachmentDescriptor *colorAttachment = this->_renderPassDescriptor.colorAttachments[0];
			colorAttachment.texture = this->_metalDrawable.texture;
			colorAttachment.loadAction  = MTLLoadActionClear;
			colorAttachment.clearColor  = MTLClearColorMake(0.0f,0.0f,0.0f,1.0f);
			colorAttachment.storeAction = MTLStoreActionStore;
			
			id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:this->_renderPassDescriptor];
			[renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
			[renderEncoder setRenderPipelineState:this->_renderPipelineState[mode]];
			[renderEncoder setVertexBuffer:this->_verticesBuffer offset:0 atIndex:0];
			[renderEncoder setVertexBuffer:this->_texcoordBuffer offset:0 atIndex:1];
			
			[renderEncoder useResource:this->_timeBuffer usage:MTLResourceUsageRead];
			[renderEncoder useResource:this->_resolutionBuffer usage:MTLResourceUsageRead];
			[renderEncoder useResource:this->_mouseBuffer usage:MTLResourceUsageRead];
				
			[renderEncoder useResource:this->_o0 usage:MTLResourceUsageSample];
			[renderEncoder useResource:this->_o1 usage:MTLResourceUsageSample];
			[renderEncoder useResource:this->_o2 usage:MTLResourceUsageSample];
			[renderEncoder useResource:this->_o3 usage:MTLResourceUsageSample];
			
			[renderEncoder useResource:this->_s0 usage:MTLResourceUsageSample];
			[renderEncoder useResource:this->_s1 usage:MTLResourceUsageSample];
			[renderEncoder useResource:this->_s2 usage:MTLResourceUsageSample];
			[renderEncoder useResource:this->_s3 usage:MTLResourceUsageSample];
				
			for(int k=0; k<[this->_uniformsData[mode] count]; k++) {
				[renderEncoder useResource:(id<MTLBuffer>)this->_params[k] usage:MTLResourceUsageRead];
			}
				
			[renderEncoder setFragmentBuffer:this->_argumentEncoderBuffer[mode] offset:0 atIndex:0];
			
			[renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:Plane::INDICES_SIZE indexType:MTLIndexTypeUInt16 indexBuffer:this->_indicesBuffer indexBufferOffset:0];
			
			[renderEncoder endEncoding];
			[commandBuffer presentDrawable:this->_metalDrawable];
			this->_drawabletexture = this->_metalDrawable.texture;
			return commandBuffer;
		}
		
		HydraMetalLayer() {
			NSLog(@"HydraMetalLayer");
		}
		
		~HydraMetalLayer() {
			
			NSLog(@"~HydraMetalLayer");

			
			this->_o0 = nil;
			this->_o1 = nil;
			this->_o2 = nil;
			this->_o3 = nil;
			
			this->_s0 = nil;
			this->_s1 = nil;
			this->_s2 = nil;
			this->_s3 = nil;
			
			this->context = nil;
			
		}
};
