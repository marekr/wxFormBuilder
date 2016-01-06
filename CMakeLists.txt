
project(wxFormBuilder)

cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)

# Path to local CMake modules.
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules )

include( CheckIncludeFile )
include( CheckLibraryExists )
include( CheckSymbolExists )
include( CheckIncludeFileCXX )
include( CheckCXXSymbolExists )
include( CheckCXXSourceCompiles )
include( CheckCXXCompilerFlag )

if( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )

    execute_process( COMMAND ${CMAKE_C_COMPILER} -dumpversion
        OUTPUT_VARIABLE GCC_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE )

    if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
        set( TO_LINKER -XLinker )
    else()
        set( TO_LINKER -Wl )
    endif()

    # Establish -Wall early, so specialized relaxations of this may come
    # subsequently on the command line, such as in pcbnew/github/CMakeLists.txt
    set( CMAKE_C_FLAGS   "-Wall ${CMAKE_C_FLAGS}" )
    set( CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS} -std=gnu++11" )

    if( GCC_VERSION VERSION_EQUAL 5.1.0 OR GCC_VERSION VERSION_EQUAL 5.2.0 )
        set( CMAKE_C_FLAGS   "-Wno-deprecated-declarations ${CMAKE_C_FLAGS}" )
        set( CMAKE_CXX_FLAGS "-Wno-deprecated-declarations ${CMAKE_CXX_FLAGS}" )
    endif()
    
    set( CMAKE_C_FLAGS_DEBUG   "-g3 -ggdb3 -DDEBUG" )
    set( CMAKE_CXX_FLAGS_DEBUG "-g3 -ggdb3 -DDEBUG" )

    set( CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG" )
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG" )

    if( APPLE )
        set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wno-overloaded-virtual" )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual" )
    endif()
endif( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )

include( BuildPackage )
BUILD_PACKAGE( "ticpp" )

ADD_DEFINITIONS( -DTIXML_USE_TICPP )

include( MinGWResourceCompiler )

#================================================
# Locations for install targets.
#================================================
if( NOT APPLE )
    # Everything without leading / is relative to CMAKE_INSTALL_PREFIX.
    set( WXFB_BIN bin
        CACHE PATH "Location of wxfb binaries." )
    set( WXFB_LIB lib/wxformbuilder
        CACHE PATH "Location of wxfb libraries." )
    set( WXFB_SHARE share/wxformbuilder
        CACHE PATH "Location of wxfb extra files." )
else()
    # everything without leading / is relative to CMAKE_INSTALL_PREFIX.
    # CMAKE_INSTALL_PREFIX is root of .dmg image
    set( WXFB_BIN ${CMAKE_INSTALL_PREFIX}
        CACHE PATH "Location of KiCad binaries." FORCE )
endif()

# See line 41 of CMakeModules/FindwxWidgets.cmake
find_package( wxWidgets 3.0.0 COMPONENTS gl aui adv html core net base xml stc richtext propgrid ribbon xrc REQUIRED )

# Include wxWidgets macros.
include( ${wxWidgets_USE_FILE} )

add_subdirectory( sdk/plugin_interface )
add_subdirectory( plugins )
add_subdirectory( src )
