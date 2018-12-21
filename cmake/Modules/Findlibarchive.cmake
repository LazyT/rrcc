# - Try to find libarchive
# Once done this will define
#
#  LIBARCHIVE_FOUND - system has libarchive
#  LIBARCHIVE_INCLUDE_DIR - the libarchive include directory
#  LIBARCHIVE_LIBRARIES - Link these to use libarchive
#  LIBARCHIVE_DEFINITIONS - Compiler switches required for using libarchive
#
#=============================================================================
#  Copyright (c) 2018 Andreas Schneider <asn@cryptomilk.org>
#
#  Distributed under the OSI-approved BSD License (the "License");
#  see accompanying file Copyright.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even the
#  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the License for more information.
#=============================================================================
#

if (UNIX)
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(_LIBARCHIVE libarchive)
  endif (PKG_CONFIG_FOUND)
endif (UNIX)

find_path(LIBARCHIVE_INCLUDE_DIR
    NAMES
        archive.h
    PATHS
        ${_LIBARCHIVE_INCLUDEDIR}
)

find_library(ARCHIVE_LIBRARY
    NAMES
        archive
    PATHS
        ${_LIBARCHIVE_LIBDIR}
)

if (ARCHIVE_LIBRARY)
    set(LIBARCHIVE_LIBRARIES
        ${LIBARCHIVE_LIBRARIES}
        ${ARCHIVE_LIBRARY}
    )
endif (ARCHIVE_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libarchive DEFAULT_MSG LIBARCHIVE_LIBRARIES LIBARCHIVE_INCLUDE_DIR)

# show the LIBARCHIVE_INCLUDE_DIR and LIBARCHIVE_LIBRARIES variables only in the advanced view
mark_as_advanced(LIBARCHIVE_INCLUDE_DIR LIBARCHIVE_LIBRARIES)

