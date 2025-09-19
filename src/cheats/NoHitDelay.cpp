//
//Orginal by Pineapple but I copied it word for word.
//
#include "NoHitDelay.h"
#include "FastPlace.h"

#include <imgui.h>

#include "../utils/MCUtils.h"
#include "../utils/MathHelper.h"

#include <imgui.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <net/minecraft/entity/EntityPlayerSP.h>

NoHitDelay::NoHitDelay() : Cheat("FastPlace", "Gets Rid Of The Delay Between Hits") {

}

void NoHitDelay::run(Minecraft *mc) {

  mc->setLeftClickMouse(0);
}


