cmake_minimum_required (VERSION 3.13)

project ("Super Smash Bros. AI")

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif ()

list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include (GNUInstallDirs)

set (M64PAI_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/m64pai)
set (M64PAI_INSTALL_LIBDIR ${M64PAI_INSTALL_PREFIX}/lib)
set (M64PAI_INSTALL_BINDIR ${M64PAI_INSTALL_PREFIX}/bin)
set (M64PAI_INSTALL_INCLUDEDIR ${M64PAI_INSTALL_PREFIX}/include)
set (M64PAI_INSTALL_SHAREDIR ${M64PAI_INSTALL_PREFIX}/share/m64pai)

set (M64PAI_BINARY_PREFIX ${CMAKE_BINARY_DIR}/m64pai)
set (M64PAI_BINARY_LIBDIR ${M64PAI_BINARY_PREFIX}/lib)
set (M64PAI_BINARY_BINDIR ${M64PAI_BINARY_PREFIX}/bin)
set (M64PAI_BINARY_INCLUDEDIR ${M64PAI_BINARY_PREFIX}/include)
set (M64PAI_BINARY_SHAREDIR ${M64PAI_BINARY_PREFIX}/share/m64pai)

set (ZLIB_SOURCE "${CMAKE_CURRENT_LIST_DIR}/zlib-1.2.11")
set (ZLIB_BINARY "${CMAKE_CURRENT_BINARY_DIR}/zlib-1.2.11")
set (SDL2_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/SDL2-2.0.10")
set (SDL2_BINARY "${CMAKE_CURRENT_BINARY_DIR}/SDL2-2.0.10")
set (FT_SOURCE "${CMAKE_CURRENT_LIST_DIR}/freetype-2.10.1")
set (FT_BINARY "${CMAKE_CURRENT_BINARY_DIR}/freetype-2.10.1")

option (USE_SYSTEM_ZLIB "" OFF)
option (USE_SYSTEM_SDL2 "" OFF)

function (require_zlib)
    if (USE_SYSTEM_ZLIB)
        find_package (ZLIB)
    endif ()
    if (NOT TARGET ZLIB::ZLIB)
        add_subdirectory (${ZLIB_SOURCE} ${ZLIB_BINARY})
        add_library (ZLIB::ZLIB ALIAS zlib)
    endif ()
endfunction ()

function (require_sdl2)
    if (USE_SYSTEM_SDL2)
        find_package (SDL2)
        if (${SDL2_FOUND})
            add_library (SDL2::SDL2 INTERFACE IMPORTED GLOBAL)
            set_target_properties (SDL2::SDL2 PROPERTIES 
                INTERFACE_INCLUDE_DIRECTORIES ${SDL2_INCLUDE_DIRS}
                INTERFACE_LINK_LIBRARIES ${SDL2_LIBRARIES})
        endif ()
    endif ()
    if (NOT TARGET SDL2::SDL2)
        add_subdirectory (${SDL2_SOURCE} ${SDL2_BINARY})
        add_library (SDL2::SDL2 ALIAS SDL2)
    endif ()
endfunction ()

add_subdirectory ("mupen64plus-core")
add_subdirectory ("mupen64plus-audio-sdl")
add_subdirectory ("mupen64plus-input-sdl")
add_subdirectory ("mupen64plus-input-ai-cuck")
add_subdirectory ("mupen64plus-rsp-hle")
add_subdirectory ("mupen64plus-video-glide64mk2")
add_subdirectory ("m64pai")

install (
    EXPORT m64paiTargets
    FILE m64pai.cmake
    NAMESPACE m64pai::
    DESTINATION ${M64PAI_INSTALL_SHAREDIR})
