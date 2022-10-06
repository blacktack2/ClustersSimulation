#ifndef LIFESIMULATIONC_WINDOWHANDLER_H
#define LIFESIMULATIONC_WINDOWHANDLER_H

#include "../control/SimulationHandler.h"
#include "SimulationRenderer.h"
#include "../control/SaveAndLoad.h"

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
    void drawIOPanel(float x, float y, float width, float height);

    int mWindowWidth;
    int mWindowHeight;

    bool mRunning;
    bool mSimulationRunning;

    SDL_Window* mWindow;
    SDL_GLContext mGlContext{};
    ImGuiIO mIo;

    SimulationHandler mSimulationHandler;
    SimulationRenderer mSimulationRenderer;

    std::string mFileSaveLocation;
    std::string mFileLoadLocations[MAX_FILE_COUNT];
    int mFileLoadIndex;
    int mFileLoadCount;
    bool mIsOverwritingFile;
};


#endif //LIFESIMULATIONC_WINDOWHANDLER_H
