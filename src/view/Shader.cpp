#include "Shader.h"

Shader::Shader(const char* vertex, const char* fragment) : BaseShader() {
    mShaderPasses.emplace_back(vertex, GL_VERTEX_SHADER);
    mShaderPasses.emplace_back(fragment, GL_FRAGMENT_SHADER);
}
