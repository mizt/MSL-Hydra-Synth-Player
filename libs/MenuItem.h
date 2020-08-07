#import <Cocoa/Cocoa.h>
#import "vector"

enum MenuType {
    TEXT = 0,
    CHECKBOX,
    RADIOBUTTON,
    SLIDER,
    BUTTON
};

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

#pragma mark TextMenuItem
@interface TextMenuItem:NSMenuItem {
    NSView    *view;
    TextField *textField;
}
@end
@implementation TextMenuItem
-(void)text:(NSString *)name {
    [textField text:name];
    [view setFrame:NSMakeRect(0,0,19+[textField bounds].size.width+19,19)];
}
-(id)init:(NSString *)name {
    self = [super init];
    if(self) {
        view = [[NSView alloc] init];
        textField = [[TextField alloc] initWithFrame:NSMakeRect(19,1,128,19)];
        [textField text:name];
        [view setFrame:NSMakeRect(0,0,19+[textField bounds].size.width+19,19)];
        [view addSubview:textField];
        [self setView:view];
    }
    return self;
}
@end

#pragma mark RadioButtonMenuItem
@interface RadioButtonMenuItem:NSMenuItem {
    std::vector<NSButton *> buttons;
    std::vector<TextField *> textFields;
    int select;
    NSView *view;
    NSView *item;
    NSString *_identifier;
    bool isEnabled;
}
@property(strong,nonatomic) NSString *identifier;
@end
@implementation RadioButtonMenuItem
@synthesize identifier;
-(void)reset {
    [buttons[0] setState:1];
}
-(bool)isDark {
    id style = [[[NSUserDefaults standardUserDefaults] persistentDomainForName:NSGlobalDomain] objectForKey:@"AppleInterfaceStyle"];
    return (style&&[style isKindOfClass:[NSString class]]&&[style caseInsensitiveCompare:@"dark"]==NSOrderedSame)?true:false;
}
-(double)doubleValue { return select; }
-(void)doubleValue:(double)value {
    if(value>=0&&value<buttons.size()) {
        if(select!=value) {
            [buttons[select] setState:0];
            [buttons[value] setState:1];
            select = value;
        }
    }
}
-(void)on {
    long num = textFields.size();
    for(int k=0; k<num; k++) {
        isEnabled = true;
        [textFields[k] setEnabled:YES];
        [textFields[k] setTextColor:[self isDark]?[NSColor whiteColor]:[NSColor blackColor]];
        [buttons[k] setEnabled:YES];
    }
}
-(void)off {
    long num = textFields.size();
    for(int k=0; k<num; k++) {
        isEnabled = false;
        [textFields[k] setEnabled:NO];
        [textFields[k] setTextColor:[self isDark]?[NSColor lightGrayColor]:[NSColor darkGrayColor]];
        [buttons[k] setEnabled:NO];
    }
}
-(void)onDown:(id)sender {
    int current = -1;
    long num = buttons.size();
    for(int i=0; i<num; i++) {
        if(buttons[i]==sender) {
            current = i;
        }
    }
    if(current==-1) return;
    if(select!=current) {
        [buttons[select] setState:0];
        [buttons[current] setState:1];
        select = current;
    }
    ((void(*)(id,SEL))objc_msgSend)(self,NSSelectorFromString(@"update"));
}
-(id)init:(NSString *)name :(std::vector<NSString *>)names :(double)value {
        
    self = [super init];
    if(self) {
        isEnabled = false;
        item = [[NSView alloc] init];
        identifier = name;
        int width = 0;
        long num = names.size();
        for(int k=0; k<num; k++) {
            buttons.push_back([[NSButton alloc] initWithFrame:NSMakeRect(0,19*((num-k)-1)+8,19,16)]);
            [buttons[k] setButtonType:NSButtonTypeRadio];
            [buttons[k] setTarget:self];
            [buttons[k] setAction:@selector(onDown:)];

            textFields.push_back([[TextField alloc] initWithFrame:NSMakeRect([buttons[0] bounds].size.width+5,19*((num-k)-1)+8,128,16+8+8)]);
            
            [textFields[k] text:names[k]];
            
            if([textFields[k] bounds].size.width>width) { width = [textFields[k] bounds].size.width; };
            [item addSubview:buttons[k]];
            [item addSubview:textFields[k]];
        }
        [item setFrame:NSMakeRect(19,0,[buttons[0] bounds].size.width+5+width+19,19*num+8+8)];
        
        view = [[NSView alloc] initWithFrame:NSMakeRect(0,0,19+[item bounds].size.width,[item bounds].size.height)];
        [view addSubview:item];

        select = value;
        [buttons[select] setState:1];
        [self setView:view];
    }
    return self;
}
@end

#pragma mark CheckBoxMenuItem
@interface CheckBoxMenuItem:NSMenuItem {
    NSView      *view;
    NSButton    *checkbox;
    TextField   *textField;
}@end

