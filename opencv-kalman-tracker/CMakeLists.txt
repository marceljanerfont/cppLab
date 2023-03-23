cmake_minimum_required(VERSION 2.8)

project(simple-opencv-kalman-tracker)

set(CMAKE_BUILD_TYPE Debug)

MESSAGE(STATUS "HELLO")

# opencv SET OPENCV PATH !!!!!
set(OpenCV_DIR C:/APIS/opencv-4.7.0)
message(STATUS "OpenCV: ${OpenCV_DIR}")
find_package(OpenCV REQUIRED HINTS ${OpenCV_DIR}/x64/vc16 NO_DEFAULT_PATH)
MESSAGE(STATUS "LINK LIBRARIES ${OpenCV_LIBS}")



set(SRC_PATH source )

set(${PROJECT_NAME}_SRC
	${SRC_PATH}/kalman_filter.h
	${SRC_PATH}/kalman_filter.cpp
	${SRC_PATH}/main.cpp
)


#########################################################
# Executable
add_executable( ${PROJECT_NAME} ${${PROJECT_NAME}_SRC} )
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )
