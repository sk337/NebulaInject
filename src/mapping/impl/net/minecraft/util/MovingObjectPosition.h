//
// Created by somepineaple on 2/6/22.
//

#ifndef PHANTOM_MOVINGOBJECTPOSITION_H
#define PHANTOM_MOVINGOBJECTPOSITION_H


#include "../../../../AbstractClass.h"
#include <net/minecraft/util/Vec3.h>


class MovingObjectPosition : public AbstractClass {
public:
    MovingObjectPosition(Phantom *phantom, jobject movingObjectPosition);

    jobject getHitVec();
    Vec3 getHitVecContainer();
    jobject getMovingObjectPosition();

    // New
    jint getTypeOfHit();  
    bool isBlock();
    bool isEntity();
    bool isMiss();

private:
    jfieldID fdHitVec;
    jfieldID fdTypeOfHit; // new
    jobject movingObjectPosition;
};

#endif //PHANTOM_MOVINGOBJECTPOSITION_H
