#include "WindowHandler.h"

#include "Logger.h"

#include "../../glm/vec3.hpp"

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
    Logger::getLogger().logMessage("Constructing Window");
}

WindowHandler::~WindowHandler() {
    Logger::getLogger().logMessage("Destroying Window");
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
    Logger::getLogger().logMessage("Initializing Window");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        Logger::getLogger().logError(std::string("Failed to initialize SDL - SDL Error: ").append(SDL_GetError()));
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
        Logger::getLogger().logError(std::string("Failed to create Window - SDL Error").append(SDL_GetError()));
        return false;
    }

    SDL_SetWindowMinimumSize(mWindow, 800, 600);
    mGlContext = SDL_GL_CreateContext(mWindow);

    if (mAllowVsync = !SDL_GL_SetSwapInterval(1))
        mAllowAdaptive = !SDL_GL_SetSwapInterval(-1);

    SDL_GL_SetSwapInterval((mEnableVsync = mAllowVsync) ? ((mVsyncAdaptive = mAllowAdaptive) ? -1 : 1) : 0);

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        Logger::getLogger().logError(std::string("Failed to initialize OpenGL"));
        return false;
    }

    glViewport(0, 0, mWindowWidth, mWindowHeight);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    mIo = ImGui::GetIO(); (void)mIo;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(mWindow, mGlContext);
    ImGui_ImplOpenGL3_Init(glslVersion.c_str());

    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

#ifdef ITERATE_ON_COMPUTE_SHADER
    mSimulationHandler.initComputeShaders();
#endif
    if (!mSimulationRenderer.init()) {
        Logger::getLogger().logError(std::string("Failed to initialize Renderer"));
        return false;
    }

    if (!getLoadableFiles(mFileLoadLocations, mFileLoadCount)) {
        Logger::getLogger().logError(std::string("Failed to read config files"));
        return false;
    }
    loadFromFile("resources/current.csdat", mSimulationHandler);
    mSimulationRenderer.updateParameters();

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

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

    while (mRunning) {
        auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        delta += (float) (currentTime - lastTime);
        if (mSimulationRunning)
            mTimeElapsed += (float)(currentTime - lastTime);
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
            ImVec2 ioPanelBounds      = ImVec2(debugPanelBounds.x, (bounds.y - messagePanelBounds.y) * 2.0f / 5.0f);
            ImVec2 interPanelBounds   = ImVec2(debugPanelBounds.x, bounds.y - debugPanelBounds.y - ioPanelBounds.y - messagePanelBounds.y);
            ImVec2 simPanelBounds     = ImVec2(bounds.x - debugPanelBounds.x, bounds.y - messagePanelBounds.y);

            ImGui::BeginGroup();

            ImGui::BeginChild("Debug", debugPanelBounds, true);
            drawDebugPanel(mspf);
            ImGui::EndChild();

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3); // Not a good solution (magic number)

            ImGui::BeginChild("Parameters", ioPanelBounds, true);
            drawIOPanel();
            ImGui::EndChild();

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3); // Not a good solution (magic number)

            ImGui::BeginChild("AtomTypes", ioPanelBounds, true);
            drawInteractionsPanel();
            ImGui::EndChild();

            ImGui::EndGroup();
            ImGui::SameLine(0, 0);

            ImGui::BeginChild("Simulation", simPanelBounds, true);
            ImVec2 panelBounds = ImGui::GetContentRegionAvail();
            ImVec2 panelPos = ImGui::GetContentRegionMax();

            ImVec2 simBounds = ImVec2(mSimulationHandler.getWidth(), mSimulationHandler.getHeight());

            float scale = std::min(panelBounds.x / simBounds.x, panelBounds.y / simBounds.y);
            simBounds.x *= scale;
            simBounds.y *= scale;

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (panelBounds.x - simBounds.x) / 2.0f);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (panelBounds.y - simBounds.y) / 2.0f);

            glViewport(0, 0, simBounds.x, simBounds.y);
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
            mIterationCount++;
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
        "[FPS: %.2f | %.2fms]", mspf == 0 ? INFINITY : 1000.0 / mspf, mspf
    );

    ImGui::Separator();

    ImGui::TextColored(
        debugTextColor,
        "Runtime: %fs", mTimeElapsed / 1000.0f
    );
    ImGui::TextColored(
        debugTextColor,
        "Iterations: %d", mIterationCount
    );

    ImGui::Separator();

    ImGui::TextColored(
        debugTextColor,
        "Total Atom Quantity: %u", mSimulationHandler.getAtomCount()
    );

    ImGui::TextColored(
        debugTextColor,
        "No. Generated Atoms: %u", mSimulationHandler.getActualAtomCount()
    );
    ImGui::TextColored(
        debugTextColor,
        "No. Atom Types: %u", mSimulationHandler.getAtomTypeCount()
    );

    if (mAllowVsync) {
        if (ImGui::Checkbox("Enable VSync", &mEnableVsync))
            if (SDL_GL_SetSwapInterval(mEnableVsync ? (mVsyncAdaptive ? -1 : 1) : 0))
                messageError("Failed to set VSync");

        if (mAllowAdaptive) {
            ImGui::BeginDisabled(!mEnableVsync);
            if (ImGui::Checkbox("Adaptive VSync", &mVsyncAdaptive))
                if (SDL_GL_SetSwapInterval(mEnableVsync ? (mVsyncAdaptive ? -1 : 1) : 0))
                    messageError("Failed to set adaptive VSync");
            ImGui::EndDisabled();
        }
    }
}

