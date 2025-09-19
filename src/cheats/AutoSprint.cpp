#include "AutoSprint.h"


#include <imgui.h>

#include "../utils/MCUtils.h"
#include "../utils/MathHelper.h"
#include "../utils/XUtils.h"
#include <imgui.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <net/minecraft/entity/EntityPlayerSP.h>



AutoSprint::AutoSprint() : Cheat("AutoSprint", "Like toggle sprint but it's always on")
{
}




void AutoSprint::run(Minecraft *mc) {
    auto player = mc->getPlayerContainer();

    if (player.getHorizontalSpeed() > 0.01) {
        player.setSprinting(true);
    } else {
        player.setSprinting(false);
    }
}



