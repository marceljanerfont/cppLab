cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(main VERSION 0.1.0 LANGUAGES CXX)

#linux: add_compile_options(-std=c++17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")

macro(get_WIN32_WINNT version)
    if (WIN32 AND CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(REPLACE "." "" ver ${ver})
        string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

        set(${version} "0x${ver}")
    endif()
endmacro()

get_WIN32_WINNT(ver)
message("Windows version: ${ver}")
add_definitions(-D_WIN32_WINNT=${ver})

add_subdirectory(src)
