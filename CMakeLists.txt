cmake_minimum_required(VERSION 3.20)
project(MagmaDemo)

option(MAGMADEMO_FIND_PACKAGE_MAGMA "Use find_package to locate Magma" OFF)

set(CMAKE_CXX_EXTENSIONS FALSE)

if(MAGMADEMO_FIND_PACKAGE_MAGMA)
    find_package(Magma REQUIRED)
else()
    include(${PROJECT_SOURCE_DIR}/cmake/Dependencies/Magma.cmake)
endif()

include(${PROJECT_SOURCE_DIR}/cmake/Dependencies/yaml-cpp.cmake)

add_executable(MagmaDemo src/main.cpp)
target_link_libraries(MagmaDemo
    PRIVATE
        Magma::Magma
        yaml-cpp::yaml-cpp
)

Magma_AddShader(ShaderVert shaders/shader.vert)
Magma_AddShader(ShaderFrag shaders/shader.frag)
