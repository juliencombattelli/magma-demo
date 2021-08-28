cmake_minimum_required(VERSION 3.20)
project(MagmaDemo)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(MAGMADEMO_IS_USED_STANDALONE ON)
else()
    set(MAGMADEMO_IS_USED_STANDALONE OFF)
endif()

# If MagmaDemo is included as part of a bigger project, assume that this project is actually
# Magma, or a project that already handles Magma package dependency.
if(MAGMADEMO_IS_USED_STANDALONE)
    option(MAGMADEMO_FIND_PACKAGE_MAGMA "Use find_package to locate Magma" OFF)
    if(MAGMADEMO_FIND_PACKAGE_MAGMA)
        find_package(Magma REQUIRED)
    else()
        include(${PROJECT_SOURCE_DIR}/cmake/Dependencies/Magma.cmake)
    endif()
endif()

set(CMAKE_CXX_EXTENSIONS FALSE)

include(${PROJECT_SOURCE_DIR}/cmake/Dependencies/yaml-cpp.cmake)

add_executable(MagmaDemo src/main.cpp)
target_link_libraries(MagmaDemo
    PRIVATE
        Magma::Magma
        yaml-cpp::yaml-cpp
)

Magma_AddShader(ShaderVert shaders/shader.vert)
Magma_AddShader(ShaderFrag shaders/shader.frag)
