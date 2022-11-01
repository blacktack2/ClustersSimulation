#include "WindowHandler.h"

#include "../../imgui/imgui_impl_sdl.h"
#include "../../imgui/imgui_impl_opengl3.h"
#include "../../imgui/imgui_stdlib.h"

#include <glad/glad.h>

#include <cstdio>
#include <chrono>
#include <cmath>
#include <filesystem>

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
    glslVersion = "#version 150";
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_FLAGS,
        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
        );
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#elif __linux__
    glslVersion = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#elif _WIN32
    glslVersion = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

    auto windowFlags = (SDL_WindowFlags)(
            SDL_WINDOW_OPENGL | // NOLINT(hicpp-signed-bitwise)
            SDL_WINDOW_RESIZABLE |
            SDL_WINDOW_ALLOW_HIGHDPI
            );

    mWindow = SDL_CreateWindow(
            "Life Simulation",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, // NOLINT(hicpp-signed-bitwise)
            mWindowWidth, mWindowHeight,
            windowFlags);

    if (mWindow == nullptr) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetWindowMinimumSize(mWindow, 800, 600);
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

#ifdef ITERATE_ON_COMPUTE_SHADER
    mSimulationHandler.initComputeShaders();
#endif
    if (!mSimulationRenderer.init()) {
        fprintf(stderr, "Failed to initialize simulation renderer!\n");
        return false;
    }

    if (!getLoadableFiles(mFileLoadLocations, mFileLoadCount)) {
        fprintf(stderr, "Failed to read config files!\n");
        return false;
    }
    loadFromFile("resources/current.csdat", mSimulationHandler);

    mSimulationHandler.initSimulation();

    return true;
}
void WindowHandler::mainloop() {
    mRunning = true;

    SDL_Event e;

    float mspf = 1;
    float delta = 0;
    auto lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int frameCounter = 0;

    while (mRunning) {
        auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        delta += (float) (currentTime - lastTime);
        lastTime = currentTime;
        if (delta > 1000.0f) {
            mspf = delta / (float) frameCounter;
            delta = 0.0f;
            frameCounter = 0;
        }
        frameCounter++;

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // NOLINT(hicpp-signed-bitwise)

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

            ImGui::SetNextWindowPos(
                ImVec2(0, 0),
                ImGuiCond_Always
            );
            ImGui::SetNextWindowSize(
                ImVec2(sdlWidth, sdlHeight),
                ImGuiCond_Always
            );
            ImGui::Begin("##root", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

            ImVec2 bounds = ImGui::GetContentRegionAvail();
            ImVec2 messagePanelBounds = ImVec2(bounds.x, 30.0f);
            ImVec2 debugPanelBounds   = ImVec2(bounds.x * 2.0f / 7.0f, (bounds.y - messagePanelBounds.y) * 1.0f / 5.0f);
            ImVec2 ioPanelBounds      = ImVec2(debugPanelBounds.x, bounds.y - debugPanelBounds.y - messagePanelBounds.y);
            ImVec2 simPanelBounds     = ImVec2(bounds.x - debugPanelBounds.x, bounds.y - messagePanelBounds.y);

            ImGui::BeginGroup();

            ImGui::BeginChild("Debug", debugPanelBounds, true);
            drawDebugPanel(mspf);
            ImGui::EndChild();

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4); // Not a good solution (magic number)

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
            ImGui::BeginChild("Parameters", ioPanelBounds, true);
            ImGui::PopStyleVar(1);
            drawIOPanel();
            ImGui::EndChild();

            ImGui::EndGroup();
            ImGui::SameLine(0, 0);

            ImGui::BeginChild("Simulation", simPanelBounds, true);
            ImVec2 simBounds = ImGui::GetContentRegionAvail();
            ImVec2 simPos = ImGui::GetContentRegionMax();
            glViewport(0, 0, simBounds.x, simBounds.y);
            // Not a good solution for setting x and y parameters (magic number)
            mSimulationRenderer.drawSimulation(
                debugPanelBounds.x + ImGui::GetCursorPosX() + 8, ImGui::GetCursorPosY() + 8, simBounds.x, simBounds.y
            );
            glViewport(0, 0, mWindowWidth, mWindowHeight);
            ImGui::EndChild();

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4); // Not a good solution (magic number)

            ImGui::BeginChild("Message", messagePanelBounds, true);
            if (mShowMessage)
                ImGui::TextColored(mMessageColor, mMessage.c_str());
            ImGui::EndChild();

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

void WindowHandler::setSize(const int& width, const int& height) {
    mWindowWidth = width;
    mWindowHeight = height;
}

const int& WindowHandler::getWidth() const {
    return mWindowWidth;
}

const int& WindowHandler::getHeight() const {
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

void WindowHandler::drawDebugPanel(const float& mspf) {
    const ImVec4 debugTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    ImGui::TextColored(
        debugTextColor,
        "[FPS: %.2f | %.2fms]", mspf == 0 ? INFINITY : 1000.0 / mspf, mspf
    );
    ImGui::TextColored(
        debugTextColor,
        "Atom Count: %u", mSimulationHandler.getAtomCount()
    );
    ImGui::TextColored(
        debugTextColor,
        "Current Atom Count: %u", mSimulationHandler.getActualAtomCount()
    );
    ImGui::TextColored(
        debugTextColor,
        "Atom Types: %u", mSimulationHandler.getAtomTypeCount()
    );
}

void WindowHandler::drawIOPanel() {
    std::string label;
    float width = ImGui::GetContentRegionAvail().x;

    mSimulationRunning = mSimulationRunning ?
        !ImGui::Button("Pause##PlayPause", ImVec2(width, 0)) :
        ImGui::Button("Play##PlayPause", ImVec2(width, 0));
    if (ImGui::Button("Single Iteration", ImVec2(width, 0))) {
        mSimulationRunning = false;
        mSimulationHandler.iterateSimulation();
    }

    if (ImGui::Button("Re-generate Atoms", ImVec2(width, 0)))
        mSimulationHandler.initSimulation();
    if (ImGui::Button("Clear Atoms", ImVec2(width, 0)))
        mSimulationHandler.clearAtoms();
    if (ImGui::Button("Clear Atom Types", ImVec2(width, 0)))
        mSimulationHandler.clearAtomTypes();
    if (ImGui::Button("Randomize Positions", ImVec2(width, 0)))
        mSimulationHandler.shuffleAtomPositions();
    if (ImGui::Button("Zero Interactions", ImVec2(width, 0)))
        mSimulationHandler.zeroAtomInteractions();
    if (ImGui::Button("Shuffle Interactions", ImVec2(width, 0)))
        mSimulationHandler.shuffleAtomInteractions();

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
        interactionRange = std::max(std::min(interactionRange, std::sqrt(simScale * simScale + simScale * simScale) / 2.0f), 1.0f);
        mSimulationHandler.setInteractionRange(interactionRange);
    }
    ImGui::Text("Collision Force");
    float collisionForce = mSimulationHandler.getCollisionForce();
    if (ImGui::InputFloat("##Collision Force", &collisionForce, 0.01f, 0.1f, "%.2f")) {
        collisionForce = std::max(std::min(collisionForce, 10.0f), 0.00f);
        mSimulationHandler.setCollisionForce(collisionForce);
    }

    unsigned int atomTypeCount = mSimulationHandler.getAtomTypeCount();
    ImGui::BeginDisabled(atomTypeCount >= MAX_ATOM_TYPES);
    if (ImGui::Button("Add Atom Type", ImVec2(width, 0)))
        mSimulationHandler.newAtomType();
    ImGui::EndDisabled();

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
        ImGui::Text("%s", atomIdStr.c_str());

        float r = c.r;
        float g = c.g;
        float b = c.b;
        ImGui::PushItemWidth((width - ImGui::GetStyle().ColumnsMinSpacing * 2) / 3);
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

        int quantity = (int) mSimulationHandler.getAtomTypeQuantity(atomTypeId);
        label = "Quantity##QuantityInt-" + atomIdStr;
        if (ImGui::InputInt(label.c_str(), &quantity)) {
            int count = mSimulationHandler.getAtomCount();
            mSimulationHandler.setAtomTypeQuantity(atomTypeId, std::min((unsigned int)quantity, MAX_ATOMS));
            int newCount = mSimulationHandler.getAtomCount();
            if (newCount > MAX_ATOMS)
                messageWarn("Too many atoms defined (max: " + std::to_string(MAX_ATOMS) + " | you have " + std::to_string(newCount) + ")");
            else if (count > MAX_ATOMS)
                messageClear();
        }

        ImGui::PushItemWidth(width * 3 / 4);
        for (unsigned int atomTypeId2 : atomTypes) {
            std::string atom2IdStr = std::to_string(atomTypeId2);
            Color atom2Color = mSimulationHandler.getAtomTypeColor(atomTypeId2);
            std::string atom2FriendlyName = mSimulationHandler.getAtomTypeFriendlyName(atomTypeId2);
            ImGui::TextColored(ImVec4(r, g, b, 1.0f), "%s", friendlyName.c_str());
            ImGui::SameLine();
            ImGui::Text("->");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(atom2Color.r, atom2Color.g, atom2Color.b, 1.0f), "%s", atom2FriendlyName.c_str());
            label = std::string().append("0##ZeroButton-").append(atomIdStr).append("-").append(atom2IdStr);
            if (ImGui::Button(label.c_str())) {
                mSimulationHandler.setInteraction(atomTypeId, atomTypeId2, 0);
            }
            ImGui::SameLine();
            float interaction = mSimulationHandler.getInteraction(atomTypeId, atomTypeId2);
            label = std::string().append("##InteractionSlider-").append(atomIdStr).append("-").append(atom2IdStr);
            if (ImGui::SliderFloat(label.c_str(), &interaction, -1.0f, 1.0f)) {
                mSimulationHandler.setInteraction(atomTypeId, atomTypeId2, interaction);
            }
        }
        label = std::string().append("Delete Atom Type [").append(friendlyName).append("]##DeleteButton-").append(atomIdStr);
        if (ImGui::Button(label.c_str(), ImVec2(width, 0))) {
            mSimulationHandler.removeAtomType(atomTypeId);
        }
        ImGui::PopItemWidth();
    }
    ImGui::Separator();

    std::string fileSaveLocation = CONFIG_FILE_LOCATION + std::string("/") + mFileSaveLocation + std::string(".") + CONFIG_FILE_EXTENSION;
    if (mIsOverwritingFile) {
        ImGui::Text("%s", ("Config '" + mFileSaveLocation + "' exists.\nOverwrite?").c_str());
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

void WindowHandler::messageInfo(std::string message) {
    mMessage = message;
    mMessageColor = MESSAGE_COL;
    mShowMessage = true;
}

void WindowHandler::messageWarn(std::string message) {
    mMessage = "[Warning] " + message;
    mMessageColor = MESSAGE_WARN_COL;
    mShowMessage = true;
}

void WindowHandler::messageError(std::string message) {
    mMessage = "[Error] " + message;
    mMessageColor = MESSAGE_ERROR_COL;
    mShowMessage = true;
}

void WindowHandler::messageClear() {
    mMessage = "";
    mMessageColor = MESSAGE_COL;
    mShowMessage = false;
}
