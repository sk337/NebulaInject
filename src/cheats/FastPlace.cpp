#include "FastPlace.h"

#include <imgui.h>

#include "../utils/MCUtils.h"
#include "../utils/MathHelper.h"
#include "../utils/XUtils.h"
#include <imgui.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <net/minecraft/entity/EntityPlayerSP.h>

void FastPlace::run(Minecraft *mc) {

  mc->setRightClickDelayTimer(static_cast<int>(delay));
}

void FastPlace::render() {

  ImGui::SliderFloat("Block Place Delay", &delay, 0, 6, "%.2f");
}
