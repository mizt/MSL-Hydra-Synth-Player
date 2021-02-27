namespace PixelFormat {
	MTLPixelFormat ARGB = MTLPixelFormatRGBA8Unorm;
	MTLPixelFormat ABGR = MTLPixelFormatBGRA8Unorm;
}

template <typename T>
class MetalBaseLayer {
	
	protected:
		
		T *_data;
		
		CAMetalLayer *_metalLayer;
		MTLRenderPassDescriptor *_renderPassDescriptor;
		
		id<MTLDevice> _device;
		id<MTLCommandQueue> _commandQueue;
		
		id<CAMetalDrawable> _metalDrawable;
		
		id<MTLTexture> _drawabletexture;
			
		id<MTLBuffer> _verticesBuffer;
		id<MTLBuffer> _indicesBuffer;
		
		std::vector<id<MTLLibrary>> _library;
		std::vector<id<MTLRenderPipelineState>> _renderPipelineState;
		std::vector<MTLRenderPipelineDescriptor *> _renderPipelineDescriptor;
		
		bool _isArgumentEncoder = false;
		std::vector<id<MTLArgumentEncoder>> _argumentEncoder;
			
		bool _isInit = false;
			
		int _width;
		int _height;
		CGRect _frame;

		bool _isGetBytes = true;
		unsigned int _mode = 0;
		
		virtual void setColorAttachment(MTLRenderPipelineColorAttachmentDescriptor *colorAttachment) {
			colorAttachment.blendingEnabled = YES;
			colorAttachment.rgbBlendOperation = MTLBlendOperationAdd;
			colorAttachment.alphaBlendOperation = MTLBlendOperationAdd;
			colorAttachment.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
			colorAttachment.sourceAlphaBlendFactor = MTLBlendFactorOne;
			colorAttachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
			colorAttachment.destinationAlphaBlendFactor = MTLBlendFactorOne;
		}

		virtual bool setupShader() {
			for(int k=0; k<this->_library.size(); k++) {
				id<MTLFunction> vertexFunction = [this->_library[k] newFunctionWithName:@"vertexShader"];
				if(!vertexFunction) return nil;
				id<MTLFunction> fragmentFunction = [this->_library[k] newFunctionWithName:@"fragmentShader"];
				if(!fragmentFunction) return nil;
				this->_renderPipelineDescriptor.push_back([MTLRenderPipelineDescriptor new]);
				if(!this->_renderPipelineDescriptor[k]) return nil;
				
				if(this->_isArgumentEncoder) this->_argumentEncoder.push_back([fragmentFunction newArgumentEncoderWithBufferIndex:0]);
				
				this->_renderPipelineDescriptor[k].depthAttachmentPixelFormat = MTLPixelFormatInvalid;
				this->_renderPipelineDescriptor[k].stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
				this->_renderPipelineDescriptor[k].colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
				if(this->_isGetBytes) {
					this->_renderPipelineDescriptor[k].colorAttachments[0].blendingEnabled = NO;
				}
				else {
					this->setColorAttachment(this->_renderPipelineDescriptor[k].colorAttachments[0]);
				}
				this->_renderPipelineDescriptor[k].sampleCount = 1;
				this->_renderPipelineDescriptor[k].vertexFunction   = vertexFunction;
				this->_renderPipelineDescriptor[k].fragmentFunction = fragmentFunction;	
				NSError *error = nil;
				this->_renderPipelineState.push_back([this->_device newRenderPipelineStateWithDescriptor:this->_renderPipelineDescriptor[k] error:&error]);
				if(error||!this->_renderPipelineState[k]) return true;
			}
			return false;
		}
		
		virtual bool updateShader(unsigned int index) {
			if(index>=this->_library.size()) return true;
			id<MTLFunction> vertexFunction = [this->_library[index] newFunctionWithName:@"vertexShader"];
			if(!vertexFunction) return nil;
			id<MTLFunction> fragmentFunction = [this->_library[index] newFunctionWithName:@"fragmentShader"];
			if(!fragmentFunction) return nil;
			if(this->_isArgumentEncoder) this->_argumentEncoder[index] = [fragmentFunction newArgumentEncoderWithBufferIndex:0];			
			this->_renderPipelineDescriptor[index].sampleCount = 1;
			this->_renderPipelineDescriptor[index].vertexFunction   = vertexFunction;
			this->_renderPipelineDescriptor[index].fragmentFunction = fragmentFunction;
			NSError *error = nil;
			this->_renderPipelineState[index] = [this->_device newRenderPipelineStateWithDescriptor:this->_renderPipelineDescriptor[index] error:&error];
			if(error||!this->_renderPipelineState[index]) return true;
			return false;
		}
		
	public:
		
		T *data() { return this->_data; }
		
		MetalBaseLayer(int x=4,int y=4) {
			this->_data = new T(x,y);
		}

		~MetalBaseLayer() {
			delete this->_data;
		}
		
		virtual bool setup() {
			
			this->_verticesBuffer = [this->_device newBufferWithBytes:_data->vertices length:_data->VERTICES_SIZE*sizeof(float) options:MTLResourceOptionCPUCacheModeDefault];
			if(!this->_verticesBuffer) return false;
			
			this->_indicesBuffer = [this->_device newBufferWithBytes:_data->indices length:_data->INDICES_SIZE*sizeof(short) options:MTLResourceOptionCPUCacheModeDefault];
			if(!this->_indicesBuffer) return false;			
			
			return true;
		} 
		
