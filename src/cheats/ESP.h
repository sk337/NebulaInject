//
// Created by somepineaple on 2025-09-19.
//

#pragma once

#include <net/minecraft/client/Minecraft.h>
#include <net/minecraft/entity/Entity.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <net/minecraft/util/AxisAlignedBB.h>

#include "Cheat.h"
#include <imgui.h>

class ESP : public Cheat {
public:
    explicit ESP(Phantom* phantom);

    void run(Minecraft* mc) override;
    void reset(Minecraft* mc) override;

private:
    Phantom* phantom;

    // Draws 2D rectangle for the entity
    void drawBox2D(Minecraft* mc, Entity& entity);

    // Converts world coordinates to screen coordinates (stub)
    bool worldToScreen(Minecraft* mc, float x, float y, float z, ImVec2& out);

    // Convenience: draws a line using ImGui
    void drawLine(const ImVec2& start, const ImVec2& end, ImU32 color, float thickness = 1.0f);
};
