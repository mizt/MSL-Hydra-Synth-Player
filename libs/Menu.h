#import <objc/runtime.h>
#import <objc/message.h>

#define USE_MENU_UI

@class RadioButtonMenuItem;
@interface NSButtonView : NSView @end @implementation NSButtonView @end

#import "MenuItem.h"

typedef std::pair<NSString *,NSMenu *> MenuList;
typedef std::pair<NSString *,NSMenuItem *> MenuItemList;

class Menu {
    
    private :
    
        NSMenu *_context;
        std::vector<MenuList> _menu;
        std::vector<MenuItemList> _item;
        std::vector<NSString *> _keys;
        std::vector<std::pair<NSString*,IMenuItem *>> _items;
    
        NSString *UIElement = nil;
        NSStatusItem *_statusItem;
    
        void(^callback)(id me,IMenuItem *);
    
        bool menuExists(NSString *key) {
            long size = _menu.size();
            for(int k=0; k<size; k++) {
                if([_menu[k].first isEqualToString:key]) return true;
            }
            return false;
        }
    
        bool itemExists(NSString *key) {
            long size = _item.size();
            for(int k=0; k<size; k++) {
                if([_item[k].first isEqualToString:key]) return true;

            }
            return false;
        }
    
        NSMenu *getMenu(NSString *key) {
            long size = _menu.size();
            for(int k=0; k<size; k++) {
                if([_menu[k].first isEqualToString:key]) return _menu[k].second;
            }
            return nil;
        }
    
        NSMenuItem *getItem(NSString *key) {
            long size = _item.size();
            for(int k=0; k<size; k++) {
                if([_item[k].first isEqualToString:key]) return _item[k].second;
            }
            return nil;
        }
    
        void root() {
            
            this->callback = nil;
            
            class_addMethod([NSSlider class],NSSelectorFromString(@"onUpdate"),imp_implementationWithBlock(^(id me) {
                NSString *name = ((NSSlider *)me).identifier;
                IMenuItem *tmp = nil;
                long len = this->_items.size();
                for(int k=0; k<len; k++) {
                    if([name isEqualToString:this->_items[k].first]) {
                        tmp = _items[k].second;
                        break;
                    }
                }
                if(this->callback) this->callback(me,tmp);
            }),"v@*");
            
            
            class_addMethod([RadioButtonMenuItem class],NSSelectorFromString(@"update"),imp_implementationWithBlock(^(id me) {
                NSString *name = ((RadioButtonMenuItem *)me).identifier;
                IMenuItem *tmp = nil;
                long len = this->_items.size();
                for(int k=0; k<len; k++) {
                    if([name isEqualToString:this->_items[k].first]) {
                        tmp = _items[k].second;
                        break;
                    }
                }
                if(this->callback) this->callback(me,tmp);
            }),"v@*");
            
            class_addMethod([NSButtonView class],NSSelectorFromString(@"update"),imp_implementationWithBlock(^(id me) {
                NSString *name = ((NSButtonView *)me).identifier;
                IMenuItem *tmp = nil;
                long len = this->_items.size();
                for(int k=0; k<len; k++) {
                    if([name isEqualToString:this->_items[k].first]) {
                        tmp = _items[k].second;
                        break;
                    }
                }
                if(this->callback) this->callback(me,tmp);
                
            }),"v@*");
            
            class_addMethod([NSButton class],NSSelectorFromString(@"update"),imp_implementationWithBlock(^(id me) {
                NSString *name = ((NSButton *)me).identifier;
                IMenuItem *tmp = nil;
                long len = this->_items.size();
                for(int k=0; k<len; k++) {
                    if([name isEqualToString:this->_items[k].first]) {
                        tmp = _items[k].second;
                        break;
                    }
                }
                if(this->callback) this->callback(me,tmp);
            }),"v@*");
            
            class_addMethod([NSMenuItem class],NSSelectorFromString(@"update"),imp_implementationWithBlock(^(id me) {
                NSString *name = [me representedObject];
                IMenuItem *tmp = nil;
                long len = this->_items.size();
                for(int k=0; k<len; k++) {
                    if([name isEqualToString:this->_items[k].first]) {
                        tmp = _items[k].second;
                        break;
                    }
                }
                if(this->callback) this->callback(me,tmp);
            }),"v@*");
            
            NSMenu *menu  = [[NSMenu alloc] init];
            
            if(UIElement) {
                _statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
                [_statusItem.button setEnabled:YES];
                [_statusItem.button setTitle:UIElement];
                [_statusItem setMenu:menu];
            }
            else {
                NSMenu *root = [[NSMenu alloc] init];
                NSMenuItem *item = [[NSMenuItem alloc] init];
                
                [NSApp setMainMenu:root];
                [root addItem:item];
                [item setSubmenu:menu];
                [menu setAutoenablesItems:NO];
            }
            
            [menu setAutoenablesItems:NO];
            this->_context = menu;
            _menu.push_back(MenuList(@"root",menu));
        }
    
