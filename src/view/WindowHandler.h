#ifndef LIFESIMULATIONC_WINDOWHANDLER_H
#define LIFESIMULATIONC_WINDOWHANDLER_H

#include "../control/LifeSimulationHandler.h"
#include "LifeSimulationRenderer.h"
#include "../control/SDLUtils.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

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
    bool loadMedia();
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

    int getWidth();
    int getHeight();

    bool hasMouseFocus();
    bool hasKeyboardFocus();
    bool isFullscreen();
    bool isMinimized();

    void freeTexture();
    bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
private:
    void handleEvent(SDL_Event& e);

    int mScreenWidth = 0;
    int mScreenHeight = 0;

    bool mRunning = false;

    bool mMouseFocus = false;
    bool mKeyboardFocus = false;
    bool mFullscreen = false;
    bool mMinimized = false;

    TTF_Font* mFont = nullptr;
    SDL_Texture* mTexture = nullptr;

    SDL_Window* mWindow = nullptr;
    SDL_Renderer* mRenderer = nullptr;

    LifeSimulationHandler* mLSHandler{};
    LifeSimulationRenderer* mLSRenderer{};
};


#endif //LIFESIMULATIONC_WINDOWHANDLER_H
