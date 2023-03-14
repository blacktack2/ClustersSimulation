/**
 * @file   Mesh.h
 * @brief  Data structure class for handling OpenGl vertex data and draw calls.
 * 
 * @author Stuart Lewis
 * @date   January 2023
 */
#pragma once
#include "glad/glad.h"

#include "../../glm/vec2.hpp"
#include "../../glm/vec3.hpp"
#include "../../glm/vec4.hpp"

/**
 * Handler class for managing vertex data and OpenGL draw calls.
 */
class Mesh {
public:
	Mesh();
	~Mesh();

	/**
	 * Draw this mesh to the current framebuffer. See glDrawArrays.
	 */
	void draw() const;
	/**
	 * Perform an instanced draw call for this mesh to the current framebuffer.
	 * See glDrawArraysInstanced.
	 * @param instancecount Number of instances to draw.
	 */
	void drawInstanced(GLsizei instancecount) const;

	/**
	 * Generate a standard quad mesh bounded between -1 and 1.
	 */
	static Mesh* generateQuad();
private:
	/**
	 * Upload vertex data to the GPU. Should be called only once, and before
	 * drawing.
	 */
	void bufferData();

	GLuint mType;

	GLuint mArrayObject;
	GLuint mNumVertices;

	GLuint mVBO;

	glm::vec3* mVertices;
	glm::vec4* mColors;
	glm::vec2* mTextureCoords;
};