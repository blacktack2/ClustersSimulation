#include "WindowHandler.h"

#include "../../imgui/imgui_impl_sdl.h"
#include "../../imgui/imgui_impl_opengl3.h"
#include "../../imgui/imgui_stdlib.h"

#include <cstdio>
#include <chrono>
#include <glad/glad.h>

#define PANEL_PADDING 10
#define PANEL_MARGINS 10

#define DEBUG_PANEL_WIDTH_MIN  200
#define DEBUG_PANEL_HEIGHT_MIN 100

#define IO_PANEL_WIDTH_MIN  200
#define IO_PANEL_HEIGHT_MIN 400

#define SIM_PANEL_WIDTH_MIN  500 + PANEL_PADDING * 2
#define SIM_PANEL_HEIGHT_MIN 500 + PANEL_PADDING * 2

#if (DEBUG_PANEL_WIDTH_MIN > IO_PANEL_WIDTH_MIN)
#define WINDOW_WIDTH_MIN DEBUG_PANEL_WIDTH_MIN + DEBUG_PANEL_WIDTH_MIN + PANEL_MARGINS * 4
#else
#define WINDOW_WIDTH_MIN DEBUG_PANEL_WIDTH_MIN + SIM_PANEL_WIDTH_MIN + PANEL_MARGINS * 4
#endif

#if (DEBUG_PANEL_HEIGHT_MIN + IO_PANEL_HEIGHT_MIN + PANEL_MARGINS * 2 > SIM_PANEL_HEIGHT_MIN)
#define WINDOW_HEIGHT_MIN DEBUG_PANEL_HEIGHT_MIN + IO_PANEL_HEIGHT_MIN + PANEL_MARGINS * 4
#else
#define WINDOW_HEIGHT_MIN SIM_PANEL_HEIGHT_MIN + PANEL_MARGINS * 2
#endif

WindowHandler::WindowHandler() :
mWindowWidth(0), mWindowHeight(0), mRunning(false), mSimulationRunning(false),
mWindow(nullptr), mLSHandler(nullptr), mLSRenderer(nullptr) {

}

WindowHandler::~WindowHandler() {
    if (mLSHandler != nullptr) {
        delete mLSHandler;
        mLSHandler = nullptr;
    }

    if (mLSRenderer != nullptr) {
        delete mLSRenderer;
        mLSRenderer = nullptr;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(mGlContext);

    if (mWindow != nullptr) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }

    SDL_Quit();
}

bool WindowHandler::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    std::string glslVersion;

