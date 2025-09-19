//
// Created by somepineaple on 4/18/22.
//

#ifndef PHANTOM_KEYMANAGER_H
#define PHANTOM_KEYMANAGER_H

#include "../utils/XUtils.h"
#include "../Phantom.h"
#include <X11/Xlib.h>

class KeyManager {
public:
    KeyManager();
    ~KeyManager();
    
    // Disable copy constructor and assignment operator to prevent issues
    KeyManager(const KeyManager&) = delete;
    KeyManager& operator=(const KeyManager&) = delete;
    
    void updateKeys(Phantom *phantom);
    
private:
    XUtils::DeviceState previousState;
    Display *display;
};

void callUpdateKeys(KeyManager *manager, Phantom *phantom);

#endif //PHANTOM_KEYMANAGER_H
