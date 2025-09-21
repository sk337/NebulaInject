//
// AutoClicker.cpp — cached Display, burst worker, no sword checks
// (fixed build errors related to XUtils::DeviceState::buttonStates being a bool*)
//
#include "AutoClicker.h"

#include <thread>
#include <algorithm>
#include <chrono>
#include "../utils/MathHelper.h"
#include "../utils/ImGuiUtils.h"
#include "../utils/XUtils.h"
#include <imgui.h>
#include <unistd.h> // close

AutoClicker::AutoClicker() : Cheat("AutoClicker", "Clicks 4 u (so ur hand doesn't break)") {
    // Defaults
    cps = 12.0f;
    onlyInGame = true;
    mineBlocks = true;

    clickTimer = std::make_unique<MSTimer>();
    eventTimer = std::make_unique<MSTimer>();
    releaseTimer = std::make_unique<MSTimer>();
    burstTimer = std::make_unique<MSTimer>();

    nextDelay = 0;
    eventDelay = 350;
    nextEventDelay = static_cast<int>(MathHelper::randFloat(0.8f, 1.2f) * (float)eventDelay);

    dropChance = 0.3f;
    spikeChance = 0.2f;
    spikeMultiplier = 1.15f;

    holdLength = 0.15f;
    holdLengthRandom = 0.05f;
    jitterMs = 2.0f;

    burstMode = false;
    burstClicks = 3;
    burstDelayMs = 150;

    showAdvanced = false;
    randomizeCps = true;
    cpsVariance = 0.1f;

    uinput_fd = -1;
    shouldRelease = false;
    keyPressed = false;
    releaseDelayMs = 50;

    // Initialize cached display and burst worker
    initializeDisplay();
    initializeUInput();

    // Start burst worker
    burstWorkerRunning = true;
    burstThread = std::thread(&AutoClicker::burstWorkerLoop, this);
}

AutoClicker::~AutoClicker() {
    // Stop burst worker
    burstWorkerRunning = false;
    {
        std::lock_guard<std::mutex> lk(burstMutex);
        burstCv.notify_all();
    }
    if (burstThread.joinable()) burstThread.join();

    cleanup();
    // unique_ptr timers will clean up
}

void AutoClicker::initializeDisplay() {
    // Cache a single Display* for the process (avoid opening/closing every tick).
    // If XOpenDisplay fails, we'll leave xDisplay == nullptr and treat device as unusable.
    if (!xDisplay) {
        xDisplay = XOpenDisplay(nullptr);
    }
}

void AutoClicker::initializeUInput() {
    // stub for uinput; keep invalid fd unless implemented
    uinput_fd = -1;
}

void AutoClicker::cleanup() {
    // close uinput if used
    if (uinput_fd != -1) {
        ::close(uinput_fd);
        uinput_fd = -1;
    }
    // close cached X display if we opened it
    if (xDisplay) {
        XCloseDisplay(xDisplay);
        xDisplay = nullptr;
    }
}

void AutoClicker::reset(Minecraft *mc) {
    (void)mc; // suppress unused parameter warning
    isHolding = false;
    shouldClick = false;
    isSpiking = false;
    isDropping = false;
    if (clickTimer) clickTimer->reset();
    if (eventTimer) eventTimer->reset();
    if (releaseTimer) releaseTimer->reset();
    if (burstTimer) burstTimer->reset();
    // flush burst queue
    {
        std::lock_guard<std::mutex> lk(burstMutex);
        burstQueue.clear();
    }
}

