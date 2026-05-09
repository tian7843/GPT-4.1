cmake_minimum_required(VERSION 3.14)
project(GrayscaleImageViewer)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# DCMTK部分
set(DCMTK_DIR "${CMAKE_SOURCE_DIR}/third_party/DCMTK")

if(NOT EXISTS "${DCMTK_DIR}/libdcmtk.a")
    message(STATUS "DCMTK not found, initial build will compile DCMTK...")
    add_subdirectory("${DCMTK_DIR}" EXCLUDE_FROM_ALL)
endif()

include_directories(
    ${Qt5Widgets_INCLUDE_DIRS}
    "${DCMTK_DIR}/include"
)

link_directories("${DCMTK_DIR}/lib")

add_executable(GrayscaleImageViewer
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/ImageWidget.cpp
    src/ImageWidget.h
    src/DcmLoader.cpp
    src/DcmLoader.h
)

target_link_libraries(GrayscaleImageViewer
    Qt5::Widgets
    ${DCMTK_DIR}/lib/libdcmtk.a
)
