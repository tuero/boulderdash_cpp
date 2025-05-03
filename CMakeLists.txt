cmake_minimum_required (VERSION 3.13)

project(boulderdash)

include(FetchContent)

# Required for python modules
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Pybind11
message("Configuring Pybind11")
FetchContent_Declare(pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    GIT_TAG master
    SYSTEM
)
FetchContent_MakeAvailable(pybind11)

# Sources
set(BOULDERDASH_SOURCES
    src/definitions.h
    src/boulderdash_base.cpp 
    src/boulderdash_base.h 
    src/util.h
)

# CPP library
add_library(boulderdash STATIC ${BOULDERDASH_SOURCES})
target_compile_features(boulderdash PUBLIC cxx_std_20)
target_include_directories(boulderdash PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Python module
pybind11_add_module(pyboulderdash EXCLUDE_FROM_ALL python/pyboulderdash.cpp)
target_link_libraries(pyboulderdash PRIVATE boulderdash)
install(TARGETS pyboulderdash DESTINATION .)

# Build tests
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(BUILD_TESTS "Build the unit tests" OFF)
    if (${BUILD_TESTS})
        enable_testing()
        add_subdirectory(test)
    endif()
endif()
