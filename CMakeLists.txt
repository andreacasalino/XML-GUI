cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(XML-Gui-Prj)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set macro-directory and find scripts
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

# include cmake custom functions
set(WITH_SOURCE_TREE ON)
include(GroupSources)
include(AutoCollect)
include(MakeLibrary)

add_subdirectory(src)
