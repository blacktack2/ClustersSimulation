#include "WindowHandler.h"

#include <cstdio>
#include <chrono>

WindowHandler::WindowHandler() {
    mScreenWidth = 0;
    mScreenHeight = 0;

    mRunning = false;

    mMouseFocus = false;
    mKeyboardFocus = false;
    mFullscreen = false;
    mMinimized = false;
}

WindowHandler::~WindowHandler() {
    freeTexture();

    if (mFont != nullptr) {
        TTF_CloseFont(mFont);
        mFont = nullptr;
    }

    if (mWindow != nullptr) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }

    if (mRenderer != nullptr) {
        SDL_DestroyRenderer(mRenderer);
        mRenderer = nullptr;
    }

    delete mLSHandler;
    mLSHandler = nullptr;
    delete mLSRenderer;
    mLSRenderer = nullptr;

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool WindowHandler::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow(
            "Life Simulation",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            mScreenWidth, mScreenHeight,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (mWindow == nullptr) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
    if (mRenderer == nullptr) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    mLSHandler = new LifeSimulationHandler();
    mLSRenderer = new LifeSimulationRenderer(mLSHandler, mRenderer);

    mLSHandler->setBounds(static_cast<float>(mScreenWidth), static_cast<float>(mScreenHeight));
    mLSHandler->initSimulation();

    mMouseFocus = true;
    mKeyboardFocus = true;

    SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

    if (TTF_Init() == -1) {
        fprintf(stderr, "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }

    if (!loadMedia()) {
        fprintf(stderr, "Failed to load media!\n");
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        fprintf(stderr, "SDL_image could not initialize! SDL_image Error:%s\n", IMG_GetError());
        return false;
    }

    return true;
}

bool WindowHandler::loadMedia() {
    mFont = TTF_OpenFont("/home/blacktack2/Documents/Rep/LifeSimulationC/OpenSans-Regular.ttf", 20);
    if (mFont == nullptr) {
        fprintf(stderr, "Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }

    SDL_Color textColor = {0xFF, 0xFF, 0x00, 0xFF};
    if (!loadFromRenderedText("The quick brown fox jumps over the lazy dog", textColor)) {
        fprintf(stderr, "Failed to render text texture!\n");
        return false;
    }
    loadFromRenderedText(" ", textColor);

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

    SDL_Color fpsTextColor = {0xFF, 0xFF, 0x00, 0xFF};
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

            loadFromRenderedText(std::to_string(static_cast<float>(fpsTotal) / 10.0f), fpsTextColor);
        }

        while (SDL_PollEvent(&e) != 0) {
            handleEvent(e);
        }

        SetRenderDrawColor(mRenderer, C_BLACK);
        SDL_RenderClear(mRenderer);

        mLSHandler->iterateSimulation();
        mLSRenderer->drawSimulation();

        SDL_Rect renderQuad = {0, 0, 50, 24};
        SDL_RenderCopyEx(mRenderer, mTexture, &renderQuad, &renderQuad, 0, nullptr, SDL_FLIP_NONE);

        SDL_RenderPresent(mRenderer);
        fpsCounter++;
    }
}

void WindowHandler::setSize(int width, int height) {
    mScreenWidth = width;
    mScreenHeight = height;
}

int WindowHandler::getWidth() {
    return mScreenWidth;
}

int WindowHandler::getHeight() {
    return mScreenHeight;
}

bool WindowHandler::hasMouseFocus() {
    return mMouseFocus;
}

bool WindowHandler::hasKeyboardFocus() {
    return mKeyboardFocus;
}

bool WindowHandler::isFullscreen() {
    return mFullscreen;
}

bool WindowHandler::isMinimized() {
    return mMinimized;
}

void WindowHandler::handleEvent(SDL_Event& e) {
    if (e.type == SDL_QUIT) {
        mRunning = false;
    } else if (e.type == SDL_WINDOWEVENT) {
        switch (e.window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                mScreenWidth  = e.window.data1;
                mScreenHeight = e.window.data2;
                SDL_RenderPresent(mRenderer);
                break;

            case SDL_WINDOWEVENT_EXPOSED:
                SDL_RenderPresent(mRenderer);
                break;

            case SDL_WINDOWEVENT_ENTER:
                mMouseFocus = true;
                break;

            case SDL_WINDOWEVENT_LEAVE:
                mMouseFocus = false;
                break;

            case SDL_WINDOWEVENT_FOCUS_GAINED:
                mKeyboardFocus = true;
                break;

            case SDL_WINDOWEVENT_FOCUS_LOST:
                mKeyboardFocus = false;
                break;

            case SDL_WINDOWEVENT_MINIMIZED:
                mMinimized = true;
                break;

            case SDL_WINDOWEVENT_MAXIMIZED:
                mMinimized = false;
                break;

            case SDL_WINDOWEVENT_RESTORED:
                mMinimized = false;
                break;
        }
    } else if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_SPACE:
                break;
        }
    }
}

void WindowHandler::freeTexture() {
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
    }
}

bool WindowHandler::loadFromRenderedText(std::string textureText, SDL_Color textColor) {
    freeTexture();

    SDL_Surface* textSurface = TTF_RenderText_Solid(mFont, textureText.c_str(), textColor);
    if (textSurface == nullptr) {
        fprintf(stderr, "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }

    mTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
    if (mTexture == nullptr) {
        fprintf(stderr, "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface);
        return false;
    }
    SDL_FreeSurface(textSurface);

    return true;
}
