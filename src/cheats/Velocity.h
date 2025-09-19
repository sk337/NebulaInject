
//
// Velocity.h
//

#ifndef PHANTOM_VELOCITY_H
#define PHANTOM_VELOCITY_H

#include "Cheat.h"
#include "../Phantom.h"
#include <net/minecraft/client/Minecraft.h>

class Velocity : public Cheat {
public:
    enum class Method {
        SCALE,
        CLAMP,
        OVERRIDE
    };

    // Constructor matching the .cpp
    explicit Velocity(Phantom *phantom);

    void run(Minecraft *mc) override;
    void reset(Minecraft *mc) override;
    void renderSettings() override;

private:
    Phantom *phantom;

    float horizontalMotion;
    float verticalMotion;
    float chance;

    Method method;
};

#endif // PHANTOM_VELOCITY_H

