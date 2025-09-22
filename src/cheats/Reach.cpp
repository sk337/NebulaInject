#include "Reach.h"
#include <net/minecraft/entity/EntityPlayerSP.h>
#include <java/util/JavaList.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <imgui.h>
#include <string>
#include <fstream>
#include <string>
#include <fstream>
Reach::Reach(Phantom *phantom) : Cheat("Reach", "Long arm hack") {
    reach = 3.2f;
    this->phantom = phantom;
    currentTarget = nullptr;
    currentDistance = 0.0;
    isAttacking = false;

    // initialize debug state
    lastEntityCount = 0;
    lastHitVecWasNull = false;
    lastVar9Fallback = false;
    lastMopCtorFound = false;
    lastAttackMethodFound = false;
    lastAttackCalled = false;
    lastFailureMsg.clear();
}

bool Reach::dumpedMethods = false;

void Reach::renderSettings() {
    // existing UI
    ImGui::SliderFloat("Hit Distance", &reach, 3.0f, 6.0f, "%.2f");

    if (currentTarget != nullptr) {
        ImGui::Text("Targeted Entity: %p", reinterpret_cast<void*>(currentTarget));
        ImGui::Text("Distance: %.2f", currentDistance);
    } else {
        ImGui::Text("No target");
    }

    ImGui::Text("Attacking: %s", isAttacking ? "Yes" : "No");

    // ===== Debug Block =====
    ImGui::Separator();
    ImGui::TextWrapped("Reach Debug Info (per tick):");

    // Entity counts
    ImGui::Text("Entities in last query: %d", lastEntityCount);

    // MOP & hit vector checks
    ImGui::Text("Last hitVec was null: %s", lastHitVecWasNull ? "Yes" : "No");
    ImGui::Text("Var9 fallback used: %s", lastVar9Fallback ? "Yes" : "No");
    ImGui::Text("Found MOP constructor: %s", lastMopCtorFound ? "Yes" : "No");
    ImGui::Text("Found attackEntity method: %s", lastAttackMethodFound ? "Yes" : "No");
    ImGui::Text("Called PlayerController.attackEntity: %s", lastAttackCalled ? "Yes" : "No");

    // Target info
    if (currentTarget) {
        ImGui::Text("Current target ptr: %p", reinterpret_cast<void*>(currentTarget));
        ImGui::Text("Current distance: %.2f", currentDistance);
    }

    if (!lastFailureMsg.empty()) {
        ImGui::TextWrapped("Last failure: %s", lastFailureMsg.c_str());
    }

    ImGui::Separator();
}


