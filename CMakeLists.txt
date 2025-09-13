cmake_minimum_required(VERSION 3.30.0..)

project(AST VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Flags.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Features.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Predefinitions.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchDependencies.cmake)


# ========= Base setup & Predefinitions =========
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(REQUIRED_CXX_STANDARD cxx_std_23) # read-only
set(CXX_STANDARD)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BENCHMARK_ENABLE_TESTING)

set(CMAKE_MSVC_RUNTIME_LIBRARY)
set(CMAKE_CXX_FLAGS_RELEASE)
set(CMAKE_CXX_FLAGS_DEBUG)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

InstallCXXStandardVersion()
InstallDebugLevel()
InstallBenchmarkDependenciesTesting()


add_subdirectory(dependencies/RapidXML)
add_subdirectory(sources)
add_subdirectory(tests)

