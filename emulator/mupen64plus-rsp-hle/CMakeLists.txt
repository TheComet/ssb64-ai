include (TestBigEndian)

project ("mupen64plus-rsp-hle"
    LANGUAGES C
    VERSION 1.0.0)

set (GNU_CFLAGS -Wall -Wextra -Werror -Wno-unused-parameter -fvisibility=hidden -ffast-math -fno-strict-aliasing)

cmake_host_system_information (RESULT IS_HOST_64BIT QUERY IS_64BIT)
test_big_endian (M64P_BIG_ENDIAN)

add_library (mupen64plus-rsp-hle SHARED
    "src/alist.c"
    "src/alist_audio.c"
    "src/alist_naudio.c"
    "src/alist_nead.c"
    "src/audio.c"
    "src/cicx105.c"
    "src/hle.c"
    "src/jpeg.c"
    "src/memory.c"
    "src/mp3.c"
    "src/musyx.c"
    "src/re2.c"
    "src/plugin.c"
    $<$<PLATFORM_ID:Linux>:src/osal_dynamiclib_unix.c>
    $<$<PLATFORM_ID:Windows>:src/osal_dynamiclib_win32.c>)

set_property (
    TARGET mupen64plus-rsp-hle
    PROPERTY POSITION_INDEPENDENT_CODE ${IS_HOST_64BIT})
target_include_directories (mupen64plus-rsp-hle
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_compile_definitions (mupen64plus-rsp-hle
    PRIVATE $<$<BOOL:${M64P_BIG_ENDIAN}>:M64P_BIG_ENDIAN>)
target_compile_options (mupen64plus-rsp-hle
    PRIVATE $<$<C_COMPILER_ID:GNU>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>
    PRIVATE $<$<C_COMPILER_ID:Clang>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>)
set_target_properties (mupen64plus-rsp-hle PROPERTIES
    NO_SONAME 1
    PREFIX ""
    OUTPUT_NAME mupen64plus-rsp-hle
    LIBRARY_OUTPUT_DIRECTORY ${M64PAI_BINARY_PREFIX})
target_link_libraries (mupen64plus-rsp-hle
    PUBLIC mupen64plus-core-api)

if (WIN32)
    install (
        TARGETS mupen64plus-rsp-hle
        EXPORT m64paiTargets
        RUNTIME DESTINATION ${M64PAI_INSTALL_PREFIX})
else ()
    install (
        TARGETS mupen64plus-rsp-hle
        EXPORT m64paiTargets
        LIBRARY DESTINATION ${M64PAI_INSTALL_PREFIX})
endif ()
