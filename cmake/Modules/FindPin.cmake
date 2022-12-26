# Original snippet: https://gist.github.com/mrexodia/f61fead0108603d04b2ca0ab045e0952

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CONTENT_URL "https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.25-98650-g8f6168173-msvc-windows.zip")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CONTENT_URL "https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.25-98650-g8f6168173-gcc-linux.tar.gz")
else()
    message(FATAL_ERROR "Cannot use PIN for '${CMAKE_SYSTEM_NAME}' platform")
endif()

message(STATUS "Setting up Intel Pin")

include(FetchContent)
FetchContent_Declare(
  IntelPIN
  URL ${CONTENT_URL}
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_GetProperties(IntelPIN)
if(NOT intelpin_POPULATED)
    FetchContent_Populate(IntelPIN)

    set(PIN_DIR "${intelpin_SOURCE_DIR}")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(TARGET_ARCH "intel64")
        set(IS_X86_64 1)
    else()
        set(TARGET_ARCH "ia32")
        set(IS_X86_64 0)
    endif()

    find_program(PIN_EXE pin PATHS ${PIN_DIR})
    add_library(IntelPIN INTERFACE)

    target_include_directories(IntelPIN SYSTEM INTERFACE
        ${PIN_DIR}/source/include/pin
        ${PIN_DIR}/source/include/pin/gen
        ${PIN_DIR}/extras/components/include
        ${PIN_DIR}/extras/xed-${TARGET_ARCH}/include/xed
    )

    target_include_directories(IntelPIN SYSTEM INTERFACE
        # ${PIN_DIR}/extras/stlport/include
        ${PIN_DIR}/extras
        # ${PIN_DIR}/extras/libstdc++/include
        ${PIN_DIR}/extras/cxx/include
        ${PIN_DIR}/extras/crt/include
        ${PIN_DIR}/extras/crt
        ${PIN_DIR}/extras/crt/include/kernel/uapi
        ${PIN_DIR}/extras/crt/include/kernel/uapi/asm-x86
        ${PIN_DIR}/extras/crt/include/arch-$<IF:${IS_X86_64},x86_64,x86>
    )

    target_compile_definitions(IntelPIN INTERFACE
        TARGET_IA32E
        HOST_IA32E
        PIN_CRT=1
    )

    find_file(PIN_CRTBEGIN_S crtbeginS
        NAMES crtbeginS.o crtbeginS.obj
        PATHS "${PIN_DIR}/${TARGET_ARCH}/runtime/pincrt"
        REQUIRED
    )

    find_file(PIN_CRTEND_S crtendS
        NAMES crtendS.o crtendS.obj
        PATHS "${PIN_DIR}/${TARGET_ARCH}/runtime/pincrt"
        REQUIRED
    )

    target_link_libraries(IntelPIN INTERFACE
        pin xed
        ${PIN_CRTBEGIN_S}
        ${PIN_CRTEND_S}
    )

    target_link_directories(IntelPIN INTERFACE
        ${PIN_DIR}/${TARGET_ARCH}/lib
        ${PIN_DIR}/${TARGET_ARCH}/lib-ext
        ${PIN_DIR}/${TARGET_ARCH}/runtime/pincrt
        ${PIN_DIR}/extras/xed-${TARGET_ARCH}/lib
    )

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(IntelPIN INTERFACE
            TARGET_WINDOWS
            _WINDOWS_H_PATH_=../um # dirty hack
            __PIN__=1
            $<IF:${IS_X86_64}, __LP64__, __i386__>
        )
        target_link_libraries(IntelPIN INTERFACE
            ntdll-$<IF:${IS_X86_64},64,32>
            kernel32
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(IntelPIN INTERFACE
            TARGET_LINUX
        )
    endif()

    if(MSVC)
        if(IS_X86_64)
            target_link_options(IntelPIN INTERFACE /BASE:0xC5000000 /ENTRY:Ptrace_DllMainCRTStartup)
        else()
            target_link_options(IntelPIN INTERFACE /BASE:0x55000000 /ENTRY:Ptrace_DllMainCRTStartup@12 /SAFESEH:NO)
        endif()
        target_link_options(IntelPIN INTERFACE /NODEFAULTLIB /EXPORT:main /IGNORE:4210 /IGNORE:4281)
        target_compile_options(IntelPIN INTERFACE /GR- /GS- /EHs- /EHa- /fp:strict /Oi- /FIinclude/msvc_compat.h /wd5208)
        target_link_libraries(IntelPIN INTERFACE pinvm pincrt)
    else()
        target_compile_options(IntelPIN INTERFACE -fPIC -fabi-version=2 -faligned-new
            -fomit-frame-pointer -fno-strict-aliasing -fno-rtti
            -Wno-unknown-pragmas -fno-stack-protector -fno-exceptions
            -funwind-tables -fasynchronous-unwind-tables
        )
        target_link_libraries(IntelPIN INTERFACE pindwarf dl-dynamic c++ c++abi m-dynamic c-dynamic unwind-dynamic)
        target_link_options(IntelPIN INTERFACE -shared -Wl,--hash-style=sysv -Wl,-Bsymbolic
            -Wl,--version-script=${PIN_DIR}/source/include/pin/pintool.ver -fabi-version=2 -nostdlib
        )
    endif()

    function(add_pintool target)
        add_library(${target} SHARED ${ARGN})
        target_link_libraries(${target} PRIVATE IntelPIN)
    endfunction()
endif()
