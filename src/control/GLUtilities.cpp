#include "GLUtilities.h"

#include <cstdio>
#include <string>

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
                            [[maybe_unused]] GLsizei length, const char *message, [[maybe_unused]] const void *userParam) {
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    fprintf(stderr, "---------------\n");
    fprintf(stderr, "Debug message (%s): \n", message);

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            fprintf(stderr, "Source: API\n");
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            fprintf(stderr, "Source: Window System\n");
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            fprintf(stderr, "Source: Shader Compiler\n");
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            fprintf(stderr, "Source: Third Party\n");
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            fprintf(stderr, "Source: Application\n");
            break;
        case GL_DEBUG_SOURCE_OTHER:
            fprintf(stderr, "Source: Other\n");
            break;
        default:
            fprintf(stderr, "Source: Unknown\n");
            break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            fprintf(stderr, "Type: Error\n");
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            fprintf(stderr, "Type: Deprecated Behaviour\n");
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            fprintf(stderr, "Type: Undefined Behaviour\n");
            break; 
        case GL_DEBUG_TYPE_PORTABILITY:
            fprintf(stderr, "Type: Portability\n");
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            fprintf(stderr, "Type: Performance\n");
            break;
        case GL_DEBUG_TYPE_MARKER:
            fprintf(stderr, "Type: Marker\n");
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            fprintf(stderr, "Type: Push Group\n");
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            fprintf(stderr, "Type: Pop Group\n");
            break;
        case GL_DEBUG_TYPE_OTHER:
            fprintf(stderr, "Type: Other\n");
            break;
        default:
            fprintf(stderr, "Type: Unknown\n");
            break;
    }

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            fprintf(stderr, "Severity: high\n");
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            fprintf(stderr, "Severity: medium\n");
            break;
        case GL_DEBUG_SEVERITY_LOW:
            fprintf(stderr, "Severity: low\n");
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            fprintf(stderr, "Severity: notification\n");
            break;
        default:
            fprintf(stderr, "Severity: Unknown\n");
    }
}

GLenum glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
            default:
                error = "__UNKNOWN_ERROR__";
                break;
        }
        fprintf(stderr, "%s | %s (LINE: %d)\n", error.c_str(), file, line);
    }
    return errorCode;
}
