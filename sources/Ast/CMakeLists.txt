file(
	GLOB_RECURSE ASTSources
	"*.h"
	"*.cpp"
)

add_library(__ASTCore STATIC ${ASTSources})
add_library(AST::Core ALIAS __ASTCore)

target_compile_features(__ASTCore PUBLIC ${CXX_STANDARD})
set_target_properties(__ASTCore PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(__ASTCore PUBLIC ../)
target_link_libraries(__ASTCore PUBLIC Utils boost_smart_ptr boost_property_tree boost_foreach)

