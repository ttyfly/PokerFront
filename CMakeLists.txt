cmake_minimum_required(VERSION 3.21)
project(poker_front VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/SDL)

# Create your game executable target as usual
add_executable(poker_front sources/main.cpp)

# Link to the actual SDL3 library.
target_link_libraries(poker_front PRIVATE SDL3::SDL3)

if(WIN32)
    add_custom_command(
        TARGET poker_front POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:poker_front>
        VERBATIM
    )
endif()
