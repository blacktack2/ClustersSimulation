#include "Shader.h"

Shader::Shader(const char* vertex, const char* fragment) : BaseShader(),
mVAO(0), mStride(0) {
    readFile(vertex, GL_VERTEX_SHADER);
    readFile(fragment, GL_FRAGMENT_SHADER);
}

void Shader::setVAO(const float* vertices, unsigned int size, unsigned int stride) {
    glGenBuffers(1, &mVBO);
    mStride = stride;
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void Shader::draw() {
    glBindVertexArray(mVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void Shader::drawInstanced(unsigned int instanceCount) {
    glBindVertexArray(mVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instanceCount);
    glBindVertexArray(0);
}
