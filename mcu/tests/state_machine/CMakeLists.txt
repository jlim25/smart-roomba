# tests/state_machine/CMakeLists.txt
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(roomba_fsm_tests)

# Add test source
target_sources(app PRIVATE 
    src/test_state_machine.c
)

# Include the state machine code we're testing
target_sources(app PRIVATE 
    ../../src/state_machine.c
)

# Add include directories
target_include_directories(app PRIVATE 
    ../../inc
)
