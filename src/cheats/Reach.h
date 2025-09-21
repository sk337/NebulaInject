//
// Reach.h — corrected AABB usage, safe debug (no ImGui in run)
//
#ifndef PHANTOM_REACH_H
#define PHANTOM_REACH_H

#include "Cheat.h"
#include "../Phantom.h"

class Reach : public Cheat {
public:
    explicit Reach(Phantom *phantom);
    ~Reach() = default;

    void renderSettings() override;
    void run(Minecraft *mc) override;

private:
    Phantom *phantom;

    // Options
    bool enabled;
    float reach;
    float minReach;
 int lastNearbyCount;
    bool usedFallback;
    // Debug only (updated in run(), displayed in renderSettings())
    bool showDebug;
    bool lastFound;
    double lastDistance;      // distance to the chosen hit (or -1)
    bool lastPointedEntity;   // whether EntityRenderer ended with a pointed entity
};
#endif // PHANTOM_REACH_H
