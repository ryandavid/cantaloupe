project(can-macos)
cmake_minimum_required(VERSION 3.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

include_directories(
    include
    ${LIBUSB_INCLUDE_DIRS}
)

link_directories(${LIBUSB_LIBRARY_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(can-macos
    src/can-macos.cpp
)

target_link_libraries(can-macos
    ${LIBUSB_LIBRARIES}
)
