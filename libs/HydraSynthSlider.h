#define HYDRA_SYNTH_SLIDER

@interface HSSlider:NSSlider {
	int mode;
	int index;
}
@property (nonatomic) int mode;
@property (nonatomic) int index;
@end

@implementation HSSlider
	@synthesize mode = _mode;
	@synthesize index = _index;
@end

#pragma mark HydraTextField
@interface HydraTextField:NSTextField {
	
}
@end
@implementation HydraTextField
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

#pragma mark HydraSliderMenuItem
@interface HydraSliderMenuItem:NSMenuItem {
	NSView *view;
	HSSlider *slider;
	HydraTextField *textField;
	BOOL isLabel;
}@end
@implementation HydraSliderMenuItem
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
-(id)init:(NSString *)key :(double)value :(double)min :(double)max :(bool)label :(int)mode :(int)index {
	self = [super init];
	if(self) {
		
		view = [[NSView alloc] init];
		
		slider = [[HSSlider alloc] initWithFrame:CGRectMake(19,8,160,16)];
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
			
			textField = [[HydraTextField alloc] initWithFrame:NSMakeRect(19+[slider bounds].size.width+5,8,128,19)];
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

namespace HydraSynthSlider {
	
	bool isSetup = false;
	
	class Slider {
		
		protected:
			
			NSMenuItem *menuItem;
			NSString *menuName;
			
		public:
					
			Slider(NSString *name, double value, double min, double max, bool label, int mode, int index) {
				menuName = name;
				menuItem = [[HydraSliderMenuItem alloc] init:name :value :min :max :label :mode :index];
			}
			
			~Slider() {
				//NSLog(@"~Slider");
				menuItem = nil;
			}
			
			NSMenuItem *item() { return menuItem; }
			NSString *name() { return menuName; }
			bool eq(NSString *name) { return ([menuName compare:name]==NSOrderedSame)?true:false; }
			
			double value() { return [(HydraSliderMenuItem *)(menuItem) doubleValue]; }
			void on() { [(HydraSliderMenuItem *)(menuItem) on]; }
			void off() { [(HydraSliderMenuItem *)(menuItem) off]; }
	};
	
	void setup(id callback) {
		
		if(!isSetup) {			
			class_addMethod([HSSlider class],NSSelectorFromString(@"onUpdate"),imp_implementationWithBlock(callback),"v@*");
			isSetup = true;
		}		
	}
	
}

