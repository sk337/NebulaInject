//
// Created by somepineaple on 2/5/22.
//

#ifndef PHANTOM_ENTITY_H
#define PHANTOM_ENTITY_H

#include "../../../../AbstractClass.h"
#include <net/minecraft/util/Vec3.h>
#include <net/minecraft/util/AxisAlignedBB.h>
class Minecraft;
class EntityPlayerSP;

class Entity : public AbstractClass {
public:
    explicit Entity(Phantom *phantom, jobject entity);

    // Get the raw jobject
    jobject getEntity();

    // Position
    jdouble getPosX();
    jdouble getPosY();
    jdouble getPosZ();
    jdouble getLastTickPosX();
    jdouble getLastTickPosZ();

    // Entity info
    jint getId();
    const char* getName();
	void setPos(jdouble x, jdouble y, jdouble z);

    // Checks for ESP
    bool isPlayer();
    bool isLocalPlayer(Minecraft* mc);

    // Hitbox and interaction
    jfloat getEyeHeight();
    jobject rayTrace(jdouble distance, jfloat partialTicks);
    jobject getPositionEyes(jfloat partialTicks);
    jobject getLook(jfloat partialTicks);
    jobject getEntityBoundingBox();
    jfloat getCollisionBorderSize();
    jobject getRidingEntity();
    jboolean canRiderInteract();
    jboolean canBeCollidedWith();

    // Containers for easier usage
    Vec3 getPositionEyesContainer(jfloat partialTicks);
    Vec3 getLookContainer(jfloat partialTicks);
    AxisAlignedBB getEntityBoundingBoxContainer();
    Entity getRidingEntityContainer();

private:
    jobject entity;

    // Fields
    jfieldID fdPosX;
    jfieldID fdLastTickPosX;
    jfieldID fdPosY;
    jfieldID fdPosZ;
    jfieldID fdLastTickPosZ;
    jfieldID fdRidingEntity;

    // Methods
    jmethodID mdGetEyeHeight;
    jmethodID mdGetId;
    jmethodID mdGetName;
    jmethodID mdRayTrace;
    jmethodID mdGetPositionEyes;
    jmethodID mdGetLook;
    jmethodID mdGetEntityBoundingBox;
    jmethodID mdGetCollisionBorderSize;
    jmethodID mdCanRiderInteract;
    jmethodID mdCanBeCollodedWith;
};

#endif //PHANTOM_ENTITY_H
