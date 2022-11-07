#include "BaseShader.h"

#include "GLUtilities.h"

#include <iostream>
#include <fstream>
#include <sstream>

std::vector<GLuint> BaseShader::mBuffers;
std::vector<GLuint> BaseShader::mPrograms;

BaseShader::BaseShader() : mProgramID(0) {
}

BaseShader::~BaseShader() = default;

void BaseShader::init() {
    mPrograms.push_back(mProgramID = glCreateProgram());

    int  success;
    char infoLog[512];
    for (auto& shaderPass : mShaderPasses) {
        GLuint shader = mShaders.emplace_back(glCreateShader(shaderPass.type));
        const char* code_c = shaderPass.code;
        glShaderSource(shader, 1, &code_c, nullptr);
        glCompileShader(shader);

        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(success != GL_TRUE) {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            fprintf(stderr, "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n%s\n", infoLog);
            mIsValid = false;
            return;
        }
        glAttachShader(mProgramID, shader);

        glDeleteShader(shader);
#ifdef _DEBUG
        glCheckError();
#endif
    }
    glLinkProgram(mProgramID);

    glGetProgramiv(mProgramID, GL_LINK_STATUS, &success);
    if(success != GL_TRUE) {
        glGetProgramInfoLog(mProgramID, 512, nullptr, infoLog);
        fprintf(stderr, "ERROR::SHADER::COMPUTE::LINKING_FAILED\n%s\n", infoLog);
        mIsValid = false;
        return;
    }
}

void BaseShader::bind() const {
    glUseProgram(mProgramID);
}

void BaseShader::unbind() { // NOLINT(readability-convert-member-functions-to-static)
    glUseProgram(0);
}

void BaseShader::setUniform(const std::string& location, const GLfloat value) {
    for (const auto& programID : mPrograms) {
        glUseProgram(programID);
        glUniform1f(glGetUniformLocation(programID, location.c_str()), value);
    }
#ifdef _DEBUG
    glCheckError();
#endif
}

void BaseShader::setUniform(const std::string& location, GLfloat value1, GLfloat value2) {
    for (const auto& programID : mPrograms) {
        glUseProgram(programID);
        glUniform2f(glGetUniformLocation(programID, location.c_str()), value1, value2);
    }
#ifdef _DEBUG
    glCheckError();
#endif
}

GLuint BaseShader::createBuffer(const GLvoid* data, const GLsizeiptr size, const GLuint binding) {
    GLuint& id = mBuffers.emplace_back();
    glGenBuffers(1, &id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#ifdef _DEBUG
    glCheckError();
#endif
    return id;
}

void BaseShader::readBuffer(const GLuint bufferID, GLvoid* data, const GLsizeiptr size) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#ifdef _DEBUG
    glCheckError();
#endif
}

void BaseShader::writeBuffer(const GLuint bufferID, GLvoid* data, const GLsizeiptr size) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_READ);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#ifdef _DEBUG
    glCheckError();
#endif
}