void Reach::run(Minecraft *mc) {


    // reset debug flags
    lastEntityCount = 0;
    lastHitVecWasNull = false;
    lastVar9Fallback = false;
    lastMopCtorFound = false;
    lastAttackMethodFound = false;
    lastAttackCalled = false;
    lastFailureMsg.clear();

    Entity renderViewEntity = mc->getRenderViewEntityContainer();
    EntityRenderer entityRenderer = mc->getEntityRendererContainer();

    if (renderViewEntity.getEntity() == nullptr) {
        lastFailureMsg = "No render view entity";
        return;
    }

    float partialTicks = mc->getTimerContainer().getPartialTicks();
    double reachDistance = reach;
    
    // Get player's eye position and look vector
    Vec3 eyePos = renderViewEntity.getPositionEyesContainer(partialTicks);
    Vec3 lookVec = renderViewEntity.getLookContainer(partialTicks);
    
    // Calculate reach end point
    Vec3 reachEnd = eyePos.addVectorContainer(
        lookVec.getXCoord() * reachDistance,
        lookVec.getYCoord() * reachDistance,
        lookVec.getZCoord() * reachDistance
    );

    // Get entities in expanded bounding box
    AxisAlignedBB boundingBox = renderViewEntity.getEntityBoundingBoxContainer();
    JavaList entities = mc->getWorldContainer().getEntitiesWithinAABBExcluding(
        renderViewEntity.getEntity(),
        boundingBox.getAddCoordContainer(
            lookVec.getXCoord() * reachDistance,
            lookVec.getYCoord() * reachDistance,
            lookVec.getZCoord() * reachDistance
        ).expand(1.0f, 1.0f, 1.0f)
    );

    lastEntityCount = entities.size();
    currentTarget = nullptr;
    currentDistance = reachDistance;
    Vec3 bestHitVec(phantom, nullptr);
    Entity bestTarget(phantom, nullptr);

    // Find the closest entity that intersects with the reach line
    for (int i = 0; i < entities.size(); i++) {
        Entity entity(phantom, entities.get(i));
        
        // Check if entity is valid using getEntity() instead of isNull()
        if (entity.getEntity() == nullptr || !entity.canBeCollidedWith()) {
            continue;
        }

        float collisionSize = entity.getCollisionBorderSize();
        AxisAlignedBB expandedBB = entity.getEntityBoundingBoxContainer().getExpandContainer(
            collisionSize, collisionSize, collisionSize
        );

        MovingObjectPosition mop = expandedBB.getCalculateInterceptContainer(eyePos.getVec3(), reachEnd.getVec3());
        
        // Check if MOP is valid using isNull() from your MovingObjectPosition class
        if (mop.isNull()) {
            continue;
        }

        Vec3 hitVec = mop.getHitVecContainer();
        // Check if hitVec is valid using getVec3() instead of isNull()
        if (hitVec.getVec3() == nullptr) {
            lastHitVecWasNull = true;
            continue;
        }

        double distance = eyePos.distanceTo(hitVec.getVec3());
        
        if (distance < currentDistance) {
            currentDistance = distance;
            bestTarget = entity;
            bestHitVec = hitVec;
        }
    }

    // Update current target for display
    currentTarget = bestTarget.getEntity();

    // Only create MOP if we have a valid target and hit vector
    if (bestTarget.getEntity() != nullptr && bestHitVec.getVec3() != nullptr) {
        jclass mopClass = phantom->getEnv()->FindClass("net/minecraft/util/MovingObjectPosition");
        if (mopClass) {
            jmethodID mopCtor = phantom->getEnv()->GetMethodID(
                mopClass,
                "<init>",
                "(Lnet/minecraft/entity/Entity;Lnet/minecraft/util/Vec3;)V"
            );
            
            lastMopCtorFound = (mopCtor != nullptr);
            
            if (mopCtor) {
                jobject newMop = phantom->getEnv()->NewObject(
                    mopClass,
                    mopCtor,
                    bestTarget.getEntity(),
                    bestHitVec.getVec3()
                );
                
                // Set the object mouse over
                mc->setObjectMouseOver(newMop);
                
                // Clean up local reference
                phantom->getEnv()->DeleteLocalRef(newMop);
            } else {
                lastFailureMsg = "MovingObjectPosition constructor not found";
            }
            
            phantom->getEnv()->DeleteLocalRef(mopClass);
        } else {
            lastFailureMsg = "MovingObjectPosition class not found";
        }
    }

    // Check if we should attack (this would be set by some external input)
    if (isAttacking && currentTarget != nullptr) {
        performAttack(mc);
        isAttacking = false; // Reset attack flag after performing
    }
}

void Reach::performAttack(Minecraft *mc) {
    if (currentTarget == nullptr) {
        return;
    }

    jobject pc = nullptr;
    jclass mcCls = phantom->getEnv()->GetObjectClass(mc->getMinecraft());
    if (mcCls) {
        jfieldID fdPlayerController = phantom->getEnv()->GetFieldID(
            mcCls, 
            "playerController", 
            "Lnet/minecraft/client/multiplayer/PlayerControllerMP;"
        );
        if (fdPlayerController) {
            pc = phantom->getEnv()->GetObjectField(mc->getMinecraft(), fdPlayerController);
        }
        phantom->getEnv()->DeleteLocalRef(mcCls);
    }

    if (pc) {
        jclass pcCls = phantom->getEnv()->GetObjectClass(pc);
        jmethodID midAttack = phantom->getEnv()->GetMethodID(
            pcCls, 
            "attackEntity", 
            "(Lnet/minecraft/entity/EntityPlayer;Lnet/minecraft/entity/Entity;)V"
        );
        
        if (midAttack == nullptr) {
            midAttack = phantom->getEnv()->GetMethodID(
                pcCls, 
                "attackEntity", 
                "(Lnet/minecraft/entity/Entity;Lnet/minecraft/entity/Entity;)V"
            );
        }

        lastAttackMethodFound = (midAttack != nullptr);

        if (midAttack) {
            EntityPlayerSP player = mc->getPlayerContainer();
            jobject playerRaw = player.getEntityPlayerSP();
            
            phantom->getEnv()->CallVoidMethod(
                pc,
                midAttack,
                playerRaw,
                currentTarget
            );
            
            lastAttackCalled = true;
        } else {
            lastFailureMsg = "PlayerController.attackEntity method not found";
        }
        
        phantom->getEnv()->DeleteLocalRef(pcCls);
        phantom->getEnv()->DeleteLocalRef(pc);
    }
}

void Reach::onAttack() {
    // This method should be called from your input handler when attack key is pressed
    isAttacking = true;
}




void Reach::reset(Minecraft *mc) {
    // Reset state
    currentTarget = nullptr;
    currentDistance = 0.0;
    isAttacking = false;
    lastFailureMsg.clear();
}
