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
