
cmake_minimum_required(VERSION 3.5)
project( cstc-cpp )

set (CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-W -Wall -Wextra -Wno-literal-suffix -g -Wno-unused-parameter)
add_compile_options(-DDEBUG_ON)

file(GLOB_RECURSE SRC "src/*")
include_directories("lib/segvcatch/lib/")

add_executable (cstc ${SRC})
# for later use
#find_package( LLVM REQUIRED )
#target_link_libraries(cstc PRIVATE ${LLVM_LIBRARIES})
#target_include_directories(cstc PRIVATE ${LLVM_INCLUDE_DIRS})
#target_link_directories(cstc PRIVATE ${LLVM_LIBRARY_DIRS})

target_link_libraries(cstc PRIVATE segvcatch)
target_include_directories(cstc PRIVATE ${SEGVCATCH_INCLUDE_DIRS})

