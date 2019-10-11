include (TestBigEndian)

project ("mupen64plus-core"
    LANGUAGES C CXX  # ASM_NASM
    VERSION 2.0.0)

set (M64P_SHAREDDIR "" CACHE STRING "extra path to search for shared data files")
option (M64P_32BIT "Force building 32-bit binaries on a 64-bit machine" OFF)
option (M64P_PIC "Generate position independent code. NOTE: Is forced off when compiling for 32-bit" ON)
option (M64P_LIRC "enable LIRC support" OFF)
option (M64P_NO_ASM "build without assembly (no dynamic recompiler or MMX/SSE code)" OFF)
option (M64P_USE_GLES "build against GLESv2 instead of OpenGL" OFF)
option (M64P_VC "build against Broadcom Videocore GLESv2" OFF)
option (M64P_NEON "(ARM only) build for hard floating point environments" OFF)
option (M64P_VFP_HARD "(ARM only) full hardware floating point ABI" OFF)
option (M64P_OSD "Enable/disable build of OpenGL On-screen display" OFF)
option (M64P_NEW_DYNAREC "Replace dynamic recompiler with Ari64's experimental dynarec" OFF)
option (M64P_OPENCV "Enable OpenCV support" OFF)
option (M64P_PARALLEL "Use threads" ON)
option (M64P_PROFILE "build gprof instrumentation into binaries for profiling" OFF)
option (M64P_DEBUGGER "build debugger API into core for front-ends.  runs slower." ON)
option (M64P_DBG_CORE "print debugging info in r4300 core" OFF)
option (M64P_DBG_COUNT "print R4300 instruction count totals (64-bit dynarec only)" OFF)
option (M64P_DBG_COMPARE "enable core-synchronized r4300 debugging" OFF)
option (M64P_DBG_TIMING "print timing data" OFF)
option (M64P_DBG_PROFILE "dump profiling data for r4300 dynarec to data file" OFF)
option (M64P_AI "Enable AI modifications from the SSB64-AI project" ON)

cmake_host_system_information (RESULT HOST_IS_64BIT QUERY IS_64BIT)
test_big_endian (M64P_BIG_ENDIAN)

find_package (ZLIB REQUIRED)
find_package (PNG REQUIRED)
find_package (SDL2 REQUIRED)
find_package (Freetype REQUIRED)
if (M64P_OPENCV)
    find_package (OpenCV COMPONENTS core REQUIRED)
endif ()

# TODO
if (M64P_VC OR M64P_USE_GLES OR M64P_NEW_DYNAREC)
    message (FATAL_ERROR "Not implemented in build system yet")
endif ()

# Platform settings
if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
    if (MSVC)
        set (ASM_DIALECT ASM_MASM)
    elseif (MINGW)
        set (ASM_DIALECT ASM_NASM)
    else ()
        message (FATAL_ERROR "Unsupported compiler")
    endif ()
