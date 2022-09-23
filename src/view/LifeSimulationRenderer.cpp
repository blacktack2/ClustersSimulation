#include "LifeSimulationRenderer.h"
#include "../../imgui/imgui.h"

LifeSimulationRenderer::LifeSimulationRenderer(LifeSimulationHandler *handler) {
    mHandler = handler;
}

LifeSimulationRenderer::~LifeSimulationRenderer() {
    mHandler = nullptr;
}

void LifeSimulationRenderer::drawSimulation(float startX, float startY, float width, float height) {
    std::vector<Atom*>* atoms = mHandler->getAtoms();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float scaleX = static_cast<float>(width) / mHandler->getWidth();
    float scaleY = static_cast<float>(height) / mHandler->getHeight();
    float atomSize = 3 * scaleX;

    for (auto atom : *atoms) {
        AtomType* at = atom->getAtomType();
        Color c = at->getColor();
        float x = startX + atom->mX * scaleX;
        float y = startY + atom->mY * scaleY;
        drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + atomSize, y + atomSize),
                          ImColor(ImVec4(c.r, c.g, c.b, 1.0f)));
    }
}
