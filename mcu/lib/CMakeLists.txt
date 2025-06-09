# Create the library target
add_library(smart_roomba_lib STATIC src/lib_motor.c)

# Add the library's header file(s)
target_include_directories(
    smart_roomba_lib 
    PUBLIC   
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# Required to set Zephyr compile options (e.g. for ARM)
target_link_libraries(hello_lib PRIVATE zephyr_interface)