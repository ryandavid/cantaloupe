project(can-macos)
cmake_minimum_required(VERSION 3.0)

find_package(PkgConfig REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

add_compile_options(-Wall -Wextra -Wswitch-enum -Wconversion -pedantic -Werror -Wshadow)

add_subdirectory(third_party)
add_subdirectory(core)
