#file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

file(GLOB_RECURSE SRCS *.cpp *.h)
message("Main sources: ${SRCS}")


# boost
set(BOOST_ROOT C:/APIS/boost_1_72_0)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${BOOST_ROOT})
#link_directories(${BOOST_ROOT}/lib64-msvc-14.2)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${BOOST_ROOT}/lib64-msvc-14.2)
set(Boost_USE_STATIC_LIBS OFF)
add_definitions(-DBOOST_ALL_DYN_LINK)
find_package(Boost 1.72.0 REQUIRED COMPONENTS date_time regex)

# gtest
set(GTEST_ROOT C:/APIS/gtest-1.8.1)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${GTEST_ROOT}/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${GTEST_ROOT}/lib/x64)
find_package(GTest REQUIRED)

message("Include paths: ${CMAKE_INCLUDE_PATH}")
message("Library paths: ${CMAKE_LIBRARY_PATH}")

#set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${BOOST_ROOT}/lib64-msvc-14.2)
#find_package(Boost 1.72.0 REQUIRED COMPONENTS date_time)

add_executable(main ${SRCS})
target_link_libraries(main
	Boost::date_time
	Boost::regex
    GTest::gtest 
    GTest::gtest_main)