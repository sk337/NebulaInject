//
// Created by somepineaple on 2/6/22.
//

#include "MovingObjectPosition.h"
#include "../entity/Entity.h"
MovingObjectPosition::MovingObjectPosition(Phantom *phantom, jobject movingObjectPosition) : AbstractClass(phantom, "MovingObjectPosition") {
    fdHitVec = getFieldID("hitVec");

    this->movingObjectPosition = movingObjectPosition;
}

jobject MovingObjectPosition::getHitVec() const {
    return getObject(movingObjectPosition, fdHitVec);
}

Vec3 MovingObjectPosition::getHitVecContainer() const {
    return Vec3(phantom, getHitVec());
}

jobject MovingObjectPosition::getMovingObjectPosition() {
    return movingObjectPosition;
}


bool MovingObjectPosition::isNull() const {
    // return true if the underlying jobject is null
    return movingObjectPosition == nullptr;
}

bool MovingObjectPosition::isBlock() const {
    // implement using JNI: check the type of the MOP
    return false; // placeholder
}

bool MovingObjectPosition::isEntity() const {
    // implement using JNI: check if MOP hit an entity
    return true; // placeholder
}

Entity MovingObjectPosition::getEntityHit() const {
    // return the hit entity if there is one
    return Entity(phantom, nullptr); // placeholder
}
