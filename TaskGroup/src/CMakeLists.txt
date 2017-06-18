file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
message("Main sources: ${SRCS}")

find_library(CPPREST_LIB cpprest)
find_package(Boost REQUIRED COMPONENTS random system thread filesystem chrono atomic date_time regex)
find_package(OpenSSL 1.0.0 REQUIRED)

add_executable(main ${SRCS})
target_link_libraries(main
    ${CPPREST_LIB}
    Boost::boost
    Boost::random
    Boost::system
    Boost::thread
    Boost::filesystem
    Boost::chrono
    Boost::atomic
    Boost::date_time
    Boost::regex
    OpenSSL::SSL
    gtest 
    gtest_main)