void AutoClicker::run(Minecraft *mc) {
    if (!mc) return;

    // only in game
    if (onlyInGame && !mc->isInGameHasFocus()) {
        isHolding = false;
        return;
    }

    // mineBlocks: stop clicking while pointing at a block
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

    // Ensure display available (try once if not)
    if (!xDisplay) {
        initializeDisplay();
        if (!xDisplay) {
            // can't read mouse state
            isHolding = false;
            return;
        }
    }

    // Get mouse state (cached Display*)
    XUtils::DeviceState mouseState = XUtils::getDeviceState(xDisplay, XUtils::mouseDeviceID);

    // Validate device state: buttonStates is a bool* in the DeviceState struct
    if (mouseState.numButtons <= 1 || mouseState.buttonStates == nullptr) {
        XUtils::isDeviceShit = true;
        isHolding = false;
        return;
    } else {
        XUtils::isDeviceShit = false;
    }

    // Determine left button held (defensive index handling)
    bool leftButtonHeld = false;
    if (mouseState.buttonStates != nullptr) {
        // Many wrappers put left at index 1; some at 0. Prefer index 1 when available.
        if (mouseState.numButtons > 1) {
            leftButtonHeld = mouseState.buttonStates[1];
        } else if (mouseState.numButtons > 0) {
            leftButtonHeld = mouseState.buttonStates[0];
        }
    }

    // Transitions
    if (leftButtonHeld && !isHolding.load()) {
        isHolding = true;
        clickTimer->reset();
        shouldClick = true;
    } else if (!leftButtonHeld && isHolding.load()) {
        isHolding = false;
        shouldClick = false;
        // clear any pending bursts
        std::lock_guard<std::mutex> lk(burstMutex);
        burstQueue.clear();
        return;
    } else if (!leftButtonHeld) {
        isHolding = false;
        shouldClick = false;
        return;
    }

    // If it's time to click
    if (isHolding && shouldClick && clickTimer->hasTimePassed(nextDelay)) {
        clickTimer->reset();
        updateValues();

        if (burstMode && burstClicks > 1) {
            // compute hold time for burst based on current timing
            int baseHold = static_cast<int>(nextDelay * holdLength);
            float randFactor = MathHelper::randFloat(1.0f - holdLengthRandom, 1.0f + holdLengthRandom);
            int holdTime = static_cast<int>(baseHold * randFactor);
            holdTime = std::clamp(holdTime, 10, std::max(10, nextDelay / 2));

            // queue a burst to be executed by the worker
            requestBurst(burstClicks, 50, holdTime); // 50ms spacing between burst clicks
            // ensure at least burstDelayMs before next auto-click (prevents immediate repeat)
            nextDelay = std::max(nextDelay, burstDelayMs);
        } else {
            performClick(mc);
        }
    }
}

void AutoClicker::requestBurst(int clicks, int spacingMs, int holdMs) {
    {
        std::lock_guard<std::mutex> lk(burstMutex);
        burstQueue.push_back({clicks, spacingMs, holdMs});
    }
    burstCv.notify_one();
}

void AutoClicker::burstWorkerLoop() {
    // Worker thread to process burst requests sequentially and with precise sleeps.
    while (burstWorkerRunning) {
        BurstRequest req;
        bool have = false;
        {
            std::unique_lock<std::mutex> lk(burstMutex);
            burstCv.wait(lk, [&]() { return !burstQueue.empty() || !burstWorkerRunning; });
            if (!burstWorkerRunning) break;
            if (!burstQueue.empty()) {
                req = burstQueue.front();
                burstQueue.erase(burstQueue.begin());
                have = true;
            }
        }

        if (!have) continue;

        // perform the requested number of clicks with spacing
        for (int i = 0; i < req.clicks && burstWorkerRunning; ++i) {
            // perform synchronous click (this thread will handle the click timing and hold)
            performClickImmediate(req.holdMs);

            if (i + 1 < req.clicks) {
                std::this_thread::sleep_for(std::chrono::milliseconds(req.spacingMs));
            }
        }
        // After a burst, small pause is fine (main run() already uses nextDelay adjustments)
    }
}

void AutoClicker::performClickImmediate(int holdMs) {
    // Directly call XUtils click synchronously (worker thread)
    // holdMs is expected to be reasonable (clamped by caller)
    XUtils::clickMouseXEvent(1, holdMs);
}

void AutoClicker::performClick(Minecraft *mc) {
    (void)mc; // suppress unused parameter warning
    // Non-blocking click: compute holdTime relative to the current nextDelay and spawn a thread
    int baseHold = static_cast<int>(nextDelay * holdLength);
    float randFactor = MathHelper::randFloat(1.0f - holdLengthRandom, 1.0f + holdLengthRandom);
    int holdTime = static_cast<int>(baseHold * randFactor);
    holdTime = std::clamp(holdTime, 10, std::max(10, nextDelay / 2));

    if (jitterMs > 0.0f) {
        int jitter = static_cast<int>(MathHelper::randFloat(0.0f, jitterMs));
        if (jitter > 0) std::this_thread::sleep_for(std::chrono::milliseconds(jitter));
    }

    std::thread([](int btn, int hold) { XUtils::clickMouseXEvent(btn, hold); }, 1, holdTime).detach();
}

