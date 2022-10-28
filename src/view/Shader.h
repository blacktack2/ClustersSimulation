#pragma once
#include "../control/BaseShader.h"

#include "glad/glad.h"

#include <string>

class Shader : public BaseShader {
public:
	Shader(const char* vertex, const char* fragment);

	void setVAO(const float* vertices, unsigned int size, unsigned int stride);

	void draw();
	void drawInstanced(unsigned int instanceCount);
private:
	GLuint mVAO;
	GLuint mVBO;

	unsigned int mStride;
};