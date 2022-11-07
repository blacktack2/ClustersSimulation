#include "Mesh.h"

Mesh::Mesh() :
mType(GL_TRIANGLES), mVBO(0), mArrayObject(0), mNumVertices(0),
mVertices(nullptr), mColors(nullptr), mTextureCoords(nullptr) {
	glGenVertexArrays(1, &mArrayObject);
}

Mesh::~Mesh() {
	glDeleteVertexArrays(1, &mArrayObject);

	delete[] mVertices;
	delete[] mColors;
	delete[] mTextureCoords;
}

void Mesh::draw() const {
	glBindVertexArray(mArrayObject);
	glDrawArrays(mType, 0, mNumVertices);
	glBindVertexArray(0);	
}

void Mesh::drawInstanced(GLsizei instancecount) const {
	glBindVertexArray(mArrayObject);
	glDrawArraysInstanced(mType, 0, mNumVertices, instancecount);
	glBindVertexArray(0);
}

Mesh* Mesh::generateQuad() {
	Mesh* mesh = new Mesh();
	mesh->mType = GL_TRIANGLE_STRIP;
	mesh->mNumVertices = 4;

	mesh->mVertices = new glm::vec3[mesh->mNumVertices];
	mesh->mVertices[0] = glm::vec3(-1.0f, -1.0f, 0.0f);
	mesh->mVertices[1] = glm::vec3( 1.0f, -1.0f, 0.0f);
	mesh->mVertices[2] = glm::vec3(-1.0f,  1.0f, 0.0f);
	mesh->mVertices[3] = glm::vec3( 1.0f,  1.0f, 0.0f);

	mesh->mColors = new glm::vec4[mesh->mNumVertices];
	mesh->mColors[0] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	mesh->mColors[1] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	mesh->mColors[2] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	mesh->mColors[3] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	mesh->mTextureCoords = new glm::vec2[mesh->mNumVertices];
	mesh->mTextureCoords[0] = glm::vec2(0.0f, 0.0f);
	mesh->mTextureCoords[1] = glm::vec2(1.0f, 0.0f);
	mesh->mTextureCoords[2] = glm::vec2(0.0f, 1.0f);
	mesh->mTextureCoords[3] = glm::vec2(1.0f, 1.0f);

	mesh->bufferData();
	return mesh;
}

void Mesh::bufferData() {
	glBindVertexArray(mArrayObject);

	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mNumVertices * sizeof(glm::vec3), mVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
}
