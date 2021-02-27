#if TARGET_OS_OSX

#else

    namespace Touch {
        int x = 0;
        int y = 0;
    }

    @interface MetalView:UIView @end
    @implementation MetalView
        +(Class)layerClass { return [CAMetalLayer class]; }
        
        -(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
            CGPoint locationPoint = [[touches anyObject] locationInView:self];
            Touch::x = locationPoint.x;
            Touch::y = locationPoint.y;
        }
    
        -(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
            CGPoint locationPoint = [[touches anyObject] locationInView:self];
            Touch::x = locationPoint.x;
            Touch::y = locationPoint.y;
        }
        
        -(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
            CGPoint locationPoint = [[touches anyObject] locationInView:self];
            Touch::x = locationPoint.x;
            Touch::y = locationPoint.y;
        }
        
        -(void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {}

    @end
    
#endif
