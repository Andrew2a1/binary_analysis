project(binject C CXX)

set(SOURCES
    src/main.cpp
)

find_package(libelf CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} libelf::libelf)
set_common_properties(${PROJECT_NAME})
