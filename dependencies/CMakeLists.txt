cmake_minimum_required(VERSION 3.30.0..)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(
	GLOB Sources
	"rapidxml/*.hpp"
)

add_library(RapidXml INTERFACE ${Sources})
target_compile_features(RapidXml INTERFACE ${CXX_STANDARD})
target_include_directories(RapidXml INTERFACE rapidxml/)
set_target_properties(RapidXml PROPERTIES LINKER_LANGUAGE CXX)