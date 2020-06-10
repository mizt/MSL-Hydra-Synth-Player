#import "MetalLayer.h"
#import <JavascriptCore/JavascriptCore.h>

class HydraMetalLayer : public MetalLayer {
	
	private:
		
		const int offset = 11;
		
		JSContext *context; 
		
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
		std::vector<NSString *> _uniforms;
		std::vector<NSMutableArray *> _uniform;

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
				
			NSString *path = this->_uniforms.size()?this->_uniforms[index]:@"./default.json";
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
			
			
			for(int n=0; n<[list count]; n++) {
				
				if([this->_uniform[index] count]<=n) {
					[this->_uniform[index] addObject:dict[list[n]]];
				}
				else {
					this->_uniform[index][n] = dict[list[n]];
				}
				
				if(this->_params.size()<=n) {
					this->_params.push_back((id)[_device newBufferWithLength:sizeof(float) options:MTLResourceOptionCPUCacheModeDefault]);
					[this->_argumentEncoder[index] setBuffer:(id<MTLBuffer>)this->_params[n] offset:0 atIndex:offset+n];
				}
				else {
					this->_params[n] = (id)[_device newBufferWithLength:sizeof(float) options:MTLResourceOptionCPUCacheModeDefault];
					[this->_argumentEncoder[index] setBuffer:(id<MTLBuffer>)this->_params[n] offset:0 atIndex:offset+n];
				}
				
			}
		}
		
		bool reloadShader(unsigned int index, dispatch_data_t data, NSString *uniform=@"u0.json") {
			this->_uniforms[index] = uniform;
			bool ret = MetalLayer::reloadShader(data,index);
			this->set(index);
			return ret;
		}
		
		bool setup() {
			
			this->context = [JSContext new];
			
			this->_starttime = CFAbsoluteTimeGetCurrent();
			
			MTLTextureDescriptor *texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:this->_width height:this->_height mipmapped:NO];
			if(!texDesc) return false;
			
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
				this->_uniform.push_back([NSMutableArray array]);
				this->set(k);
			}
									
			return true;
		} 
		
		bool init(int width,int height,std::vector<NSString *> shaders={@"defalt.metallib"},std::vector<NSString *> uniforms={@"u0.json"}, bool isGetBytes=false) {
			for(int k=0; k<uniforms.size(); k++) this->_uniforms.push_back(uniforms[k]);
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
			
			for(int k=0; k< [this->_uniform[mode] count]; k++) {
				float *tmp = (float *)[(id<MTLBuffer>)this->_params[k] contents];
				if([[_uniform[mode][k] className] compare:@"__NSCFString"]==NSOrderedSame) {
					tmp[0] = [[context evaluateScript:[NSString stringWithFormat:@"(%@)();",this->_uniform[mode][k]]] toDouble];
				}
				else {
					tmp[0] = [_uniform[mode][k] doubleValue];
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
			//[renderEncoder setVertexBuffer:this->_texcoordBuffer offset:0 atIndex:1];
			
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
				
			for(int n=0; n<[this->_uniform[mode] count]; n++) {
				[renderEncoder useResource:(id<MTLBuffer>)this->_params[n] usage:MTLResourceUsageRead];
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
