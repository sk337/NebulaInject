#include "MovingObjectPosition.h"

MovingObjectPosition::MovingObjectPosition(Phantom *phantom, jobject movingObjectPosition)
    : AbstractClass(phantom, "MovingObjectPosition") {
    fdHitVec = getFieldID("hitVec");
    fdTypeOfHit = getFieldID("typeOfHit"); // maps to the enum field
    this->movingObjectPosition = movingObjectPosition;
}

jobject MovingObjectPosition::getHitVec() {
    return getObject(movingObjectPosition, fdHitVec);
}

Vec3 MovingObjectPosition::getHitVecContainer() {
    return Vec3(phantom, getHitVec());
}

jobject MovingObjectPosition::getMovingObjectPosition() {
    return movingObjectPosition;
}

jint MovingObjectPosition::getTypeOfHit() {
    return getInt(movingObjectPosition, fdTypeOfHit);
}

bool MovingObjectPosition::isBlock() {
    return getTypeOfHit() == 1; // In MCP: BLOCK = 1
}

bool MovingObjectPosition::isEntity() {
    return getTypeOfHit() == 2; // ENTITY = 2
}

bool MovingObjectPosition::isMiss() {
    return getTypeOfHit() == 0; // MISS = 0
}