		virtual id<MTLCommandBuffer> setupCommandBuffer(unsigned int mode) {
						
			id<MTLCommandBuffer> commandBuffer = [this->_commandQueue commandBuffer];
			MTLRenderPassColorAttachmentDescriptor *colorAttachment = this->_renderPassDescriptor.colorAttachments[0];
			colorAttachment.texture = this->_metalDrawable.texture;
			colorAttachment.loadAction  = MTLLoadActionClear;
			colorAttachment.clearColor  = MTLClearColorMake(0.0f,0.0f,0.0f,0.0f);
			colorAttachment.storeAction = MTLStoreActionStore;
			
			id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:this->_renderPassDescriptor];
			[renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
			[renderEncoder setRenderPipelineState:this->_renderPipelineState[mode]];
			[renderEncoder setVertexBuffer:this->_verticesBuffer offset:0 atIndex:0];
			[renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:_data->INDICES_SIZE indexType:MTLIndexTypeUInt16 indexBuffer:this->_indicesBuffer indexBufferOffset:0];
			[renderEncoder endEncoding];
			
			[commandBuffer presentDrawable:this->_metalDrawable];
			this->_drawabletexture = this->_metalDrawable.texture;
			return commandBuffer;
		}
		
		virtual bool init(int width,int height,std::vector<NSString *> shaders={@"default.metallib"}, bool isGetBytes=false) {
			
			this->_frame.size.width  = this->_width  = width;
			this->_frame.size.height = this->_height = height;
			if(this->_metalLayer==nil) {
				this->_metalLayer = [CAMetalLayer layer];
			}			
			this->_device = MTLCreateSystemDefaultDevice();
			this->_metalLayer.device = this->_device;
			this->_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

#if TARGET_OS_OSX

			this->_metalLayer.colorspace = [[NSScreen mainScreen] colorSpace].CGColorSpace;//CGColorSpaceCreateDeviceRGB();
#else
			this->_metalLayer.colorspace = CGColorSpaceCreateDeviceRGB();
#endif
			
			
			this->_metalLayer.opaque = NO;
			this->_metalLayer.framebufferOnly = NO;

#if TARGET_OS_OSX

			this->_metalLayer.displaySyncEnabled = YES;
#endif
			
			this->_metalLayer.drawableSize = CGSizeMake(this->_width,this->_height);
			this->_commandQueue = [this->_device newCommandQueue];
			if(!this->_commandQueue) return false;
			NSError *error = nil;
			for(int k=0; k<shaders.size(); k++) {
				id<MTLLibrary> lib= [this->_device newLibraryWithFile:[NSString stringWithFormat:@"%@/%@",[[NSBundle mainBundle] resourcePath],shaders[k]] error:&error];
				if(lib) {
					//NSLog(@"%@",[NSString stringWithFormat:@"%@/%@",[[NSBundle mainBundle] resourcePath],shaders[k]]);				
					this->_library.push_back(lib);
					if(error) return false;
				}
				else {
					return false;
				}
			}
			this->_isGetBytes = isGetBytes;
			if(this->setupShader()) return false;	
			this->_isInit = this->setup();
			return this->_isInit;
		}
		
		bool isInit() {
			return this->_isInit;
		}

		void mode(unsigned int n) {
			this->_mode = n; 
		}
		
		id<MTLTexture> drawableTexture() { 
			return this->_drawabletexture; 
		}
		
		void cleanup() { 
			this->_metalDrawable = nil; 
		}
		
		bool reloadShader(dispatch_data_t data, unsigned int index) {
			NSError *error = nil;
			this->_library[index] = [this->_device newLibraryWithData:data error:&error];
			if(error||!this->_library[index]) return true;
			if(this->updateShader(index)) return true;
			return false;
		}
		
		void resize(CGRect frame) {
			this->_frame = frame;
		}
		
		id<MTLCommandBuffer> prepareCommandBuffer(long mode) {
			if(!this->_metalDrawable) {
				this->_metalDrawable = [this->_metalLayer nextDrawable];
			}
			if(!this->_metalDrawable) {
				this->_renderPassDescriptor = nil;
			}
			else {
				if(this->_renderPassDescriptor==nil) this->_renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
			}
			if(this->_metalDrawable&&this->_renderPassDescriptor) {
				return this->setupCommandBuffer(mode);
			}
			return nil;
		}
		
		void update(void (^onComplete)(id<MTLCommandBuffer>)) {
			
			if(this->_isInit==false) return; 
						
			unsigned int mode = this->_mode;
			if(mode>=this->_library.size()) mode = this->_library.size()-1;
			if(this->_renderPipelineState[mode]) {
				id<MTLCommandBuffer> commandBuffer = this->prepareCommandBuffer(mode);
				if(commandBuffer) {
					[commandBuffer addCompletedHandler:onComplete];
					[commandBuffer commit];
					[commandBuffer waitUntilCompleted];
				}
			}
		}
		
		CAMetalLayer *layer() {
			return this->_metalLayer;
		}
};
