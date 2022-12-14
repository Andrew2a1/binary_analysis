project(bdis C CXX)

set(SOURCES
    src/main.cpp
    src/CapstoneWrapper.cpp
    src/DisasmUtils.cpp
    src/LinearDisasm.cpp
    src/RecursiveDisasm.cpp
)

set(HEADERS
    include/CapstoneWrapper.h
    include/Disasm.h
    include/DisasmUtils.h
    include/LinearDisasm.h
    include/RecursiveDisasm.h
)

find_package(fmt CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(capstone CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE loader fmt::fmt CLI11::CLI11 capstone::capstone)
set_common_properties(${PROJECT_NAME})
