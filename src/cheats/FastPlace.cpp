//
//Terribly made by Kirby...
//

#include "FastPlace.h"

#include <imgui.h>

#include "../utils/MCUtils.h"
#include "../utils/MathHelper.h"

#include <imgui.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <net/minecraft/entity/EntityPlayerSP.h>

FastPlace::FastPlace() : Cheat("FastPlace", "Places blocks faster than normal!") {

}

void FastPlace::run(Minecraft *mc) {

  mc->setRightClickDelayTimer(0); //get fucked
}

void FastPlace::renderSettings() {


}
