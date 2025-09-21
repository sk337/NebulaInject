#ifndef PHANTOM_MOVINGOBJECTPOSITION_H
#define PHANTOM_MOVINGOBJECTPOSITION_H

#include "../../../../AbstractClass.h"
#include "Vec3.h"

class MovingObjectPosition : public AbstractClass {
public:
    jobject movingObjectPosition;

MovingObjectPosition(Phantom *phantom, jobject movingObjectPosition);
// previously: jobject getObject() const;
jobject getRawObject() const;

    bool isNull() const;
bool isBlock() const;
    // returns hitVec as a Vec3 wrapper
    Vec3 getHitVecContainer() const;
jobject getMovingObjectPosition();
private:
    jfieldID fdHitVec;
    jobject obj;
};

#endif // PHANTOM_MOVINGOBJECTPOSITION_H
