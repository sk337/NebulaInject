//
// Created by somepineaple on 1/25/22.
//

#include "Phantom.h"
#include <fstream>
#include <net/minecraft/client/Minecraft.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <net/minecraft/entity/EntityPlayerSP.h>
#include <string>
#include <thread>
/* #include <net/minecraft/client/multiplayer/PlayerControllerMP.h> */
#include "cheats/AutoClicker.h"
#include "ui/PhantomWindow.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>

#include "cheats/AutoSprint.h"
#include "cheats/Clip.h"
#include "cheats/ESP.h"
#include "cheats/FastPlace.h"
#include "cheats/NoHitDelay.h"
#include "cheats/Reach.h"
#include "cheats/ReachInput.h"
#include "cheats/STap.h"
#include "cheats/Velocity.h"
#include "ui/KeyManager.h"

Phantom::Phantom() {
  running = false;
  jvm = nullptr;
  env = nullptr;

  jsize count;
  if (JNI_GetCreatedJavaVMs(&jvm, 1, &count) != JNI_OK || count == 0)
    return;

  jint res = jvm->GetEnv((void **)&env, JNI_VERSION_1_8);
  if (res == JNI_EDETACHED)
    res = jvm->AttachCurrentThread((void **)&env, nullptr);
  if (res != JNI_OK)
    return;

  Mapping::setup();

  cheats.push_back(new AutoClicker());
  cheats.push_back(new ESP(this));
  cheats.push_back(new AutoSprint());
  cheats.push_back(new NoHitDelay());

  cheats.push_back(new Velocity(this));
  Reach *reachCheat = new Reach(this);
  cheats.push_back(reachCheat);
  cheats.push_back(new ReachInput(reachCheat));
  cheats.push_back(new FastPlace());

  cheats.push_back(new STap(this));
}

void Phantom::runClient() {
  running = true;


  auto *mc = new Minecraft(this);
  auto *window = new NebulaWindow(700, 500, "Phantom");
  window->setup();
  auto *keyManager = new KeyManager();

  while (running) {
    EntityPlayerSP player = mc->getPlayerContainer();
    WorldClient world = mc->getWorldContainer();
    if (!player.getEntityPlayerSP() || !world.getWorld()) {
      window->update(cheats, running, false);

      continue;
    }

    std::thread(callUpdateKeys, keyManager, this).detach();
    for (Cheat *cheat : cheats) {
      if (cheat->enabled)
        cheat->run(mc);
      else
        cheat->reset(mc);
    }
    window->update(cheats, running, true);
  }

  window->destruct();
  jvm->DetachCurrentThread();
  delete mc;
  delete window;
  delete keyManager;
}

void Phantom::onKey(int key) {
  for (Cheat *cheat : cheats) {
    if (cheat->binding) {
      cheat->bind = key;
      cheat->binding = false;
      return;
    }
  }

  for (Cheat *cheat : cheats) {
    if (cheat->bind == key) {
      cheat->enabled = !cheat->enabled;
    }
  }
}

JavaVM *Phantom::getJvm() { return jvm; }

JNIEnv *Phantom::getEnv() { return env; }

void Phantom::setRunning(bool p_running) { this->running = p_running; }

bool Phantom::isRunning() const { return running; }
