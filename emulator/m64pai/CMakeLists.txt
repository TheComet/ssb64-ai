project ("m64pai"
    LANGUAGES C
    VERSION 1.0.0)

find_package (Python COMPONENTS Development REQUIRED)

set (M64PAI_TYPES
    Controller
    Emulator
    Fighter
    CaptainFalcon
    DonkeyKong
    Fox
    Jigglypuff
    Kirby
    Link
    Luigi
    Mario
    Ness
    Pikachu
    Samus
    Yoshi
    Plugin
    CuckedInputPlugin
    SSB64
    Stage
    DreamLand
    HyruleCastle
    KongoJungle
    MushroomKingdom
    PeachsCastle
    PlanetZebes
    SaffronCity
    SectorZ
    YoshisIsland)

set (EXAMPLE_SCRIPTS
    run_game.py)

string (REGEX REPLACE ";?([A-Za-z0-9]+)" "    X(\\1) \\\\\n" M64PAI_TYPES_X_MACROS "${M64PAI_TYPES}")
string (REGEX REPLACE ";?([A-Za-z0-9]+)" "from .m64pai import \\1\n" M64PAI_TYPES_PYTHON_INIT "${M64PAI_TYPES}")
configure_file ("templates/m64pai_types_list.h.in" "m64pai_types_list.h")
configure_file ("templates/__init__.py.in" "${CMAKE_BINARY_DIR}/m64pai/__init__.py")

add_library (m64pai SHARED
    "src/m64pai_module.c"
    "src/m64pai_ssb64_memory.c"
    "src/m64pai_type_Controller.c"
    "src/m64pai_type_Emulator.c"
    "src/m64pai_type_Fighter.c"
    "src/m64pai_type_Fighter_Pikachu.c"
    "src/m64pai_type_Fighter_Mario.c"
    "src/m64pai_type_Fighter_Luigi.c"
    "src/m64pai_type_Fighter_Fox.c"
    "src/m64pai_type_Fighter_DonkeyKong.c"
    "src/m64pai_type_Fighter_Samus.c"
    "src/m64pai_type_Fighter_Link.c"
    "src/m64pai_type_Fighter_Yoshi.c"
    "src/m64pai_type_Fighter_CaptainFalcon.c"
    "src/m64pai_type_Fighter_Kirby.c"
    "src/m64pai_type_Fighter_Jigglypuff.c"
    "src/m64pai_type_Fighter_Ness.c"
    "src/m64pai_type_Plugin.c"
    "src/m64pai_type_Plugin_CuckedInputPlugin.c"
    "src/m64pai_type_SSB64.c"
    "src/m64pai_type_Stage.c"
    "src/m64pai_type_Stage_DreamLand.c"
    "src/m64pai_type_Stage_PeachsCastle.c"
    "src/m64pai_type_Stage_SectorZ.c"
    "src/m64pai_type_Stage_KongoJungle.c"
    "src/m64pai_type_Stage_PlanetZebes.c"
    "src/m64pai_type_Stage_HyruleCastle.c"
    "src/m64pai_type_Stage_YoshisIsland.c"
    "src/m64pai_type_Stage_SaffronCity.c"
    "src/m64pai_type_Stage_MushroomKingdom.c"
    $<$<PLATFORM_ID:Linux>:src/osal_dynamiclib_unix.c>
    $<$<PLATFORM_ID:Windows>:src/osal_dynamiclib_win32.c>)
target_include_directories (m64pai
    PRIVATE
        "include"
        ${CMAKE_CURRENT_BINARY_DIR}
        ${MUPEN64PLUS_CORE_INCLUDE_DIR})
set_target_properties (m64pai PROPERTIES
    PREFIX ""
    DBG_POSTFIX ""
    OUTPUT_NAME "m64pai"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/m64pai")
target_link_libraries (m64pai
    PRIVATE mupen64plus-core-api
    PRIVATE Python::Python)

add_custom_command (
    OUTPUT ${CMAKE_BINARY_DIR}/m64pai/share/m64pai/data
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/m64pai/share/m64pai
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/m64pai/share/m64pai/data
    COMMENT "Creating data symlink in build tree"
    VERBATIM)
add_custom_target (m64pai-share-dir DEPENDS ${CMAKE_BINARY_DIR}/m64pai/share/m64pai/data)
add_dependencies (m64pai m64pai-share-dir)

foreach (SCRIPT ${EXAMPLE_SCRIPTS})
    add_custom_command (
        OUTPUT ${CMAKE_BINARY_DIR}/${SCRIPT}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/python/${SCRIPT}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/python/${SCRIPT} ${CMAKE_BINARY_DIR}/${SCRIPT}
        COMMENT "Copying example script ${SCRIPT} to ${CMAKE_BINARY_DIR}"
        VERBATIM)
    add_custom_target (m64pai-script-${SCRIPT} ALL
        DEPENDS ${CMAKE_BINARY_DIR}/${SCRIPT})
endforeach ()

if (WIN32)
    install (
        TARGETS m64pai
        EXPORT m64paiTargets
        RUNTIME DESTINATION ${M64PAI_INSTALL_PREFIX})
else ()
    install (
        TARGETS m64pai
        EXPORT m64paiTargets
        LIBRARY DESTINATION ${M64PAI_INSTALL_PREFIX})
endif ()

install (
    FILES ${CMAKE_BINARY_DIR}/m64pai/__init__.py
    DESTINATION ${M64PAI_INSTALL_PREFIX})

install (
    DIRECTORY "data"
    DESTINATION ${M64PAI_INSTALL_SHAREDIR})
