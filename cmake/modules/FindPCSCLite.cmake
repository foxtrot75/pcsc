# - Find PCSC-Lite
# Find the native PCSC-Lite includes and library
#
#  PCSCLITE_INCLUDE_DIR - where to find winscard.h, wintypes.h, etc.
#  PCSCLITE_LIBRARIES   - List of libraries when using PCSC-Lite.
#  PCSCLITE_FOUND       - True if PCSC-Lite found.

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_PCSCLITE libpcsclite)

IF(NOT PCSCLITE_FOUND)

FIND_PATH(PCSCLITE_INCLUDE_DIR
  NAMES winscard.h
  HINTS /usr/include/PCSC
        /usr/local/include/PCSC
        ${PC_PCSCLITE_INCLUDEDIR}
        ${PC_PCSCLITE_INCLUDE_DIRS}
        ${PC_PCSCLITE_INCLUDE_DIRS}/PCSC
        ${CMAKE_INSTALL_PREFIX}/include
)
FIND_LIBRARY(PCSCLITE_LIBRARY
  NAMES pcsclite libpcsclite PCSC
  HINTS ${PC_PCSCLITE_LIBDIR}
        ${PC_PCSCLITE_LIBRARY_DIRS}
        ${CMAKE_INSTALL_PREFIX}/lib
        ${CMAKE_INSTALL_PREFIX}/lib64
  PATHS /usr/local/lib
        /usr/local/lib64
        /usr/lib
        /usr/lib64
)

# handle the QUIETLY and REQUIRED arguments and set PCSCLITE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PCSCLite DEFAULT_MSG PCSCLITE_LIBRARY PCSCLITE_INCLUDE_DIR)

IF(PCSCLITE_FOUND)
  SET(PCSCLITE_LIBRARIES ${PCSCLITE_LIBRARY})
ELSE(PCSCLITE_FOUND)
  SET(PCSCLITE_LIBRARIES )
ENDIF(PCSCLITE_FOUND)

MARK_AS_ADVANCED(PCSCLITE_LIBRARY PCSCLITE_INCLUDE_DIR)
ENDIF(NOT PCSCLITE_FOUND)
