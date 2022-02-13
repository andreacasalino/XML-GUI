SET(BUILD_XML_Parser_SAMPLES OFF CACHE BOOL "Override xml parser sample option" FORCE)
include(FetchContent)
FetchContent_Declare(
  xml_parser
  GIT_REPOSITORY https://github.com/andreacasalino/XML-parser.git
  GIT_TAG        89dd0d7a2e0ac49a1ae5b3feaed31e55136126d3
)
FetchContent_MakeAvailable(xml_parser)

include(FetchContent)
FetchContent_Declare(
  http_lib
  GIT_REPOSITORY https://github.com/andreacasalino/Http-GUI.git
  GIT_TAG        e5280d1d30ff5f61b2ce8a43509ce91e36f058a0
)
FetchContent_MakeAvailable(http_lib)

set(PROJECT_SHORTNAME "XML-GUI")

add_executable(${PROJECT_SHORTNAME}
XML-GUI.cpp 
src/XML-Server.h 
src/XML-Server.cpp
)

target_include_directories(${PROJECT_SHORTNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_SHORTNAME} PUBLIC
HttpGui
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