#ifdef __APPLE__
    // GL 3.2 Core + GLSL 150
    glslVersion = "#version 150";
    SDL_GL_SetAttribute( // required on Mac OS
        SDL_GL_CONTEXT_FLAGS,
        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
        );
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#elif __linux__
    // GL 3.2 Core + GLSL 150
    glslVersion = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#elif _WIN32
    // GL 3.0 + GLSL 130
    glslVersion = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

    SDL_WindowFlags windowFlags = (SDL_WindowFlags)(
            SDL_WINDOW_OPENGL |
            SDL_WINDOW_RESIZABLE |
            SDL_WINDOW_ALLOW_HIGHDPI
            );

    mWindow = SDL_CreateWindow(
            "Life Simulation",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            mWindowWidth, mWindowHeight,
            windowFlags);

    if (mWindow == nullptr) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetWindowMinimumSize(mWindow, WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN);
    mGlContext = SDL_GL_CreateContext(mWindow);

    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        fprintf(stderr, "Failed to initialize glad!");
        return false;
    }

    glViewport(0, 0, mWindowWidth, mWindowHeight);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    mIo = ImGui::GetIO(); (void)mIo;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(mWindow, mGlContext);
    ImGui_ImplOpenGL3_Init(glslVersion.c_str());

    glClearColor(0, 0, 0, 1);

    mLSHandler = new LifeSimulationHandler();
    mLSRenderer = new LifeSimulationRenderer(*mLSHandler);

    mLSHandler->setBounds(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight));
    mLSHandler->initSimulation();

    return true;
}
void WindowHandler::mainloop() {
    mRunning = true;

    SDL_Event e;

    int fpsHistory[] = {0, 0, 0, 0, 0};
    int fpsIndex = 0;
    int fpsTotal = 0;
    int fpsCounter = 0;

    int lastFpsCheck = 0;

    ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            ImVec2(static_cast<float>(PANEL_PADDING), static_cast<float>(PANEL_PADDING))
            );

    while (mRunning) {
        int currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (currentTime - lastFpsCheck >= 1000) {
            lastFpsCheck = currentTime;
            fpsIndex++;
            if (fpsIndex > 4) {
                fpsIndex = 0;
            }
            fpsTotal -= fpsHistory[fpsIndex];
            fpsTotal += fpsCounter;
            fpsHistory[fpsIndex] = fpsCounter;

            fpsCounter = 0;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        while (SDL_PollEvent(&e) != 0) {
            handleEvent(e);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(mWindow);
        ImGui::NewFrame();

        {
            int sdlWidth = 0;
            int sdlHeight = 0;
            SDL_GetWindowSize(mWindow, &sdlWidth, &sdlHeight);

            ImVec4 debugPanelBounds = ImVec4(
                    static_cast<float>(PANEL_MARGINS),
                    static_cast<float>(PANEL_MARGINS),
                    std::max((sdlWidth - PANEL_MARGINS * 4) * 2 / 7, DEBUG_PANEL_WIDTH_MIN),
                    std::max((sdlHeight - PANEL_MARGINS * 4) / 5, DEBUG_PANEL_HEIGHT_MIN)
                    );
            ImVec4 ioPanelBounds = ImVec4(
                    static_cast<float>(PANEL_MARGINS),
                    static_cast<float>(debugPanelBounds.y + debugPanelBounds.w + PANEL_MARGINS * 2),
                    std::max((sdlWidth - PANEL_MARGINS * 4) * 2 / 7, IO_PANEL_WIDTH_MIN),
                    std::max((sdlHeight - PANEL_MARGINS * 4) * 4 / 5, IO_PANEL_HEIGHT_MIN)
            );
            ImVec4 simPanelBounds = ImVec4(
                    static_cast<float>(std::max(debugPanelBounds.x + debugPanelBounds.z, ioPanelBounds.x + ioPanelBounds.z) + PANEL_MARGINS * 2),
                    static_cast<float>(PANEL_MARGINS),
                    std::max((sdlWidth - PANEL_MARGINS * 4) * 5 / 7, SIM_PANEL_WIDTH_MIN),
                    std::max(sdlHeight - PANEL_MARGINS * 2, SIM_PANEL_HEIGHT_MIN)
            );

            ImGui::SetNextWindowPos(
                    ImVec2(debugPanelBounds.x, debugPanelBounds.y),
                    ImGuiCond_Always
                    );
            ImGui::SetNextWindowSize(
                    ImVec2(debugPanelBounds.z, debugPanelBounds.w),
                    ImGuiCond_Always
                    );

            ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            drawDebugPanel(static_cast<float>(fpsTotal) / 10.0f);

            ImGui::End();

            ImGui::SetNextWindowPos(
                    ImVec2(ioPanelBounds.x, ioPanelBounds.y),
                    ImGuiCond_Always
                    );
            ImGui::SetNextWindowSize(
                    ImVec2(ioPanelBounds.z, ioPanelBounds.w),
                    ImGuiCond_Always
            );

            ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            drawIOPanel(ioPanelBounds.x, ioPanelBounds.y, ioPanelBounds.z, ioPanelBounds.w);

            ImGui::End();

            ImGui::SetNextWindowPos(
                    ImVec2(simPanelBounds.x, simPanelBounds.y),
                    ImGuiCond_Always
                    );
            ImGui::SetNextWindowSize(
                    ImVec2(simPanelBounds.z, simPanelBounds.w),
                    ImGuiCond_Always
            );

            ImGui::Begin("Simulation", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

            mLSRenderer->drawSimulation(simPanelBounds.x, simPanelBounds.y, simPanelBounds.z, simPanelBounds.w);

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(mWindow);

        fpsCounter++;

        if (mSimulationRunning) {
            mLSHandler->iterateSimulation();
        }
    }
}

void WindowHandler::setSize(int width, int height) {
    mWindowWidth = width;
    mWindowHeight = height;
}

int WindowHandler::getWidth() const {
    return mWindowWidth;
}

int WindowHandler::getHeight() const {
    return mWindowHeight;
}

void WindowHandler::handleEvent(SDL_Event& e) {
    ImGui_ImplSDL2_ProcessEvent(&e);

    switch (e.type) {
        case SDL_QUIT:
            mRunning = false;
            break;
        case SDL_WINDOWEVENT:
            switch (e.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    mWindowWidth  = e.window.data1;
                    mWindowHeight = e.window.data2;
                    glViewport(0, 0, mWindowWidth, mWindowHeight);
                    break;
            }
            break;
        case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
                case SDLK_SPACE:
                    mSimulationRunning = !mSimulationRunning;
                    break;
            }
            break;
    }
}

void WindowHandler::drawDebugPanel(float fps) {
    const ImVec4 debugTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    ImGui::TextColored(
            debugTextColor,
            "[FPS: %f]", fps
    );
    ImGui::TextColored(
            debugTextColor,
            "Atom Count: %zu", mLSHandler->getAtoms().size()
    );
}

void WindowHandler::drawIOPanel(float x, float y, float width, float height) {
    std::string label;
    if (mSimulationRunning) {
        if (ImGui::Button("Pause")) {
            mSimulationRunning = false;
        }
    } else {
        if (ImGui::Button("Play")) {
            mSimulationRunning = true;
        }
    }
    if (ImGui::Button("Re-Init Simulation")) {
        mLSHandler->initSimulation();
    }
    if (ImGui::Button("Clear Simulation")) {
        mLSHandler->clearSimulation();
    }
    if (ImGui::Button("Randomize Positions")) {
        mLSHandler->shuffleAtomPositions();
    }
    if (ImGui::Button("Shuffle Interactions")) {
        mLSHandler->shuffleAtomInteractions();
    }

    LifeSimulationRules& rules = mLSHandler->getLSRules();

    if (ImGui::Button("Add Atom Type")) {
        rules.newAtomType();
    }
    std::vector<AtomType*>& atomTypes = rules.getAtomTypes();
    for (AtomType* atomType : atomTypes) {
        Color c = atomType->getColor();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(c.r, c.g, c.b, 1.0));
        // Textbox for the friendly name of each AtomType
        std::string friendlyName = atomType->getFriendlyName();
        label = "##FriendlyNameText-" + std::to_string(atomType->getId());
        if (ImGui::InputText(label.c_str(), &friendlyName)) {
            atomType->setFriendlyName(friendlyName);
        }
        ImGui::PopStyleColor(1);

        // Three 0.0-1.0 float inputs for the rgb color of each AtomType
        float r = c.r;
        float g = c.g;
        float b = c.b;
        ImGui::PushItemWidth((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ColumnsMinSpacing * 2) / 3);
        label = "##ColorRedSlider-" + std::to_string(atomType->getId());
        if (ImGui::SliderFloat(label.c_str(), &r, 0.0f, 1.0f)) {
            atomType->setColorR(r);
        }
        ImGui::SameLine();
        label = "##ColorGreenSlider-" + std::to_string(atomType->getId());
        if (ImGui::SliderFloat(label.c_str(), &g, 0.0f, 1.0f)) {
            atomType->setColorG(g);
        }
        ImGui::SameLine();
        label = "##ColorBlueSlider-" + std::to_string(atomType->getId());
        if (ImGui::SliderFloat(label.c_str(), &b, 0.0f, 1.0f)) {
            atomType->setColorB(b);
        }
        ImGui::PopItemWidth();

        int quantity = atomType->getQuantity();
        label = "Quantity##QuantityInt-" + std::to_string(atomType->getId());
        if (ImGui::InputInt(label.c_str(), &quantity)) {
            atomType->setQuantity(quantity);
        }

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2);
        for (AtomType* atomType2 : atomTypes) {
            label = "0##ZeroButton-" + std::to_string(atomType->getId()) + "-" + std::to_string(atomType2->getId());
            if (ImGui::Button(label.c_str())) {
                rules.setInteraction(atomType->getId(), atomType2->getId(), 0);
            }
            ImGui::SameLine();
            float interaction = rules.getInteraction(atomType->getId(), atomType2->getId());
            label = atomType->getFriendlyName() + "->" + atomType2->getFriendlyName() + "##InteractionSlider-"
                    + std::to_string(atomType->getId()) + "-" + std::to_string(atomType2->getId());
            if (ImGui::SliderFloat(label.c_str(), &interaction, -1.0f, 1.0f)) {
                rules.setInteraction(atomType->getId(), atomType2->getId(), interaction);
            }
        }
        ImGui::PopItemWidth();
    }
}
