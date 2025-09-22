
//
// ReachInput.cpp
//

#include "ReachInput.h"
#include "../Phantom.h"
#include "Reach.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <X11/Xlib.h>
#include "../utils/XUtils.h"

ReachInput::ReachInput(Reach *reachCheat)
    : Cheat("ReachInput", "Handles attack key for Reach"), reach(reachCheat)
{
    keyPressed.store(false);
    inputThreadRunning.store(true);
    xDisplay = XOpenDisplay(nullptr);

    inputThread = std::thread([this]() { this->inputLoop(); });
}

ReachInput::~ReachInput() {
    inputThreadRunning.store(false);
    if (inputThread.joinable()) inputThread.join();
    if (xDisplay) XCloseDisplay(xDisplay);
}

void ReachInput::run(Minecraft *mc) {
    (void)mc; // nothing per-tick needed here, input thread handles attack
}

void ReachInput::reset(Minecraft *mc) {
    (void)mc;
    keyPressed.store(false);
}

void ReachInput::inputLoop() {
    while (inputThreadRunning.load()) {
        if (!xDisplay) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }

        // Poll left mouse button (or change to whatever key you want)
        XUtils::DeviceState mouseState = XUtils::getDeviceState(xDisplay, XUtils::mouseDeviceID);
        bool leftDown = false;
        if (mouseState.numButtons > 1 && mouseState.buttonStates) {
            leftDown = mouseState.buttonStates[1]; // left button
        }

        if (leftDown && !keyPressed.load()) {
            keyPressed.store(true);
            if (reach) reach->onAttack();
        } else if (!leftDown && keyPressed.load()) {
            keyPressed.store(false);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
