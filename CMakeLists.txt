cmake_minimum_required(VERSION 3.14)
project(SabreCal)

find_package(ROOT REQUIRED Spectrum)
set(SABRECAL_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(SABRECAL_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
