cmake_minimum_required(VERSION 3.16)

# Project setup
project(LightEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(lib/Utils)

# --------------------------------------------------
# NewSyntaxParser - parser engine as a static library (Syntax/ + Tokenizer/
# only; skip its main.cpp and tests, and avoid re-adding Utils).
# --------------------------------------------------
file(GLOB SYNPARSER_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/NewSyntaxParser/src/Syntax/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/NewSyntaxParser/src/Tokenizer/*.cpp)

# Node.cpp is a debug tree-printer that uses Utils::Font::reset / Utils::Text::Stream.
# The current Utils submodule provides both, so it can be compiled normally.

add_library(synparser STATIC ${SYNPARSER_SOURCES})
target_compile_features(synparser PUBLIC cxx_std_23)
target_include_directories(synparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/NewSyntaxParser/src)
target_link_libraries(synparser PUBLIC utils)

# --------------------------------------------------
# LightEngine library
# --------------------------------------------------
file(GLOB_RECURSE LIGHTENGINE_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(LightEngine ${LIGHTENGINE_SOURCES})
add_library(LightEngine::LightEngine ALIAS LightEngine)

target_compile_features(LightEngine PUBLIC cxx_std_23)
# Utils types appear in public headers (IP, RGB, FragmentedStorage) -> PUBLIC.
# synparser::Token/Node appear in the Console command headers -> PUBLIC.
target_link_libraries(LightEngine PUBLIC utils synparser)
target_include_directories(LightEngine
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# --------------------------------------------------
# Example / demo app (was the old executable)
# --------------------------------------------------
# Absolute path to the grammar/token data, baked into the example binaries so
# they run from any working directory (not just the repo root).
set(LE_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/LightEngine/Commands/data")

add_executable(LightEngine_demo examples/demo/main.cpp)
target_link_libraries(LightEngine_demo PRIVATE LightEngine::LightEngine)
target_compile_definitions(LightEngine_demo PRIVATE LE_DATA_DIR="${LE_DATA_DIR}")

# Full-functionality walkthrough: programmer, static + animated effects,
# store/recall of groups and presets. See examples/presets/main.cpp.
add_executable(LightEngine_presets examples/presets/main.cpp)
target_link_libraries(LightEngine_presets PRIVATE LightEngine::LightEngine)
target_compile_definitions(LightEngine_presets PRIVATE LE_DATA_DIR="${LE_DATA_DIR}")

# add_executable(LightEngine_commands examples/commands/main.cpp)
# target_link_libraries(LightEngine_commands PRIVATE LightEngine::LightEngine)
# target_compile_definitions(LightEngine_commands PRIVATE LE_DATA_DIR="${LE_DATA_DIR}")
