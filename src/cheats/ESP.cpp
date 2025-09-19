//
// Created by Kirby!. (ngl ts dont work it just crashes imma wait till i know more to get it to work)
//

#include "ESP.h"
#include "cmath"
ESP::ESP(Phantom *phantom)
    : Cheat("ESP", "Draws 2D boxes around entities"), phantom(phantom) {}


void ESP::run(Minecraft* mc) {
    auto world = mc->getWorldContainer();

    JavaList entities = world.getEntities();
    int size = entities.size();

    for (int i = 0; i < size; ++i) {
        jobject obj = entities.get(i);        // get the raw jobject
        Entity entity(phantom, obj);          // wrap it in your Entity class

        if (!entity.isPlayer() || entity.isLocalPlayer(mc))
            continue;

        drawBox2D(mc, entity);
    }
}


void ESP::reset(Minecraft *mc) {
  // Nothing to reset
}

void ESP::drawBox2D(Minecraft *mc, Entity &entity) {
  AxisAlignedBB bb = entity.getEntityBoundingBoxContainer();

  // Get 8 corners of the bounding box
  ImVec2 screenCorners[8];

  jdouble x[] = {bb.getMinX(), bb.getMaxX()};
  jdouble y[] = {bb.getMinY(), bb.getMaxY()};
  jdouble z[] = {bb.getMinZ(), bb.getMaxZ()};

  bool valid = true;
  int idx = 0;
  for (jdouble xi : x) {
    for (jdouble yi : y) {
      for (jdouble zi : z) {
        if (!worldToScreen(mc, xi, yi, zi, screenCorners[idx++])) {
          valid = false; // corner is off-screen
        }
      }
    }
  }

  if (!valid)
    return;

  // Find 2D bounding rectangle
  float minX = screenCorners[0].x, minY = screenCorners[0].y;
  float maxX = minX, maxY = minY;
  for (int i = 1; i < 8; ++i) {
    if (screenCorners[i].x < minX)
      minX = screenCorners[i].x;
    if (screenCorners[i].y < minY)
      minY = screenCorners[i].y;
    if (screenCorners[i].x > maxX)
      maxX = screenCorners[i].x;
    if (screenCorners[i].y > maxY)
      maxY = screenCorners[i].y;
  }

  ImU32 color = IM_COL32(255, 0, 0, 255); // red
  drawLine({minX, minY}, {maxX, minY}, color);
  drawLine({maxX, minY}, {maxX, maxY}, color);
  drawLine({maxX, maxY}, {minX, maxY}, color);
  drawLine({minX, maxY}, {minX, minY}, color);
}

/*
bool ESP::worldToScreen(Minecraft* mc, float x, float y, float z, ImVec2& out) {
    jobject cameraObj = mc->getRenderViewEntity();
    if (!cameraObj)
        return false; // camera not ready

    Entity camera(phantom, cameraObj);

    double camX = camera.getPosX();
    double camY = camera.getPosY();
    double camZ = camera.getPosZ();

    Vec3 lookVec = camera.getLookContainer(1.0f);
    if (!lookVec.getObject())
        return false; // look vector not ready

    double relX = x - camX;
    double relY = y - camY;
    double relZ = z - camZ;

    double dot = relX*lookVec.getXCoord() + relY*lookVec.getYCoord() + relZ*lookVec.getZCoord();
    if (dot <= 0) return false;

    double dist = sqrt(relX*relX + relY*relY + relZ*relZ);
    if (dist < 1.0 || dist > 200.0) return false;

    int screenWidth = 1920;
    int screenHeight = 1080;
    double fov = 70.0 * M_PI / 180.0;
    double scale = (screenHeight / 2.0) / tan(fov / 2.0);

    out.x = static_cast<float>(screenWidth / 2.0 + (relX * scale / dist));
    out.y = static_cast<float>(screenHeight / 2.0 - (relY * scale / dist));

    if (out.x < -200 || out.x > screenWidth + 200 || out.y < -200 || out.y > screenHeight + 200)
        return false;

    return true;
}
*/
