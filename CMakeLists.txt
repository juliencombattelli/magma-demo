cmake_minimum_required(VERSION 3.20)
project(MagmaDemo)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(MAGMADEMO_IS_USED_STANDALONE ON)
else()
    set(MAGMADEMO_IS_USED_STANDALONE OFF)
endif()

if(MAGMADEMO_IS_USED_STANDALONE)
    option(MAGMADEMO_FIND_PACKAGE_MAGMA "Use find_package to locate Magma" OFF)
    if(MAGMADEMO_FIND_PACKAGE_MAGMA)
        find_package(Magma REQUIRED)
    else()
        include(${PROJECT_SOURCE_DIR}/cmake/Dependencies/Magma.cmake)
    endif()
else()
    # If MagmaDemo is included as part of a bigger project, ensure that this bigger project
    # is actually Magma, as this demo was not designed to be used in any other way.
    if(NOT CMAKE_PROJECT_NAME STREQUAL "Magma")
        message(FATAL_ERROR "MagmaDemo cannot be used as a subproject.")
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
