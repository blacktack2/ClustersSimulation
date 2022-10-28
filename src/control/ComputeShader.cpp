#include "ComputeShader.h"

#include "GLUtilities.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

ComputeShader::ComputeShader(const char* filename) : BaseShader() {
    readFile(filename, GL_COMPUTE_SHADER);
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
