cmake_minimum_required(VERSION 3.20)
project(binary_analysis C CXX)

include(CTest)
enable_testing()

include(cmake/common.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

add_subdirectory(loader)
add_subdirectory(pintools)
add_subdirectory(apps)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
