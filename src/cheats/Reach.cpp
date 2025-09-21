//
// Reach.cpp — robust AABB + fallback scan, safe debug (no ImGui in run)
//
#include "Reach.h"

#include <java/util/JavaList.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <imgui.h>
#include "../utils/ImGuiUtils.h"

// Note: header (Reach.h) unchanged from the last version (it already declares new debug fields).
Reach::Reach(Phantom *phantom) : Cheat("Reach", "Long arm hack") {
    this->phantom = phantom;

    enabled = true;
    reach = 3.2f;
    minReach = 1.5f;
    showDebug = false;

    // debug state init
    lastFound = false;
    lastDistance = -1.0;
    lastPointedEntity = false;

    // new debug fields
    lastNearbyCount = 0;
    usedFallback = false;
}

void Reach::renderSettings() {
    ImGui::Checkbox("Enabled", &enabled);
    ImGui::SameLine();
    ImGui::TextDisabled("(toggle)");

    ImGui::SliderFloat("Hit Distance", &reach, minReach, 6.0f, "%.2f");
    ImGui::SameLine();
    ImGuiUtils::drawHelper("Max distance used for entity hit detection.");

    ImGui::Checkbox("Show debug info", &showDebug);
    if (showDebug) {
        ImGui::Separator();
        ImGui::Text("Last found target: %s", lastFound ? "Yes" : "No");
        ImGui::Text("Last pointed entity set: %s", lastPointedEntity ? "Yes" : "No");
        if (lastFound) {
            ImGui::Text("Last distance: %.3f", lastDistance);
        }
        ImGui::Text("Nearby query count: %d", lastNearbyCount);
        ImGui::Text("Fallback used: %s", usedFallback ? "Yes" : "No");
    }
}