elseif (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Darwin")
    set (ASM_DIALECT ASM_NASM)
elseif (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    set (ASM_DIALECT ASM_NASM)
    
    # m64p debugger uses dis-asm.h and a few other tools
    if (M64P_DEBUGGER)
        find_path (DISASM_INCLUDE_DIR dis-asm.h)
        if (NOT DISASM_INCLUDE_DIR)
            message (FATAL_ERROR "Please install the development package of binutils (on Ubuntu that's binutils-dev)")
        endif ()
    endif ()
elseif (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Android")
    set (ASM_DIALECT ASM_NASM)
elseif (${CMAKE_HOST_SYSTEM_NAME} MATCHES "FreeBSD")
    set (ASM_DIALECT ASM_NASM)
else ()
    message (FATAL_ERROR "Unsupported host system")
endif ()

enable_language(${ASM_DIALECT})

set (GNU_CFLAGS -Wall -ffast-math -fno-strict-aliasing -fvisibility=hidden -flto)
set (GNU_CXXFLAGS -fvisibility-inlines-hidden)
set (GNU_ASFLAGS)
set (GNU_LDFLAGS)

# tweak flags for 32-bit build on 64-bit system
if (HOST_IS_64BIT)
    if (M64P_32BIT)
        list (APPEND GNU_CFLAGS -m32)
        list (APPEND GNU_LDFLAGS -Wl,-m,elf_i386)
    endif ()
endif ()
if (M64P_PROFILE)
    list (APPEND GNU_CFLAGS -pg -g)
endif ()
if (M64P_DEBUGGER)
    list (APPEND GNU_LDFLAGS -lopcodes -lbfd)
endif ()
if (NOT HOST_IS_64BIT OR M64P_32BIT)
    set (M64P_PIC OFF)
endif ()

# Determine arch of dynamic recompiler
if (NOT M64P_NO_ASM)
    if (HOST_IS_64BIT)
        set (DYNAREC_ARCH "x86_64")
        list (APPEND GNU_LDFLAGS -Wl,-version-script,${CMAKE_CURRENT_LIST_DIR}/src/api/api_export.ver)
    else ()
        set (DYNAREC_ARCH "x86")
    endif ()
endif ()
set (DYNAREC_SOURCES
    "src/device/r4300/recomp.c"
    "src/device/r4300/${DYNAREC_ARCH}/assemble.c"
    "src/device/r4300/${DYNAREC_ARCH}/dynarec.c"
    "src/device/r4300/${DYNAREC_ARCH}/regcache.c"
    "src/device/r4300/${DYNAREC_ARCH}/dyna_start.asm")

# minizip lib
add_library (minizip STATIC
    "subprojects/minizip/ioapi.c"
    "subprojects/minizip/zip.c"
    "subprojects/minizip/unzip.c")
target_include_directories (minizip
    PUBLIC "subprojects/minizip")
target_compile_definitions (minizip
    PRIVATE NOCRYPT NOUNCRYPT)

# md5 hash lib
add_library (md5lib STATIC
    "subprojects/md5/md5.c")
target_include_directories (md5lib
    PUBLIC "subprojects/md5")

## xxhash
add_library (xxhash STATIC
    "subprojects/xxhash/xxhash.c")
target_include_directories (xxhash
    PUBLIC "subprojects/xxhash")

## oglft
add_library (oglft STATIC
    "subprojects/oglft/OGLFT.cpp")
target_include_directories (oglft
    PUBLIC "subprojects/OGLFT")
target_link_libraries (oglft
    PRIVATE
        ${SDL2_LIBRARIES}
        Freetype::Freetype)

add_library (mupen64plus-core SHARED
    "src/api/ai.c"
    "src/api/callbacks.c"
    "src/api/common.c"
    "src/api/config.c"
    "src/api/debugger.c"
    "src/api/frontend.c"
    "src/api/vidext.c"
    "src/backends/api/video_capture_backend.c"
    "src/backends/plugins_compat/audio_plugin_compat.c"
    "src/backends/plugins_compat/input_plugin_compat.c"
    "src/backends/clock_ctime_plus_delta.c"
    "src/backends/dummy_video_capture.c"
    "src/backends/file_storage.c"
    "src/device/cart/cart.c"
    "src/device/cart/af_rtc.c"
    "src/device/cart/cart_rom.c"
    "src/device/cart/eeprom.c"
    "src/device/cart/flashram.c"
    "src/device/cart/sram.c"
    "src/device/controllers/game_controller.c"
    "src/device/controllers/paks/biopak.c"
    "src/device/controllers/paks/mempak.c"
    "src/device/controllers/paks/rumblepak.c"
    "src/device/controllers/paks/transferpak.c"
    "src/device/dd/dd_controller.c"
    "src/device/device.c"
    "src/device/gb/gb_cart.c"
    "src/device/gb/mbc3_rtc.c"
    "src/device/gb/m64282fp.c"
    "src/device/memory/memory.c"
    "src/device/pif/bootrom_hle.c"
    "src/device/pif/cic.c"
    "src/device/pif/n64_cic_nus_6105.c"
    "src/device/pif/pif.c"
    "src/device/r4300/cached_interp.c"
    "src/device/r4300/cp0.c"
    "src/device/r4300/cp1.c"
    "src/device/r4300/idec.c"
    "src/device/r4300/interrupt.c"
    "src/device/r4300/pure_interp.c"
    "src/device/r4300/r4300_core.c"
    "src/device/r4300/tlb.c"
    "src/device/rcp/ai/ai_controller.c"
    "src/device/rcp/mi/mi_controller.c"
    "src/device/rcp/pi/pi_controller.c"
    "src/device/rcp/rdp/fb.c"
    "src/device/rcp/rdp/rdp_core.c"
    "src/device/rcp/ri/ri_controller.c"
    "src/device/rcp/rsp/rsp_core.c"
    "src/device/rcp/si/si_controller.c"
    "src/device/rcp/vi/vi_controller.c"
    "src/device/rdram/rdram.c"
    "src/main/main.c"
    "src/main/util.c"
    "src/main/cheat.c"
    "src/main/eventloop.c"
    "src/main/rom.c"
    "src/main/savestates.c"
    "src/main/screenshot.c"
    "src/main/sdl_key_converter.c"
    "src/main/workqueue.c"
    "src/plugin/plugin.c"
    "src/plugin/dummy_video.c"
    "src/plugin/dummy_audio.c"
    "src/plugin/dummy_input.c"
    "src/plugin/dummy_rsp.c"
    $<$<PLATFORM_ID:Linux>:src/osal/dynamiclib_unix.c>
    $<$<PLATFORM_ID:Linux>:src/osal/files_unix.c>
    $<$<NOT:$<BOOL:${M64P_NO_ASM}>>:${DYNAREC_SOURCES}>
    $<$<BOOL:${M64P_DEBUGGER}>:src/debugger/dbg_debugger.c>
    $<$<BOOL:${M64P_DEBUGGER}>:src/debugger/dbg_decoder.c>
    $<$<BOOL:${M64P_DEBUGGER}>:src/debugger/dbg_memory.c>
    $<$<BOOL:${M64P_DEBUGGER}>:src/debugger/dbg_breakpoints.c>
    $<$<BOOL:${M64P_DBG_COUNT}>:src/device/r4300/instr_counters.c>
    $<$<BOOL:${M64P_DBG_PROFILE}>:src/main/profile.c>)

set_property (TARGET mupen64plus-core PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories (mupen64plus-core
    PRIVATE $<$<COMPILE_LANGUAGE:C>:${CMAKE_CURRENT_LIST_DIR}/src>
    PRIVATE $<$<COMPILE_LANGUAGE:ASM_NASM>:${CMAKE_CURRENT_LIST_DIR}/src/asm_defines>
    PRIVATE $<$<COMPILE_LANGUAGE:ASM_MASM>:${CMAKE_CURRENT_LIST_DIR}/src/asm_defines>)
target_compile_options (mupen64plus-core
    PRIVATE $<$<C_COMPILER_ID:GNU>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>
    PRIVATE $<$<C_COMPILER_ID:GNU>:$<$<COMPILE_LANGUAGE:CXX>:${GNU_CXXFLAGS}>>
    PRIVATE $<$<C_COMPILER_ID:Clang>:$<$<COMPILE_LANGUAGE:C>:${GNU_CFLAGS}>>
    PRIVATE $<$<C_COMPILER_ID:Clang>:$<$<COMPILE_LANGUAGE:CXX>:${GNU_CXXFLAGS}>>)
target_link_options (mupen64plus-core
    PRIVATE $<$<C_COMPILER_ID:GNU>:${GNU_LDFLAGS}>)
target_compile_definitions (mupen64plus-core
    PRIVATE M64P_CORE_PROTOTYPES
    PRIVATE $<$<BOOL:${M64P_BIG_ENDIAN}>:M64P_BIG_ENDIAN>
    PRIVATE $<$<BOOL:${M64P_OSD}>:M64P_OSD>
    PRIVATE $<$<BOOL:${M64P_NO_ASM}>:NO_ASM>
    PRIVATE $<$<NOT:$<BOOL:${M64P_NO_ASM}>>:DYNAREC>
    PRIVATE $<$<BOOL:${M64P_LIRC}>:WITH_LIRC>
    PRIVATE $<$<BOOL:${M64P_PARALLEL}>:M64P_PARALLEL>
    PRIVATE $<$<BOOL:${M64P_DEBUGGER}>:DBG>
    PRIVATE $<$<BOOL:${M64P_DEBUGGER}>:USE_LIBOPCODES_GE_2_29> # assumes binutils >=2.29
    PRIVATE $<$<BOOL:${M64P_DBG_COMPARE}>:COMPARE_CORE>
    PRIVATE $<$<BOOL:${M64P_DBG_COUNT}>:COUNT_INSTR>
    PRIVATE $<$<BOOL:${M64P_DBG_CORE}>:CORE_DBG>
    PRIVATE $<$<BOOL:${M64P_DBG_TIMING}>:PROFILE>
    PRIVATE $<$<BOOL:${M64P_AI}>:M64P_AI>)
set_target_properties (mupen64plus-core PROPERTIES
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME mupen64plus
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/m64pai")
target_link_libraries (mupen64plus-core
    PRIVATE
        minizip
        md5lib
        oglft
        xxhash
        PNG::PNG
        ${SDL2_LIBRARIES}
        ZLIB::ZLIB
        $<$<BOOL:${M64P_OPENCV}>:${OpenCV_LIBRARIES}>
        $<$<BOOL:${M64P_OSD}>:Freetype::Freetype>
        $<$<BOOL:${M64P_DBG_TIMING}>:rt>)

add_library(mupen64plus-core-api INTERFACE IMPORTED GLOBAL)
set_target_properties(mupen64plus-core-api PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/src/api")
