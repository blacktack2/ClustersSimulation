#include "LifeSimulationRenderer.h"

#include "../../imgui/imgui.h"

LifeSimulationRenderer::LifeSimulationRenderer(LifeSimulationHandler& handler) :
mHandler(handler) {

}

LifeSimulationRenderer::~LifeSimulationRenderer() {

}

void LifeSimulationRenderer::drawSimulation(float startX, float startY, float width, float height) {
    std::vector<Atom*>& atoms = mHandler.getAtoms();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(
            ImVec2(startX, startY), ImVec2(startX + width, startY + height),
            ImColor(0.2f, 0.2f, 0.2f)
            );

    float scaleX = static_cast<float>(width) / mHandler.getWidth();
    float scaleY = static_cast<float>(height) / mHandler.getHeight();
    float atomSize = 3 * scaleX;

    for (Atom* atom : atoms) {
        AtomType* atomType = atom->getAtomType();
        Color c = atomType->getColor();
        float x = startX + atom->mX * scaleX;
        float y = startY + atom->mY * scaleY;
        drawList->AddCircleFilled(
                ImVec2(x, y), atomSize,
                ImColor(ImVec4(c.r, c.g, c.b, 1.0f))
                );
    }
}
