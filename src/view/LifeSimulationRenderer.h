#ifndef LIFESIMULATIONC_LIFESIMULATIONRENDERER_H
#define LIFESIMULATIONC_LIFESIMULATIONRENDERER_H

#include "../control/LifeSimulationHandler.h"
#include "../control/SDLUtils.h"

#include <SDL2/SDL.h>
#include <cstdio>

class LifeSimulationRenderer {
public:
    LifeSimulationRenderer(LifeSimulationHandler* handler, SDL_Renderer* renderer);
    ~LifeSimulationRenderer();

    void drawSimulation();
private:
    LifeSimulationHandler* mHandler = nullptr;
    SDL_Renderer* mRenderer = nullptr;
};


#endif //LIFESIMULATIONC_LIFESIMULATIONRENDERER_H
