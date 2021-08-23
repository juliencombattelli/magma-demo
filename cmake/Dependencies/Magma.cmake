# FetchContent_MakeAvailable was added in CMake 3.14
cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(
    magma
    GIT_REPOSITORY https://github.com/juliencombattelli/magma.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(magma)
