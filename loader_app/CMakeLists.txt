project(loader_app C CXX)

set(SOURCES
    src/Loader.cpp
)

find_package(fmt CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include ${fmt_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} loader fmt::fmt)
set_common_properties(${PROJECT_NAME})

