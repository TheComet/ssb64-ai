project ("mupen64plus-audio-sdl"
    LANGUAGES C
    VERSION 2.0.0)

set (GNU_CFLAGS -Wall -Wextra -Werror -Wno-unused-parameter -Wno-sign-compare -fvisibility=hidden -ffast-math -fno-strict-aliasing)

cmake_host_system_information (RESULT IS_HOST_64BIT QUERY IS_64BIT)

require_sdl2()
find_package (SpeexDSP)
find_package (Samplerate)

add_library (mupen64plus-audio-sdl SHARED
    "src/circular_buffer.c"
    "src/main.c"
    "src/sdl_backend.c"
    "src/volume.c"
    "src/resamplers/resamplers.c"
    "src/resamplers/trivial.c"
    $<$<PLATFORM_ID:Linux>:src/osal_dynamiclib_unix.c>
    $<$<PLATFORM_ID:Darwin>:src/osal_dynamiclib_unix.c>
    $<$<PLATFORM_ID:Windows>:src/osal_dynamiclib_win32.c>
    $<$<BOOL:${SPEEXDSP_FOUND}>:src/resamplers/speex.c>
    $<$<BOOL:${SAMPLERATE_FOUND}>:src/resamplers/src.c>)

set_property (
    TARGET mupen64plus-audio-sdl
    PROPERTY POSITION_INDEPENDENT_CODE ${IS_HOST_64BIT})
target_include_directories (mupen64plus-audio-sdl
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_compile_definitions (mupen64plus-audio-sdl
    PRIVATE $<$<BOOL:${SPEEXDSP_FOUND}>:USE_SPEEX>
    PRIVATE $<$<BOOL:${SAMPLERATE_FOUND}>:USE_SRC>)
target_compile_options (mupen64plus-audio-sdl
    PRIVATE $<$<C_COMPILER_ID:GNU>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>
    PRIVATE $<$<C_COMPILER_ID:Clang>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>)
set_target_properties (mupen64plus-audio-sdl PROPERTIES
    NO_SONAME 1
    PREFIX ""
    OUTPUT_NAME mupen64plus-audio-sdl
    LIBRARY_OUTPUT_DIRECTORY ${M64PAI_BINARY_PREFIX})
target_link_libraries (mupen64plus-audio-sdl
    PRIVATE mupen64plus-core-api
    PRIVATE $<$<BOOL:${SPEEXDSP_FOUND}>:SpeexDSP::SpeexDSP>
    PRIVATE $<$<BOOL:${SAMPLERATE_FOUND}>:Samplerate::Samplerate>
    PRIVATE SDL2::SDL2)

if (WIN32)
    install (
        TARGETS mupen64plus-audio-sdl
        EXPORT m64paiTargets
        RUNTIME DESTINATION ${M64PAI_INSTALL_PREFIX})
else ()
    install (
        TARGETS mupen64plus-audio-sdl
        EXPORT m64paiTargets
        LIBRARY DESTINATION ${M64PAI_INSTALL_PREFIX})
endif ()
