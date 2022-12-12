project(binfo C CXX)

set(SOURCES
    src/main.cpp
    src/LoaderCLI.cpp
)

set(HEADERS
    include/LoaderCLI.h
)

find_package(fmt CONFIG REQUIRED)
find_package(tabulate CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} loader fmt::fmt tabulate::tabulate CLI11::CLI11)
set_common_properties(${PROJECT_NAME})

