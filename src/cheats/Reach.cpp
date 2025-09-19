//
// Created by Kirby! Remade Reach.cpp for smoother, more discrete hit detection
//

#include "Reach.h"
#include <imgui.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <java/util/JavaList.h>
#include <net/minecraft/client/renderer/EntityRenderer.h>
#include <net/minecraft/entity/Entity.h>
#include <net/minecraft/util/Vec3.h>
#include <net/minecraft/client/Minecraft.h>

Reach::Reach(Phantom *phantom) : Cheat("Reach", "Long arm hack") {
    this->phantom = phantom;
    reach = 3.2f;
}

// Slider for reach distance
void Reach::renderSettings() {
    ImGui::SliderFloat("Hit Distance", &reach, 3.0f, 6.0f, "%.2f");
}

// Linear interpolation helper

Vec3 lerpVec3(const Vec3 &from, const Vec3 &to, float t, Phantom *phantom) {
    // Compute offset vector from "from" to "to", then scale it by t
    double dx = (to.getXCoord() - from.getXCoord()) * t;
    double dy = (to.getYCoord() - from.getYCoord()) * t;
    double dz = (to.getZCoord() - from.getZCoord()) * t;

    // Use addVectorContainer to produce the new Vec3
    return from.addVectorContainer(dx, dy, dz);
}


// Main logic
void Reach::run(Minecraft *mc) {
    Entity viewEntity = mc->getRenderViewEntityContainer();
    EntityRenderer renderer = mc->getEntityRendererContainer();

    if (!viewEntity.getEntity()) return;

    float partialTicks = mc->getTimerContainer().getPartialTicks();
    Vec3 eyePos = viewEntity.getPositionEyesContainer(partialTicks);
    Vec3 lookVec = viewEntity.getLookContainer(partialTicks);

    Vec3 targetVec = eyePos.addVectorContainer(lookVec.getXCoord() * reach,
                                               lookVec.getYCoord() * reach,
                                               lookVec.getZCoord() * reach);

    renderer.setPointedEntity(nullptr);

    AxisAlignedBB bb = viewEntity.getEntityBoundingBoxContainer();
    JavaList entities = mc->getWorldContainer().getEntitiesWithinAABBExcluding(
        viewEntity.getEntity(), bb.getAddCoordContainer(lookVec.getXCoord() * reach,
                                                        lookVec.getYCoord() * reach,
                                                        lookVec.getZCoord() * reach).expand(1,1,1)
    );

    Vec3 hitVec(phantom, nullptr);
    double closestDist = reach;
    Entity ridingEntity(phantom, nullptr);

    for (int i = 0; i < entities.size(); i++) {
        Entity ent(mc->getPhantom(), entities.get(i));
        if (!ent.canBeCollidedWith()) continue;

        float border = ent.getCollisionBorderSize();
        AxisAlignedBB entityBB = ent.getEntityBoundingBoxContainer().getExpandContainer(border, border, border);
        MovingObjectPosition mop = entityBB.getCalculateInterceptContainer(eyePos.getVec3(), targetVec.getVec3());

        if (mop.getMovingObjectPosition()) hitVec = mop.getHitVecContainer();

        if (entityBB.isVecInside(eyePos.getVec3()) || (mop.getMovingObjectPosition() && eyePos.distanceTo(hitVec.getVec3()) < closestDist)) {
            ridingEntity = viewEntity.getRidingEntityContainer();
            if (!ridingEntity.getEntity() || ent.getId() != ridingEntity.getId()) {
                renderer.setPointedEntity(ent.getEntity());
                hitVec = lerpVec3(eyePos, hitVec, 0.7f, phantom); // smooth lerp
                closestDist = eyePos.distanceTo(hitVec.getVec3());
            }
        }
    }

    if (renderer.getPointedEntityContainer().getEntity() && closestDist > 3.0) {
        jclass movingObjectClass = mc->getClass("net.minecraft.util.MovingObjectPosition");
        jmethodID ctor = phantom->getEnv()->GetMethodID(movingObjectClass, "<init>", "(Lnet/minecraft/entity/Entity;Lnet/minecraft/util/Vec3;)V");
        mc->setObjectMouseOver(phantom->getEnv()->NewObject(movingObjectClass, ctor, renderer.getPointedEntity(), hitVec.getVec3()));
    }
}
