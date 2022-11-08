#include "GLUtilities.h"

#include "../view/Logger.h"

#include <cstdio>
#include <string>

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
                            [[maybe_unused]] GLsizei length, const char *message, [[maybe_unused]] const void *userParam) {
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::string sourceString;
    std::string typeString;
    std::string severityString;

    switch (source) {
        case GL_DEBUG_SOURCE_API            : sourceString = "API"            ; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM  : sourceString = "Window System"  ; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceString = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY    : sourceString = "Third Party"    ; break;
        case GL_DEBUG_SOURCE_APPLICATION    : sourceString = "Application"    ; break;
        case GL_DEBUG_SOURCE_OTHER          : sourceString = "Other"          ; break;
        default                             : sourceString = "Unknown"        ; break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR              : typeString = "Error"              ; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeString = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR : typeString = "Undefined Behavior" ; break; 
        case GL_DEBUG_TYPE_PORTABILITY        : typeString = "Portability"        ; break;
        case GL_DEBUG_TYPE_PERFORMANCE        : typeString = "Performance"        ; break;
        case GL_DEBUG_TYPE_MARKER             : typeString = "Marker"             ; break;
        case GL_DEBUG_TYPE_PUSH_GROUP         : typeString = "Push Group"         ; break;
        case GL_DEBUG_TYPE_POP_GROUP          : typeString = "Pop Group"          ; break;
        case GL_DEBUG_TYPE_OTHER              : typeString = "Other"              ; break;
        default                               : typeString = "Unknown"            ; break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH        : severityString = "High"        ; break;
        case GL_DEBUG_SEVERITY_MEDIUM      : severityString = "Medium"      ; break;
        case GL_DEBUG_SEVERITY_LOW         : severityString = "Low"         ; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityString = "Notification"; break;
        default                            : severityString = "Unknown"     ; break;
    }

    if (severity == GL_DEBUG_SEVERITY_HIGH)
        Logger::getLogger().logError(
            std::string("OpenGL exception - Source: ").append(sourceString)
            .append(" - Type:").append(typeString)
            .append(" - Severity: ").append(severityString)
        );
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
        Logger::getLogger().logWarning(
            std::string("OpenGL warning - Source: ").append(sourceString)
            .append(" - Type:").append(typeString)
            .append(" - Severity: ").append(severityString)
        );
    else
        Logger::getLogger().logMessage(
            std::string("OpenGL message - Source: ").append(sourceString)
            .append(" - Type:").append(typeString)
            .append(" - Severity: ").append(severityString)
        );
}

void glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
            case GL_INVALID_ENUM                 : error = "INVALID_ENUM"                 ; break;
            case GL_INVALID_VALUE                : error = "INVALID_VALUE"                ; break;
            case GL_INVALID_OPERATION            : error = "INVALID_OPERATION"            ; break;
            case GL_STACK_OVERFLOW               : error = "STACK_OVERFLOW"               ; break;
            case GL_STACK_UNDERFLOW              : error = "STACK_UNDERFLOW"              ; break;
            case GL_OUT_OF_MEMORY                : error = "OUT_OF_MEMORY"                ; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            default                              : error = "__UNKNOWN_ERROR__"            ; break;
        }
        Logger::getLogger().logError(
            std::string("OpenGL Error - ").append(error)
            .append(" (").append(file).append(" LINE:").append(std::to_string(line)).append(")")
        );
        fprintf(stderr, "%s | %s (LINE: %d)\n", error.c_str(), file, line);
    }
}