@implementation CheckBoxMenuItem
-(bool)isDark {
    id style = [[[NSUserDefaults standardUserDefaults] persistentDomainForName:NSGlobalDomain] objectForKey:@"AppleInterfaceStyle"];
    return (style&&[style isKindOfClass:[NSString class]]&&[style caseInsensitiveCompare:@"dark"]==NSOrderedSame)?true:false;
}
-(double)doubleValue { return [checkbox state]; }
-(void)doubleValue:(double)value {
    if(value==0||value==1) {
        [checkbox setState:value];
    }
}
-(void)text:(NSString *)name {
    [textField text:name];
    [view setFrame:NSMakeRect(0,0,19+[checkbox bounds].size.width+3+[textField bounds].size.width+19,16+8+8)];

}

-(void)on {
    [checkbox setEnabled:YES];
    [textField setTextColor:[self isDark]?[NSColor whiteColor]:[NSColor blackColor]];
}

-(void)off {
    [checkbox setEnabled:NO];
    [textField setTextColor:[self isDark]?[NSColor lightGrayColor]:[NSColor darkGrayColor]];
}
-(id)init:(NSString *)name :(double)value {
    self = [super init];
    if(self) {
        view = [[NSView alloc] init];
        checkbox = [[NSButton alloc] initWithFrame:NSMakeRect(19,0,19,16+8+8)];
        [checkbox setButtonType:NSButtonTypeSwitch];
        [checkbox setTarget:checkbox];
        [checkbox setAction:NSSelectorFromString(@"update")];
        [checkbox setIdentifier:name];
        if(value>0) [checkbox setState:1];
        [view addSubview:checkbox];
        textField = [[TextField alloc] initWithFrame:NSMakeRect(19+3+[checkbox bounds].size.width,8,128,16+8+8)];
        [textField text:name];
        [view setFrame:NSMakeRect(0,0,19+[checkbox bounds].size.width+3+[textField bounds].size.width+19,16+8+8)];
        [view addSubview:textField];
        [self setView:view];
    }
    return self;
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
-(id)init:(NSString *)name :(double)min :(double)max :(double)value :(bool)label {
    self = [super init];
    if(self) {
        
        view = [[NSView alloc] init];
        
        slider = [[NSSlider alloc] initWithFrame:CGRectMake(19,8,(200-40),16)];
        [slider setMinValue:min];
        [slider setMaxValue:max];
        [slider setFloatValue:value];
        [view addSubview:slider];
        [slider setTarget:slider];
        [slider setIdentifier:name];
        [slider setAction:NSSelectorFromString(@"onUpdate")];
        
        isLabel = label;
        
        if(isLabel) {
            textField = [[TextField alloc] initWithFrame:NSMakeRect(19+[slider bounds].size.width+5,8,128,19)];
            [textField text:name];
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

#pragma mark ButtonMenuItem
@interface ButtonMenuItemView:NSButtonView {
    bool isEnabled;
    TextField *textField;
    NSTrackingRectTag trackingRect;
}
@end
@implementation ButtonMenuItemView
-(bool)isDark {
    id style = [[[NSUserDefaults standardUserDefaults] persistentDomainForName:NSGlobalDomain] objectForKey:@"AppleInterfaceStyle"];
    return (style&&[style isKindOfClass:[NSString class]]&&[style caseInsensitiveCompare:@"dark"]==NSOrderedSame)?true:false;
}
-(void)text:(NSString *)name {
    isEnabled = true;
    [textField text:name];
    [self setFrame:NSMakeRect(0,0,19+[textField bounds].size.width+19,18)];
}
-(id)init:(NSString *)name {
    self = [super init];
    if(self) {
        textField = [[TextField alloc] initWithFrame:NSMakeRect(19,1,128,18)];
        [textField text:name];
        [self addSubview:textField];
        [self setFrame:NSMakeRect(0,0,19+[textField bounds].size.width+19,18)];
        [self setIdentifier:name];
    }
    return self;
}
-(void)on {
    isEnabled = true;
    [textField setEnabled:YES];
    [textField setTextColor:[self isDark]?[NSColor whiteColor]:[NSColor blackColor]];
}
-(void)off {
    isEnabled = false;
    [textField setEnabled:NO];
    [textField setTextColor:[self isDark]?[NSColor lightGrayColor]:[NSColor grayColor]];
}
-(void)mouseDown:(NSEvent *)theEvent {
    if(isEnabled) {
        [textField setTextColor:[self isDark]?[NSColor blackColor]:[NSColor whiteColor]];
        dispatch_time_t tmp = dispatch_time(DISPATCH_TIME_NOW,(int64_t)(NSEC_PER_SEC*0.1));
        dispatch_after(tmp,dispatch_get_main_queue(),^(void){
            if(self->isEnabled) {
                [self->textField setTextColor:[self isDark]?[NSColor whiteColor]:[NSColor blackColor]];
                ((void(*)(id,SEL))objc_msgSend)(self,NSSelectorFromString(@"update"));
            }
        });
    }
}
-(void)mouseEntered:(NSEvent *)theEvent {
    if(isEnabled) {
        [textField setTextColor:[self isDark]?[NSColor lightGrayColor]:[NSColor grayColor]];
    }
}
-(void)mouseExited:(NSEvent *)theEvent {
    if(isEnabled) {
        [textField setTextColor:[self isDark]?[NSColor whiteColor]:[NSColor blackColor]];
    }
}
-(void)setFrame:(NSRect)frame {
    [super setFrame:frame];
    [self removeTrackingRect:trackingRect];
    trackingRect = [self addTrackingRect:frame owner:self userData:NULL assumeInside:NO];
}
@end

@interface ButtonMenuItem:NSMenuItem {
    ButtonMenuItemView  *view;
}
@end
@implementation ButtonMenuItem
-(void)text:(NSString *)name { [view text:name]; }
-(void)on { [view on]; }
-(void)off { [view off]; }
-(id)init:(NSString *)name {
    self = [super init];
    if(self) {
        view = [[ButtonMenuItemView alloc] init:name];
        [view setFrame:NSMakeRect(0,0,19+[view bounds].size.width+19,19)];
        [self setView:view];
        [view on];
    }
    return self;
}
@end

#pragma mark IMenuItem
class IMenuItem {
    protected :
        NSMenuItem  *menuItem;
        MenuType menutype;
        NSString  *menuName;
        NSString *parentName = nil;
    
    public :
    
        NSMenuItem *item() { return menuItem; }
        MenuType type() { return menutype; }
        NSString *name() { return menuName; }
        bool eq(NSString *name,bool isParent=false) {
            return (!isParent)?
                ([menuName isEqualToString:name]?true:false):
                (parentName)?[parentName isEqualToString:name]:false;
        }    
        bool isParent() { return (parentName)?true:false; }
        virtual double value() { return 0; }
        virtual void value(double v) {}
        virtual void unbind() {}
        virtual void on() {}
        virtual void off() {}
        virtual void reset() {}
    
        IMenuItem() {}
        virtual ~IMenuItem() {}
};

#pragma mark MenuItem
namespace MenuItem {
    class Text : public IMenuItem {
        public :
            Text(NSString *name,NSString *key,NSString *parent=nil) {
                menutype = MenuType::TEXT;
                menuName = name;
                parentName = parent;
                menuItem = [[NSMenuItem alloc] initWithTitle:name action:NSSelectorFromString(@"update") keyEquivalent:key];
                [menuItem setTarget:menuItem];
                [menuItem setRepresentedObject:name];
            }
            void unbind() { [menuItem setAction:nil]; }
            void on() { [menuItem setEnabled:YES]; }
            void off() { [menuItem setEnabled:NO]; }
    };
    class RadioButton : public IMenuItem {
        public :
            RadioButton(NSString *name,std::vector<NSString *> names,double value,NSString *parent=nil) {
                menuName = name;
                parentName = parent;
                menutype = MenuType::RADIOBUTTON;
                menuItem = [[RadioButtonMenuItem alloc] init:name :names :value];
            }
            double value() { return [(RadioButtonMenuItem *)(menuItem) doubleValue]; }
            void value(double v) { [(RadioButtonMenuItem *)(menuItem) doubleValue:v]; }
            void on() { [(RadioButtonMenuItem *)(menuItem) on]; }
            void off() { [(RadioButtonMenuItem *)(menuItem) off]; }
            void reset() { [(RadioButtonMenuItem *)(menuItem) reset]; }
        
    };
    class CheckBox : public IMenuItem {
        public :
            CheckBox(NSString *name,double value,NSString *parent=nil) {
                menuName = name;
                parentName = parent;
                menutype = MenuType::CHECKBOX;
                menuItem = [[CheckBoxMenuItem alloc] init:name :value];
            }
            double value() { return [(CheckBoxMenuItem *)(menuItem) doubleValue]; }
            void value(double v) {  [(CheckBoxMenuItem *)(menuItem) doubleValue:v]; }
            void on() { [(CheckBoxMenuItem *)(menuItem) on]; }
            void off() { [(CheckBoxMenuItem *)(menuItem) off]; }
        
    };
    class Slider : public IMenuItem {
        public :
            Slider(NSString *name, double min, double max, double value,bool label,NSString *parent=nil) {
                menuName = name;
                parentName = parent;
                menutype = MenuType::SLIDER;
                menuItem = [[SliderMenuItem alloc] init:name :min :max :value :label];
            }
            double value() { return [(SliderMenuItem *)(menuItem) doubleValue]; }
            void on() { [(SliderMenuItem *)(menuItem) on]; }
            void off() { [(SliderMenuItem *)(menuItem) off]; }
        
    };
    class Button : public IMenuItem {
        public :
            Button(NSString *name,NSString *parent=nil) {
                menuName = name;
                parentName = parent;
                menutype = MenuType::BUTTON;
                menuItem = [[ButtonMenuItem alloc] init:name];
            }
            void on() { [(ButtonMenuItem *)(menuItem) on]; }
            void off() { [(ButtonMenuItem *)(menuItem) off]; }
    };
}
