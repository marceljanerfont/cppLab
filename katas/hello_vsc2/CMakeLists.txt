cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(main VERSION 0.1.0 LANGUAGES CXX)
add_compile_options(-std=c++11)

if (WIN32)
    set(APIS_DIRS C:/apis)
else (WIN32)
    set(APIS_DIRS /apis)
endif (WIN32)

message("************************: ${APIS_DIRS}")

# gtest
include_directories(${APIS_DIRS}/gtest-1.8.0/include)
link_directories(${APIS_DIRS}/gtest-1.8.0/lib)

set(LIBS ${LIBS} gtest gtest_main)

add_subdirectory(src)