/*
 Robust run:
  - Build expanded AABB as before, extract raw jobject and call getEntitiesWithinAABBExcluding(...)
  - If that returns zero elements, fallback to scanning the full entity list (mc->getWorldContainer().getEntities())
  - In both cases, compute intercepts with AxisAlignedBB and choose the closest hit <= reach
  - Update EntityRenderer and setObjectMouseOver accordingly
  - No ImGui calls here (UI only in renderSettings)
*/
void Reach::run(Minecraft *mc) {
    if (!enabled || !mc) return;

    // reset debug state for this tick
    lastFound = false;
    lastDistance = -1.0;
    lastPointedEntity = false;
    lastNearbyCount = 0;
    usedFallback = false;

    // get the render view entity and ensure it's valid
    Entity renderViewEntity = mc->getRenderViewEntityContainer();
    if (renderViewEntity.getEntity() == nullptr) return;

    // partial ticks (defensive)
    float partialTicks = 0.0f;
    try {
        partialTicks = mc->getTimerContainer().getPartialTicks();
    } catch (...) {
        partialTicks = 0.0f;
    }

    // compute eye pos and look vector
    Vec3 eyePos = renderViewEntity.getPositionEyesContainer(partialTicks);
    Vec3 lookVec = renderViewEntity.getLookContainer(partialTicks);

    // ray end point using configured reach
    double maxDist = static_cast<double>(reach);
    Vec3 rayEnd = eyePos.addVectorContainer(lookVec.getXCoord() * maxDist,
                                            lookVec.getYCoord() * maxDist,
                                            lookVec.getZCoord() * maxDist);

    // reset pointed entity
    EntityRenderer entityRenderer = mc->getEntityRendererContainer();
    entityRenderer.setPointedEntity(nullptr);

    // Prepare AxisAlignedBB wrapper and extract raw jobject for the query
    AxisAlignedBB boundingBox = renderViewEntity.getEntityBoundingBoxContainer();
    AxisAlignedBB afterAdd = boundingBox.getAddCoordContainer(lookVec.getXCoord() * maxDist,
                                                              lookVec.getYCoord() * maxDist,
                                                              lookVec.getZCoord() * maxDist);
    AxisAlignedBB extendedBB = afterAdd.getExpandContainer(1.0, 1.0, 1.0);

    jobject aabbJObject = extendedBB.getAABB(); // raw jobject for world query
    JavaList nearby(phantom, aabbJObject);
    if (aabbJObject) {
        nearby = mc->getWorldContainer().getEntitiesWithinAABBExcluding(renderViewEntity.getEntity(), aabbJObject);
    } else {
        // if we couldn't form an AABB jobject, fallback immediately
        nearby = JavaList(phantom, nullptr);
    }

    // If the AABB query returned nothing (size==0), fall back to iterating entire world entity list
    if (nearby.size() == 0) {
        usedFallback = true;
        JavaList allEntities = mc->getWorldContainer().getEntities();
        lastNearbyCount = allEntities.size();
        // We'll manually set nearbyCandidates by scanning allEntities and performing same tests below.
        // To reuse the same loop, we'll iterate over allEntities directly.
        double bestDistance = maxDist;
        Vec3 bestHit(phantom, nullptr);
        jobject bestEntityObj = nullptr;

        for (int i = 0; i < allEntities.size(); ++i) {
            jobject elem = allEntities.get(i);
            if (!elem) continue;
            // skip renderViewEntity itself
            if (elem == renderViewEntity.getEntity()) continue;

            Entity candidate(mc->getPhantom(), elem);
            if (!candidate.canBeCollidedWith()) continue;

            float collisionBorder = candidate.getCollisionBorderSize();
            AxisAlignedBB candidateBB = candidate.getEntityBoundingBoxContainer().getExpandContainer(collisionBorder, collisionBorder, collisionBorder);
            MovingObjectPosition mop = candidateBB.getCalculateInterceptContainer(eyePos.getVec3(), rayEnd.getVec3());

            Vec3 candidateHit(phantom, nullptr);
            if (mop.getMovingObjectPosition() != nullptr) {
                candidateHit = mop.getHitVecContainer();
            }

            if (candidateBB.isVecInside(eyePos.getVec3())) {
                // immediate hit
                if (0.0 <= bestDistance) {
                    entityRenderer.setPointedEntity(candidate.getEntity());
                    bestHit = (mop.getMovingObjectPosition() == nullptr ? eyePos : candidateHit);
                    bestDistance = 0.0;
                    bestEntityObj = candidate.getEntity();
                }
                continue;
            }

            if (mop.getMovingObjectPosition() != nullptr) {
                double dist = eyePos.distanceTo(candidateHit.getVec3());
                if (dist <= maxDist && (dist < bestDistance || bestEntityObj == nullptr)) {
                    Entity riding = renderViewEntity.getRidingEntityContainer();
                    if (riding.getEntity() != nullptr && candidate.getId() == riding.getId()) {
                        if (bestDistance == 0.0) {
                            entityRenderer.setPointedEntity(candidate.getEntity());
                            bestHit = candidateHit;
                            bestEntityObj = candidate.getEntity();
                        }
                    } else {
                        entityRenderer.setPointedEntity(candidate.getEntity());
                        bestHit = candidateHit;
                        bestDistance = dist;
                        bestEntityObj = candidate.getEntity();
                    }
                }
            }
        }

        // apply result (same as below)
        if (bestEntityObj != nullptr && entityRenderer.getPointedEntityContainer().getEntity() != nullptr && bestDistance > 3.0) {
            jclass MovingObjectPositionClass = mc->getClass("net.minecraft.util.MovingObjectPosition");
            if (MovingObjectPositionClass != nullptr) {
                jmethodID ctor = phantom->getEnv()->GetMethodID(MovingObjectPositionClass, "<init>", "(Lnet/minecraft/entity/Entity;Lnet/minecraft/util/Vec3;)V");
                if (ctor != nullptr) {
                    jobject mopObj = phantom->getEnv()->NewObject(MovingObjectPositionClass, ctor, entityRenderer.getPointedEntity(), bestHit.getVec3());
                    if (mopObj != nullptr) {
                        mc->setObjectMouseOver(mopObj);
                    }
                }
            }
            lastFound = true;
            lastDistance = bestDistance;
            lastPointedEntity = true;
        } else {
            lastFound = false;
            lastDistance = -1.0;
            lastPointedEntity = (entityRenderer.getPointedEntityContainer().getEntity() != nullptr);
        }

        return; // done with fallback
    }

    // If we got here, the AABB query returned candidates (fast path)
    usedFallback = false;
    lastNearbyCount = nearby.size();

    double bestDistance = maxDist;
    Vec3 bestHit(phantom, nullptr);
    jobject bestEntityObj = nullptr;

    for (int i = 0; i < nearby.size(); ++i) {
        jobject elem = nearby.get(i);
        if (!elem) continue;
        // skip render view entity if present
        if (elem == renderViewEntity.getEntity()) continue;

        Entity candidate(mc->getPhantom(), elem);
        if (!candidate.canBeCollidedWith()) continue;

        float collisionBorder = candidate.getCollisionBorderSize();
        AxisAlignedBB candidateBB = candidate.getEntityBoundingBoxContainer().getExpandContainer(collisionBorder, collisionBorder, collisionBorder);
        MovingObjectPosition mop = candidateBB.getCalculateInterceptContainer(eyePos.getVec3(), rayEnd.getVec3());

        Vec3 candidateHit(phantom, nullptr);
        if (mop.getMovingObjectPosition() != nullptr) {
            candidateHit = mop.getHitVecContainer();
        }

        if (candidateBB.isVecInside(eyePos.getVec3())) {
            if (0.0 <= bestDistance) {
                entityRenderer.setPointedEntity(candidate.getEntity());
                bestHit = (mop.getMovingObjectPosition() == nullptr ? eyePos : candidateHit);
                bestDistance = 0.0;
                bestEntityObj = candidate.getEntity();
            }
            continue;
        }

        if (mop.getMovingObjectPosition() != nullptr) {
            double dist = eyePos.distanceTo(candidateHit.getVec3());
            if (dist <= maxDist && (dist < bestDistance || bestEntityObj == nullptr)) {
                Entity riding = renderViewEntity.getRidingEntityContainer();
                if (riding.getEntity() != nullptr && candidate.getId() == riding.getId()) {
                    if (bestDistance == 0.0) {
                        entityRenderer.setPointedEntity(candidate.getEntity());
                        bestHit = candidateHit;
                        bestEntityObj = candidate.getEntity();
                    }
                } else {
                    entityRenderer.setPointedEntity(candidate.getEntity());
                    bestHit = candidateHit;
                    bestDistance = dist;
                    bestEntityObj = candidate.getEntity();
                }
            }
        }
    }

    // apply result for fast path
    if (bestEntityObj != nullptr && entityRenderer.getPointedEntityContainer().getEntity() != nullptr && bestDistance > 3.0) {
        jclass MovingObjectPositionClass = mc->getClass("net/minecraft/util/MovingObjectPosition");
        if (MovingObjectPositionClass != nullptr) {
            jmethodID ctor = phantom->getEnv()->GetMethodID(MovingObjectPositionClass, "<init>", "(Lnet/minecraft/entity/Entity;Lnet/minecraft/util/Vec3;)V");
            if (ctor != nullptr) {
                jobject mopObj = phantom->getEnv()->NewObject(MovingObjectPositionClass, ctor, entityRenderer.getPointedEntity(), bestHit.getVec3());
                if (mopObj != nullptr) {
                    mc->setObjectMouseOver(mopObj);
                }
            }
        }
        lastFound = true;
        lastDistance = bestDistance;
        lastPointedEntity = true;
    } else {
        lastFound = false;
        lastDistance = -1.0;
        lastPointedEntity = (entityRenderer.getPointedEntityContainer().getEntity() != nullptr);
    }
}
