//
// ReachInput.h — simple hold-to-attack for Reach
//
#ifndef PHANTOM_REACHINPUT_H
#define PHANTOM_REACHINPUT_H

#include <X11/Xlib.h>
#include "Cheat.h"
#include "../Phantom.h"

class Reach;

class ReachInput : public Cheat {
public:
    explicit ReachInput(Reach *reachCheat);
    ~ReachInput();

    void run(Minecraft *mc) override;
    void reset(Minecraft *mc) override;

private:
    Reach *reach;
    Display *xDisplay = nullptr;
};

#endif // PHANTOM_REACHINPUT_H
