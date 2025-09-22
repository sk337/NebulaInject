//
// Reach.h — robust reach with positional fallback and debug counters
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
   void onAttack(); // Remove the Minecraft parameter
    virtual void reset(Minecraft *mc) override;
private:
    Phantom *phantom;
 jobject currentTarget = nullptr;
    double currentDistance = 0.0;
    // options
    bool enabled;
    float reach;
    float minReach;
    bool showDebug;
 static bool dumpedMethods; // tracks whether we've dumped
	void dumpPlayerControllerHeader(JNIEnv* env, jobject mc);
bool isAttacking;
    void performAttack(Minecraft *mc);
    // per-tick debug/state (run() updates these; renderSettings reads them)
    bool lastFound;
    double lastDistance;
    bool lastPointedEntity;

    // extended debug counters
    int lastNearbyCount;       // number of items returned from AABB query
    bool usedFallback;         // true if we scanned full entity list
    int lastEntityListCount;   // total entity list size when fallback used
    int lastCandidatesChecked; // how many candidate entities we actually checked
	//
 int lastEntityCount = 0;           // how many entities were in the query range
    bool lastHitVecWasNull = false;    // whether a found MOP had null hitVec
    bool lastVar9Fallback = false;     // whether fallback to eye position was used for var9
    bool lastMopCtorFound = false;     // whether MovingObjectPosition ctor was found
    bool lastAttackMethodFound = false;// whether PlayerController.attackEntity method was found
    bool lastAttackCalled = false;     // whether we called attackEntity
    std::string lastFailureMsg;        // optional failure text for debug
};
#endif // PHANTOM_REACH_H
