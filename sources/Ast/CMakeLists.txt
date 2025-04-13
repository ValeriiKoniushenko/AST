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
