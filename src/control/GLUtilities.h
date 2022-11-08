#pragma once
#include "glad/glad.h"

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
                            [[maybe_unused]] GLsizei length, const char *message, [[maybe_unused]] const void *userParam);

void glCheckError_(const char* file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)
