//
// AutoClicker.h — improved version
//
#ifndef PHANTOM_AUTOCLICKER_H
#define PHANTOM_AUTOCLICKER_H
#include "../Phantom.h"
#include "../utils/MSTimer.h"
#include "Cheat.h"

class AutoClicker : public Cheat {
public:
  explicit AutoClicker();
  ~AutoClicker();

  // Override base class methods
  virtual void run(Minecraft *mc) override;
  virtual void reset(Minecraft *mc) override;
  virtual void renderSettings() override;

private:
  void updateValues();

  // Timers
  MSTimer *clickTimer;
  MSTimer *eventTimer;
  MSTimer *releaseTimer;
  MSTimer *burstTimer;
  bool isHolding;
  bool shouldClick;

  // Core options
  float cps; // target clicks per second
  bool onlyInGame;
  bool mineBlocks; // only click when pointing at a block (or any hit if false)
  bool onlySword;  // only click while holding a sword

  // Advanced/randomization
  float holdLength;       // fraction of interval to hold (0..0.99)
  float holdLengthRandom; // +/- variance multiplier for hold length (0..0.5)
  float jitterMs;    // additional small random ms added to each click delay
  bool randomizeCps; // allows per-click small CPS variance
  float cpsVariance; // variance fraction of CPS (eg 0.05 => +/-5%)

  // Wayland/uinput support
  int uinput_fd;

  // State tracking members
  bool shouldRelease;
  bool keyPressed;
  int releaseDelayMs;
  int burstCount;

  // Helper methods
  void initializeUInput();
  void cleanup();
  void sendKeyEvent(int key, int value);
  void handleKeyRelease(Minecraft *mc);
  void handleBurstMode(Minecraft *mc);
  void performClick(Minecraft *mc);

  // Burst mode
  bool burstMode;
  int burstClicks;  // clicks per burst
  int burstDelayMs; // delay between bursts

  // Timing / state
  int nextDelay;  // computed ms until next click
  int eventDelay; // base ms between event changes
  int nextEventDelay;
  float dropChance;
  float spikeChance;
  float spikeMultiplier; // how much to multiply during spike
  bool isSpiking;
  bool isDropping;
  bool showAdvanced;

  // internal
  bool pendingBurst; // currently executing a burst
};

#endif // PHANTOM_AUTOCLICKER_H
