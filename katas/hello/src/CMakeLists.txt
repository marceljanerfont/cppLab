file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
message("Main sources: ${SRCS}")
add_executable(main ${SRCS})
target_link_libraries(main gtest gtest_main)