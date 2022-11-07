#include "ComputeShader.h"

#include "GLUtilities.h"

ComputeShader::ComputeShader(const char* code) : BaseShader() {
    mShaderPasses.emplace_back(code, GL_COMPUTE_SHADER);
}

void ComputeShader::run(GLuint x, GLuint y, GLuint z) {
    bind();
    glDispatchCompute(x, y, z);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    unbind();
#ifdef _DEBUG
    glCheckError();
#endif
}
