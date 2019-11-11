include (TestBigEndian)

project ("mupen64plus-video-glide64mk2"
    LANGUAGES C
    VERSION 1.0.0)

option (M64P_HIRES "Enables/Disables support for hires textures and texture filters" ON)
option (M64P_TXCDXTN_EXT "Enable/Disable external txc_dxtn library" OFF)
option (M64P_USE_GLES "" OFF)

cmake_host_system_information (RESULT IS_HOST_64BIT QUERY IS_64BIT)
cmake_host_system_information (RESULT HOST_HAS_MMX QUERY HAS_MMX)
cmake_host_system_information (RESULT HOST_HAS_SSE QUERY HAS_SSE)
test_big_endian (M64P_BIG_ENDIAN)

require_zlib()
find_package (PNG REQUIRED)
find_package (OpenGL COMPONENTS OpenGL REQUIRED)
require_sdl2()
if (M64P_HIRES)
    find_package (Boost COMPONENTS system filesystem)
    if (NOT Boost_FOUND)
        message (WARNING "Boost not found: M64P_HIRES (high resolution texture support) will be disabled")
        set (M64P_HIRES OFF)
    endif ()
endif ()

set (GNU_CFLAGS -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -fvisibility=hidden -ffast-math -fno-strict-aliasing)
set (GNU_CXXFLAGS -fvisibility-inlines-hidden)

if (HOST_HAS_MMX)
    list (APPEND GNU_CFLAGS -mmmx)
endif ()
if (HOST_HAS_SSE)
    list (APPEND GNU_CFLAGS -msse)
endif ()

add_library (mupen64plus-video-glide64mk2 SHARED
    "src/Glide64/3dmath.cpp"
    "src/Glide64/Combine.cpp"
    "src/Glide64/Config.cpp"
    "src/Glide64/CRC.cpp"
    "src/Glide64/Debugger.cpp"
    "src/Glide64/DepthBufferRender.cpp"
    "src/Glide64/FBtoScreen.cpp"
    "src/Glide64/Ini.cpp"
    "src/Glide64/Keys.cpp"
    "src/Glide64/Main.cpp"
    "src/Glide64/rdp.cpp"
    "src/Glide64/TexBuffer.cpp"
    "src/Glide64/TexCache.cpp"
    "src/Glide64/Util.cpp"
    $<$<BOOL:${M64P_USE_GLES}>:src/Glitch64/OGLEScombiner.cpp>
    $<$<BOOL:${M64P_USE_GLES}>:src/Glitch64/OGLESgeometry.cpp>
    $<$<BOOL:${M64P_USE_GLES}>:src/Glitch64/OGLESglitchmain.cpp>
    $<$<BOOL:${M64P_USE_GLES}>:src/Glitch64/OGLEStextures.cpp>
    $<$<NOT:$<BOOL:${M64P_USE_GLES}>>:src/Glitch64/OGLcombiner.cpp>
    $<$<NOT:$<BOOL:${M64P_USE_GLES}>>:src/Glitch64/OGLgeometry.cpp>
    $<$<NOT:$<BOOL:${M64P_USE_GLES}>>:src/Glitch64/OGLglitchmain.cpp>
    $<$<NOT:$<BOOL:${M64P_USE_GLES}>>:src/Glitch64/OGLtextures.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/Ext_TxFilter.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TxFilterExport.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TxFilter.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TxCache.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TxTexCache.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TxHiResCache.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TxQuantize.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TxUtil.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TextureFilters.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TextureFilters_2xsai.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TextureFilters_hq2x.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TextureFilters_hq4x.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TxImage.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TxReSample.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/TxDbg.cpp>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/tc-1.1+/fxt1.c>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/tc-1.1+/wrapper.c>
    $<$<BOOL:${M64P_HIRES}>:src/GlideHQ/tc-1.1+/texstore.c>
    $<$<NOT:$<BOOL:${M64P_TXCDXTN_EXT}>>:src/GlideHQ/tc-1.1+/s2tc/s2tc_algorithm.cpp>
    $<$<NOT:$<BOOL:${M64P_TXCDXTN_EXT}>>:src/GlideHQ/tc-1.1+/s2tc/s2tc_libtxc_dxtn.cpp>
    $<$<PLATFORM_ID:Linux>:src/Glide64/osal_dynamiclib_unix.c>
    $<$<PLATFORM_ID:Darwin>:src/Glide64/osal_dynamiclib_unix.c>
    $<$<PLATFORM_ID:Windows>:src/Glide64/osal_dynamiclib_win32.c>)
set_property (
    TARGET mupen64plus-video-glide64mk2
    PROPERTY POSITION_INDEPENDENT_CODE ${IS_HOST_64BIT})
target_include_directories (mupen64plus-video-glide64mk2
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/Glitch64/inc)
target_compile_definitions (mupen64plus-video-glide64mk2
    PRIVATE $<$<BOOL:${M64P_BIG_ENDIAN}>:M64P_BIG_ENDIAN>
    PRIVATE $<$<BOOL:${M64P_HIRES}>:DUMP_CACHE>
    PRIVATE $<$<BOOL:${M64P_HIRES}>:TEXTURE_FILTER>
    PRIVATE $<$<C_COMPILER_ID:GNU>:GCC>
    PRIVATE $<$<C_COMPILER_ID:Clang>:GCC>)
target_compile_options (mupen64plus-video-glide64mk2
    PRIVATE $<$<C_COMPILER_ID:GNU>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:$<$<COMPILE_LANGUAGE:CXX>:${GNU_CXXFLAGS}>>
    PRIVATE $<$<C_COMPILER_ID:Clang>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:$<$<COMPILE_LANGUAGE:CXX>:${GNU_CXXFLAGS}>>)
set_target_properties (mupen64plus-video-glide64mk2 PROPERTIES
    NO_SONAME 1
    PREFIX ""
    OUTPUT_NAME mupen64plus-video-glide64mk2
    LIBRARY_OUTPUT_DIRECTORY ${M64PAI_BINARY_PREFIX})
target_link_libraries (mupen64plus-video-glide64mk2
    PRIVATE mupen64plus-core-api
    PRIVATE ZLIB::ZLIB
    PRIVATE PNG::PNG
    PRIVATE OpenGL::GL
    PRIVATE SDL2::SDL2
    PRIVATE $<$<BOOL:${M64P_HIRES}>:Boost::system>
    PRIVATE $<$<BOOL:${M64P_HIRES}>:Boost::filesystem>)

if (WIN32)
    install (
        TARGETS mupen64plus-video-glide64mk2
        EXPORT m64paiTargets
        RUNTIME DESTINATION ${M64PAI_INSTALL_PREFIX})
else ()
    install (
        TARGETS mupen64plus-video-glide64mk2
        EXPORT m64paiTargets
        LIBRARY DESTINATION ${M64PAI_INSTALL_PREFIX})
endif ()