        Menu() { NSLog(@"Menu"); }
        Menu(const Menu& me) {}
        virtual ~Menu() {}
    
    public :
    
        constexpr static const double VERSION = 4.0;
    
        static Menu *$() {
            static Menu *instance=nullptr;
            if(instance==nullptr) {
            
                instance = new Menu();
                
                NSDictionary *info = [[NSBundle mainBundle] infoDictionary];
                if([info objectForKey:@"LSUIElement"]) {
                    if([[info objectForKey:@"LSUIElement"] intValue]) {
                        instance->UIElement = @"↓";//"[info objectForKey:@"CFBundleName"]; // @"↓"; // 
                    }
                }
                
                instance->root();
            }
            return instance;
        }
    
    Menu *add(NSString *key) {
        
        if(menuExists(key)) {
            NSLog(@"%@ is already defined",key);
            return this;
        }
            
        NSMenu *menu  = [[NSMenu alloc] init];
        NSMenuItem *item = [[NSMenuItem alloc] init];
        
        [menu setTitle:key];
        [item setSubmenu:menu];
        [[NSApp mainMenu] addItem:item];
        [menu setAutoenablesItems:NO];
        
        this->_context = menu;
        _menu.push_back(MenuList(key,menu));
        
        return this;
    }
    
    
    Menu *on(void(^b)(id me,IMenuItem *)) {
        this->callback = b;
        return this;
    }
    
    Menu *hr(NSString *parentName) {
        IMenuItem *tmp = nil;
        long len = this->_items.size();
        for(int k=0; k<len; k++) {
            
            if([parentName isEqualToString:this->_items[k].first]) {
                tmp = _items[k].second;
                break;
            }
        }
        NSMenu *menu = getMenu(parentName);
        if(menu==nil) {
            if(tmp) {
                menu  = [[NSMenu alloc] init];
                [menu setTitle:parentName];
                [tmp->item() setSubmenu:menu];
                _menu.push_back(MenuList(parentName,menu));
            }
            else {
                NSLog(@"%@ is not found",parentName);
                return this;
            }
        }
        this->_context = menu;
        return hr();
    }
    
    Menu *hr() {
        if(this->_context) {
            NSMenuItem *childItem = [NSMenuItem separatorItem];
            [this->_context addItem:childItem];
        }
        return this;
    }
    
