project(loader C CXX)

set(SOURCES
    src/Binary.cpp
    src/BFDLoader.cpp
    src/Symbol.cpp
)

set(HEADERS
    include/Symbol.h
    include/Section.h
    include/Binary.h
    include/BinaryLoader.h
    include/BFDLoader.h
)

set(LOADERAPP
    test/LoadSelf.cpp
)

find_package(LibBfd)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE ${LIBBFD_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LIBBFD_LIBRARIES})
set_common_properties(${PROJECT_NAME})
