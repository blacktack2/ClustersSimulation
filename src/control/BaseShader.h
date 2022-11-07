#pragma once
#include "glad/glad.h"

#include <string>
#include <vector>

class BaseShader {
public:
	BaseShader();
	~BaseShader();

	virtual void init();

	void bind() const;
	void unbind();

	static void setUniform(const std::string& location, GLfloat value);
	static void setUniform(const std::string& location, GLfloat value1, GLfloat value2);

	[[nodiscard]] inline const bool& isValid() const { return mIsValid; }

	static GLuint createBuffer(const GLvoid* data, GLsizeiptr size, GLuint binding);
	static void readBuffer(GLuint bufferID, GLvoid* data, GLsizeiptr size);
	static void writeBuffer(GLuint bufferID, GLvoid* data, GLsizeiptr size);
protected:
	GLuint mProgramID;
	std::vector<GLuint> mShaders;

	bool mIsValid = true;

	struct ShaderPass {
		ShaderPass(const char* c, GLuint t) : code(c), type(t) {}

		const char* code;
		GLuint type;
	};
	std::vector<ShaderPass> mShaderPasses;

	static std::vector<GLuint> mBuffers;
	static std::vector<GLuint> mPrograms;

	const std::string SHADER_DIR = "shaders/";
};