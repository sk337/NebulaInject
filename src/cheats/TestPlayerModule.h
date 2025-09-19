#pragma once
#ifndef NEBULA_TESTPLAYER_H
#define NEBULA_TESTPLAYER_H
#include "Cheat.h"
#include <net/minecraft/client/Minecraft.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <net/minecraft/entity/Entity.h>


class TestPlayerModule : public Cheat {
    Phantom* phantom;
    std::vector<Entity> spawnedPlayers;
    bool spawned = false; // <-- add this flag

public:
    TestPlayerModule(Phantom* phantom);

    void run(Minecraft* mc) override {
        if (!spawned) {
            spawnTestPlayer(mc, "Dummy1", 5.0f, 5.0f);
            spawnTestPlayer(mc, "Dummy2", -5.0f, -5.0f);
            spawned = true; // <-- mark as spawned
        }
    }

    void reset(Minecraft* mc) override {
        auto world = mc->getWorldContainer();
        for (auto& entity : spawnedPlayers) {
            // Remove entities from world here if needed
        }
        spawnedPlayers.clear();
        spawned = false; // <-- allow spawning again if reset
    }

    void spawnTestPlayer(Minecraft* mc, const std::string& name, float xOffset, float zOffset);
};
#endif
