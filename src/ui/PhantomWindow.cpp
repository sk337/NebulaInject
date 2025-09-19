// NebulaWindow.cpp — patched for theme system, fonts and ImGui backend
// correctness
#include "PhantomWindow.h"
#include "../utils/ImGuiUtils.h"
#include "../utils/XUtils.h"
#include "NebulaFontLoader.h"
#include <SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <string>
#include <vector>

// Constants for UI layout
static constexpr float SIDEBAR_WIDTH = 160.0f;
static constexpr float SETTINGS_WIDTH = 540.0f;
static constexpr float SIDEBAR_HEIGHT = 380.0f;
static constexpr float TOPBAR_HEIGHT = 40.0f;

// Nerd Font icons (use plain const char* string literals)
#define ICON_FA_MODULES "\uf085"
#define ICON_FA_SETTINGS "\uf013"
#define ICON_NF_CHEVRON_RIGHT "\ueab2"
#define ICON_NF_CHEVRON_DOWN "\ueab7"
#define ICON_NF_STAR "\uf02a"
#define ICON_NF_PLANET "\uf7ac"
#define ICON_NF_GALAXY "\ue2af"
#define ICON_NF_ROCKET "\uf135"
#define ICON_NF_KEY "\uf805"
#define ICON_NF_TOGGLE_ON "\uf205"
#define ICON_NF_TOGGLE_OFF "\uf204"
#define ICON_NF_COSMIC "\ue2ae"
#define ICON_NF_SPARKLES "\uf890"

NebulaWindow::NebulaWindow(int width, int height, const char *title) {
  this->width = width;
  this->height = height;
  this->title = title;
  this->style = 0;
  this->window = nullptr;
  this->glContext = nullptr;
}

static void AlignForWidth(float width, float alignment = 0.5f) {
  float avail = ImGui::GetContentRegionAvail().x;
  float off = (avail - width) * alignment;
  if (off > 0.0f)
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
}

