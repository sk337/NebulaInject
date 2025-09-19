//
// Created by somepineaple on 4/18/22.
//
#include "KeyManager.h"
#include <imgui.h>
#include <cstdlib>
#include <cstring>

KeyManager::KeyManager() : display(nullptr) {
    previousState.keyStates = nullptr;
    previousState.numKeys = 0;
    
    // Initialize display connection once
    display = XOpenDisplay(nullptr);
    if (!display) {
        // Handle error - could throw exception or set error flag
        return;
    }
}

KeyManager::~KeyManager() {
    if (previousState.keyStates) {
        free(previousState.keyStates);
        previousState.keyStates = nullptr;
    }
    
    if (display) {
        XCloseDisplay(display);
        display = nullptr;
    }
}

void KeyManager::updateKeys(Phantom *phantom) {
    if (!display) {
        return; // Can't proceed without valid display connection
    }
    
    // Add error handling for getDeviceState
    XUtils::DeviceState currentState;
    try {
        currentState = XUtils::getDeviceState(display, XUtils::keyboardDeviceID);
    } catch (...) {
        return; // Handle error gracefully
    }
    
    // Validate currentState
    if (currentState.keyStates == nullptr || currentState.numKeys <= 0) {
        return;
    }
    
    // Basic check to see if we didn't just change input devices
    if (previousState.keyStates != nullptr && 
        previousState.numKeys == currentState.numKeys) {
        for (int i = 0; i < currentState.numKeys; i++) {
            if (previousState.keyStates[i] != currentState.keyStates[i]) {
                if (currentState.keyStates[i]) { // Key pressed
                    phantom->onKey(i);
                }
            }
        }
    }
    
    // Reallocate if necessary
    if (previousState.keyStates == nullptr || 
        previousState.numKeys != currentState.numKeys) {
        
        // Free old memory
        if (previousState.keyStates) {
            free(previousState.keyStates);
        }
        
        // Allocate new memory
        previousState.keyStates = static_cast<bool*>(
            malloc(currentState.numKeys * sizeof(bool))
        );
        
        if (!previousState.keyStates) {
            previousState.numKeys = 0;
            return; // Memory allocation failed
        }
    }
    
    previousState.numKeys = currentState.numKeys;
    memcpy(previousState.keyStates, currentState.keyStates, 
           currentState.numKeys * sizeof(bool));
    
    // Clean up currentState if it was dynamically allocated
    // (This depends on how XUtils::getDeviceState works)
}

void callUpdateKeys(KeyManager *manager, Phantom *phantom) {
    if (manager && phantom) {
        manager->updateKeys(phantom);
    }
}
