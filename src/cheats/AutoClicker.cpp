//
// AutoClicker.cpp — cached Display, burst worker, no sword checks
//
#include "AutoClicker.h"

#include <thread>
#include <algorithm>
#include <chrono>
#include <imgui.h>
#include <unistd.h> // close

#include "../utils/MathHelper.h"
#include "../utils/ImGuiUtils.h"
#include "../utils/XUtils.h"
#include "net/minecraft/client/Minecraft.h"
#include <net/minecraft/entity/EntityPlayerSP.h>

AutoClicker::AutoClicker() : Cheat("AutoClicker", "Clicks 4 u (so ur hand doesn't break)") {
    cps = 12.0f;
    onlyInGame = true;
    mineBlocks = true;

    clickTimer   = std::make_unique<MSTimer>();
    eventTimer   = std::make_unique<MSTimer>();
    releaseTimer = std::make_unique<MSTimer>();
    burstTimer   = std::make_unique<MSTimer>();

    nextDelay     = 0;
    eventDelay    = 350;
    nextEventDelay = static_cast<int>(MathHelper::randFloat(0.8f, 1.2f) * (float)eventDelay);

    dropChance     = 0.3f;
    spikeChance    = 0.2f;
    spikeMultiplier = 1.15f;

    holdLength       = 0.15f;
    holdLengthRandom = 0.05f;
    jitterMs         = 2.0f;

    burstMode    = false;
    burstClicks  = 3;
    burstDelayMs = 150;

    showAdvanced = false;
    randomizeCps = true;
    cpsVariance  = 0.1f;

    uinput_fd     = -1;
    shouldRelease = false;
    keyPressed    = false;
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
}

void AutoClicker::initializeDisplay() {
    if (!xDisplay) {
        xDisplay = XOpenDisplay(nullptr);
    }
}

void AutoClicker::initializeUInput() {
    uinput_fd = -1;
}

void AutoClicker::cleanup() {
    if (uinput_fd != -1) {
        ::close(uinput_fd);
        uinput_fd = -1;
    }
    if (xDisplay) {
        XCloseDisplay(xDisplay);
        xDisplay = nullptr;
    }
}

void AutoClicker::reset(Minecraft *mc) {
    (void)mc;
    isHolding   = false;
    shouldClick = false;
    isSpiking   = false;
    isDropping  = false;

    if (clickTimer) clickTimer->reset();
    if (eventTimer) eventTimer->reset();
    if (releaseTimer) releaseTimer->reset();
    if (burstTimer) burstTimer->reset();

    {
        std::lock_guard<std::mutex> lk(burstMutex);
        burstQueue.clear();
    }
}

void AutoClicker::run(Minecraft *mc) {
    if (!mc) return;

    if (onlyInGame && !mc->isInGameHasFocus()) {
        isHolding = false;
        return;
    }

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

    if (!xDisplay) {
        initializeDisplay();
        if (!xDisplay) {
            isHolding = false;
            return;
        }
    }

    XUtils::DeviceState mouseState = XUtils::getDeviceState(xDisplay, XUtils::mouseDeviceID);

    if (mouseState.numButtons <= 1 || mouseState.buttonStates == nullptr) {
        XUtils::isDeviceShit = true;
        isHolding = false;
        return;
    } else {
        XUtils::isDeviceShit = false;
    }

    bool leftButtonHeld = false;
    if (mouseState.numButtons > 1) {
        leftButtonHeld = mouseState.buttonStates[1];
    } else if (mouseState.numButtons > 0) {
        leftButtonHeld = mouseState.buttonStates[0];
    }

    if (leftButtonHeld && !isHolding.load()) {
        isHolding = true;
        clickTimer->reset();
        shouldClick = true;
    } else if (!leftButtonHeld) {
        isHolding = false;
        shouldClick = false;
        std::lock_guard<std::mutex> lk(burstMutex);
        burstQueue.clear();
        return;
    }

    if (isHolding && shouldClick && clickTimer->hasTimePassed(nextDelay)) {
        clickTimer->reset();
        updateValues();

        if (burstMode && burstClicks > 1) {
            int baseHold  = static_cast<int>(nextDelay * holdLength);
            float randFac = MathHelper::randFloat(1.0f - holdLengthRandom, 1.0f + holdLengthRandom);
            int holdTime  = static_cast<int>(baseHold * randFac);
            holdTime      = std::clamp(holdTime, 10, std::max(10, nextDelay / 2));

            requestBurst(burstClicks, 50, holdTime);
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

        for (int i = 0; i < req.clicks && burstWorkerRunning; ++i) {
            performClickImmediate(req.holdMs);
            if (i + 1 < req.clicks) {
                std::this_thread::sleep_for(std::chrono::milliseconds(req.spacingMs));
            }
        }
    }
}

void AutoClicker::performClickImmediate(int holdMs) {
    XUtils::clickMouseXEvent(1, holdMs);
}

void AutoClicker::performClick(Minecraft *mc) {
    (void)mc;
    int baseHold  = static_cast<int>(nextDelay * holdLength);
    float randFac = MathHelper::randFloat(1.0f - holdLengthRandom, 1.0f + holdLengthRandom);
    int holdTime  = static_cast<int>(baseHold * randFac);
    holdTime      = std::clamp(holdTime, 10, std::max(10, nextDelay / 2));

    if (jitterMs > 0.0f) {
        int jitter = static_cast<int>(MathHelper::randFloat(0.0f, jitterMs));
        if (jitter > 0) std::this_thread::sleep_for(std::chrono::milliseconds(jitter));
    }

    std::thread([](int btn, int hold) { XUtils::clickMouseXEvent(btn, hold); }, 1, holdTime).detach();
}
