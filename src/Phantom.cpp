//
// Created by somepineaple on 1/25/22.
//
#include "Phantom.h"

#include <net/minecraft/client/Minecraft.h>
#include <net/minecraft/client/multiplayer/WorldClient.h>
#include <net/minecraft/entity/EntityPlayerSP.h>

#include <fstream>
#include <string>
#include <thread>

#include "mapping/Mapping.h"
/* #include <net/minecraft/client/multiplayer/PlayerControllerMP.h> */
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "cheats/AutoClicker.h"
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
#include "ui/PhantomWindow.h"

std::string GetMinecraftWindowTitle() {
    Display *display = XOpenDisplay(nullptr);
    if (!display) return "";

    Window root = DefaultRootWindow(display);
    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char *prop = nullptr;

    Atom netWmName = XInternAtom(display, "_NET_WM_NAME", True);
    Atom utf8String = XInternAtom(display, "UTF8_STRING", True);

    std::string title;

    Window parent, *children;
    unsigned int nChildren;
    if (XQueryTree(display, root, &root, &parent, &children, &nChildren)) {
        for (unsigned int i = 0; i < nChildren; ++i) {
            if (XGetWindowProperty(display, children[i], netWmName, 0, (~0L),
                                   False, utf8String, &actualType,
                                   &actualFormat, &nItems, &bytesAfter,
                                   &prop) == Success) {
                if (prop) {
                    title = std::string(reinterpret_cast<char *>(prop));
                    XFree(prop);
                    break;
                }
            }
        }
        if (children) XFree(children);
    }

    XCloseDisplay(display);
    return title;
}

GameVersions Phantom::DetectGameVersion() {
    std::string title = GetMinecraftWindowTitle();
    bool is1_7 = title.find("1.7.10") != std::string::npos;

    if (title.find("Lunar Client") != std::string::npos)
        return is1_7 ? LUNAR_1_7_10 : LUNAR_1_8;

    bool vanillaMappings = title.find("Badlion Minecraft") != std::string::npos;

    CM *cm = Mapping::getClass("net/minecraft/launchwrapper/LaunchClassLoader");
    if (!cm) {
        std::cerr << "Failed to get mapping for LaunchClassLoader" << std::endl;
        return CASUAL_1_8;  // fallback
    }

    CM *cm2 = Mapping::getClass("net/minecraft/launchwrapper/Launch");
    jclass launchClazz = env->FindClass(cm2->name);

    if (cm && launchClazz && !vanillaMappings) {
        return is1_7 ? FORGE_1_7_10 : FORGE_1_8;
    }

    if (Mapping::getClass("net/digitalingot/featheropt/FeatherCoreMod"))
        return FEATHER_1_8;

    return is1_7 ? CASUAL_1_7_10 : CASUAL_1_8;
}

Phantom::Phantom() {
    running = false;
    jvm = nullptr;
    env = nullptr;

    jsize count;
    if (JNI_GetCreatedJavaVMs(&jvm, 1, &count) != JNI_OK || count == 0) return;

    jint res = jvm->GetEnv((void **)&env, JNI_VERSION_1_8);
    if (res == JNI_EDETACHED)
        res = jvm->AttachCurrentThread((void **)&env, nullptr);
    if (res != JNI_OK) return;

    GameVersions version = DetectGameVersion();
    Mapping::Initialize(version);

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
