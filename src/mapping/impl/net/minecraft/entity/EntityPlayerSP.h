//
// This code was copied from UDP-CPP: https://github.com/UnknownDetectionParty/UDP-CPP 
// Also was slighty edited by kirby
//

#ifndef PHANTOM_ENTITYPLAYERSP_H
#define PHANTOM_ENTITYPLAYERSP_H

#include "../../../../AbstractClass.h"
#include <net/minecraft/entity/Entity.h>


class Phantom;
class Minecraft;
class EntityPlayerSP : public AbstractClass {
public:
	EntityPlayerSP(Phantom *phantom, Minecraft *mc);

	jdouble getPosX();
	jdouble getPosY();
	jdouble getPosZ();

    jfloat getEyeHeight();

    jfloat getRotationYaw();
    jfloat getRotationPitch();

	jint getId();
	const char *getName();
    const char *getFormattedDisplayName();

    /* jobject getLook(jfloat partialTicks); */
    /* Vec3 getLookContainer(jfloat partialTicks); */
 jobject getHeldItem();

    // Returns true if holding any sword
    bool isHoldingSword();
	void setRotationYaw(jfloat yaw);
	void setRotationPitch(jfloat pitch);
	void setSprinting(jboolean sprinting);
	Entity asEntity();


    void setMotionY(jdouble motionY);

// Add this new method to allow setting X and Z together
void setMotionHorizontal(jdouble motionX, jdouble motionZ);
    // New setter for full motion
    void setMotion(jdouble motionX, jdouble motionY, jdouble motionZ);

    void setVelocity(jdouble horizontal, jdouble vertical);
    void setPosition(jdouble x, jdouble y, jdouble z);

    /* Vec3 getPositionVector(); */

    void setSelfWidth(jfloat width);
    void setSelfHeight(jfloat height);

    // Motion getters
    jdouble getMotionX();
    jdouble getMotionY();          // NEW
    jdouble getMotionZ();
    jdouble getHorizontalSpeed();

    jfloat getSelfWidth();
    jfloat getSelfHeight();

    jint getHurtTime();
    jint getMaxHurtTime();

    jobject getEntityPlayerSP();
private:
	jfieldID fdPosX;
	jfieldID fdPosY;
	jfieldID fdPosZ;
	jfieldID fdRotationYaw;
	jfieldID fdRotationPitch;
    jfieldID fdSelfWidth, fdSelfHeight;
    jfieldID fdMotionX, fdMotionY, fdMotionZ;
    jfieldID fdHurtTime, fdMaxHurtTime;
    jmethodID mdSetPosition;

jobject callObjectMethod(jobject obj, jmethodID method, ...);
    jmethodID mdSetVelocity;
    /* jmethodID mdGetLook; */
    /* jmethodID mdGetPositionVector; */
	jmethodID mdGetId;
   jfieldID fdInventory;     // InventoryPlayer
    jfieldID fdCurrentItem;   // int currentItem
    jmethodID mdGetStackInSlot; // ItemStack getStackInSlot(int)
	jmethodID mdGetName;
	jmethodID mdSetSprinting;
    jmethodID mdGetEyeHeight;
    jmethodID mdGetDisplayName;

    jmethodID mdIChatComponentGetFmtTxt;

	Minecraft *mc;
};

#endif //PHANTOM_ENTITYPLAYERSP_H
