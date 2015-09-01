# - Build config of TinyXML++
# Defines several variables needed for automatic
# download and build of source archive.
#

# We need "TiXmlNode::TINYXML_ELEMENT" which is present
# in TinyXML 2.6.0 and later.
FIND_PACKAGE( "ticpp" )
SET( PKG_FOUND "${TICPP_FOUND}" )

IF( PKG_FOUND )
  RETURN()
ENDIF( PKG_FOUND )

# Cache entries
SET( TICPP_URL
  "https://github.com/rjpcomputing/ticpp/archive/8c7e691e56f9a0e2ad483b01dcd3e64b1b3de276.zip"
  CACHE STRING "URL of the TinyXML++ source archive" )
MARK_AS_ADVANCED( TICPP_URL )


# Setup the needed variables
SET( PKG_URL "${TICPP_URL}" )
SET( PKG_MD5 "878d4fd98d8aff9882fc54ea8d11b2d5" )
SET( PKG_ARCHIVE_PREFIX "/ticpp-8c7e691e56f9a0e2ad483b01dcd3e64b1b3de276" )
SET( PKG_EXTRACT_PREFIX "/ticpp-8c7e691e56f9a0e2ad483b01dcd3e64b1b3de276" )

# Enable TICPP sillyness
ADD_DEFINITIONS( -DTIXML_USE_TICPP )
SET( PKG_CMAKELISTS "
IF( TIXML_USE_STL )
  ADD_DEFINITIONS( \"-DTIXML_USE_STL\" )
ENDIF( TIXML_USE_STL )

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

INCLUDE_DIRECTORIES(
  \"\${CMAKE_CURRENT_SOURCE_DIR}\"
  )
ADD_LIBRARY(
  \"ticpp\" STATIC
  # Source:
  \"tinystr.h\"
  \"tinystr.cpp\"
  \"tinyxml.h\"
  \"tinyxml.cpp\"
  \"tinyxmlerror.cpp\"
  \"tinyxmlparser.cpp\"
  \"ticpp.cpp\"
  \"ticpp.h\"
  \"ticpprc.h\"
  )

SET( TICPP_INCLUDE_DIRS
  \"\${CMAKE_CURRENT_SOURCE_DIR}\"
  PARENT_SCOPE )
SET( TICPP_LIBRARIES
  \"ticpp\"
  PARENT_SCOPE )
" )
