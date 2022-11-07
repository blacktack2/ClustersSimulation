#pragma once
#include "../control/BaseShader.h"

#include "glad/glad.h"

#include <string>

class Shader : public BaseShader {
public:
	Shader(const char* vertex, const char* fragment);
};