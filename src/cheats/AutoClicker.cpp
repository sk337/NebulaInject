//
// Created by somepineaple on 1/30/22.
//
#include "AutoClicker.h"
#include <thread>
#include "../utils/MathHelper.h"
#include "../utils/ImGuiUtils.h"
#include <imgui.h>
#include "../utils/XUtils.h"
#include "net/minecraft/client/Minecraft.h"
#include <net/minecraft/entity/EntityPlayerSP.h>
AutoClicker::AutoClicker() : Cheat("AutoClicker", "Clicks 4 u (so ur hand doesn't break)") {
    cps = 12.0f;
    onlyInGame = true;
    mineBlocks = true;
    onlySword = false;
    clickTimer = new MSTimer();
    eventTimer = new MSTimer();
    releaseTimer = new MSTimer();
    nextDelay = 0;
    eventDelay = 350;
    nextEventDelay = (int)(MathHelper::randFloat(0.8f, 1.2f) * (float)eventDelay);
    dropChance = 0.3f;
    spikeChance = 0.2f;
    spikeMultiplier = 1.15f;
    holdLength = 0.15f;
    holdLengthRandom = 0.05f;
    jitterMs = 2.0f;
    burstMode = false;
    burstClicks = 3;
    burstDelayMs = 150;
    isSpiking = false;
    isDropping = false;
    showAdvanced = false;
    pendingBurst = false;
    burstCount = 0;
    burstTimer = new MSTimer();
    isHolding = false;
    shouldClick = false;
    randomizeCps = true;
    cpsVariance = 0.1f;
}

AutoClicker::~AutoClicker() {
    // Clean up timers
    delete clickTimer;
    delete eventTimer;
    delete releaseTimer;
    delete burstTimer;
}

void AutoClicker::run(Minecraft *mc) {
    if (!mc) return;
    
    if (onlyInGame && !mc->isInGameHasFocus()) {
        isHolding = false;
        return;
    }

    auto player = mc->getPlayerContainer();
    if (onlySword && !player.isHoldingSword()) {
        isHolding = false;
        return;
    }

    // Stop clicking if looking at a block and mineBlocks is enabled
    if (mineBlocks) {
        jobject mopObj = mc->getObjectMouseOver();
        if (mopObj != nullptr) {
            MovingObjectPosition mop(mc->getPhantom(), mopObj);
            if (!mop.isNull()) {
                isHolding = false;
                return;
            }
        }
    }

    // Get mouse state
    Display *dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    
    XUtils::DeviceState mouseState = XUtils::getDeviceState(dpy, XUtils::mouseDeviceID);
    XCloseDisplay(dpy);
    
    if (mouseState.numButtons == 0) {
        XUtils::isDeviceShit = true;
        isHolding = false;
        return;
    } else {
        XUtils::isDeviceShit = false;
    }

    // Check if left mouse button is being held (button 1 in X11)
    bool leftButtonHeld = mouseState.buttonStates[1];
    
    if (leftButtonHeld && !isHolding) {
        // Just started holding - reset timer and start clicking
        isHolding = true;
        clickTimer->reset();
        shouldClick = true;
    } else if (!leftButtonHeld && isHolding) {
        // Stopped holding - stop clicking
        isHolding = false;
        shouldClick = false;
        pendingBurst = false;
        burstCount = 0;
        return;
    } else if (!leftButtonHeld) {
        // Not holding at all
        isHolding = false;
        shouldClick = false;
        return;
    }

    // Only click if we're holding and timer has passed
    if (isHolding && shouldClick && clickTimer->hasTimePassed(nextDelay)) {
        clickTimer->reset();
        updateValues();
        
        // Handle burst mode or single click
        if (burstMode && burstClicks > 1) {
            handleBurstMode(mc);
        } else {
            performClick(mc);
        }
    }
}

void AutoClicker::handleBurstMode(Minecraft *mc) {
    if (!pendingBurst) {
        // Start burst
        pendingBurst = true;
        burstCount = 0;
        burstTimer->reset();
    }

    if (burstCount < burstClicks && burstTimer->hasTimePassed(50)) { // 50ms between burst clicks
        performClick(mc);
        burstCount++;
        burstTimer->reset();
    }

    if (burstCount >= burstClicks) {
        pendingBurst = false;
        // Add burst delay to next regular delay
        nextDelay += burstDelayMs;
    }
}

void AutoClicker::performClick(Minecraft *mc) {
    // Calculate hold time
    int holdTime = (int)((float)nextDelay * holdLength * 
        MathHelper::randFloat(1.0f - holdLengthRandom, 1.0f + holdLengthRandom));
    
    // Ensure reasonable hold time (minimum 10ms, maximum half of next delay)
    holdTime = std::max(10, std::min(holdTime, nextDelay / 2));
    
    // Click in new detached thread so the delay doesn't affect other modules and frame times
    std::thread(XUtils::clickMouseXEvent, 1, holdTime).detach();
}

