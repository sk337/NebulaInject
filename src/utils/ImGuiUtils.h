// ImGuiUtils.h
// Created by somepineapple on 2/1/22
// Updated for Nebula theme
//

#pragma once

#include <string>
#include <imgui.h>

// --- Nebula palette ---
struct NebulaPalette {
    ImVec4 Bg             = ImVec4(0.03f, 0.03f, 0.06f, 1.00f);
    ImVec4 ChildBg        = ImVec4(0.05f, 0.04f, 0.08f, 0.95f);
    ImVec4 PopupBg        = ImVec4(0.06f, 0.05f, 0.10f, 0.95f);
    ImVec4 Border         = ImVec4(0.18f, 0.10f, 0.28f, 1.00f);

    ImVec4 Text           = ImVec4(0.86f, 0.80f, 0.95f, 1.00f);
    ImVec4 TextMuted      = ImVec4(0.60f, 0.52f, 0.70f, 1.00f);
    ImVec4 TextDisabled   = ImVec4(0.50f, 0.45f, 0.55f, 1.00f); // for disabled/info text

    ImVec4 Primary        = ImVec4(0.30f, 0.14f, 0.55f, 1.00f);
    ImVec4 PrimaryHover   = ImVec4(0.38f, 0.20f, 0.62f, 1.00f);
    ImVec4 PrimaryActive  = ImVec4(0.46f, 0.28f, 0.70f, 1.00f);

    ImVec4 Accent         = ImVec4(0.70f, 0.40f, 1.00f, 1.00f); // accent color used in headers/etc

    ImVec4 Button         = ImVec4(0.18f, 0.09f, 0.35f, 1.00f);
    ImVec4 ButtonHover    = ImVec4(0.25f, 0.14f, 0.45f, 1.00f);
    ImVec4 ButtonActive   = ImVec4(0.34f, 0.22f, 0.58f, 1.00f);

    ImVec4 FrameBg        = ImVec4(0.08f, 0.06f, 0.12f, 1.00f);
    ImVec4 FrameBgHover   = ImVec4(0.12f, 0.09f, 0.18f, 1.00f);

    ImVec4 CheckMark      = ImVec4(0.92f, 0.52f, 1.00f, 1.00f);
};

// Expose the Nebula palette instance (define `nebula` in ImGuiUtils.cpp)
extern NebulaPalette nebula;

class ImGuiUtils {
public:
    // Draws a small "(?)" helper icon with a tooltip description
    static void drawHelper(const char* desc);

    // Apply the Nebula theme to ImGui (default nebula)
    static void styleColorsNebula();

    // Deeper/darker variant
    static void NebulaDarkColors();

    // Other theme variations (used by UI)
    static void styleColorsNebulaGold();
    static void styleColorsCinder();
    static void styleColorsStellar();
    static void styleColorsDeepSpace();

    // Accessor for the palette from other files
    static NebulaPalette& getPalette() { return nebula; }
};
