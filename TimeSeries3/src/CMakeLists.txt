#file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

file(GLOB_RECURSE SRCS *.cpp *.h *.hpp)
message("Main sources: ${SRCS}")


# boost
set(BOOST_ROOT C:/APIS/boost_1_80_0/include)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${BOOST_ROOT})

# gtest
set(GTEST_ROOT C:/APIS/gtest-1.8.1)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${GTEST_ROOT}/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${GTEST_ROOT}/lib/x64)
find_package(GTest REQUIRED)


include_directories(${CMAKE_INCLUDE_PATH})
message("Include paths: ${CMAKE_INCLUDE_PATH}")
message("Library paths: ${CMAKE_LIBRARY_PATH}")


add_executable(main ${SRCS})
target_link_libraries(main
    GTest::gtest 
    GTest::gtest_main)