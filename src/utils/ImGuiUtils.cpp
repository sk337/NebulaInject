// ImGuiUtils.cpp
#include "ImGuiUtils.h"
#include <imgui.h>
#include <iostream>

// Define the global palette
NebulaPalette nebula = {
    // Bg, ChildBg, PopupBg, Border
    ImVec4(0.03f, 0.03f, 0.06f, 1.00f),
    ImVec4(0.05f, 0.04f, 0.08f, 0.95f),
    ImVec4(0.06f, 0.05f, 0.10f, 0.95f),
    ImVec4(0.18f, 0.10f, 0.28f, 1.00f),

    // Text, TextMuted, TextDisabled
    ImVec4(0.86f, 0.80f, 0.95f, 1.00f),
    ImVec4(0.60f, 0.52f, 0.70f, 1.00f),
    ImVec4(0.5f, 0.45f, 0.55f, 1.00f),

    // Primary, PrimaryHover, PrimaryActive
    ImVec4(0.30f, 0.14f, 0.55f, 1.00f),
    ImVec4(0.38f, 0.20f, 0.62f, 1.00f),
    ImVec4(0.46f, 0.28f, 0.70f, 1.00f),

    // Button, ButtonHover, ButtonActive
    ImVec4(0.18f, 0.09f, 0.35f, 1.00f),
    ImVec4(0.25f, 0.14f, 0.45f, 1.00f),
    ImVec4(0.34f, 0.22f, 0.58f, 1.00f),

    // FrameBg, FrameBgHover
    ImVec4(0.08f, 0.06f, 0.12f, 1.00f),
    ImVec4(0.12f, 0.09f, 0.18f, 1.00f),

    // CheckMark, Accent
    ImVec4(0.92f, 0.52f, 1.00f, 1.00f),
    ImVec4(0.70f, 0.40f, 1.00f, 1.00f)
};

void ImGuiUtils::drawHelper(const char *desc) {
    ImGui::PushStyleColor(ImGuiCol_Text, nebula.Text);
    ImGui::TextDisabled("(?)");
    ImGui::PopStyleColor();
    
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushStyleColor(ImGuiCol_PopupBg, nebula.PopupBg);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(nebula.Text.x * 0.35f, nebula.Text.y * 0.3f, nebula.Text.z * 0.36f, 1.0f));
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor(2);
        ImGui::EndTooltip();
    }
}

void ImGuiUtils::NebulaDarkColors() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* c = style.Colors;

    // Backgrounds
    c[ImGuiCol_WindowBg]      = nebula.Bg;
    c[ImGuiCol_ChildBg]       = nebula.ChildBg;
    c[ImGuiCol_PopupBg]       = nebula.PopupBg;

    // Text
    c[ImGuiCol_Text]          = nebula.Text;
    c[ImGuiCol_TextDisabled]  = nebula.TextDisabled;

    // Frames / Inputs
    c[ImGuiCol_FrameBg]       = nebula.FrameBg;
    c[ImGuiCol_FrameBgHovered]= nebula.FrameBgHover;
    c[ImGuiCol_FrameBgActive] = nebula.FrameBgHover;

    // Buttons
    c[ImGuiCol_Button]        = nebula.Button;
    c[ImGuiCol_ButtonHovered] = nebula.ButtonHover;
    c[ImGuiCol_ButtonActive]  = nebula.ButtonActive;

    // Headers / Tabs
    c[ImGuiCol_Header]        = nebula.Primary;
    c[ImGuiCol_HeaderHovered] = nebula.PrimaryHover;
    c[ImGuiCol_HeaderActive]  = nebula.PrimaryActive;
    c[ImGuiCol_Tab]           = nebula.Button;
    c[ImGuiCol_TabHovered]    = nebula.ButtonHover;
    c[ImGuiCol_TabActive]     = nebula.Primary;

    // Scrollbars / Grab
    c[ImGuiCol_ScrollbarBg]   = nebula.ChildBg;
    c[ImGuiCol_ScrollbarGrab] = nebula.FrameBg;
    c[ImGuiCol_ScrollbarGrabHovered] = nebula.FrameBgHover;

    // Misc
    c[ImGuiCol_Border]        = nebula.Border;
    c[ImGuiCol_CheckMark]     = nebula.CheckMark;
    c[ImGuiCol_TitleBg]       = nebula.Primary;
    c[ImGuiCol_TitleBgActive] = nebula.PrimaryActive;
    c[ImGuiCol_ModalWindowDimBg] = ImVec4(0,0,0,0.45f);

    // Style tweaks
    style.WindowRounding = 10.0f;
    style.ChildRounding  = 8.0f;
    style.FrameRounding  = 6.0f;
    style.PopupRounding  = 8.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding   = 6.0f;

    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing  = ImVec2(10, 6);
    style.IndentSpacing = 16;
    style.ScrollbarSize = 14;

    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize  = 1.0f;
}

