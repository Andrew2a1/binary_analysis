cmake_minimum_required(VERSION 3.20)
project(binary_analysis C CXX)

include(CTest)
enable_testing()

include(cmake/common.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CONAN_TOOLCHAIN_FILE ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

if(EXISTS ${CONAN_TOOLCHAIN_FILE})
    include(${CONAN_TOOLCHAIN_FILE})
endif()

add_subdirectory(loader)
add_subdirectory(loader_app)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
