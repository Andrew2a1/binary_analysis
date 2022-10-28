macro(setup_conan)
    set(CONAN_TOOLCHAIN_FILE ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

    execute_process(COMMAND conan install ${CMAKE_SOURCE_DIR} -pr:b=default -s build_type=${CMAKE_BUILD_TYPE} --build=missing
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND_ECHO STDOUT
    )

    include(${CONAN_TOOLCHAIN_FILE})
endmacro()