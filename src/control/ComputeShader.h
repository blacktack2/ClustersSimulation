/**
 * @file   ComputeShader.h
 * @brief  Compute implementation of the BaseShader class
 * 
 * @author Stuart Lewis
 * @date   January 2023
 */
#pragma once
#include "BaseShader.h"

#include <array>
#include <string>
#include <vector>

/**
 * Compute implementation of the BaseShader class
 */
class ComputeShader : public BaseShader {
public:
	explicit ComputeShader(const char* filename);

	/**
	 * Execute a Compute Shader pass. See glDispatchCompute.
	 * @param x Number of work groups to be launched in the X dimension.
	 * @param y Number of work groups to be launched in the Y dimension.
	 * @param z Number of work groups to be launched in the Z dimension.
	 */
	void run(GLuint x, GLuint y, GLuint z);
};
