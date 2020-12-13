cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0042 NEW)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Avoid linux warnings: Cannot create package registry file
set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY ON)


option(DEBUG "Debug cmake build" ON)
option(DEBUG_LOG "Application Debug Logging" ON)

# only windows specific stuff
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(DEBUG_LOG)
add_definitions(
    # This will set the spd preprocessor log level to trace
    -DSPDLOG_ACTIVE_LEVEL=0
    # Activate the preprocessor debug and trace logging calls
    -DSPDLOG_DEBUG_ON
    -DSPDLOG_TRACE_ON
)
endif()

# allow creation of custom file sinks
add_definitions(-DSPDLOG_NO_FINAL)

if(NOT WIN32)
    # add compiler flags
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas" CACHE STRING "" FORCE)
endif()

# set macro-directory and find scripts
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/XML-parser/cmake/")
# include cmake custom functions
set(WITH_SOURCE_TREE ON)
include(GroupSources)
include(AutoCollect)
include(Log)
include(SetInstallRPath)

project (addon-xml)
add_subdirectory(XML-parser/Lib)

include_directories(${CMAKE_JS_INC})
file(GLOB SOURCE_FILES "xmlAddon.cc")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} 
			${CMAKE_JS_LIB}
			XMLPrs::XML-Parser)

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REGEX REPLACE "[\r\n\"]" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})

# define NPI_VERSION
add_definitions(-DNAPI_VERSION=3)
