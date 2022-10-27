#include "SimulationRenderer.h"

#include "../../imgui/imgui.h"

#ifdef ITERATE_ON_COMPUTE_SHADER
#include "glad/glad.h"
#endif

SimulationRenderer::SimulationRenderer(SimulationHandler& handler) :
mHandler(handler) {

}

SimulationRenderer::~SimulationRenderer() {

}

void SimulationRenderer::drawSimulation(float startX, float startY, float width, float height) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    //ImGui::Image((ImTextureID) mHandler.getRenderTexture(), ImVec2(width, height));
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(
        ImVec2(startX, startY), ImVec2(startX + width, startY + height),
        ImColor(0.2f, 0.2f, 0.2f)
    );

    float scaleX = static_cast<float>(width) / mHandler.getWidth();
    float scaleY = static_cast<float>(height) / mHandler.getHeight();
    float atomSize = std::max(3.0 * scaleX, 3.0);
    auto& atoms = mHandler.getAtoms();
    for (int i = 0; i < mHandler.getAtomCount(); i++) {
        float x = startX + atoms[i].x * scaleX;
        float y = startY + atoms[i].y * scaleY;
        Color c = mHandler.getAtomTypeColor(atoms[i].atomType);
        drawList->AddCircleFilled(
            ImVec2(x, y), atomSize,
            ImColor(c.r, c.g, c.b, 1.0f)
        );
        drawList->AddCircle(
            ImVec2(x, y), atomSize,
            ImColor(0.0f, 0.0f, 0.0f, 0.5f),
            0, 2.0f
        );
    }
#else
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(
        ImVec2(startX, startY), ImVec2(startX + width, startY + height),
        ImColor(0.2f, 0.2f, 0.2f)
    );

    float scaleX = static_cast<float>(width) / mHandler.getWidth();
    float scaleY = static_cast<float>(height) / mHandler.getHeight();
    float atomSize = std::max(3.0 * scaleX, 3.0);

    for (Atom& atom : mHandler.getAtoms()) {
        Color c = atom.mAtomType->getColor();
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
#endif
}
