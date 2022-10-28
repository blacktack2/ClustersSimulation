#pragma once
#include "glad/glad.h"

#include <string>
#include <vector>

class BaseShader {
public:
	BaseShader();
	~BaseShader();

	virtual void init();

	void bind();
	void unbind();

	static void setUniform(std::string location, GLfloat value);
	static void setUniform(std::string location, GLfloat value1, GLfloat value2);

	inline bool isValid() { return mIsValid; }

	static GLuint createBuffer(const GLvoid* data, GLsizeiptr size, GLuint binding);
	static void readBuffer(GLuint bufferID, GLvoid* data, GLsizeiptr size);
	static void writeBuffer(GLuint bufferID, GLvoid* data, GLsizeiptr size);
protected:
	void readFile(const char* filename, GLuint type);

	GLuint mProgramID;
	std::vector<GLuint> mShaders;

	bool mIsValid = true;

	struct ShaderPass {
		std::string code;
		GLuint type;
	};
	std::vector<ShaderPass> mShaderPasses;

	static std::vector<GLuint> mBuffers;
	static std::vector<GLuint> mPrograms;

	const std::string SHADER_DIR = "shaders/";
};