void AutoClicker::renderSettings() {
    ImGui::SliderFloat("CPS", &cps, 4.0f, 20.0f);

    ImGui::Checkbox("Only in game", &onlyInGame);
    ImGui::SameLine();
    ImGuiUtils::drawHelper("If checked, this will only click when you are in game.");

    ImGui::Checkbox("Mine blocks only", &mineBlocks);
    ImGui::SameLine();
    ImGuiUtils::drawHelper("Stops clicking when looking at a block.");

    ImGui::Checkbox("Advanced Mode", &showAdvanced);

    if (showAdvanced) {
        if (ImGui::CollapsingHeader("Humanization")) {
            ImGui::Checkbox("Humanize CPS", &randomizeCps);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Adds natural variation to clicking speed.");

            if (randomizeCps) {
                ImGui::SliderFloat("CPS variance", &cpsVariance, 0.0f, 0.3f);
                ImGui::SameLine();
                ImGuiUtils::drawHelper("How much the CPS can vary (0.1 = ±10%).");
            }

            ImGui::SliderFloat("Hold Length", &holdLength, 0.0f, 0.99f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("How long to hold the button down relative to interval.");

            ImGui::SliderFloat("Hold Length Random", &holdLengthRandom, 0.0f, 0.5f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Add extra randomness to hold length.");

            ImGui::SliderFloat("Jitter (ms)", &jitterMs, 0.0f, 10.0f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Random timing jitter in milliseconds.");
        }

        if (ImGui::CollapsingHeader("Burst Mode")) {
            ImGui::Checkbox("Burst mode", &burstMode);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Occasionally send multiple clicks in rapid succession.");

            if (burstMode) {
                ImGui::SliderInt("Burst clicks", &burstClicks, 2, 10);
                ImGui::SliderInt("Burst delay (ms)", &burstDelayMs, 50, 1000);
            }
        }

        if (ImGui::CollapsingHeader("Events")) {
            ImGui::SliderInt("Event Delay", &eventDelay, 0, 10000);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("How long between mode changes.");

            ImGui::SliderFloat("Spike Chance", &spikeChance, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Chance to temporarily increase CPS.");

            ImGui::SliderFloat("Drop Chance", &dropChance, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Chance to temporarily decrease CPS.");

            ImGui::SliderFloat("Spike Multiplier", &spikeMultiplier, 1.0f, 2.0f);
            ImGui::SameLine();
            ImGuiUtils::drawHelper("Multiplier for CPS during spikes.");
        }

        if (ImGui::CollapsingHeader("Debug")) {
            ImGui::Text("Is Holding: %s", isHolding.load() ? "Yes" : "No");
            ImGui::Text("Should Click: %s", shouldClick.load() ? "Yes" : "No");
            ImGui::Text("Next Delay: %d ms", nextDelay);
            ImGui::Text("Is Spiking: %s", isSpiking.load() ? "Yes" : "No");
            ImGui::Text("Is Dropping: %s", isDropping.load() ? "Yes" : "No");
            ImGui::Text("Burst Mode: %s", burstMode ? "Yes" : "No");
        }
    }
}

void AutoClicker::updateValues() {
    // keep event probabilities sane
    float totalProb = spikeChance + dropChance;
    if (totalProb > 1.0f && totalProb > 0.0f) {
        spikeChance = spikeChance / totalProb;
        dropChance = dropChance / totalProb;
    }

    // change spike/drop state occasionally
    if (eventTimer->hasTimePassed(nextEventDelay)) {
        float r = MathHelper::randFloat(0.0f, 1.0f);
        if (r < spikeChance) {
            isSpiking = true;
            isDropping = false;
        } else if (r < spikeChance + dropChance) {
            isDropping = true;
            isSpiking = false;
        } else {
            isSpiking = false;
            isDropping = false;
        }
        eventTimer->reset();
        nextEventDelay = static_cast<int>(eventDelay * MathHelper::randFloat(0.8f, 1.2f));
    }

    // compute effective CPS
    float effectiveCPS = cps;
    if (isSpiking) {
        effectiveCPS *= MathHelper::randFloat(spikeMultiplier, spikeMultiplier + 0.05f);
    } else if (isDropping) {
        effectiveCPS *= MathHelper::randFloat(0.7f, 0.95f);
    }

    if (randomizeCps) {
        effectiveCPS *= MathHelper::randFloat(1.0f - cpsVariance, 1.0f + cpsVariance);
    }

    effectiveCPS = std::clamp(effectiveCPS, 1.0f, 25.0f);

    // build natural variation window
    float minCPS = std::max(1.0f, effectiveCPS - 2.0f);
    float maxCPS = effectiveCPS + 2.0f;

    float chosenCPS = MathHelper::randFloat(minCPS, maxCPS);
    nextDelay = static_cast<int>(1000.0f / chosenCPS);

    // jitter
    if (jitterMs > 0.0f) {
        nextDelay += static_cast<int>(MathHelper::randFloat(0.0f, jitterMs));
    }

    // clamp delay
    nextDelay = std::clamp(nextDelay, 10, 5000);
}
