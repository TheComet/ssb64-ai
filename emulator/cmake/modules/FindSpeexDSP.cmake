# Use pkg-config to find library locations in *NIX environments.
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_search_module(PC_SPEEXDSP QUIET speexdsp)
endif()

# Find the include directory.
find_path(SPEEXDSP_INCLUDE_DIR "speexdsp.h"
    HINTS ${PC_SPEEXDSP_INCLUDE_DIRS})

# Find the version.  I don't know if there is a correct way to find this on
# Windows - the config.h in the tarball is wrong for 0.1.19.
if(PC_SPEEXDSP_VERSION)
    set(SPEEXDSP_VERSION "${PC_SPEEXDSP_VERSION}")
endif()

# Find the library.
find_library(SPEEXDSP_LIBRARY "speexdsp"
    HINTS ${PC_SPEEXDSP_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SpeexDSP
    FOUND_VAR SPEEXDSP_FOUND
    REQUIRED_VARS SPEEXDSP_INCLUDE_DIR SPEEXDSP_LIBRARY
    VERSION_VAR SPEEXDSP_VERSION
)

if(SPEEXDSP_FOUND)
    # Imported target.
    add_library(SpeexDSP::SpeexDSP UNKNOWN IMPORTED)
    set_target_properties(SpeexDSP::SpeexDSP PROPERTIES
        INTERFACE_COMPILE_OPTIONS "${PC_SPEEXDSP_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${SPEEXDSP_INCLUDE_DIR}"
        IMPORTED_LOCATION "${SPEEXDSP_LIBRARY}")
endif()