void AutoClicker::renderSettings() {
    ImGui::SliderFloat("CPS", &cps, 4.0f, 20.0f);
    
    ImGui::Checkbox("Only in game", &onlyInGame);
    ImGui::SameLine();
    ImGuiUtils::drawHelper("If checked, this will only click when you are in game, otherwise, this will click "
                           "anytime, on any window. You could go to clickspeedtest.net and check ur clicking speed if "
                           "this is not checked");

    ImGui::Checkbox("Mine blocks only", &mineBlocks);
    ImGui::SameLine();
    ImGuiUtils::drawHelper("Stops clicking when looking at a block");

    ImGui::Checkbox("Only click while holding sword", &onlySword);
    ImGui::SameLine();
    ImGuiUtils::drawHelper("Prevents auto-click when not holding a sword");

    ImGui::Checkbox("Advanced Mode", &showAdvanced);
    
    if (showAdvanced) {
        if (ImGui::CollapsingHeader("Humanization")) {
            ImGui::Checkbox("Humanize CPS", &randomizeCps);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Adds natural variation to clicking speed");
            
            if (randomizeCps) {
                ImGui::SliderFloat("CPS variance", &cpsVariance, 0.0f, 0.3f);
                ImGui::SameLine();
                ImGuiUtils::drawHelper("How much the CPS can vary (0.1 = ±10%)");
            }

            ImGui::SliderFloat("Hold Length", &holdLength, 0.0f, 0.99f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("How long to hold the button down for in terms of the delay before the next click");

            ImGui::SliderFloat("Hold Length Random", &holdLengthRandom, 0.0f, 0.5f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Add extra randomness to hold length");

            ImGui::SliderFloat("Jitter (ms)", &jitterMs, 0.0f, 10.0f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Random timing jitter in milliseconds");
        }

        if (ImGui::CollapsingHeader("Burst Mode")) {
            ImGui::Checkbox("Burst mode", &burstMode);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Occasionally send multiple clicks in rapid succession");
            
            if (burstMode) {
                ImGui::SliderInt("Burst clicks", &burstClicks, 2, 10);
                ImGui::SliderInt("Burst delay (ms)", &burstDelayMs, 50, 1000);
            }
        }

        if (ImGui::CollapsingHeader("Events")) {
            ImGui::SliderInt("Event Delay", &eventDelay, 0, 10000);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("How long between switching from regular to spiking to dropping in milliseconds");

            ImGui::SliderFloat("Spike Chance", &spikeChance, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Chance to temporarily increase CPS");

            ImGui::SliderFloat("Drop Chance", &dropChance, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Chance to temporarily decrease CPS");

            ImGui::SliderFloat("Spike Multiplier", &spikeMultiplier, 1.0f, 2.0f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("How much to multiply CPS during spikes");
        }

        // Debug info
        if (ImGui::CollapsingHeader("Debug")) {
            ImGui::Text("Is Holding: %s", isHolding ? "Yes" : "No");
            ImGui::Text("Should Click: %s", shouldClick ? "Yes" : "No");
            ImGui::Text("Next Delay: %d ms", nextDelay);
            ImGui::Text("Is Spiking: %s", isSpiking ? "Yes" : "No");
            ImGui::Text("Is Dropping: %s", isDropping ? "Yes" : "No");
            ImGui::Text("Burst Mode: %s", burstMode ? "Yes" : "No");
            if (burstMode) {
                ImGui::Text("Pending Burst: %s", pendingBurst ? "Yes" : "No");
                ImGui::Text("Burst Count: %d/%d", burstCount, burstClicks);
            }
        }
    }
}

void AutoClicker::updateValues() {
    // Clamp spike and drop chances
    if (spikeChance + dropChance > 1.0f) {
        spikeChance = 0.5f;
        dropChance = 0.5f;
    }

    // Update spike/drop state
    if (eventTimer->hasTimePassed(nextEventDelay)) {
        float randomFloat = MathHelper::randFloat(0.0f, 1.0f);
        if (randomFloat < spikeChance) {
            isSpiking = true;
            isDropping = false;
        } else if (randomFloat < spikeChance + dropChance) {
            isDropping = true;
            isSpiking = false;
        } else {
            isDropping = false;
            isSpiking = false;
        }
        eventTimer->reset();
        nextEventDelay = (int)((float)eventDelay * MathHelper::randFloat(0.8f, 1.2f));
    }

    // Calculate effective CPS
    float effectiveCPS = cps;
    
    // Apply spike/drop effects
    if (isSpiking) {
        effectiveCPS *= MathHelper::randFloat(spikeMultiplier, spikeMultiplier + 0.05f);
    } else if (isDropping) {
        effectiveCPS *= MathHelper::randFloat(0.7f, 0.95f);
    }
    
    // Apply CPS randomization
    if (randomizeCps) {
        effectiveCPS *= MathHelper::randFloat(1.0f - cpsVariance, 1.0f + cpsVariance);
    }

    // Ensure reasonable CPS bounds
    effectiveCPS = std::max(1.0f, std::min(effectiveCPS, 25.0f));

    // Calculate CPS range for more natural variation
    float minCPS = std::max(1.0f, effectiveCPS - 2.0f);
    float maxCPS = effectiveCPS + 2.0f;

    // Calculate next delay
    nextDelay = (int)(1000.0f / MathHelper::randFloat(minCPS, maxCPS));

    // Apply jitter
    if (jitterMs > 0.0f) {
        nextDelay += (int)MathHelper::randFloat(0.0f, jitterMs);
    }

    // Ensure reasonable delay bounds
    nextDelay = std::max(10, std::min(nextDelay, 5000));
}
