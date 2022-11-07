#include "Shader.h"

Shader::Shader(const char* vertex, const char* fragment) : BaseShader() {
    readFile(vertex, GL_VERTEX_SHADER);
    readFile(fragment, GL_FRAGMENT_SHADER);
}
