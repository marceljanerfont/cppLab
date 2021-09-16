file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
message("Main sources: ${SRCS}")
add_executable(main ${SRCS})