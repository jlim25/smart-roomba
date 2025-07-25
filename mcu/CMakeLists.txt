# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)

set(BOARD stm32f4_disco)

# =============================================================================
# AUTOMATIC OVERLAY SELECTION
# =============================================================================

# Select configuration overlay based on build type
if(NOT DEFINED OVERLAY_CONFIG)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(OVERLAY_CONFIG "conf/release.conf")
        message(STATUS "🚀 RELEASE BUILD: Using release.conf overlay")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(OVERLAY_CONFIG "conf/debug.conf")  
        message(STATUS "🐛 DEBUG BUILD: Using debug.conf overlay")
    else()
        # Default to debug if build type not specified or unknown
        set(OVERLAY_CONFIG "conf/debug.conf")
        message(STATUS "❓ UNKNOWN BUILD TYPE '${CMAKE_BUILD_TYPE}': Defaulting to debug.conf")
    endif()
    
    message(STATUS "Configuration files:")
    message(STATUS "  Base: prj.conf")
    message(STATUS "  Overlay: ${OVERLAY_CONFIG}")
else()
    message(STATUS "📝 MANUAL OVERRIDE: Using overlay ${OVERLAY_CONFIG}")
endif()

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

# Add your C source files here
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# =============================================================================
# APPLICATION-LEVEL DEBUG MACROS
# =============================================================================

# Define application-level DEBUG macro (separate from Zephyr's CONFIG_DEBUG)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(app PRIVATE DEBUG=1)
    message(STATUS "✅ Application DEBUG macro enabled")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(app PRIVATE NDEBUG=1)
    message(STATUS "✅ Application NDEBUG macro enabled")
endif()

# Generate a Unix timestamp for the build
string(TIMESTAMP BUILD_TIMESTAMP_UNIX "%s")
target_compile_definitions(app PRIVATE 
    BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    BUILD_TIMESTAMP="${BUILD_TIMESTAMP_UNIX}"
)