    Menu *addItem(NSString *childName,MenuType type, NSString *json=@"{}",NSString *parentName=nil) {
        
        if(itemExists(childName)) {
            NSLog(@"%@ is already defined",childName);
            return this;
        }
        
        if(this->_context) {
            
            NSError *error = nil;
            NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:[[json stringByReplacingOccurrencesOfString:@"'" withString:@"\""] dataUsingEncoding:NSUTF8StringEncoding] options:NSJSONReadingAllowFragments error:&error];
            
            if(type==MenuType::TEXT) {
                
                NSString *key = (dict[@"key"]!=nil)?dict[@"key"]:@"";
                
                if(![key isEqualToString:@""]) {
                    long len = _keys.size();
                    for(int k=0; k<len; k++) {
                        if([_keys[k] isEqualToString:key]) {
                            NSLog(@"%@ : \"%@\" is already defined",childName,key);
                            key = @"";
                            break;
                        }
                    }
                    if(![key isEqualToString:@""]) {
                        //NSLog(@"key is \"%@\"",key);
                        _keys.push_back(key);
                    }
                }
                _items.push_back(std::pair<NSString *,IMenuItem *>(childName,new MenuItem::Text(childName,key,parentName)));
                IMenuItem *tmp = _items[_items.size()-1].second;
                tmp->on();
                NSMenuItem *childItem = tmp->item();
                [this->_context addItem:childItem];
                _item.push_back(MenuItemList(childName,childItem));
                
            }
            else if(type==MenuType::CHECKBOX) {
                
                double cbVal = (dict[@"value"]!=nil)?[dict[@"value"] doubleValue]:0;

                _items.push_back(std::pair<NSString *,IMenuItem *>(childName,new MenuItem::CheckBox(childName,cbVal,parentName)));
                IMenuItem *tmp = _items[_items.size()-1].second;
                NSMenuItem *childItem = tmp->item();
                [this->_context addItem:childItem];
                _item.push_back(MenuItemList(childName,childItem));
                
            }
            else if(type==MenuType::RADIOBUTTON) {
                
                if(dict[@"label"]) {
                    
                    long count = [dict[@"label"] count];
                    
                    if(count>=2) {
                        
                        std::vector<NSString *> rbLabelArr;
                        for(int k=0; k<count; k++) rbLabelArr.push_back(dict[@"label"][k]);
                        
                        double rbVal = (dict[@"value"]!=nil)?[dict[@"value"] doubleValue]:0;
                        if(rbVal<0) rbVal=0;
                        if(rbVal>(rbLabelArr.size()-1)) rbVal=(rbLabelArr.size()-1);
                        
                        _items.push_back(std::pair<NSString *,IMenuItem *>(childName,new MenuItem::RadioButton(childName,rbLabelArr,rbVal,parentName)));
                        IMenuItem *tmp = _items[_items.size()-1].second;
                        NSMenuItem *childItem = tmp->item();
                        [this->_context addItem:childItem];
                        _item.push_back(MenuItemList(childName,childItem));
                    }
                    else {
                        NSLog(@"Error : label must be at least 2");
                    }
                }
                else {
                    NSLog(@"Error : label must be at least 2");
                }
            }
            else if(type==MenuType::BUTTON) {
                _items.push_back(std::pair<NSString *,IMenuItem *>(childName,new MenuItem::Button(childName)));
                IMenuItem *tmp = _items[_items.size()-1].second;
                NSMenuItem *childItem = tmp->item();
                [this->_context addItem:childItem];
                _item.push_back(MenuItemList(childName,childItem));
            }
            else if(type==MenuType::SLIDER) {
                double sliderMin = (dict[@"min"]!=nil)?[dict[@"min"] doubleValue]:0;
                double sliderMax = (dict[@"max"]!=nil)?[dict[@"max"] doubleValue]:1.0;
                if(sliderMin>sliderMax) { double tmp = sliderMin; sliderMin = sliderMax; sliderMax = tmp; }
                double sliderVal = (dict[@"value"]!=nil)?[dict[@"value"] doubleValue]:0.0;
                if(sliderVal<sliderMin) sliderVal = sliderMin;
                if(sliderVal>sliderMax) sliderVal = sliderMax;
                bool isLabel = true;
                if(dict[@"label"]!=nil&&[dict[@"label"] boolValue]==NO) isLabel = false;
                _items.push_back(std::pair<NSString *,IMenuItem *>(childName,new MenuItem::Slider(childName,sliderMin,sliderMax,sliderVal,isLabel,parentName)));
                IMenuItem *tmp = _items[_items.size()-1].second;
                NSMenuItem *childItem = tmp->item();
                [this->_context addItem:childItem];
                _item.push_back(MenuItemList(childName,childItem));
            }
        }
        return this;
    }
    
    Menu *addItem(NSString* parentName,NSString *childName,MenuType type,NSString *json=@"{}") {
        
        if(itemExists(childName)) {
            NSLog(@"Error : %@ is already defined",childName);
            return this;
        }
        
        IMenuItem *tmp = nil;
        
        long len = this->_items.size();
        for(int k=0; k<len; k++) {
            if([parentName isEqualToString:this->_items[k].first]) {
                tmp = _items[k].second;
                if(tmp->type()==MenuType::TEXT) {
                    //NSLog(@"parent : %@",parentName);
                    break;
                }
                else {
                    
                    NSLog(@"Error : %@ is not MenuType::TEXT",parentName);
                    return this;
                }
            }
        }
        
        NSMenu *menu = getMenu(parentName);
        if(menu==nil) {
            
            if(tmp) {
                tmp->unbind();
                menu  = [[NSMenu alloc] init];
                [menu setTitle:parentName];
                [tmp->item() setSubmenu:menu];
                _menu.push_back(MenuList(parentName,menu));
            }
            else {
                NSLog(@"Error : %@ is not found",parentName);
                return this;
            }
        }
        this->_context = menu;
        return addItem(childName,type,json,parentName);
    }
    
    Menu *reset(NSString *key) {
        
        long len = this->_items.size();
        for(int k=0; k<len; k++) {
            if([key isEqualToString:this->_items[k].first]) {
                (_items[k].second)->reset();
                break;
            }
        }
        return this;
    }
    
    int value(NSString *key) {
        long len = this->_items.size();
        for(int k=0; k<len; k++) {
            if([key isEqualToString:this->_items[k].first]) {
                return _items[k].second->value();
            }
        }
        return 0;
    }
    
    
    Menu *value(NSString *key, double v) {
     
         long len = this->_items.size();
         for(int k=0; k<len; k++) {
             if([key isEqualToString:this->_items[k].first]) {
                 _items[k].second->value(v);
                 break;
            }
        }
        return this;
     }
     
    
    Menu *enabled(NSString *key, bool b) {
        
        long len = this->_items.size();
        for(int k=0; k<len; k++) {
            if([key isEqualToString:this->_items[k].first]) {
                //NSLog(@"%@",key);
                (b)?(_items[k].second)->on():(_items[k].second)->off();
                break;
            }
        }
        return this;
    }
    
    
    
    
};
