//
// Some of this code was copied from UDP-CPP: https://github.com/UnknownDetectionParty/UDP-CPP
//

#include "Minecraft.h"

#include <net/minecraft/entity/EntityPlayerSP.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>

Minecraft::Minecraft(Phantom *phantom) : AbstractClass::AbstractClass(phantom, "Minecraft") {
    smdGetMinecraft = getMethodID("getMinecraft");

    fdPlayer = getFieldID("player");
    fdWorld = getFieldID("world");
    fdObjectMouseOver = getFieldID("objectMouseOver");
    fdPointedEntity = getFieldID("pointedEntity");
    fdGameSettings = getFieldID("gameSettings");
    fdInGameHasFocus = getFieldID("inGameHasFocus");
    fdTimer = getFieldID("timer");
    fdEntityRenderer = getFieldID("entityRenderer");
    fdRightClickDelayTimer = getFieldID("rightClickDelayTimer");
    fdLeftClickMouse = getFieldID("leftClickMouse");

    mdGetRenderViewEntity = getMethodID("getRenderViewEntity");

    // init reflection backup
    reflectiveObjectMouseOverField = nullptr;
    useReflectionForObjectMouseOver = false;

    // If the direct field lookup failed (likely due to obfuscation), try reflection
    if (fdObjectMouseOver == 0) {
        // Try to get a JNIEnv* from the Phantom or AbstractClass helper
        JNIEnv *env = phantom->getEnv(); // adjust if your Phantom API name differs
        if (env) {
            jclass mcClass = env->FindClass("net/minecraft/client/Minecraft");
            if (mcClass) {
                findObjectMouseOverFieldViaReflection(env, mcClass);
                env->DeleteLocalRef(mcClass);
            }
        }
    }
}

void Minecraft::findObjectMouseOverFieldViaReflection(JNIEnv *env, jclass mcClass) {
    // Use reflection: Class.getDeclaredFields() and inspect each field's type to find a field
    // whose type has a "typeOfHit" member (matching MovingObjectPosition)
    jclass classClass = env->FindClass("java/lang/Class");
    jclass fieldClass = env->FindClass("java/lang/reflect/Field");
    if (!classClass || !fieldClass) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        return;
    }

    jmethodID getDeclaredFieldsMID = env->GetMethodID(classClass, "getDeclaredFields", "()[Ljava/lang/reflect/Field;");
    jmethodID setAccessibleMID = env->GetMethodID(fieldClass, "setAccessible", "(Z)V");
    jmethodID getNameMID = env->GetMethodID(fieldClass, "getName", "()Ljava/lang/String;");
    jmethodID getTypeMID = env->GetMethodID(fieldClass, "getType", "()Ljava/lang/Class;");

    if (!getDeclaredFieldsMID || !setAccessibleMID || !getNameMID || !getTypeMID) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        return;
    }

    jobjectArray fields = (jobjectArray) env->CallObjectMethod(mcClass, getDeclaredFieldsMID);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return;
    }

    jsize len = env->GetArrayLength(fields);
    for (jsize i = 0; i < len; ++i) {
        jobject field = env->GetObjectArrayElement(fields, i);

        // Get the field's type (Class)
        jobject typeClass = env->CallObjectMethod(field, getTypeMID);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(field);
            continue;
        }

        // Try to find a declared field named "typeOfHit" on the typeClass.
        // If it exists, this type looks like MovingObjectPosition.
        jmethodID getDeclaredFieldMID = env->GetMethodID(classClass, "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;");
        if (!getDeclaredFieldMID) {
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(typeClass);
            env->DeleteLocalRef(field);
            continue;
        }

        jstring typeOfHitName = env->NewStringUTF("typeOfHit");
        jobject found = env->CallObjectMethod(typeClass, getDeclaredFieldMID, typeOfHitName);

        if (env->ExceptionCheck()) {
            // getDeclaredField threw NoSuchFieldException — this is the normal "not this one" case.
            env->ExceptionClear();
            env->DeleteLocalRef(typeOfHitName);
            env->DeleteLocalRef(typeClass);
            env->DeleteLocalRef(field);
            continue;
        }

        // If we got here, the field type has a "typeOfHit" member — candidate found.
        // Make the Minecraft field accessible and store it as a global ref to the Field object.
        env->CallVoidMethod(field, setAccessibleMID, JNI_TRUE);
        if (env->ExceptionCheck()) env->ExceptionClear();

        reflectiveObjectMouseOverField = env->NewGlobalRef(field);
        useReflectionForObjectMouseOver = true;

        // cleanup
        env->DeleteLocalRef(found);
        env->DeleteLocalRef(typeOfHitName);
        env->DeleteLocalRef(typeClass);
        env->DeleteLocalRef(field);
        break;
    }

    // cleanup array
    env->DeleteLocalRef(fields);
    // Note: do not delete reflectiveObjectMouseOverField (global ref) — keep it for later use.
}

