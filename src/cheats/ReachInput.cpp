//
// ReachInput.cpp — simplified hold-to-attack
//
#include "ReachInput.h"
#include "Reach.h"
#include <X11/Xlib.h>
#include "../utils/XUtils.h"

ReachInput::ReachInput(Reach *reachCheat)
    : Cheat("ReachInput", "Handles attack key for Reach"), reach(reachCheat)
{
    xDisplay = XOpenDisplay(nullptr);
}

void ReachInput::run(Minecraft *mc) {
    if (!xDisplay || !reach) return;

    // Poll left mouse button
    XUtils::DeviceState mouseState = XUtils::getDeviceState(xDisplay, XUtils::mouseDeviceID);
    bool leftDown = false;

    if (mouseState.numButtons > 0 && mouseState.buttonStates)
        leftDown = mouseState.buttonStates[0]; // left click = index 0

    // Fire attack every tick while left is held
    if (leftDown)
        reach->onAttack();
}

void ReachInput::reset(Minecraft *mc) {
    (void)mc;
}
