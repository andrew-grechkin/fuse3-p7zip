# Find the FUSE3 includes and library
#
#  FUSE3_INCLUDE_DIR - where to find fuse.h, etc.
#  FUSE3_LIBRARIES   - List of libraries when using FUSE3.
#  FUSE3_FOUND       - True if FUSE3 lib is found.

# check if already in cache, be silent
if (FUSE3_INCLUDE_DIR)
    SET (FUSE3_FIND_QUIETLY TRUE)
endif (FUSE3_INCLUDE_DIR)

set(FUSE3_NAMES fuse3)
set(FUSE3_SUFFIXES fuse3)

# find includes
find_path (FUSE3_INCLUDE_DIR fuse.h
    PATHS /opt /opt/local /usr/pkg
    PATH_SUFFIXES ${FUSE3_SUFFIXES}
)

# find lib
find_library (FUSE3_LIBRARIES NAMES ${FUSE3_NAMES})

include ("FindPackageHandleStandardArgs")
find_package_handle_standard_args (
    "FUSE3" DEFAULT_MSG FUSE3_INCLUDE_DIR FUSE3_LIBRARIES
)

mark_as_advanced (FUSE3_INCLUDE_DIR FUSE3_LIBRARIES)
