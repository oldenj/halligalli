project(halligalli-lib)

### JSON AS TARGET ###
Include(FetchContent REQUIRED)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(lib_SOURCES
	Arc.cpp
	DataManager.cpp
	FullPricer.cpp
	KShortestPricer.cpp
	Network.cpp
	Node.cpp
	ObjPricerGFCG.cpp
	Path.cpp
	initialize.cpp
	loguru.cpp
	parse_lp.cpp
	reporting.cpp
)

add_library(halligalli STATIC ${lib_SOURCES})
target_link_libraries(halligalli stdc++fs ${SCIP_LIB} ${Boost_LIBRARIES} dl nlohmann_json::nlohmann_json)

# not needed because we do include_directories in root CMakeLists
#target_include_directories(lib
#	PUBLIC src
#)
