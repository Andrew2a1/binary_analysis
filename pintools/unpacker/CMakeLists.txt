project(unpacker C CXX)

set(SOURCES
    src/main.cpp
)

set(HEADERS
)

find_package(Pin)
add_pintool(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE pinutils)
set_common_properties(${PROJECT_NAME})

add_test(NAME ${PROJECT_NAME}_test COMMAND ${PIN_EXE} -t ${CMAKE_BINARY_DIR}/lib/lib${PROJECT_NAME}.so -- /bin/true)
