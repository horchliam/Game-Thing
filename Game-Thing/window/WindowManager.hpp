//
//  WindowManager.hpp
//  Game-Thing
//
//  Created by Liam Horch on 1/27/23.
//

#ifndef WindowManager_hpp
#define WindowManager_hpp

class WindowManager {
public:
    WindowManager(unsigned int width,unsigned int height);
    bool SetupWindow();
private:
    unsigned int width;
    unsigned int height;
};

#endif /* WindowManager_hpp */
