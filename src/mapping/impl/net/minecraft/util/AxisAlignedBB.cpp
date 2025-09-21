#include "AxisAlignedBB.h"
#include <net/minecraft/util/MovingObjectPosition.h>

AxisAlignedBB::AxisAlignedBB(Phantom *phantom, jobject aabb) : AbstractClass(phantom, "AxisAlignedBB") {
    mdExpand = getMethodID("expand");
    mdAddCoord = getMethodID("addCoord");
    mdIsVecInside = getMethodID("isVecInside");
    mdCalculateIntercept = getMethodID("calculateIntercept");

    fdMaxX = getFieldID("maxX");
    fdMaxY = getFieldID("maxY");
    fdMaxZ = getFieldID("maxZ");
    fdMinX = getFieldID("minX");
    fdMinY = getFieldID("minY");
    fdMinZ = getFieldID("minZ");

    this->aabb = aabb;
}

jobject AxisAlignedBB::expand(jdouble x, jdouble y, jdouble z) {
    if (!aabb) return nullptr;
    return getObject(aabb, mdExpand, x, y, z);
}

jobject AxisAlignedBB::addCoord(jdouble x, jdouble y, jdouble z) {
    if (!aabb) return nullptr;
    return getObject(aabb, mdAddCoord, x, y, z);
}

jobject AxisAlignedBB::calculateIntercept(jobject vec1, jobject vec2) {
    if (!aabb) return nullptr;
    return getObject(aabb, mdCalculateIntercept, vec1, vec2);
}

jboolean AxisAlignedBB::isVecInside(jobject vec) {
    if (!aabb || !vec) return false;
    return getBoolean(aabb, mdIsVecInside, vec);
}

jdouble AxisAlignedBB::getMaxX() {
    if (!aabb) return 0.0;
    return getDouble(aabb, fdMaxX);
}

jdouble AxisAlignedBB::getMaxY() {
    if (!aabb) return 0.0;
    return getDouble(aabb, fdMaxY);
}

jdouble AxisAlignedBB::getMaxZ() {
    if (!aabb) return 0.0;
    return getDouble(aabb, fdMaxZ);
}

jdouble AxisAlignedBB::getMinX() {
    if (!aabb) return 0.0;
    return getDouble(aabb, fdMinX);
}

jdouble AxisAlignedBB::getMinY() {
    if (!aabb) return 0.0;
    return getDouble(aabb, fdMinY);
}

jdouble AxisAlignedBB::getMinZ() {
    if (!aabb) return 0.0;
    return getDouble(aabb, fdMinZ);
}

AxisAlignedBB AxisAlignedBB::getExpandContainer(jdouble x, jdouble y, jdouble z) {
    return AxisAlignedBB(phantom, expand(x, y, z));
}

AxisAlignedBB AxisAlignedBB::getAddCoordContainer(jdouble x, jdouble y, jdouble z) {
    return AxisAlignedBB(phantom, addCoord(x, y, z));
}

MovingObjectPosition AxisAlignedBB::getCalculateInterceptContainer(jobject vec1, jobject vec2) {
    return MovingObjectPosition(phantom, calculateIntercept(vec1, vec2));
}

jobject AxisAlignedBB::getAABB() {
    return aabb;
}
