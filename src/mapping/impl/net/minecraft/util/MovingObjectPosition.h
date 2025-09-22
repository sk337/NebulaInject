#ifndef PHANTOM_MOVINGOBJECTPOSITION_H
#define PHANTOM_MOVINGOBJECTPOSITION_H

#include "../../../../AbstractClass.h"
#include "Vec3.h"

// forward declare to avoid recursive include
class Entity;

class MovingObjectPosition : public AbstractClass {
public:
    jobject movingObjectPosition;

    MovingObjectPosition(Phantom *phantom, jobject movingObjectPosition);
    jobject getRawObject() const;
jobject getHitVec() const;
    bool isNull() const;
    bool isBlock() const;
    bool isEntity() const;
    Entity getEntityHit() const;   // only needs forward declaration here

    Vec3 getHitVecContainer() const;
    jobject getMovingObjectPosition();

private:
    jfieldID fdHitVec;
    jobject obj;
};

#endif // PHANTOM_MOVINGOBJECTPOSITION_H
