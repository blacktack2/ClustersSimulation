#include "LifeSimulationRenderer.h"

LifeSimulationRenderer::LifeSimulationRenderer(LifeSimulationHandler *handler, SDL_Renderer *renderer) {
    mHandler = handler;
    mRenderer = renderer;
}

LifeSimulationRenderer::~LifeSimulationRenderer() {
    mHandler = nullptr;
    mRenderer = nullptr;
}

void LifeSimulationRenderer::drawSimulation() {
    std::vector<Atom*>* atoms = mHandler->getAtoms();
    for (auto atom : *atoms) {
        SetRenderDrawColor(mRenderer, atom->getAtomType()->getColor());
        SDL_Rect fillRect = {(int) atom->mX, (int) atom->mY, 3, 3};
        SDL_RenderFillRect(mRenderer, &fillRect);
    }
}
