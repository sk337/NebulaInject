#include "MovingObjectPosition.h"

MovingObjectPosition::MovingObjectPosition(Phantom *phantom, jobject mop)
    : AbstractClass(phantom, "MovingObjectPosition") {
    // mapping earlier defined field "hitVec"
    fdHitVec = getFieldID("hitVec");
    this->obj = mop;
}


jobject MovingObjectPosition::getRawObject() const {
    return obj;
}

Vec3 MovingObjectPosition::getHitVecContainer() const {
    if (!obj) return Vec3(phantom, nullptr);
    jobject v = getObject(obj, fdHitVec); // now unambiguous because no zero-arg getObject in this class
    return Vec3(phantom, v);
}


bool MovingObjectPosition::isNull() const {
    return obj == nullptr;
}
bool MovingObjectPosition::isBlock() const {
    if (!obj) return false;
    jclass cls = phantom->getEnv()->GetObjectClass(obj);
    jfieldID typeField = phantom->getEnv()->GetFieldID(cls, "typeOfHit", "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;");
    jobject typeObj = phantom->getEnv()->GetObjectField(obj, typeField);
    
    jclass typeCls = phantom->getEnv()->GetObjectClass(typeObj);
    jfieldID blockEnum = phantom->getEnv()->GetStaticFieldID(typeCls, "BLOCK", "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;");
    jobject blockVal = phantom->getEnv()->GetStaticObjectField(typeCls, blockEnum);

    return phantom->getEnv()->IsSameObject(typeObj, blockVal);
}

jobject MovingObjectPosition::getMovingObjectPosition() {
    return movingObjectPosition;
}
