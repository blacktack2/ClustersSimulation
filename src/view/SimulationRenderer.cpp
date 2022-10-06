#include "SimulationRenderer.h"

#include "../../imgui/imgui.h"

SimulationRenderer::SimulationRenderer(SimulationHandler& handler) :
mHandler(handler) {

}

SimulationRenderer::~SimulationRenderer() {

}

void SimulationRenderer::drawSimulation(float startX, float startY, float width, float height) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(
            ImVec2(startX, startY), ImVec2(startX + width, startY + height),
            ImColor(0.2f, 0.2f, 0.2f)
            );

    float scaleX = static_cast<float>(width) / mHandler.getWidth();
    float scaleY = static_cast<float>(height) / mHandler.getHeight();
    float atomSize = std::max(3.0 * scaleX, 3.0);

    for (AtomType& atomType : mHandler.getLSRules().getAtomTypes()) {
        Color c = atomType.getColor();
        for (Atom& atom : atomType.getAtoms()) {
            float x = startX + atom.mX * scaleX;
            float y = startY + atom.mY * scaleY;
            drawList->AddCircleFilled(
                ImVec2(x, y), atomSize,
                ImColor(ImVec4(c.r, c.g, c.b, 1.0f))
            );
            drawList->AddCircle(
                ImVec2(x, y), atomSize,
                ImColor(ImVec4(0.0f, 0.0f, 0.0f, 0.5f)),
                0, 2.0f
            );
        }
    }
}
