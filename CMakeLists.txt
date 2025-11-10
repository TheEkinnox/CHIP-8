cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(USE_FOLDERS ON)
project(CHIP-8 LANGUAGES CXX C)

set(BASE_FOLDER ${CMAKE_FOLDER})

# Setup cpp version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Setup debug defines
if (MSVC)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEBUG /D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEBUG /D_DEBUG")
else()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
endif()

################
# Dependencies #
################
set(CMAKE_FOLDER deps)
include(FetchContent)

# Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG d7a7eda959e6dff41274e06c3b2621f4e40e5963
)
FetchContent_MakeAvailable(raylib)
set(RAYLIB_INCLUDE_DIR ${raylib_SOURCE_DIR}/src)

########
# ROMs #
########

# set target
set(TARGET_NAME ROMs)

file(GLOB_RECURSE ROM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/roms/*)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ROM_FILES})

add_library(${TARGET_NAME} INTERFACE ${ROM_FILES})

message(STATUS "Added ${TARGET_NAME} resource target with files: ${ROM_FILES}")

##############
# Executable #
##############

set(CMAKE_FOLDER ${BASE_FOLDER})

# Set compile options
if (MSVC)
    add_compile_options(/W4 /WX)
    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(/Zc:preprocessor /Zc:u8EscapeEncoding)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Add source files
set(TARGET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB_RECURSE SOURCE_FILES
    ${TARGET_SOURCE_DIR}/*.c
    ${TARGET_SOURCE_DIR}/*.cc # C with classe
    ${TARGET_SOURCE_DIR}/*.cpp
    ${TARGET_SOURCE_DIR}/*.cxx
    ${TARGET_SOURCE_DIR}/*.c++
)

# Add header files
set(TARGET_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE HEADER_FILES
    ${TARGET_INCLUDE_DIR}/*.h
    ${TARGET_INCLUDE_DIR}/*.hpp
    ${TARGET_INCLUDE_DIR}/*.inl
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES} ${SOURCE_FILES})

# Setup executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${TARGET_INCLUDE_DIR} ${RAYLIB_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()