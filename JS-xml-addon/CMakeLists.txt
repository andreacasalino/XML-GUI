cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0042 NEW)
set (CMAKE_CXX_STANDARD 17)

project (xml-addon)

set(LIB_OPT ON)
# set macro-directory and find scripts
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/XML-parser/cmake/")
# include cmake custom functions
set(WITH_SOURCE_TREE ON)
include(GroupSources)
include(AutoCollect)
include(Log)
include(SetInstallRPath)
add_subdirectory(XML-parser/Lib)

include_directories(${CMAKE_JS_INC})
file(GLOB SOURCE_FILES "src/xmlAddon.cc" "src/OptionalString.cpp" "src/JsonArrayStream.cpp")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} 
			${CMAKE_JS_LIB}
			XMLPrs::XML-Parser
		     )

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REGEX REPLACE "[\r\n\"]" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})

# define NPI_VERSION
add_definitions(-DNAPI_VERSION=3)
