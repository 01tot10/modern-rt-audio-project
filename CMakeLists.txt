cmake_minimum_required(VERSION 3.1)
project(RTNeural VERSION 1.0.0)
include(cmake/CXXStandard.cmake)

add_subdirectory(RTNeural)

include(cmake/ChooseBackend.cmake)

option(BUILD_PROJECTS "Build projects" OFF)
if(BUILD_PROJECTS)
    message(STATUS "Configuring projects...")
    add_subdirectory(projects)
endif()
