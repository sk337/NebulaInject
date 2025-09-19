
#include "Velocity.h"
#include <algorithm> // for std::clamp
#include <cstdlib>
#include <imgui.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <net/minecraft/entity/EntityPlayerSP.h>
Velocity::Velocity(Phantom *phantom)
    : Cheat("Velocity", "Modifies entity velocity discreetly"),
      phantom(phantom), horizontalMotion(100), verticalMotion(100), chance(100),
      method(Method::SCALE) {}

void Velocity::run(Minecraft *mc) {
  EntityPlayerSP player = mc->getPlayerContainer();

  // Only act on hurt ticks
  if (player.getMaxHurtTime() > 0 &&
      player.getHurtTime() == player.getMaxHurtTime()) {
    if (chance != 100 && chance < 1 + (rand() % 100))
      return;

    double hFactor = horizontalMotion / 100.0;
    double vFactor = verticalMotion / 100.0;

    switch (method) {
    case Method::SCALE:
      player.setMotionHorizontal(player.getMotionX() * hFactor,
                                 player.getMotionZ() * hFactor);
      player.setMotionY(player.getMotionY() * vFactor);
      break;

    case Method::CLAMP:
      player.setMotionHorizontal(
          std::clamp(player.getMotionX(), -hFactor, hFactor),
          std::clamp(player.getMotionZ(), -hFactor, hFactor));
      player.setMotionY(std::clamp(player.getMotionY(), -vFactor, vFactor));
      break;

    case Method::OVERRIDE:
      player.setMotionHorizontal(hFactor, hFactor);
      player.setMotionY(vFactor);
      break;
    }
  }
}

void Velocity::reset(Minecraft *mc) {
  // Optional: nothing needed here
}

void Velocity::renderSettings() {
  ImGui::SliderFloat("Chance (%)", &chance, 0, 100, "%1.0f");
  ImGui::SliderFloat("Horizontal (%)", &horizontalMotion, 0, 100, "%1.0f");
  ImGui::SliderFloat("Vertical (%)", &verticalMotion, 0, 100, "%1.0f");

  const char *methods[] = {"SCALE", "CLAMP", "OVERRIDE"};
  int currentMethod = static_cast<int>(method);
  if (ImGui::Combo("Method", &currentMethod, methods, IM_ARRAYSIZE(methods))) {
    method = static_cast<Method>(currentMethod);
  }
}
