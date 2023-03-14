/**
 * @file   WindowHandler.h
 * @brief  Handler for creating and managing the application UI.
 * 
 * @author Stuart Lewis
 * @date   January 2023
 */
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
 * Handler class for creation and handling of the application window and user input.
 */
class WindowHandler {
public:
    WindowHandler();
    ~WindowHandler();

    /**
     * Initialise the window and all relevant media/renderers.
     * Must be called before see WindowHandler::mainloop.
     * @returns true if initialization succeeds, otherwise false
     */
    bool init();
    /**
     * Starts the application main-loop.
     */
    void mainloop();

    /**
     * Set the default bounds of the window.
     * @param screenWidth Width (in pixels) of the window.
     * @param screenHeight Height (in pixels) of the window.
     */
    void setSize(int screenWidth, int screenHeight);

    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;
private:
    /**
     * Process user input.
     * @param e Input data to process. Should be aquired using SDL_PollEvent().
     */
    void handleEvent(SDL_Event& e);

    /**
     * Draw panel containing debug information to the window.
     * @param fps Current fps to be displayed.
     */
    void drawDebugPanel(float fps);
    /**
     * Draw panel containing general simulation configuration widgets.
     */
    void drawIOPanel();
    /**
    * Draw panel containing configuration widgets for AtomType interactions.
    */
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
