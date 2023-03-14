/**
 * @file   Shader.h
 * @brief  Graphics implementation of the BaseShader class
 * 
 * @author Stuart Lewis
 * @date   January 2023
 */
#pragma once
#include "../control/BaseShader.h"

#include "glad/glad.h"

#include <string>

/**
 * Graphics implementation of the BaseShader class for handling vertex/fragment shaders.
 */
class Shader : public BaseShader {
public:
	Shader(const char* vertex, const char* fragment);
};