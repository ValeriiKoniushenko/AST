file(
	GLOB_RECURSE ASTSources
	"*.h"
	"*.cpp"
)

add_library(ASTCore STATIC ${ASTSources})
add_library(AST::Core ALIAS ASTCore)

target_compile_features(ASTCore PUBLIC ${CXX_STANDARD})
set_target_properties(ASTCore PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(ASTCore PUBLIC ../)
target_link_libraries(ASTCore PUBLIC Utils boost_smart_ptr boost_property_tree boost_foreach RapidXml spdlog)

include(CheckCXXCompilerFlag)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	check_cxx_compiler_flag("-g" COMPILER_SUPPORTS_OG)
	if(COMPILER_SUPPORTS_OG)
		target_compile_options(ASTCore PRIVATE -O3)
	endif()
else()
	check_cxx_compiler_flag("-O3" COMPILER_SUPPORTS_O3)
	if(COMPILER_SUPPORTS_O3)
		target_compile_options(ASTCore PRIVATE -g)
	endif()
endif()

