//
// AutoClicker.h — improved, Display cached, burst worker, no sword checks
//
#ifndef PHANTOM_AUTOCLICKER_H
#define PHANTOM_AUTOCLICKER_H

#include "../Phantom.h"
#include "../utils/MSTimer.h"
#include "Cheat.h"
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <X11/Xlib.h>

class AutoClicker : public Cheat {
public:
    explicit AutoClicker();
    ~AutoClicker();

    // Cheat overrides
    void run(Minecraft *mc) override;
    void reset(Minecraft *mc) override;
    void renderSettings() override;

private:
    // Initialization / cleanup
    void initializeDisplay();
    void initializeUInput();
    void cleanup();

    // Click logic
    void updateValues();
    void performClickImmediate(int holdMs); // performs a click synchronously (used by worker)
    void performClick(Minecraft *mc);       // non-blocking: spawns a thread for a single click

    // Burst worker
    struct BurstRequest { int clicks; int spacingMs; int holdMs; };
    void burstWorkerLoop();
    void requestBurst(int clicks, int spacingMs, int holdMs);

    // Timers
    std::unique_ptr<MSTimer> clickTimer;
    std::unique_ptr<MSTimer> eventTimer;
    std::unique_ptr<MSTimer> releaseTimer;
    std::unique_ptr<MSTimer> burstTimer;

    // State (atomic where accessed across threads)
    std::atomic<bool> isHolding{false};
    std::atomic<bool> shouldClick{false};
    std::atomic<bool> isSpiking{false};
    std::atomic<bool> isDropping{false};

    // Display caching
    Display *xDisplay{nullptr};

    // Burst threading
    std::thread burstThread;
    std::mutex burstMutex;
    std::condition_variable burstCv;
    std::vector<BurstRequest> burstQueue;
    std::atomic<bool> burstWorkerRunning{false};

    // Core options
    float cps; // target clicks per second
    bool onlyInGame;
    bool mineBlocks;

    // Humanization / randomness
    float holdLength;       // fraction of interval to hold (0..0.99)
    float holdLengthRandom; // +/- variance multiplier for hold length (0..0.5)
    float jitterMs;
    bool randomizeCps;
    float cpsVariance;

    // Burst mode
    bool burstMode;
    int burstClicks;
    int burstDelayMs;

    // Events
    int nextDelay;  // ms to next click
    int eventDelay;
    int nextEventDelay;
    float dropChance;
    float spikeChance;
    float spikeMultiplier;

    // Misc
    bool showAdvanced;
    int uinput_fd;
    bool shouldRelease;
    bool keyPressed;
    int releaseDelayMs;
};

#endif // PHANTOM_AUTOCLICKER_H
