cmake_minimum_required(VERSION 3.5)
project(halligalli LANGUAGES CXX VERSION 1.0.1)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# CXX17 (mainly for filesystem header)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For linting etc.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable test targets
enable_testing()

### DOXYGEN TARGET ###
find_package(Doxygen)
if(DOXYGEN_FOUND)
	add_custom_target(doc COMMAND ${DOXYGEN_EXECUTABLE} "src/doxyfile"
		WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
		COMMENT "Building user's documentation into build/doxygen ..."
    )
endif()

### BOOST AS TARGET ###
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.75.0 REQUIRED COMPONENTS system thread program_options)

### CATCH2 AS TARGET ###
Include(FetchContent REQUIRED)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.1)

FetchContent_MakeAvailable(Catch2)

### SCIP AS TARGET ###
find_library(SCIP_LIB NAMES scip libscip REQUIRED)
set(SCIP_LIB ${scip} CACHE FILEPATH "Location of the scip library")

# TODO this can apparently be done per target (couldn't make it work), but this should be fine
# since headers are shared across all targets anyway
### INCLUDE PATHS ###
include_directories(src/include src/lib ${Boost_INCLUDE_DIRS})

### SOURCES ###
add_subdirectory(src/app)
add_subdirectory(src/lib)
add_subdirectory(src/test)