jobject Minecraft::getObjectMouseOver() {
    // Fast path: direct field id (MCP/dev names)
    if (fdObjectMouseOver != 0) {
        return getObject(getMinecraft(), fdObjectMouseOver);
    }

    // Reflection fallback: use cached java.lang.reflect.Field if available
    if (useReflectionForObjectMouseOver && reflectiveObjectMouseOverField != nullptr) {
        JNIEnv *env = phantom->getEnv(); // adjust if your Phantom API name differs
        if (!env) return nullptr;

        // Field.get(Object)
        jclass fieldClass = env->FindClass("java/lang/reflect/Field");
        if (!fieldClass) {
            if (env->ExceptionCheck()) env->ExceptionClear();
            return nullptr;
        }
        jmethodID getMID = env->GetMethodID(fieldClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
        if (!getMID) {
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(fieldClass);
            return nullptr;
        }

        jobject mc = getMinecraft();
        jobject value = env->CallObjectMethod(reflectiveObjectMouseOverField, getMID, mc);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(fieldClass);
            return nullptr;
        }

        env->DeleteLocalRef(fieldClass);
        return value; // caller should treat as local ref
    }

    // nothing found
    return nullptr;
}

jobject Minecraft::getMinecraft() {
	return getObject(smdGetMinecraft);
}

jobject Minecraft::getPlayer() {
	return getObject(getMinecraft(), fdPlayer);
}

jobject Minecraft::getWorld() {
	return getObject(getMinecraft(), fdWorld);
}

jobject Minecraft::getGameSettings() {
    return getObject(getMinecraft(), fdGameSettings);
}

jboolean Minecraft::isInGameHasFocus() {
    return getBoolean(getMinecraft(), fdInGameHasFocus);
}

jobject Minecraft::getRenderViewEntity() {
    return getObject(getMinecraft(), mdGetRenderViewEntity);
}



void Minecraft::setObjectMouseOver(jobject object) {
    setObject(getMinecraft(), fdObjectMouseOver, object);
}

jobject Minecraft::getPointedEntity() {
    return getObject(getMinecraft(), fdPointedEntity);
}

void Minecraft::setPointedEntity(jobject object) {
    setObject(getMinecraft(), fdPointedEntity, object);
}

jobject Minecraft::getTimer() {
    return getObject(getMinecraft(), fdTimer);
}

jobject Minecraft::getEntityRenderer() {
    return getObject(getMinecraft(), fdEntityRenderer);
}

jint Minecraft::getRightClickDelayTimer() {
    return getInt(getMinecraft(), fdRightClickDelayTimer);
}

void Minecraft::setRightClickDelayTimer(jint rightClickDelayTimer) {
    setInt(getMinecraft(), fdRightClickDelayTimer, rightClickDelayTimer);
}

jint Minecraft::getLeftClickMouse() {
    return getInt(getMinecraft(), fdLeftClickMouse);
}

void Minecraft::setLeftClickMouse(jint leftClickMouse) {
    setInt(getMinecraft(), fdLeftClickMouse, leftClickMouse);
}

EntityPlayerSP Minecraft::getPlayerContainer() {
	return EntityPlayerSP(phantom, this);
}

WorldClient Minecraft::getWorldContainer() {
	return WorldClient(phantom, this);
}

GameSettings Minecraft::getGameSettingsContainer() {
    return GameSettings(phantom, getGameSettings());
}

Entity Minecraft::getRenderViewEntityContainer() {
    return Entity(phantom, getRenderViewEntity());
}

Timer Minecraft::getTimerContainer() {
    return Timer(phantom, getTimer());
}

EntityRenderer Minecraft::getEntityRendererContainer() {
    return EntityRenderer(phantom, getEntityRenderer());
}

Phantom *Minecraft::getPhantom() {
    return phantom;
}
