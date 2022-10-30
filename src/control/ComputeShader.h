#pragma once
#include "BaseShader.h"

#include <array>
#include <string>
#include <vector>

class ComputeShader : public BaseShader {
public:
	explicit ComputeShader(const char* filename);

	void run(GLuint x, GLuint y, GLuint z);
};
