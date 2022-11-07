#pragma once
#include "glad/glad.h"

#include "../../glm/vec2.hpp"
#include "../../glm/vec3.hpp"
#include "../../glm/vec4.hpp"

class Mesh {
public:
	Mesh();
	~Mesh();

	void draw() const;
	void drawInstanced(GLsizei instancecount) const;

	static Mesh* generateQuad();
private:
	void bufferData();

	GLuint mType;

	GLuint mArrayObject;
	GLuint mNumVertices;

	GLuint mVBO;

	glm::vec3* mVertices;
	glm::vec4* mColors;
	glm::vec2* mTextureCoords;
};