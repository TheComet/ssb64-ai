project ("mupen64plus-input-ai-cuck"
    LANGUAGES C
    VERSION 1.0.0)

set (GNU_CFLAGS -Wall -Wextra -Werror)

add_library (mupen64plus-input-ai-cuck SHARED
    "src/plugin.c"
    $<$<PLATFORM_ID:Linux>:src/osal_dynamiclib_unix.c>
    $<$<PLATFORM_ID:Windows>:src/osal_dynamiclib_win32.c>)
set_property (TARGET mupen64plus-input-ai-cuck PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories (mupen64plus-input-ai-cuck
    PRIVATE ${MUPEN64PLUS_CORE_INCLUDE_DIR}
    PRIVATE $<$<COMPILE_LANGUAGE:C>:${CMAKE_CURRENT_LIST_DIR}/src>)
target_compile_options (mupen64plus-input-ai-cuck
    PRIVATE $<$<C_COMPILER_ID:GNU>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>
    PRIVATE $<$<C_COMPILER_ID:Clang>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>)
set_target_properties (mupen64plus-input-ai-cuck PROPERTIES
    NO_SONAME 1
    PREFIX ""
    OUTPUT_NAME mupen64plus-input-ai-cuck
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries (mupen64plus-input-ai-cuck
    PUBLIC mupen64plus-core-api)

if (WIN32)
    install (
        TARGETS mupen64plus-input-ai-cuck
        EXPORT m64paiTargets
        RUNTIME DESTINATION ${M64PAI_INSTALL_PREFIX})
else ()
    install (
        TARGETS mupen64plus-input-ai-cuck
        EXPORT m64paiTargets
        LIBRARY DESTINATION ${M64PAI_INSTALL_PREFIX})
endif ()
