SET(BUILD_XML_Parser_SAMPLES OFF CACHE BOOL "Override xml parser sample option" FORCE)
include(FetchContent)
FetchContent_Declare(
  xml_parser
  GIT_REPOSITORY https://github.com/andreacasalino/XML-parser.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(xml_parser)

include(FetchContent)
FetchContent_Declare(
  http_lib
  GIT_REPOSITORY https://github.com/andreacasalino/Http-GUI.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(http_lib)

add_executable(XML-GUI 
XML-GUI.cpp 
src/OptionalString.h 
src/OptionalString.cpp 
src/XML-model.h 
src/XML-model.cpp
)

target_include_directories(XML-GUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(XML-GUI PUBLIC
Http-GUI
XML-Parser
)

if(WIN32)
	add_custom_command(TARGET XML-GUI POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:XML-GUI> ${CMAKE_SOURCE_DIR}/App/XML-GUI.exe
	)
else()
	add_custom_command(TARGET XML-GUI POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:XML-GUI> ${CMAKE_SOURCE_DIR}/App/XML-GUI
	)
endif()

target_compile_definitions(XML-GUI 
PUBLIC
  -D EXAMPLE_FOLDER="${FETCHCONTENT_BASE_DIR}/xml_parser-src/Examples/"
)
