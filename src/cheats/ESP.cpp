#include "ESP.h"
#include <imgui.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <net/minecraft/entity/EntityPlayerSP.h>
#include <cmath>

ESP::ESP(Phantom *phantom)
    : Cheat("ESP", "Draws 2D boxes around entities"), phantom(phantom) {}

void ESP::run(Minecraft* mc) {
    if (!mc) return;

    auto world = mc->getWorldContainer();


    JavaList entities = world.getEntities();
    int size = entities.size();

    for (int i = 0; i < size; ++i) {
        jobject obj = entities.get(i);
        if (!obj) continue;

        Entity entity(phantom, obj);
        if (!entity.getEntity() || !entity.isPlayer() || entity.isLocalPlayer(mc))
            continue;

        drawBox2D(mc, entity);
    }
}

void ESP::reset(Minecraft *mc) {
    // Nothing to reset
}

void ESP::drawBox2D(Minecraft* mc, Entity &entity) {
    if (!entity.getEntity()) return;

    AxisAlignedBB bb = entity.getEntityBoundingBoxContainer();

    ImVec2 screenCorners[8];
    bool valid = true;

    jdouble x[] = {bb.getMinX(), bb.getMaxX()};
    jdouble y[] = {bb.getMinY(), bb.getMaxY()};
    jdouble z[] = {bb.getMinZ(), bb.getMaxZ()};

    int idx = 0;
    for (jdouble xi : x) {
        for (jdouble yi : y) {
            for (jdouble zi : z) {
                // If worldToScreen is commented out, just skip rendering
                screenCorners[idx++] = ImVec2(0,0);
            }
        }
    }

    if (!valid) return;

    float minX = screenCorners[0].x, minY = screenCorners[0].y;
    float maxX = minX, maxY = minY;

    for (int i = 1; i < 8; ++i) {
        if (screenCorners[i].x < minX) minX = screenCorners[i].x;
        if (screenCorners[i].y < minY) minY = screenCorners[i].y;
        if (screenCorners[i].x > maxX) maxX = screenCorners[i].x;
        if (screenCorners[i].y > maxY) maxY = screenCorners[i].y;
    }

    ImU32 color = IM_COL32(255, 0, 0, 255);
    drawLine({minX, minY}, {maxX, minY}, color);
    drawLine({maxX, minY}, {maxX, maxY}, color);
    drawLine({maxX, maxY}, {minX, maxY}, color);
    drawLine({minX, maxY}, {minX, minY}, color);
}
