#include "WindowHandler.h"

#include "../../imgui/imgui_impl_sdl.h"
#include "../../imgui/imgui_impl_opengl3.h"

#include <cstdio>
#include <chrono>
#include <glad/glad.h>

#define PANEL_PADDING 10
#define PANEL_MARGINS 10

#define DEBUG_PANEL_WIDTH_MIN  200
#define DEBUG_PANEL_HEIGHT_MIN 100

#define IO_PANEL_WIDTH_MIN  200
#define IO_PANEL_HEIGHT_MIN 400

#define SIM_PANEL_WIDTH_MIN  500
#define SIM_PANEL_HEIGHT_MIN 500

#if (DEBUG_PANEL_WIDTH_MIN > IO_PANEL_WIDTH_MIN)
#define WINDOW_WIDTH_MIN DEBUG_PANEL_WIDTH_MIN + DEBUG_PANEL_WIDTH_MIN + PANEL_MARGINS * 2
#else
#define WINDOW_WIDTH_MIN DEBUG_PANEL_WIDTH_MIN + SIM_PANEL_WIDTH_MIN + PANEL_MARGINS * 2
#endif

#define WINDOW_HEIGHT_MIN SIM_PANEL_HEIGHT_MIN + PANEL_MARGINS * 2

WindowHandler::WindowHandler() :
mWindowWidth(0), mWindowHeight(0), mRunning(false),
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
        return -1;
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
    mLSRenderer = new LifeSimulationRenderer(mLSHandler);

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

    ImVec4 fpsTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
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
                    std::max(sdlWidth * 2 / 7 - PANEL_MARGINS * 2, DEBUG_PANEL_WIDTH_MIN),
                    std::max(sdlHeight / 5 - PANEL_MARGINS * 2, DEBUG_PANEL_HEIGHT_MIN)
                    );
            ImVec4 ioPanelBounds = ImVec4(
                    static_cast<float>(PANEL_MARGINS),
                    static_cast<float>(debugPanelBounds.y + debugPanelBounds.w + PANEL_MARGINS * 2),
                    std::max(sdlWidth * 2 / 7 - PANEL_MARGINS * 2, IO_PANEL_WIDTH_MIN),
                    std::max(sdlHeight * 4 / 5 - PANEL_MARGINS * 2, IO_PANEL_HEIGHT_MIN)
            );
            ImVec4 simPanelBounds = ImVec4(
                    static_cast<float>(std::max(debugPanelBounds.x + debugPanelBounds.z, ioPanelBounds.x + ioPanelBounds.z) + PANEL_MARGINS * 2),
                    static_cast<float>(PANEL_MARGINS),
                    std::max(sdlWidth * 5 / 7 - PANEL_MARGINS * 2, SIM_PANEL_WIDTH_MIN),
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

            ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoResize);

            ImGui::Dummy(ImVec2(0.0f, 1.0f));
            ImGui::TextColored(
                    fpsTextColor,
                    "[FPS: %f]", static_cast<float>(fpsTotal) / 10.0f
                    );

            ImGui::End();

            ImGui::SetNextWindowPos(
                    ImVec2(ioPanelBounds.x, ioPanelBounds.y),
                    ImGuiCond_Always
                    );
            ImGui::SetNextWindowSize(
                    ImVec2(ioPanelBounds.z, ioPanelBounds.w),
                    ImGuiCond_Always
            );

            ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_NoResize);

            ImGui::End();

            ImGui::SetNextWindowPos(
                    ImVec2(simPanelBounds.x, simPanelBounds.y),
                    ImGuiCond_Always
                    );
            ImGui::SetNextWindowSize(
                    ImVec2(simPanelBounds.z, simPanelBounds.w),
                    ImGuiCond_Always
            );

            ImGui::Begin("Simulation", nullptr, ImGuiWindowFlags_NoResize);

            mLSRenderer->drawSimulation(simPanelBounds.x, simPanelBounds.y, simPanelBounds.z, simPanelBounds.w);

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(mWindow);

        fpsCounter++;

        mLSHandler->iterateSimulation();
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
                    break;
            }
            break;
    }
}