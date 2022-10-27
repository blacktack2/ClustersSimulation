#pragma once

#include "glad/glad.h"

#include <array>
#include <string>
#include <vector>

class ComputeShader {
public:
	ComputeShader();

	void loadFile(const char* filename);

	GLuint createBuffer(const GLvoid* data, GLsizeiptr size, GLuint binding);

	void setGroupCount(int pass, GLuint x, GLuint y, GLuint z);

	void run();

	void readBuffer(GLuint bufferID, GLvoid* data, GLsizeiptr size);
	void writeBuffer(GLuint bufferID, GLvoid* data, GLsizeiptr size);

	bool isValid();
private:
	struct ShaderPass {
		GLuint computeShader;
		GLuint program;
		GLuint numGroupsX;
		GLuint numGroupsY;
		GLuint numGroupsZ;
	};

	bool mIsValid = true;

	std::vector<ShaderPass> mPasses{};
	std::vector<GLuint> mBuffers{};

	const std::string SHADER_DIR = "shaders/";
};
