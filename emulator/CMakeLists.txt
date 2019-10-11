cmake_minimum_required (VERSION 3.12)

project ("Super Smash Bros. AI")

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif ()

###############################################################################
# Build all mupen libs
###############################################################################

add_subdirectory ("mupen64plus-core")
add_subdirectory ("mupen64plus-input-ai-cuck")
add_subdirectory ("mupen64plus-video-glide64mk2")
add_subdirectory ("m64pai")

function (build_mupen_component NAME)
    set (ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}")
    file (GLOB_RECURSE SOURCES "${ROOT_DIR}/src/*.c" "${ROOT_DIR}/src/*.cpp")
    file (GLOB_RECURSE HEADERS "${ROOT_DIR}/src/*.h" "${ROOT_DIR}/src/*.hpp")

    if (NAME MATCHES "mupen64plus-core")
        set (LIBNAME "${CMAKE_SHARED_LIBRARY_PREFIX}mupen64plus${CMAKE_SHARED_LIBRARY_SUFFIX}.2.0.0")
    elseif (NAME MATCHES "mupen64plus-rsp-cxd4")
        set (LIBNAME "${NAME}-sse2${CMAKE_SHARED_LIBRARY_SUFFIX}")
    else ()
        set (LIBNAME "${NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif ()

    if (UNIX)
        set (OUTPUT_LIB_FILE "${ROOT_DIR}/projects/unix/${LIBNAME}")
        set (FINAL_LIB_FILE ${CMAKE_BINARY_DIR}/m64pai/${LIBNAME})
        add_custom_command (OUTPUT ${FINAL_LIB_FILE}
            COMMAND make ARGS all -j DEBUGGER=1 DBG=1
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OUTPUT_LIB_FILE} ${FINAL_LIB_FILE}
            WORKING_DIRECTORY "${ROOT_DIR}/projects/unix"
            DEPENDS ${SOURCES} ${HEADERS}
            COMMENT "Building ${NAME} library"
            VERBATIM)
        add_custom_target (${NAME} DEPENDS ${FINAL_LIB_FILE})
    else ()
        message (FATAL_ERROR "Windows is currently not supported")
    endif ()
endfunction ()

set (MUPEN_COMPONENTS
    #mupen64plus-core
    #mupen64plus-ai
    mupen64plus-audio-sdl
    mupen64plus-input-sdl
    #mupen64plus-rsp-cxd4
    #mupen64plus-video-glide64
    mupen64plus-video-glide64mk2
    #mupen64plus-video-rice
)

foreach (COMPONENT ${MUPEN_COMPONENTS})
    message (STATUS ${COMPONENT})
    build_mupen_component (${COMPONENT})
endforeach ()

