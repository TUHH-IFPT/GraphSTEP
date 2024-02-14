include(FetchContent)

cmake_minimum_required(VERSION 3.5)
project(GraphSTEP VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
cmake_policy(SET CMP0079 NEW)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

###############################################################################
# External dependencies

# Directory for external libraries
set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NODE_ADDON "graphstepAddon")

# STEPCODE
set( STEPCODE_ROOT_DIR ${EXTERNAL_DIR}/stepcode )
if( NOT IS_DIRECTORY ${STEPCODE_ROOT_DIR} )
    message( FATAL_ERROR "STEPcode dir does not exist" )
endif( NOT IS_DIRECTORY ${STEPCODE_ROOT_DIR} )

set( STEPCODE_BUILD_DIR ${STEPCODE_ROOT_DIR}/build )
if( NOT IS_DIRECTORY ${STEPCODE_BUILD_DIR} )
    message( FATAL_ERROR "STEPcode build dir does not exist. You need to build STEPcode first..." )
endif( NOT IS_DIRECTORY ${STEPCODE_BUILD_DIR} )

# nlohmann_json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# cpr
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG 3b15fa82ea74739b574d705fea44959b58142eb8)
FetchContent_MakeAvailable(cpr)

###############################################################################
include_directories(${CMAKE_JS_INC} 
                    ${STEPCODE_ROOT_DIR}/src/base ${STEPCODE_ROOT_DIR}/src/clstepcore
                    ${STEPCODE_ROOT_DIR}/src/cldai ${STEPCODE_ROOT_DIR}/src/clutils
                    ${STEPCODE_ROOT_DIR}/src/cleditor ${STEPCODE_BUILD_DIR}/include
                    ${STEPCODE_ROOT_DIR}/include ${CMAKE_BINARY_DIR}
)
# Add the source files needed to build the native addon
set(ADDON_SOURCES src/NodeJsWrapper.cpp)

add_subdirectory(src)

# Node addon library
add_library(${NODE_ADDON} SHARED ${ADDON_SOURCES})
set_target_properties(${NODE_ADDON} PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(${NODE_ADDON} PUBLIC src)
target_link_libraries(${NODE_ADDON} GraphSTEPLib)

# Command line interface
add_executable(${PROJECT_NAME} cli.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} GraphSTEPLib)