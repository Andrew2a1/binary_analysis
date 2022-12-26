project(binject C CXX)

set(SOURCES
    src/main.cpp
    src/ElfFile.cpp
    src/ElfInject.cpp
)

set(HEADERS
    include/ElfFile.h
    include/InjectInfo.h
    include/ElfInject.h
)

find_package(libelf CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE libelf::libelf CLI11::CLI11)
set_common_properties(${PROJECT_NAME})
