#include "TestPlayerModule.h"

#include <net/minecraft/entity/EntityPlayerSP.h>
TestPlayerModule::TestPlayerModule(Phantom *phantom)
    : Cheat("TestPlayers", "Spawns dummy players for ESP testing"),
      phantom(phantom) {}

void TestPlayerModule::spawnTestPlayer(Minecraft *mc, const std::string &name,
                                       float xOffset, float zOffset) {
  auto world = mc->getWorldContainer();
  auto localSP = mc->getPlayerContainer();

  Entity localEntity(phantom, localSP.getEntityPlayerSP());

  Phantom *p = phantom;

  // Find the Entity class
  jclass entityClass = p->getEnv()->FindClass("net/minecraft/entity/Entity");
  if (!entityClass)
    return;

  // Find the constructor (takes a World)
  jmethodID ctor = p->getEnv()->GetMethodID(
      entityClass, "<init>",
      "(Lnet/minecraft/client/multiplayer/WorldClient;)V");
  if (!ctor)
    return;

  // Create the dummy entity in the world
  jobject dummyObj =
      p->getEnv()->NewObject(entityClass, ctor, world.getWorld());
  Entity dummy(p, dummyObj);

  // Set position near local player
  dummy.setPos(localEntity.getPosX() + xOffset, localEntity.getPosY(),
               localEntity.getPosZ() + zOffset);

  // Optionally set name here (depends on your Entity wrapper)
  // dummy.setName(name.c_str());

  // Add entity to world
  jclass worldClass = p->getEnv()->GetObjectClass(world.getWorld());
  jmethodID addEntity = p->getEnv()->GetMethodID(
      worldClass, "addEntity", "(Lnet/minecraft/entity/Entity;)V");
  if (addEntity)
    p->getEnv()->CallVoidMethod(world.getWorld(), addEntity, dummy.getEntity());

  // Save for later removal
  spawnedPlayers.push_back(dummy);
}
