project(loader_app C CXX)

set(SOURCES
    src/Loader.cpp
    src/LoaderCLI.cpp
)

set(HEADERS
    include/LoaderCLI.h
)

find_package(fmt CONFIG REQUIRED)
find_package(tabulate CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE include ${fmt_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} loader fmt::fmt tabulate::tabulate)
set_common_properties(${PROJECT_NAME})

