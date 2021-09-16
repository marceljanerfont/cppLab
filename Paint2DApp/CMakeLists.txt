cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(TARGET_NAME Paint2DApp)

project(${TARGET_NAME} VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Turn on automatic invocation of the MOC, UIC & RCC
# if CMAKE_AUTOMOC is OFF then we get error LNK2001: unresolved external symbol QMetaObject Widget::metaObject
set(CMAKE_AUTOMOC ON) 
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)

# Find the QtWidgets library SET QT PATH !!!!!
set(QT_DIR C:/Qt/Qt5.12.11/5.12.11/msvc2017_64)
set(CMAKE_PREFIX_PATH  ${QT_DIR}/lib/cmake)
message(STATUS "QT_DIR: ${QT_DIR}")
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

# opencv SET OPENCV PATH !!!!!
set(OpenCV_DIR C:/APIS/opencv-4.1.2-cuda-11.3)
message(STATUS "OpenCV: ${OpenCV_DIR}")
find_package(OpenCV REQUIRED HINTS ${OpenCV_DIR}/x64/vc16 NO_DEFAULT_PATH)
MESSAGE(STATUS "LINK LIBRARIES ${OpenCV_LIBS}")

###############################################
# Find sources
file(GLOB_RECURSE SRCS *.cpp *.h)
message(STATUS "SRCS: ${SRCS}")
add_executable(${TARGET_NAME} ${SRCS})
###############################################

# Add the Qt5 Widgets for linking
target_link_libraries(${TARGET_NAME} Qt5::Core Qt5::Widgets Qt5::Gui)

# opencv
target_include_directories(${TARGET_NAME} PUBLIC ${OpenCV_DIR}/include)
target_link_directories(${TARGET_NAME} PUBLIC ${OpenCV_DIR}/x64/vc16/lib)
target_link_libraries(${TARGET_NAME} debug opencv_core412d optimized opencv_core412
                                     debug opencv_imgproc412d.lib optimized opencv_imgproc412.lib)

# copy QT dll
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #DEBUG
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QT_DIR}/bin/Qt5Widgetsd.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QT_DIR}/bin/Qt5Guid.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QT_DIR}/bin/Qt5Cored.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    # RELEASE
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QT_DIR}/bin/Qt5Widgets.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QT_DIR}/bin/Qt5Gui.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QT_DIR}/bin/Qt5Core.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
)

