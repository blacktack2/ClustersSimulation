cmake_minimum_required(VERSION 3.17)

project(ClustersSimulation)

set(CMAKE_CXX_STANDARD 17)

file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})
file(COPY src/shaders DESTINATION ${CMAKE_BINARY_DIR})

find_package(OpenGL REQUIRED)

if (WIN32)
    set(WHERE-IS-SDL "c:/programs/sdl/lib/x64")
    set(WHERE-IS-SDL-HEADERS "c:/programs/sdl/include")
    set(WHERE-IS-GLAD "c:/programs/glad/src/glad.c")
    set(WHERE-IS-GLAD-HEADERS "c:/programs/glad/include")
endif()

if (UNIX)
    set(WHERE-IS-GLAD "~/programs/glad/src/glad.c")
    set(WHERE-IS-GLAD-HEADERS "~/programs/glad/include")

    if (APPLE)
        set(WHERE-IS-SDL "${CMAKE_SOURCE_DIR}/lib")
        set(WHERE-IS-SDL-HEADERS "${CMAKE_SOURCE_DIR}/lib/SDL2.framework/Versions/Current/Headers")
    endif()

    if(NOT APPLE)
        find_package(SDL2 REQUIRED)
        include_directories(${SDL2_INCLUDE_DIRS})
    endif()
endif()

if (WIN32 OR APPLE)
    find_library(SDL SDL2 PATHS ${WHERE-IS-SDL})
    if (WIN32)
        find_library(SDLmain SDL2main PATHS ${WHERE-IS-SDL})
    endif()

    include_directories(
            "${WHERE-IS-SDL-HEADERS}"
    )
endif()

add_library("glad" ${WHERE-IS-GLAD})
include_directories(
        "${WHERE-IS-GLAD-HEADERS}"
)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")
file(GLOB_RECURSE IMGUI_SRC CONFIGURE_DEPENDS "imgui/*.h" "imgui/*.cpp")
file(GLOB_RECURSE GLM_SRC CONFIGURE_DEPENDS "glm/*.h" "glm/*.hpp")

set(EXECUTABLES ${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_NAME}_GPU)
foreach (executable IN LISTS EXECUTABLES)
    message("Creating executable: " ${executable}.exe)
    if (WIN32)
        add_executable(${executable} WIN32 ${SRC} ${IMGUI_SRC} ${GLM_SRC})
        target_link_libraries(${executable}
                "glad"
                ${SDL}
                ${SDLmain}
                ${OPENGL_gl_LIBRARY}
                )
    else()
        add_executable(${executable} ${SRC} ${IMGUI_SRC} ${GLM_SRC})
        target_link_libraries(${executable}
                "glad"
                ${OPENGL_gl_LIBRARY}
                )
        if (APPLE)
            target_link_libraries(${executable}
                    ${SDL}
                    )
        endif()
        if (UNIX AND NOT APPLE)
            target_link_libraries(${executable}
                    ${SDL2_LIBRARIES}
                    ${CMAKE_DL_LIBS}
                    )
        endif()
    endif()
endforeach()

target_compile_definitions(${CMAKE_PROJECT_NAME}_GPU PUBLIC ITERATE_ON_COMPUTE_SHADER)
