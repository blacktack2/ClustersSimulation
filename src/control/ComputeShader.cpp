#include "ComputeShader.h"

#include "GLUtilities.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

ComputeShader::ComputeShader() {

}

void ComputeShader::loadFile(const char* filename) {
    int  success;
    char infoLog[512];

    std::string code;
    try {
        std::ifstream file;
        file.open(SHADER_DIR + filename);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        code = stream.str();
    } catch (std::ifstream::failure e) {
        fprintf(stderr, "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n");
        mIsValid = false;
        return;
    }
    ShaderPass pass = mPasses.emplace_back(ShaderPass{glCreateShader(GL_COMPUTE_SHADER), glCreateProgram(), 0, 0, 0});
    const char* code_c = code.c_str();
    glShaderSource(pass.computeShader, 1, &code_c, nullptr);
    glCompileShader(pass.computeShader);

    glGetShaderiv(pass.computeShader, GL_COMPILE_STATUS, &success);
    if(success != GL_TRUE) {
        glGetShaderInfoLog(pass.computeShader, 512, nullptr, infoLog);
        fprintf(stderr, "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n%s\n", infoLog);
        mIsValid = false;
        return;
    }
    glAttachShader(pass.program, pass.computeShader);
    glLinkProgram(pass.program);

    glGetProgramiv(pass.program, GL_LINK_STATUS, &success);
    if(success != GL_TRUE) {
        glGetProgramInfoLog(pass.program, 512, nullptr, infoLog);
        fprintf(stderr, "ERROR::SHADER::COMPUTE::LINKING_FAILED\n%s\n", infoLog);
        mIsValid = false;
        return;
    }

    glDeleteShader(pass.computeShader);
#ifdef _DEBUG
    glCheckError();
#endif
}

GLuint ComputeShader::createBuffer(const GLvoid* data, GLsizeiptr size, GLuint binding) {
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

void ComputeShader::readBuffer(GLuint bufferID, GLvoid* data, GLsizeiptr size) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#ifdef _DEBUG
    glCheckError();
#endif
}

void ComputeShader::writeBuffer(GLuint bufferID, GLvoid* data, GLsizeiptr size) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_READ);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#ifdef _DEBUG
    glCheckError();
#endif
}

void ComputeShader::setGroupCount(int pass, GLuint x, GLuint y, GLuint z) {
    mPasses[pass].numGroupsX = x;
    mPasses[pass].numGroupsY = y;
    mPasses[pass].numGroupsZ = z;
}

void ComputeShader::run() {
    for (auto& pass : mPasses) {
        glUseProgram(pass.program);
        glDispatchCompute(pass.numGroupsX, pass.numGroupsY, pass.numGroupsZ);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
#ifdef _DEBUG
        glCheckError();
#endif
    }
}

bool ComputeShader::isValid() {
	return mIsValid;
}
