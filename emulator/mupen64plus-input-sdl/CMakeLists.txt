project ("mupen64plus-input-sdl"
    LANGUAGES C
    VERSION 2.5.9)

set (GNU_CFLAGS -Wall -Wextra -Werror -Wno-sign-compare -Wno-unused-parameter -ffast-math -fno-strict-aliasing -fvisibility=hidden)

find_package (SDL2 REQUIRED)

cmake_host_system_information (RESULT IS_HOST_64BIT QUERY IS_64BIT)

add_library (mupen64plus-input-sdl SHARED
    "src/autoconfig.c"
    "src/config.c"
    "src/plugin.c"
    "src/sdl_key_converter.c"
    $<$<PLATFORM_ID:Linux>:src/osal_dynamiclib_unix.c>
    $<$<PLATFORM_ID:Windows>:src/osal_dynamiclib_win32.c>)
set_property (
    TARGET mupen64plus-input-sdl
    PROPERTY POSITION_INDEPENDENT_CODE ${IS_HOST_64BIT})
target_include_directories (mupen64plus-input-sdl
    PRIVATE $<$<COMPILE_LANGUAGE:C>:${SDL2_INCLUDE_DIRS}>
    PRIVATE $<$<COMPILE_LANGUAGE:C>:${CMAKE_CURRENT_LIST_DIR}/src>)
target_compile_options (mupen64plus-input-sdl
    PRIVATE $<$<C_COMPILER_ID:GNU>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>
    PRIVATE $<$<C_COMPILER_ID:Clang>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>)
target_compile_definitions (mupen64plus-input-sdl
    PRIVATE _GNU_SOURCE)
set_target_properties (mupen64plus-input-sdl PROPERTIES
    NO_SONAME 1
    PREFIX ""
    OUTPUT_NAME mupen64plus-input-sdl
    LIBRARY_OUTPUT_DIRECTORY ${M64PAI_BINARY_PREFIX})
target_link_libraries (mupen64plus-input-sdl
    PUBLIC mupen64plus-core-api
    PRIVATE ${SDL2_LIBRARIES})

if (WIN32)
    install (
        TARGETS mupen64plus-input-sdl
        EXPORT m64paiTargets
        RUNTIME DESTINATION ${M64PAI_INSTALL_PREFIX})
else ()
    install (
        TARGETS mupen64plus-input-sdl
        EXPORT m64paiTargets
        LIBRARY DESTINATION ${M64PAI_INSTALL_PREFIX})
endif ()
