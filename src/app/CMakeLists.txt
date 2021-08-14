project(halligalli-app)

set(app_SOURCES
	halligalli.cpp
)

add_executable(solver ${app_SOURCES})

target_link_libraries(solver PRIVATE halligalli)

# this would be for per target include directories
# but we just do include_directories from root CMakeLists
#target_include_directories(app PRIVATE lib)
