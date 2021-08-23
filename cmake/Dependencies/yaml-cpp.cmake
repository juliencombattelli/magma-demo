# FetchContent_MakeAvailable was added in CMake 3.14
cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY  https://github.com/jbeder/yaml-cpp
    GIT_TAG         yaml-cpp-0.7.0
    GIT_SHALLOW     TRUE
)

set(YAML_CPP_BUILD_CONTRIB OFF CACHE INTERNAL "")
set(YAML_CPP_BUILD_TOOLS OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(yaml-cpp)