void WindowHandler::drawIOPanel() {
    std::string label;
    float width = ImGui::GetContentRegionAvail().x;
    static const ImVec2 REMAINING_WIDTH = ImVec2(-FLT_MIN, 0);
    ImVec2 HALF_WIDTH = ImVec2(width / 2.0f, 0);
    ImGui::PushItemWidth(-FLT_MIN);

    label = (mSimulationRunning ? "Pause" : "Play") + std::string("##PlayPause");
    if (ImGui::Button(label.c_str(), HALF_WIDTH)) {
        mSimulationRunning = !mSimulationRunning;
        Logger::getLogger().logMessage(mSimulationRunning ? "Starting Simulation" : "Stopping Simulation");
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip(((mSimulationRunning ? "Pause" : "Play") + std::string(" the simulation [SPACE].")).c_str());
    ImGui::SameLine(0, 0);
    if (ImGui::Button("Iterate", REMAINING_WIDTH)) {
        mSimulationRunning = false;
        mSimulationHandler.iterateSimulation();
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Perform a single iteration of the simulation.");

    ImGui::Text("Start Condition");
    const char* START_CONDITION_NAMES[] = {
        "Random",
        "Equidistant",
        "Random Equidistant",
        "Rings",
    };
    ImGui::Combo("##Start Condition", (int*)&mSimulationHandler.startCondition, START_CONDITION_NAMES, (int)StartConditionMax);

    ImGui::Separator();
    ImGui::Text("Atoms");
    if (ImGui::Button("Generate", HALF_WIDTH)) {
        mSimulationHandler.initSimulation();
        mTimeElapsed = 0.0f;
        mIterationCount = 0;
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Generate new atoms according to the set parameters.");
    ImGui::SameLine(0, 0);
    if (ImGui::Button("Clear##Clear Atoms", REMAINING_WIDTH))
        mSimulationHandler.clearAtoms();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Clear all atoms from the simulation.");

    ImGui::Separator();
    ImGui::Text("Atom Types");
    if (ImGui::Button("Clear##Clear Atom Types", HALF_WIDTH))
        mSimulationHandler.clearAtomTypes();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Clear all atom types from the current configuration.");
    ImGui::SameLine(0, 0);
    size_t atomTypeCount = mSimulationHandler.getAtomTypeCount();
    ImGui::BeginDisabled(atomTypeCount >= MAX_ATOM_TYPES);
    if (ImGui::Button("Add New", REMAINING_WIDTH))
        mSimulationHandler.newAtomType();
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Add a new atom type to the configuration.");

    ImGui::Separator();
    ImGui::Text("Interactions");
    if (ImGui::Button("Zero", HALF_WIDTH))
        mSimulationHandler.zeroAtomInteractions();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Set all interaction values to 0.");
    ImGui::SameLine(0, 0);
    if (ImGui::Button("Shuffle", REMAINING_WIDTH))
        mSimulationHandler.shuffleAtomInteractions();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Set all interaction values randomly.");

    ImGui::Separator();
    ImGui::Checkbox("Lock Aspect", &mLockRatio);

    ImGui::BeginTable("Inputs", 2, ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_SizingStretchSame, ImVec2(width, 0));

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Width");
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("Height");

    ImGui::TableNextRow();
    float simWidth = mSimulationHandler.getWidth();
    float simHeight = mSimulationHandler.getHeight();
    float simWidthMin  = std::max(MIN_SIM_WIDTH , simHeight / 10.0f);
    float simHeightMin = std::max(MIN_SIM_HEIGHT, simWidth  / 10.0f);
    ImGui::TableSetColumnIndex(0);
    ImGui::SetNextItemWidth(-FLT_MIN);
    bool changeW = ImGui::DragScalar("##Simulation Width", ImGuiDataType_Float, &simWidth, 10.0f, &simWidthMin, &MAX_SIM_WIDTH, "%.0f");
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    bool changeH = ImGui::DragScalar("##Simulation Height", ImGuiDataType_Float, &simHeight, 10.0f, &simHeightMin, &MAX_SIM_HEIGHT, "%.0f") && !changeW;
    if (changeW || changeH) {
        simWidth  = std::max(simWidth , simWidthMin );
        simHeight = std::max(simHeight, simHeightMin);
        mSimulationHandler.setBounds(
            changeW ? simWidth  : (mLockRatio ? simWidth  * simHeight / mSimulationHandler.getHeight() : simWidth),
            changeH ? simHeight : (mLockRatio ? simHeight * simWidth  / mSimulationHandler.getWidth()  : simHeight)
        );
        mSimulationRenderer.updateParameters();
    }

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Range");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Maximum distance which two atoms can interact from.");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::SetNextItemWidth(-FLT_MIN);
    float interactionRangeMax = std::sqrt(simWidth * simWidth + simHeight * simHeight) / 2.0f;
    float interactionRange = mSimulationHandler.getInteractionRange();
    if (ImGui::DragScalar("##Max Interaction Range", ImGuiDataType_Float, &interactionRange, 1.0f, &MIN_INTERACTION_RANGE, &interactionRangeMax, "%.0f"))
        mSimulationHandler.setInteractionRange(std::min(interactionRange, interactionRangeMax));

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Atom Diameter");
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("Time Delta");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::SetNextItemWidth(-FLT_MIN);
    float atomDiameterMax = std::min(simWidth, simHeight) / 2.0f;
    float atomDiameter = mSimulationHandler.getAtomDiameter();
    if (ImGui::DragScalar("##Atom Diameter", ImGuiDataType_Float, &atomDiameter, 1.0f, &MIN_ATOM_DIAMETER, &atomDiameterMax, "%.0f")) {
        mSimulationHandler.setAtomDiameter(std::min(atomDiameter, atomDiameterMax));
        mSimulationRenderer.updateParameters();
    }

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    float dt = mSimulationHandler.getDt();
    if (ImGui::DragScalar("##Time Delta", ImGuiDataType_Float, &dt, 0.01f, &MIN_DT, &MAX_DT, "%.2f"))
        mSimulationHandler.setDt(dt);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Drag Frc.");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("0 = no drag, 1 = max drag.");
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("Collision Frc.");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::SetNextItemWidth(-FLT_MIN);
    float drag = 1.0f - mSimulationHandler.getDrag();
    if (ImGui::DragScalar("##Drag Force", ImGuiDataType_Float, &drag, 0.01f, &MIN_DRAG, &MAX_DRAG, "%.2f"))
        mSimulationHandler.setDrag(1.0f - drag);

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    float collisionForce = mSimulationHandler.getCollisionForce();
    if (ImGui::DragScalar("##Collision Force", ImGuiDataType_Float, &collisionForce, 0.01f, &MIN_COLLISION_FORCE, &MAX_COLLISION_FORCE, "%.2f"))
        mSimulationHandler.setCollisionForce(collisionForce);

    ImGui::EndTable();
    
    ImGui::Separator();

    std::string fileSaveLocation = CONFIG_FILE_LOCATION + std::string("/") + mFileSaveLocation + std::string(".") + CONFIG_FILE_EXTENSION;
    if (mIsOverwritingFile) {
        ImGui::Text("%s", ("Config '" + std::string(mFileSaveLocation) + "' exists.\nOverwrite?").c_str());
        if (ImGui::Button("Yes")) {
            saveToFile(fileSaveLocation, mSimulationHandler);
            mFileLoadLocations[mFileLoadCount++] = mFileSaveLocation;
            mIsOverwritingFile = false;
        }
        ImGui::SameLine(0, 0);
        if (ImGui::Button("No")) {
            mIsOverwritingFile = false;
        }
    } else {
        if (ImGui::Button("Save")) {
            if (mFileLoadCount == MAX_FILE_COUNT) {
                messageError("Maximum config file count reached [" + std::to_string(MAX_FILE_COUNT) + "]. Delete something to make room for more.");
            } else if (std::filesystem::exists(fileSaveLocation)) {
                mIsOverwritingFile = true;
            } else {
                if (saveToFile(fileSaveLocation, mSimulationHandler))
                    mFileLoadLocations[mFileLoadCount++] = mFileSaveLocation;
                else
                    messageError("Failed to save file '" + std::string(mFileSaveLocation) + "'");
            }
        }
        ImGui::SameLine(0, 0);
        ImGui::InputText("##Save Location", mFileSaveLocation, 20);
    }

    float wid = ImGui::CalcTextSize("Delete ").x;
    if (ImGui::Button("Load", ImVec2(wid, 0))) {
        if (mFileLoadCount == 0) {
            messageError("No config files found. Save a configuration to be able to load.");
        } else {
            if (loadFromFile(CONFIG_FILE_LOCATION + std::string("/") + mFileLoadLocations[mFileLoadIndex] + std::string(".") + CONFIG_FILE_EXTENSION, mSimulationHandler)) {
                mSimulationHandler.initSimulation();
                mTimeElapsed = 0.0f;
                mIterationCount = 0;
            } else {
                messageError("Failed to load file '" + std::string(mFileSaveLocation) + "'");
            }
            mSimulationRenderer.updateParameters();
        }
    }
    ImGui::SameLine(0, 0);
    struct Funcs { static bool ItemGetter(void* data, int n, const char** out_str) { *out_str = ((std::string*)data)[n].c_str(); return true; } };
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - wid);
    ImGui::Combo(
        "##Loadable Files", &mFileLoadIndex,
        &Funcs::ItemGetter,
        mFileLoadLocations,
        mFileLoadCount
    );
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip(mFileLoadLocations[mFileLoadIndex].c_str());
    ImGui::SameLine(0, 0);
    if (ImGui::Button("Delete", ImVec2(wid, 0))) {
        if (deleteFile((CONFIG_FILE_LOCATION + std::string("/") + mFileLoadLocations[mFileLoadIndex].c_str() + std::string(".") + CONFIG_FILE_EXTENSION))) {
            for (int i = mFileLoadIndex; i < mFileLoadCount - 1; i++)
                mFileLoadLocations[i] = mFileLoadLocations[i + 1];
            mFileLoadCount--;
            mFileLoadIndex = 0;
        } else {
            messageError("Failed to delete file '" + std::string(mFileSaveLocation) + "'");
        }
    }

    ImGui::PopItemWidth();
}

void WindowHandler::drawInteractionsPanel() {
    std::string label;
    float width = ImGui::GetContentRegionAvail().x;
    static const ImVec2 REMAINING_WIDTH = ImVec2(-FLT_MIN, 0);
    ImVec2 HALF_WIDTH = ImVec2(width / 2.0f, 0);
    ImGui::PushItemWidth(-FLT_MIN);

    std::vector<atom_type_id> atomTypes = mSimulationHandler.getAtomTypeIds();

    if (ImGui::Button("Lock All", HALF_WIDTH))
        for (atom_type_id atomTypeId : atomTypes)
            mBulkLock[atomTypeId] = true;
    ImGui::SameLine(0, 0);
    if (ImGui::Button("Unlock All", REMAINING_WIDTH))
        for (atom_type_id atomTypeId : atomTypes)
            mBulkLock[atomTypeId] = false;

    ImGui::Text("Quantity");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Set the quantity for all unlocked atom types.");
    label = "##BulkQuantity";
    if (ImGui::InputInt(label.c_str(), (int*) &mBulkQuantity, 0)) {
        int count = mSimulationHandler.getAtomCount();
        for (atom_type_id atomTypeId : atomTypes)
            if (!mBulkLock[atomTypeId])
                mSimulationHandler.setAtomTypeQuantity(atomTypeId, std::min(mBulkQuantity, (unsigned int) MAX_ATOMS));
        int newCount = mSimulationHandler.getAtomCount();
        if (newCount > MAX_ATOMS)
            messageWarn("Too many atoms defined (max: " + std::to_string(MAX_ATOMS) + " | you have " + std::to_string(newCount) + ")");
        else if (count > MAX_ATOMS)
            messageClear();
    }

    for (atom_type_id atomTypeId : atomTypes) {
        ImGui::Separator();
        std::string atomIdStr = std::to_string(atomTypeId);
        glm::vec3 c = mSimulationHandler.getAtomTypeColor(atomTypeId);

        ImVec4 imC = ImVec4(c.r, c.g, c.b, 1.0f);

        ImGui::Text(("ID: " + atomIdStr).c_str());

        ImGui::PushStyleColor(ImGuiCol_Border, mBulkLock[atomTypeId] ?
            ImVec4(0.8f, 0.8f, 0.8f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0);
        label = "Lock##QuantityLock-" + atomIdStr;
        if (ImGui::Button(label.c_str()))
            mBulkLock[atomTypeId] = !mBulkLock[atomTypeId];
        ImGui::PopStyleVar(1);
        ImGui::PopStyleColor(1);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Lock this atom type from bulk assignments.");

        ImGui::SameLine(0, 0);

        ImGui::PushStyleColor(ImGuiCol_Border, imC);
        std::string friendlyName = mSimulationHandler.getAtomTypeFriendlyName(atomTypeId);
        label = "##FriendlyNameText-" + atomIdStr;
        if (ImGui::InputText(label.c_str(), &friendlyName))
            mSimulationHandler.setAtomTypeFriendlyName(atomTypeId, friendlyName);
        ImGui::PopStyleColor(1);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Display name of this atom type.");

        ImGui::Text("Color");
        label = "##ColorSlider-" + atomIdStr;
        if (ImGui::ColorEdit3(label.c_str(), (float*) &imC))
            mSimulationHandler.setAtomTypeColor(atomTypeId, glm::vec3{imC.x, imC.y, imC.z});

        ImGui::Text("Quantity");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("How many of this atom type should be present.\nMust press {generate} to update.");

        unsigned int quantity = mSimulationHandler.getAtomTypeQuantity(atomTypeId);
        label = "##QuantityInt-" + atomIdStr;
        if (ImGui::InputInt(label.c_str(), (int*) &quantity, 0)) {
            int count = mSimulationHandler.getAtomCount();
            mSimulationHandler.setAtomTypeQuantity(atomTypeId, std::min(quantity, (unsigned int) MAX_ATOMS));
            int newCount = mSimulationHandler.getAtomCount();
            if (newCount > MAX_ATOMS)
                messageWarn("Too many atoms defined (max: " + std::to_string(MAX_ATOMS) + " | you have " + std::to_string(newCount) + ")");
            else if (count > MAX_ATOMS)
                messageClear();
        }

        for (atom_type_id atomTypeId2 : atomTypes) {
            std::string atom2IdStr = std::to_string(atomTypeId2);
            glm::vec3 atom2Color = mSimulationHandler.getAtomTypeColor(atomTypeId2);
            std::string atom2FriendlyName = mSimulationHandler.getAtomTypeFriendlyName(atomTypeId2);
            ImGui::TextColored(imC, "%s", friendlyName.c_str());
            ImGui::SameLine();
            ImGui::Text("->");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(atom2Color.r, atom2Color.g, atom2Color.b, 1.0f), "%s", atom2FriendlyName.c_str());
            label = std::string().append("0##ZeroButton-").append(atomIdStr).append("-").append(atom2IdStr);
            if (ImGui::Button(label.c_str()))
                mSimulationHandler.setInteraction(atomTypeId, atomTypeId2, 0);
            ImGui::SameLine(0, 0);
            float interaction = mSimulationHandler.getInteraction(atomTypeId, atomTypeId2);
            label = std::string().append("##InteractionSlider-").append(atomIdStr).append("-").append(atom2IdStr);
            if (ImGui::SliderFloat(label.c_str(), &interaction, MIN_INTERACTION, MAX_INTERACTION))
                mSimulationHandler.setInteraction(atomTypeId, atomTypeId2, interaction);
        }
        label = std::string().append("Delete Atom Type [").append(friendlyName).append("]##DeleteButton-").append(atomIdStr);
        ImGui::PushStyleColor(ImGuiCol_Border, imC);
        if (ImGui::Button(label.c_str(), REMAINING_WIDTH))
            mSimulationHandler.removeAtomType(atomTypeId);
        ImGui::PopStyleColor(1);
    }
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
