# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)

set(BOARD stm32f4_disco)

# Include extra/custom modules via kconfig (libraries)
# set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../modules/moduleName")
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Name the project
project (
    smart_roomba_mcu
    DESCRIPTION "Real-time control for Smart Roomba"
    VERSION 0.1.0
    LANGUAGES C
)

target_sources(app PRIVATE src/main.c)

# Configure build type specific options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(app PRIVATE -g -O0)
    target_compile_definitions(app PRIVATE DEBUG=1) # This defines DEBUG for the code
    message(STATUS "Building DEBUG version with full logging")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(app PRIVATE -O2 -DNDEBUG)
    message(STATUS "Building RELEASE version with minimal logging")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(app PRIVATE -g -O2 -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(app PRIVATE -Os -DNDEBUG)
endif()

# Link any custom library to the application
# target_link_libraries(app PRIVATE smart_roomba_lib)