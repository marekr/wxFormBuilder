# - Find TinyXML
# Find the native TinyXML includes and library
#
#   TINYXML_FOUND        - True if TinyXML found.
#   TINYXML_INCLUDE_DIRS - where to find tinyxml.h, etc.
#   TINYXML_LIBRARIES    - List of libraries when using TinyXML.
#
#   TINYXML_VERSION       - Full version of TinyXML.
#   TINYXML_VERSION_MAJOR - Major version of TinyXML.
#   TINYXML_VERSION_MINOR - Minor version of TinyXML.
#   TINYXML_VERSION_PATCH - Patch version of TinyXML.
#

LIST( APPEND TICPP_NAMES "ticpp" "libticpp" )

FIND_PATH(
  TICPP_INCLUDE_DIRS "ticpp.h"
  )
FIND_LIBRARY(
  TICPP_LIBRARIES
  NAMES ${TICPP_NAMES}
  )
MARK_AS_ADVANCED(
  TICPP_INCLUDE_DIRS
  TICPP_LIBRARIES
  )

# handle the QUIETLY and REQUIRED arguments and set TICPP_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( "FindPackageHandleStandardArgs" )
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    "ticpp"
    REQUIRED_VARS 
    TICPP_LIBRARIES 
    TICPP_INCLUDE_DIRS
  )