void ImGuiUtils::styleColorsNebula() {
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    // Apply nebula color scheme
    colors[ImGuiCol_Text]                 = nebula.Text;
    colors[ImGuiCol_TextDisabled]         = nebula.TextDisabled;
    colors[ImGuiCol_WindowBg]             = nebula.Bg;
    colors[ImGuiCol_ChildBg]              = nebula.ChildBg;
    colors[ImGuiCol_PopupBg]              = nebula.PopupBg;
    colors[ImGuiCol_Border]               = nebula.Border;
    colors[ImGuiCol_FrameBg]              = nebula.FrameBg;
    colors[ImGuiCol_FrameBgHovered]       = nebula.FrameBgHover;
    colors[ImGuiCol_FrameBgActive]        = nebula.FrameBgHover;
    colors[ImGuiCol_TitleBg]              = nebula.Primary;
    colors[ImGuiCol_TitleBgActive]        = nebula.PrimaryActive;
    colors[ImGuiCol_MenuBarBg]            = nebula.ChildBg;
    colors[ImGuiCol_ScrollbarBg]          = nebula.ChildBg;
    colors[ImGuiCol_ScrollbarGrab]        = nebula.FrameBg;
    colors[ImGuiCol_ScrollbarGrabHovered] = nebula.FrameBgHover;
    colors[ImGuiCol_CheckMark]            = nebula.CheckMark;
    colors[ImGuiCol_SliderGrab]           = nebula.FrameBg;
    colors[ImGuiCol_SliderGrabActive]     = nebula.FrameBgHover;
    colors[ImGuiCol_Button]               = nebula.Button;
    colors[ImGuiCol_ButtonHovered]        = nebula.ButtonHover;
    colors[ImGuiCol_ButtonActive]         = nebula.ButtonActive;
    colors[ImGuiCol_Header]               = nebula.Primary;
    colors[ImGuiCol_HeaderHovered]        = nebula.PrimaryHover;
    colors[ImGuiCol_HeaderActive]         = nebula.PrimaryActive;
    colors[ImGuiCol_Separator]            = nebula.Border;
    colors[ImGuiCol_ResizeGrip]           = nebula.FrameBg;
    colors[ImGuiCol_Tab]                  = nebula.Button;
    colors[ImGuiCol_TabHovered]           = nebula.ButtonHover;
    colors[ImGuiCol_TabActive]            = nebula.Primary;

    // Layout settings
    style->FramePadding = ImVec2(8, 4);
    style->ItemSpacing = ImVec2(12, 6);
    style->IndentSpacing = 16;
    style->ScrollbarSize = 14;

    style->WindowRounding = 10;
    style->ChildRounding = 8;
    style->FrameRounding = 8;
    style->PopupRounding = 8;
    style->ScrollbarRounding = 10;
    style->GrabRounding = 8;
    style->TabRounding = 8;

    style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style->WindowMenuButtonPosition = ImGuiDir_Right;
    style->DisplaySafeAreaPadding = ImVec2(6, 6);

    style->WindowBorderSize = 1.0f;
    style->ChildBorderSize = 1.0f;
    style->FrameBorderSize = 0.0f;
}

// A gold-tinged variant
void ImGuiUtils::styleColorsNebulaGold() {
    NebulaPalette saved = nebula;
    nebula.Accent = ImVec4(0.95f, 0.78f, 0.35f, 1.0f);
    nebula.Primary = ImVec4(0.55f, 0.40f, 0.10f, 1.0f);
    nebula.PrimaryHover = ImVec4(0.65f, 0.48f, 0.18f, 1.0f);
    nebula.Button = ImVec4(0.25f, 0.18f, 0.08f, 1.0f);
    styleColorsNebula();
    nebula = saved;
}

// Cinder: dark/red-orange variant
void ImGuiUtils::styleColorsCinder() {
    NebulaPalette saved = nebula;
    nebula.Bg = ImVec4(0.06f, 0.03f, 0.03f, 1.00f);
    nebula.Primary = ImVec4(0.60f, 0.18f, 0.12f, 1.00f);
    nebula.PrimaryHover = ImVec4(0.72f, 0.28f, 0.18f, 1.00f);
    nebula.Button = ImVec4(0.32f, 0.12f, 0.10f, 1.00f);
    styleColorsNebula();
    nebula = saved;
}

// Stellar: lighter / brighter variant
void ImGuiUtils::styleColorsStellar() {
    NebulaPalette saved = nebula;
    nebula.Bg = ImVec4(0.95f, 0.95f, 0.98f, 1.0f);
    nebula.Text = ImVec4(0.10f, 0.08f, 0.12f, 1.0f);
    nebula.Button = ImVec4(0.85f, 0.75f, 0.95f, 1.0f);
    nebula.FrameBg = ImVec4(0.95f, 0.93f, 0.97f, 1.0f);
    styleColorsNebula();
    nebula = saved;
}

// Deep space: very dark, high contrast variant
void ImGuiUtils::styleColorsDeepSpace() {
    NebulaPalette saved = nebula;
    nebula.Bg = ImVec4(0.01f, 0.01f, 0.02f, 1.0f);
    nebula.ChildBg = ImVec4(0.03f, 0.03f, 0.06f, 0.95f);
    nebula.Text = ImVec4(0.95f, 0.92f, 1.0f, 1.0f);
    nebula.Button = ImVec4(0.10f, 0.06f, 0.16f, 1.0f);
    nebula.ButtonHover = ImVec4(0.20f, 0.12f, 0.32f, 1.0f);
    styleColorsNebula();
    nebula = saved;
}
