get_filename_component(MAIN_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TARGET_NAME "__Ast_${MAIN_NAME}")

set(SOURCES RapidXMLSTD.cpp)
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE CXX)

file(GLOB HEADERS "*.hpp")

add_library(${TARGET_NAME} STATIC ${SOURCES} ${HEADERS})
add_library(Ast::${MAIN_NAME} ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME} PUBLIC .)
