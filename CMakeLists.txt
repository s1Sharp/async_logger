cmake_minimum_required (VERSION 3.24)
project(logger VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# Set a default build type if not passed
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release")
endif()

################################
# packages
################################

include(scripts/install_deps.cmake)
find_package(JsonCpp REQUIRED)

################################
# compile files
################################

file(GLOB_RECURSE SOURCES src/*/*.cpp src/*/*.cc) 
file(GLOB_RECURSE HEADERS src/*/*.hpp src/*/*.h )

add_executable(${PROJECT_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc
  ${SOURCES}
  ${HEADERS}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src
  FILES ${SOURCES}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src
  FILES ${HEADERS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${jsoncpp_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  JsonCpp::JsonCpp
)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
set(GLOBAL_INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES})
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

