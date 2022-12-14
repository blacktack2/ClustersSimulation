#include "SimulationRenderer.h"

#include "Logger.h"

#include "../../glm/vec3.hpp"

#include "../../imgui/imgui.h"

#ifdef ITERATE_ON_COMPUTE_SHADER
#include "glad/glad.h"
#endif

const char* SHADER_CODE_VERT =
#include "../shaders/Atom.vert"
;
const char* SHADER_CODE_FRAG =
#include "../shaders/Atom.frag"
;

SimulationRenderer::SimulationRenderer(SimulationHandler& handler) :
mHandler(handler)
#ifdef ITERATE_ON_COMPUTE_SHADER
, mShader(SHADER_CODE_VERT, SHADER_CODE_FRAG), mFrameBuffer(0), mTexture(0), mQuad(nullptr)
#endif
{
    Logger::getLogger().logMessage("Constructing Renderer");
}

SimulationRenderer::~SimulationRenderer() {
    Logger::getLogger().logMessage("Destroying Renderer");
#ifdef ITERATE_ON_COMPUTE_SHADER
    if (mQuad != nullptr)
        delete mQuad;
#endif
}

void SimulationRenderer::updateParameters() {
#ifdef ITERATE_ON_COMPUTE_SHADER
    mShader.setUniform(SIMULATION_BOUNDS_UNIFORM, mHandler.getWidth(), mHandler.getHeight());
    mShader.setUniform(ATOM_DIAMETER_UNIFORM, mHandler.getAtomDiameter());
#endif
}

bool SimulationRenderer::init() { // NOLINT(readability-convert-member-functions-to-static)
    Logger::getLogger().logMessage("Initializing Renderer");
#ifdef ITERATE_ON_COMPUTE_SHADER
    mQuad = Mesh::generateQuad();

    glGenFramebuffers(1, &mFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);

    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture, 0);
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);
#ifdef _DEBUG
    GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Logger::getLogger().logError(std::string("OpenGL Framebuffer error - Status: ").append(std::to_string(status)));
        return false;
    }
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    mShader.init();
    if (!mShader.isValid()) {
        Logger::getLogger().logError(std::string("Failed to initialize shader"));
        return false;
    }
#endif
    return true;
}

void SimulationRenderer::drawSimulation([[maybe_unused]] float startX, [[maybe_unused]] float startY, float width, float height) {
#ifdef ITERATE_ON_COMPUTE_SHADER
    mShader.bind();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    if (imageWidth != width || imageHeight != height) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, imageWidth = width, imageHeight = height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        mShader.setUniform(SCREEN_BOUNDS_UNIFORM, imageWidth, imageHeight);
    }

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    mQuad->drawInstanced(mHandler.getActualAtomCount());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    mShader.unbind();

    ImGui::Image((ImTextureID) (uintptr_t) mTexture, ImVec2(width, height));
#else
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 clipMin = ImVec2(startX, startY);
    ImVec2 clipMax = ImVec2(clipMin.x + width, clipMin.y + height);

    drawList->AddRectFilled(
        clipMin, clipMax,
        ImColor(0.2f, 0.2f, 0.2f)
    );

    ImGui::PushClipRect(clipMin, clipMax, true);
    float scaleX = static_cast<float>(width) / mHandler.getWidth();
    float scaleY = static_cast<float>(height) / mHandler.getHeight();
    float atomSize = std::max(mHandler.getAtomDiameter() * scaleX, 3.0f);

    auto& atoms = mHandler.getAtoms();
    for (int i = 0; i < mHandler.getActualAtomCount(); i++) {
        const Atom& atom = atoms[i];
        glm::vec3 c = mHandler.getAtomTypeColor(atom.atomType);
        float x = clipMin.x + atom.x * scaleX;
        float y = clipMin.y + atom.y * scaleY;
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

    ImGui::PopClipRect();
#endif
}
