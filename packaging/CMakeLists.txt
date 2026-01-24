include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if (NOT DEFINED boulderdash_INSTALL_CMAKEDIR)
   set(boulderdash_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/boulderdash"
       CACHE STRING "Path to boulderdash CMake files")
endif ()

install(TARGETS boulderdash EXPORT boulderdash_Targets
        RUNTIME COMPONENT boulderdash_Runtime
        LIBRARY COMPONENT boulderdash_Runtime
        NAMELINK_COMPONENT boulderdash_Development
        ARCHIVE COMPONENT boulderdash_Development
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(DIRECTORY "${boulderdash_SOURCE_DIR}/include/" 
        TYPE INCLUDE
        COMPONENT boulderdash_Development)

if (BUILD_SHARED_LIBS)
    set(type shared)
    message(STATUS "Building boulderdash shared lib")
else ()
    set(type static)
    message(STATUS "Building boulderdash static lib")
endif ()

install(EXPORT boulderdash_Targets
        DESTINATION "${boulderdash_INSTALL_CMAKEDIR}"
        NAMESPACE boulderdash::
        FILE boulderdash-${type}-targets.cmake
        COMPONENT boulderdash_Development)

write_basic_package_version_file(
    boulderdashConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/boulderdashConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/boulderdashConfigVersion.cmake"
        DESTINATION "${boulderdash_INSTALL_CMAKEDIR}"
        COMPONENT boulderdash_Development)
