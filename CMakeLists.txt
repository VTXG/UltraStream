cmake_minimum_required(VERSION 4.00)

project(UltraStream LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE ULTRASTREAM_SOURCES
    "source/*.cpp"
)

add_library(UltraStream ${ULTRASTREAM_SOURCES})

target_include_directories(UltraStream PUBLIC include)