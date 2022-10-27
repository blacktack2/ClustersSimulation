#include "WindowHandler.h"

#include "../../imgui/imgui_impl_sdl.h"
#include "../../imgui/imgui_impl_opengl3.h"
#include "../../imgui/imgui_stdlib.h"

#include <glad/glad.h>

#include <array>
#include <cstdio>
#include <chrono>
#include <filesystem>

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
mWindow(nullptr), mSimulationHandler(), mSimulationRenderer(mSimulationHandler),
mFileSaveLocation("sampleFile"), mFileLoadLocations(), mFileLoadIndex(0), mFileLoadCount(0), mIsOverwritingFile(false) {

}

WindowHandler::~WindowHandler() {
    saveToFile("resources/current.csdat", mSimulationHandler);

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

#ifdef ITERATE_ON_COMPUTE_SHADER
    mSimulationHandler.initComputeShaders();
#endif

    if (!getLoadableFiles(mFileLoadLocations, mFileLoadCount)) {
        fprintf(stderr, "Failed to read config files!\n");
        return false;
    }
    loadFromFile("resources/current.csdat", mSimulationHandler);

    mSimulationHandler.initSimulation();

#ifdef _DEBUG
    glCheckError();
#endif

//#ifdef ITERATE_ON_COMPUTE_SHADER
//#ifdef _DEBUG
//    int flags;
//    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
//    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
//        glEnable(GL_DEBUG_OUTPUT);
//        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
//        glDebugMessageCallback(glDebugOutput, nullptr);
//        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
//    } 
//#endif
//#endif

    return true;
}
void WindowHandler::mainloop() {
    mRunning = true;

    SDL_Event e;

    float mspfTotal = 0;
    float mspf = 1;
    float delta = 0;
    unsigned int lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int frameCounter = 0;

    ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            ImVec2(static_cast<float>(PANEL_PADDING), static_cast<float>(PANEL_PADDING))
            );

    while (mRunning) {
        unsigned int currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        delta += currentTime - lastTime;
        lastTime = currentTime;
        if (delta > 1000.0f) {
            mspf = delta / frameCounter;
            delta = 0.0f;
            frameCounter = 0;
        }
        frameCounter++;

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

            drawDebugPanel(mspf);

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

            mSimulationRenderer.drawSimulation(simPanelBounds.x, simPanelBounds.y, simPanelBounds.z, simPanelBounds.w);

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(mWindow);

        if (mSimulationRunning) {
            mSimulationHandler.iterateSimulation();
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

void WindowHandler::drawDebugPanel(float mspf) {
    const ImVec4 debugTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    ImGui::TextColored(
            debugTextColor,
            "[FPS: %.2f | %.2fms]", 1000.0f / mspf, mspf
    );
    ImGui::TextColored(
            debugTextColor,
            "Atom Count: %zu", mSimulationHandler.getAtomCount()
    );
}

void WindowHandler::drawIOPanel(float x, float y, float width, float height) {
    std::string label;

    mSimulationRunning = mSimulationRunning ?
        !ImGui::Button("Pause##PlayPause", ImVec2(ImGui::GetContentRegionAvail().x, 0)) :
        ImGui::Button("Play##PlayPause", ImVec2(ImGui::GetContentRegionAvail().x, 0));
    if (ImGui::Button("Single Iteration", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        mSimulationRunning = false;
        mSimulationHandler.iterateSimulation();
    }

    ImGui::Button("Re-generate Atoms", ImVec2(ImGui::GetContentRegionAvail().x, 0)) ?
        mSimulationHandler.initSimulation() : 0;
    ImGui::Button("Clear Atoms", ImVec2(ImGui::GetContentRegionAvail().x, 0)) ?
        mSimulationHandler.clearAtoms() : 0;
    ImGui::Button("Clear Atom Types", ImVec2(ImGui::GetContentRegionAvail().x, 0)) ?
        mSimulationHandler.clearAtomTypes() : 0;
    ImGui::Button("Randomize Positions", ImVec2(ImGui::GetContentRegionAvail().x, 0)) ?
        mSimulationHandler.shuffleAtomPositions() : 0;
    ImGui::Button("Zero Interactions", ImVec2(ImGui::GetContentRegionAvail().x, 0)) ?
        mSimulationHandler.zeroAtomInteractions() : 0;
    ImGui::Button("Shuffle Interactions", ImVec2(ImGui::GetContentRegionAvail().x, 0)) ?
        mSimulationHandler.shuffleAtomInteractions() : 0;

    ImGui::Text("Simulation Scale");
    float simScale = mSimulationHandler.getWidth();
    if (ImGui::InputFloat("##Simulation Scale", &simScale, 1.0f, 10.0f, "%.0f")) {
        simScale = std::max(std::min(simScale, 1000000.0f), 1.0f);
        mSimulationHandler.setBounds(simScale, simScale);
    }
    ImGui::Text("Time Delta (dt)");
    float dt = mSimulationHandler.getDt();
    if (ImGui::InputFloat("##Time Delta", &dt, 0.01f, 0.1f, "%.2f")) {
        dt = std::max(std::min(dt, 10.0f), 0.01f);
        mSimulationHandler.setDt(dt);
    }
    ImGui::Text("Drag Force");
    float drag = mSimulationHandler.getDrag();
    if (ImGui::InputFloat("##Drag Force", &drag, 0.01f, 0.1f, "%.2f")) {
        drag = std::max(std::min(drag, 1.0f), 0.0f);
        mSimulationHandler.setDrag(drag);
    }
    ImGui::Text("Max Interaction Range");
    float interactionRange = mSimulationHandler.getInteractionRange();
    if (ImGui::InputFloat("##Max Interaction Range", &interactionRange, 1.0f, 10.0f, "%.0f")) {
        interactionRange = std::max(std::min(interactionRange, simScale / 2.0f), 1.0f);
        mSimulationHandler.setInteractionRange(interactionRange);
    }
    ImGui::Text("Collision Force");
    float collisionForce = mSimulationHandler.getCollisionForce();
    if (ImGui::InputFloat("##Collision Force", &collisionForce, 0.01f, 0.1f, "%.2f")) {
        collisionForce = std::max(std::min(collisionForce, 10.0f), 0.00f);
        mSimulationHandler.setCollisionForce(collisionForce);
    }

    if (ImGui::Button("Add Atom Type", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        mSimulationHandler.newAtomType();
    }
    std::vector<unsigned int> atomTypes = mSimulationHandler.getAtomTypeIds();
    for (unsigned int atomTypeId : atomTypes) {
        ImGui::Separator();
        std::string atomIdStr = std::to_string(atomTypeId);
        Color c = mSimulationHandler.getAtomTypeColor(atomTypeId);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(c.r, c.g, c.b, 1.0));
        std::string friendlyName = mSimulationHandler.getAtomTypeFriendlyName(atomTypeId);
        label = "##FriendlyNameText-" + atomIdStr;
        if (ImGui::InputText(label.c_str(), &friendlyName)) {
            mSimulationHandler.setAtomTypeFriendlyName(atomTypeId, friendlyName);
        }
        ImGui::PopStyleColor(1);

        ImGui::SameLine();
        ImGui::Text(atomIdStr.c_str());

        float r = c.r;
        float g = c.g;
        float b = c.b;
        ImGui::PushItemWidth((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ColumnsMinSpacing * 2) / 3);
        label = "##ColorRedSlider-" + atomIdStr;
        if (ImGui::SliderFloat(label.c_str(), &r, 0.0f, 1.0f)) {
            mSimulationHandler.setAtomTypeColorR(atomTypeId, r);
        }
        ImGui::SameLine();
        label = "##ColorGreenSlider-" + atomIdStr;
        if (ImGui::SliderFloat(label.c_str(), &g, 0.0f, 1.0f)) {
            mSimulationHandler.setAtomTypeColorG(atomTypeId, g);
        }
        ImGui::SameLine();
        label = "##ColorBlueSlider-" + atomIdStr;
        if (ImGui::SliderFloat(label.c_str(), &b, 0.0f, 1.0f)) {
            mSimulationHandler.setAtomTypeColorB(atomTypeId, b);
        }
        ImGui::PopItemWidth();

        int quantity = mSimulationHandler.getAtomTypeQuantity(atomTypeId);
        label = "Quantity##QuantityInt-" + atomIdStr;
        if (ImGui::InputInt(label.c_str(), &quantity)) {
            mSimulationHandler.setAtomTypeQuantity(atomTypeId, quantity);
        }

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 3 / 4);
        for (unsigned int atomTypeId2 : atomTypes) {
            std::string atom2IdStr = std::to_string(atomTypeId2);
            Color atom2Color = mSimulationHandler.getAtomTypeColor(atomTypeId2);
            std::string atom2FriendlyName = mSimulationHandler.getAtomTypeFriendlyName(atomTypeId2);
            ImGui::TextColored(ImVec4(r, g, b, 1.0f), friendlyName.c_str());
            ImGui::SameLine();
            ImGui::Text("->");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(atom2Color.r, atom2Color.g, atom2Color.b, 1.0f), atom2FriendlyName.c_str());
            label = "0##ZeroButton-" + atomIdStr + "-" + atom2IdStr;
            if (ImGui::Button(label.c_str())) {
                mSimulationHandler.setInteraction(atomTypeId, atomTypeId2, 0);
            }
            ImGui::SameLine();
            float interaction = mSimulationHandler.getInteraction(atomTypeId, atomTypeId2);
            label = "##InteractionSlider-" + atomIdStr + "-" + atom2IdStr;
            if (ImGui::SliderFloat(label.c_str(), &interaction, -1.0f, 1.0f)) {
                mSimulationHandler.setInteraction(atomTypeId, atomTypeId2, interaction);
            }
        }
        label = "Delete Atom Type [" + friendlyName + "]##DeleteButton-" + atomIdStr;
        if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            mSimulationHandler.removeAtomType(atomTypeId);
        }
        ImGui::PopItemWidth();
    }
    ImGui::Separator();

    std::string fileSaveLocation = CONFIG_FILE_LOCATION + std::string("/") + mFileSaveLocation + std::string(".") + CONFIG_FILE_EXTENSION;
    if (mIsOverwritingFile) {
        ImGui::Text(("Config '" + mFileSaveLocation + "' exists.\nOverwrite?").c_str());
        if (ImGui::Button("Yes")) {
            saveToFile(fileSaveLocation, mSimulationHandler);
            mFileLoadLocations[mFileLoadCount++] = mFileSaveLocation;
            mIsOverwritingFile = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            mIsOverwritingFile = false;
        }
    } else {
        if (ImGui::Button("Save")) {
            if (std::filesystem::exists(fileSaveLocation)) {
                mIsOverwritingFile = true;
            } else {
                saveToFile(fileSaveLocation, mSimulationHandler);
                mFileLoadLocations[mFileLoadCount++] = mFileSaveLocation;
            }
        }
        ImGui::SameLine();
        ImGui::InputText("##Save Location", &mFileSaveLocation);
    }

    if (ImGui::Button("Load")) {
        loadFromFile(CONFIG_FILE_LOCATION + std::string("/") + mFileLoadLocations[mFileLoadIndex] + std::string(".") + CONFIG_FILE_EXTENSION, mSimulationHandler);
        mSimulationHandler.initSimulation();
    }
    ImGui::SameLine();
    struct Funcs { static bool ItemGetter(void* data, int n, const char** out_str) { *out_str = ((std::string*)data)[n].c_str(); return true; } };
    ImGui::Combo(
        "##Loadable Files", &mFileLoadIndex,
        &Funcs::ItemGetter,
        mFileLoadLocations,
        mFileLoadCount
    );
}
