
//
// ReachInput.h
//

#ifndef PHANTOM_REACHINPUT_H
#define PHANTOM_REACHINPUT_H

#include "Cheat.h"
#include <thread>
#include <atomic>
#include <X11/Xlib.h>

class Reach;

class ReachInput : public Cheat {
public:
    explicit ReachInput(Reach *reachCheat);
    ~ReachInput();

    void run(Minecraft *mc) override;
    void reset(Minecraft *mc) override;

private:
    Reach *reach;
    std::atomic<bool> keyPressed;
    std::atomic<bool> inputThreadRunning;
    std::thread inputThread;

    Display *xDisplay;

    void inputLoop();
};

#endif // PHANTOM_REACHINPUT_H
