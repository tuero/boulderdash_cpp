cmake_minimum_required (VERSION 3.23)

project(boulderdash VERSION 1.0.4)

option(BUILD_PYTHON_MODULE "Build the python library wrapper" OFF)

# Let boulderdash_SHARED_LIBS override BUILD_SHARED_LIBS
if(DEFINED boulderdash_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${boulderdash_SHARED_LIBS}")
endif()


# Required for python modules
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Sources
set(BOULDERDASH_SOURCES
    include/boulderdash/boulderdash.h 
    include/boulderdash/boulderdash_base.h 
    include/boulderdash/definitions.h
    src/boulderdash_base.cpp 
    src/util.h
)

# CPP library
add_library(boulderdash)
add_library(boulderdash::boulderdash ALIAS boulderdash)
set_target_properties(boulderdash PROPERTIES
                      VERSION ${boulderdash_VERSION}
                      SOVERSION ${boulderdash_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
)
target_include_directories(
    boulderdash PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
target_compile_features(boulderdash PUBLIC cxx_std_20)
target_sources(boulderdash PRIVATE ${BOULDERDASH_SOURCES})


# Include the install rules if the user wanted them (included by default when top-level)
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)
option(boulderdash_INCLUDE_PACKAGING "Include packaging rules for boulderdash" "${is_top_level}")
if(boulderdash_INCLUDE_PACKAGING)
    add_subdirectory(packaging)
endif()


# Pybind11
if(BUILD_PYTHON_MODULE)
    include(FetchContent)
    message("Configuring Pybind11")
    FetchContent_Declare(pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11
        GIT_TAG master
        SYSTEM
    )
    FetchContent_MakeAvailable(pybind11)

    # Python module
    pybind11_add_module(pyboulderdash python/pyboulderdash.cpp)
    target_link_libraries(pyboulderdash PRIVATE boulderdash)
    install(TARGETS pyboulderdash DESTINATION .)
endif()


# Build tests
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(BUILD_TESTS "Build the unit tests" OFF)
    if (${BUILD_TESTS})
        enable_testing()
        add_subdirectory(test)
    endif()
endif()
