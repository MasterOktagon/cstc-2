
cmake_minimum_required(VERSION 3.5)
project( cstc-cpp )

set(ExecutableName "cstc")

set (CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Debug mode if required

add_compile_options(-W -Wall -Wextra -Wno-literal-suffix -Wno-unused-parameter)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building in Debug mode...")
    add_compile_options(-DDEBUG_ENABLED -g)
else()
    message(STATUS "Building in Release mode...")
endif()

# add files in src folder

file(GLOB_RECURSE SRC "src/*")

# Main exectuable

add_executable ( ${ExecutableName} ${SRC})

# check for segvcatch lib
if (IS_DIRECTORY "lib/segvcatch/lib/")
    message(STATUS "optional dependency segvcatch found")
    include_directories("lib/segvcatch/lib/")
    target_link_libraries(${ExecutableName} PRIVATE segvcatch)
    target_include_directories(${ExecutableName} PRIVATE ${SEGVCATCH_INCLUDE_DIRS})
else()
    message(STATUS "optional dependency segvcatch wasn't found")
endif()

# Tests

set(TestName "${ExecutableName}-tests")
find_package(Catch2 2.13)
if (${Catch2_FOUND})
    # for tests no main file
    list(REMOVE_ITEM SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_executable(${TestName} ${SRC})
    target_link_libraries(${TestName} PRIVATE Catch2::Catch2WithMain)

    # run tests

    add_custom_command(TARGET ${TestName}
        POST_BUILD
        COMMENT "Running tests..."
        COMMAND $<TARGET_FILE:${TestName}>
    )
    
else()
    message(WARNING "Testing disabled - Catch2 not found!")
endif()

