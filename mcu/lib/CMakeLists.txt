# Create the library target
FILE(GLOB lib_sources src/*.c)
add_library(smart_roomba_lib STATIC ${lib_sources})

# Add the library's header file(s)
target_include_directories(
    smart_roomba_lib 
    PUBLIC   
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# Required to set Zephyr compile options (e.g. for ARM)
target_link_libraries(smart_roomba_lib PRIVATE zephyr_interface)