void NebulaWindow::setup() {
  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
      0) {
    printf("Error: %s\n", SDL_GetError());
    exit(1);
  }

  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  auto window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

  window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       width, height, window_flags);
  if (!window) {
    printf("Error creating window: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  glContext = SDL_GL_CreateContext(window);
  if (!glContext) {
    printf("Error creating GL context: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  SDL_GL_MakeCurrent(window, glContext);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  // 1) Create ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  // 2) Initialize Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window, glContext);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // 3) Load fonts
  NebulaFontLoader::loadFonts();

  // 4) Create font texture
  ImGui_ImplOpenGL3_CreateFontsTexture();

  // 5) Apply initial theme (Nebula)
  ImGuiUtils::styleColorsNebula();

  // Global style tuning
  ImGuiStyle &styleRef = ImGui::GetStyle();
  styleRef.WindowRounding = 12.0f;
  styleRef.FrameRounding = 8.0f;
  styleRef.PopupRounding = 8.0f;
  styleRef.ScrollbarRounding = 10.0f;
  styleRef.GrabRounding = 8.0f;
  styleRef.TabRounding = 8.0f;

  styleRef.WindowPadding = ImVec2(10, 10);
  styleRef.FramePadding = ImVec2(12, 6);
  styleRef.ItemSpacing = ImVec2(10, 8);
  styleRef.ItemInnerSpacing = ImVec2(8, 6);
}

// Global state variables
std::string CurrentMenu = "Nothing";
int Tab = 2;

void NebulaWindow::update(const std::vector<Cheat *> &cheats, bool &running,
                          bool inGame) {
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT)
      running = false;
    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_CLOSE &&
        event.window.windowID == SDL_GetWindowID(window))
      running = false;
  }

  // Start the Dear ImGui frame
  ImGui_ImplSDL2_NewFrame(window);
  ImGui_ImplOpenGL3_NewFrame();
  ImGui::NewFrame();

  // Draw widgets with Nebula styling
  {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
    ImGui::Begin("Nebula Interface", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove);

    // Header
    ImGui::SetCursorPos(ImVec2(20, 15));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiUtils::getPalette().Primary);
    ImGui::Text("========== [ NEBULA CONTROL MATRIX ] ==========");
    ImGui::PopStyleColor();

    // Animated star field
    ImGui::SetCursorPos(ImVec2(50, 40));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiUtils::getPalette().Text);
    ImGui::Text("= - = - = - = - NEBULA - = - = - = - =");
    ImGui::PopStyleColor();

    // Sidebar
    ImGui::SetCursorPos(ImVec2(40, 110));
    ImGui::BeginChild("NebulaSidebar", ImVec2(SIDEBAR_WIDTH, SIDEBAR_HEIGHT),
                      true, ImGuiWindowFlags_NoScrollbar);

    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiUtils::getPalette().Text);
    ImGui::Text("[* MODULES *]");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();

    if (Tab == 1 && inGame) {
      for (Cheat *cheat : cheats) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        std::string buttonText = "*( " + std::string(cheat->getName()) + " )*";
        if (ImGui::Button(buttonText.c_str(), ImVec2(135, 32))) {
          CurrentMenu = cheat->getName();
        }
        ImGui::PopStyleVar();

        if (ImGui::IsItemHovered()) {
          ImGui::SameLine();
          ImGuiUtils::drawHelper(cheat->getDescription());
        }
        ImGui::Spacing();
      }
    }
    ImGui::EndChild();

    // Settings area
    ImGui::SetCursorPos(ImVec2(205, 110));
    ImGui::BeginChild("NebulaSettings", ImVec2(SETTINGS_WIDTH, SIDEBAR_HEIGHT),
                      true);

    if (Tab == 1) {
      if (inGame) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGuiUtils::getPalette().Primary);
        ImGui::Text("0 * 0 ACTIVE MODULES 0 * 0");
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();

        for (Cheat *cheat : cheats) {
          if (CurrentMenu == cheat->getName()) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            ImGui::PushStyleColor(ImGuiCol_CheckMark,
                                  ImGuiUtils::getPalette().CheckMark);
            ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                  ImGuiUtils::getPalette().FrameBg);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,
                                  ImGuiUtils::getPalette().FrameBgHover);

            std::string checkboxText =
                "★ Enable " + std::string(cheat->getName()) + " ★";
            ImGui::Checkbox(checkboxText.c_str(), &cheat->enabled);
            ImGui::PopStyleColor(3);

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImGuiUtils::getPalette().TextMuted);
            ImGui::Text("* * * Configuration Matrix * * *");
            ImGui::PopStyleColor();

            ImGui::Indent();
            cheat->renderSettings();
            ImGui::Unindent();

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImGuiUtils::getPalette().Button);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImGuiUtils::getPalette().ButtonHover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImGuiUtils::getPalette().ButtonActive);

            if (ImGui::Button("* Bind Key *", ImVec2(120, 28))) {
              cheat->binding = true;
            }

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(ImGuiUtils::getPalette().Text.x,
                                         ImGuiUtils::getPalette().Text.y,
                                         ImGuiUtils::getPalette().Text.z,
                                         cheat->binding ? 0.95f : 0.85f));
            if (cheat->binding) {
              ImGui::Text("{ Binding: <...> }");
            } else {
              ImGui::Text("o Key: <%d> o", cheat->bind);
            }
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar();
          }
        }
      } else {
        ImGui::PushStyleColor(ImGuiCol_Text,
                              ImGuiUtils::getPalette().TextMuted);
        ImGui::Text("o * o Please join a world to access modules o * o");
        ImGui::PopStyleColor();
      }
    }

    // Customization tab
    if (Tab == 2) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImGuiUtils::getPalette().Primary);
      ImGui::Text("* o * NEBULA CUSTOMIZATION MATRIX * o *");
      ImGui::PopStyleColor();
      ImGui::Separator();
      ImGui::Spacing();

      // Placeholder selectors
      XUtils::renderMouseSelector();
      ImGui::Spacing();
      XUtils::renderKeyboardSelector();
      ImGui::Spacing();

      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
      ImGui::PushStyleColor(ImGuiCol_Text, ImGuiUtils::getPalette().Text);
      ImGui::Text("* Theme Selection *");
      ImGui::PopStyleColor();

      // Theme selector
      const char *themeItems = "Dark Nebula\0Cosmic Gold\0Void Cinder\0Stellar "
                               "Light\0Eclipse Dark\0Classic\0Deep Space\0";
      if (ImGui::Combo("##ThemeStyle", &style, themeItems)) {
        switch (style) {
        case 0:
          ImGuiUtils::styleColorsNebula();
          break;
        case 1:
          ImGuiUtils::styleColorsNebula();
          break; // Placeholder for gold theme
        case 2:
          ImGui::StyleColorsDark();
          break;
        case 3:
          ImGui::StyleColorsLight();
          break;
        case 4:
          ImGui::StyleColorsDark();
          break;
        case 5:
          ImGui::StyleColorsClassic();
          break;
        case 6:
          ImGuiUtils::NebulaDarkColors();
          break;
        }

        // Preserve layout tweaks
        ImGuiStyle &s = ImGui::GetStyle();
        s.WindowRounding = 12.0f;
        s.FrameRounding = 8.0f;
        s.PopupRounding = 8.0f;
        s.ScrollbarRounding = 10.0f;
        s.GrabRounding = 8.0f;
        s.TabRounding = 8.0f;
      }
      ImGui::PopStyleVar();
    }

    ImGui::EndChild();

    // Topbar
    ImGui::SetCursorPos(ImVec2(220, 65));
    ImGui::BeginChild("NebulaTopbar", ImVec2(320, 35), true,
                      ImGuiWindowFlags_NoScrollbar);

    ImGuiStyle &Style = ImGui::GetStyle();
    float sectionWidth = 0.0f;
    sectionWidth += ImGui::CalcTextSize("* Modules *").x;
    sectionWidth += Style.ItemSpacing.x;
    sectionWidth += ImGui::CalcTextSize("* Settings *").x;
    AlignForWidth(sectionWidth);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);

    // Dynamic button colors
    ImVec4 activeColor = ImGuiUtils::getPalette().Primary;
    ImVec4 inactiveColor = ImVec4(ImGuiUtils::getPalette().Button.x,
                                  ImGuiUtils::getPalette().Button.y,
                                  ImGuiUtils::getPalette().Button.z, 0.85f);

    ImGui::PushStyleColor(ImGuiCol_Button,
                          Tab == 1 ? activeColor : inactiveColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImGuiUtils::getPalette().ButtonHover);
    if (ImGui::Button("* Modules *"))
      Tab = 1;
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button,
                          Tab == 2 ? activeColor : inactiveColor);
    if (ImGui::Button("* Settings *"))
      Tab = 2;

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    ImGui::EndChild();

    // Title area
    ImGui::SetCursorPos(ImVec2(40, 75));
    ImGui::PushStyleColor(ImGuiCol_Text, ImGuiUtils::getPalette().Primary);
    ImGui::Text("*~*~ NEBULA ~*~*");
    ImGui::PopStyleColor();

    ImGui::End();
  }

  // Render
  ImGui::Render();
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

  // Gentle background
  ImVec4 bg = ImGuiUtils::getPalette().Bg;
  glClearColor(bg.x, bg.y, bg.z, bg.w);
  glClear(GL_COLOR_BUFFER_BIT);

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(window);
}

void NebulaWindow::destruct() {
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
