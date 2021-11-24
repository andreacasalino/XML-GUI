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

set(PROJECT_SHORTNAME "XML-GUI")

add_executable(${PROJECT_SHORTNAME}
XML-GUI.cpp 
src/OptionalString.h 
src/OptionalString.cpp 
src/XML-model.h 
src/XML-model.cpp
)

target_include_directories(${PROJECT_SHORTNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_SHORTNAME} PUBLIC
Http-GUI
XML-Parser
)

target_compile_definitions(${PROJECT_SHORTNAME} 
PUBLIC
  -D EXAMPLE_FOLDER="${FETCHCONTENT_BASE_DIR}/xml_parser-src/Examples/"
)

install(TARGETS ${PROJECT_SHORTNAME})
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/JS/ DESTINATION bin/JS)
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/img/ DESTINATION bin/img)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/XML-GUI.html DESTINATION bin/)

