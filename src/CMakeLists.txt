add_subdirectory(Http-GUI/Lib)
add_subdirectory(XML-parser/Lib)

add_executable(XML-GUI 
XML-GUI.cpp 
src/OptionalString.h 
src/OptionalString.cpp 
src/XML-model.h 
src/XML-model.cpp
)

target_include_directories(XML-GUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(XML-GUI PUBLIC
gui::Http-GUI
XMLPrs::XML-Parser
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

