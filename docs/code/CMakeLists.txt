cmake_minimum_required(VERSION 3.16)
project(uvbook)

add_compile_options(-fPIC)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE "Debug" )
link_directories(/usr/lib /usr/local/lib)

set(NODE_INCLUDE_PATH "/usr/local/include/node")

include_directories(${NODE_INCLUDE_PATH})
link_libraries(uv)

include_directories(../../src)

add_executable(helloworld helloworld/main.c)
add_executable(idle-basic idle-basic/main.c)
add_executable(uvcat uvcat/main.c)
IF (WIN32)

ENDIF ()

