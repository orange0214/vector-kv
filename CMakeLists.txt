cmake_minimum_required(VERSION 3.20)

project(VectorKV
    VERSION 0.1.0
    DESCRIPTION "A C++ in-memory vector database with HNSW search and WAL recovery"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put all build artifacts in predictable locations.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Generate compile_commands.json for editors / clangd.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to a Release build when the user does not specify one.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------
option(VECTORKV_BUILD_TESTS "Build unit tests (GoogleTest)" ON)
option(VECTORKV_BUILD_BENCHMARKS "Build benchmarks (Google Benchmark)" ON)
option(VECTORKV_BUILD_EXAMPLES "Build example executables" ON)

# ---------------------------------------------------------------------------
# Core library target: vectorkv_core
#
# Business logic (VectorStore, HNSW, WAL, etc.) will be added here later.
# For now it only contains an infrastructure placeholder so every target
# compiles and links cleanly.
# ---------------------------------------------------------------------------
add_library(vectorkv_core
    src/version.cpp
    src/distance.cpp
    src/vector_store.cpp
    src/brute_force_index.cpp
    src/vector_db.cpp
    src/wal.cpp
)

target_include_directories(vectorkv_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(vectorkv_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(vectorkv_core PRIVATE /W4)
else()
    target_compile_options(vectorkv_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# A namespaced alias so consumers can link against VectorKV::core.
add_library(VectorKV::core ALIAS vectorkv_core)

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------
if(VECTORKV_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(VECTORKV_BUILD_BENCHMARKS)
    add_subdirectory(bench)
endif()

if(VECTORKV_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ---------------------------------------------------------------------------
# Configuration summary
# ---------------------------------------------------------------------------
message(STATUS "------------------------------------------------------------")
message(STATUS "VectorKV ${PROJECT_VERSION}")
message(STATUS "  Build type     : ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard   : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Tests          : ${VECTORKV_BUILD_TESTS}")
message(STATUS "  Benchmarks     : ${VECTORKV_BUILD_BENCHMARKS}")
message(STATUS "  Examples       : ${VECTORKV_BUILD_EXAMPLES}")
message(STATUS "------------------------------------------------------------")
