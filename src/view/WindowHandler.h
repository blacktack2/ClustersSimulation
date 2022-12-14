#pragma once
#include "../control/SaveAndLoad.h"
#include "../control/SimulationHandler.h"
#include "SimulationRenderer.h"

#include "../../imgui/imgui.h"

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

/**
 * Handler class for creation and handling of the application window, and user input.
 */
class WindowHandler {
public:
    WindowHandler();
    ~WindowHandler();
    /**
     * Create the window and all relevant media and renderers.
     * @returns <c>false</c> if initialization failed.
     *          <c>true</c> if initialization succeeded.
     */
    bool init();
    /**
     * Start the applications main loop.
     */
    void mainloop();

    /**
     * Set the default bounds of the application window.
     * @param screenWidth Width (in pixels) of the window.
     * @param screenHeight Height (in pixels) of the window.
     */
    void setSize(int screenWidth, int screenHeight);

    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;
private:
    void handleEvent(SDL_Event& e);

    void drawDebugPanel(float fps);
    void drawIOPanel();
    void drawInteractionsPanel();

    void messageInfo(std::string message);
    void messageWarn(std::string message);
    void messageError(std::string message);
    void messageClear();

    int mWindowWidth;
    int mWindowHeight;

    bool mRunning;
    bool mSimulationRunning;

    bool mLockRatio = true;

    float mTimeElapsed = 0.0f;
    unsigned int mIterationCount = 0;

    bool mEnableVsync = false;
    bool mVsyncAdaptive = false;
    bool mAllowVsync = false;
    bool mAllowAdaptive = false;

    std::vector<bool> mBulkLock = std::vector<bool>(MAX_ATOM_TYPES, false);
    unsigned int mBulkQuantity = 200u;

    bool mShowMessage = false;
    std::string mMessage;
    ImVec4 mMessageColor = MESSAGE_COL;

    SDL_Window* mWindow;
    SDL_GLContext mGlContext{};
    ImGuiIO mIo;

    SimulationHandler mSimulationHandler;
    SimulationRenderer mSimulationRenderer;

    char mFileSaveLocation[20];
    std::string mFileLoadLocations[MAX_FILE_COUNT];
    int mFileLoadIndex;
    int mFileLoadCount;
    bool mIsOverwritingFile;

    const ImVec4 MESSAGE_COL = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    const ImVec4 MESSAGE_WARN_COL = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    const ImVec4 MESSAGE_ERROR_COL = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
};
