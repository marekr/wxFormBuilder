

include_directories( BEFORE ${INC_BEFORE} )
include_directories(
    ./
    ../sdk/plugin_interface
    ${TICPP_INCLUDE_DIRS}
    ${INC_AFTER}
    )


set( WXFB_SRCS
    splashscreen.cpp
    maingui.cpp
    ./md5/md5.cc
    ./dbg_stack_trace/stack.cpp
    ./codegen/codegen.cpp
    ./codegen/codeparser.cpp
    ./codegen/codewriter.cpp
    ./codegen/cppcg.cpp
    ./codegen/luacg.cpp
    ./codegen/phpcg.cpp
    ./codegen/pythoncg.cpp
    ./codegen/xrccg.cpp
    )
SET( models_SRCS
    ./model/database.cpp
    ./model/objectbase.cpp
    ./model/types.cpp
    ./model/xrcfilter.cpp
    )

SET( utils_SRCS
     "./utils/annoyingdialog.cpp"
     "./utils/encodingutils.cpp"
     "./utils/typeconv.cpp"
     "./utils/filetocarray.cpp"
     "./utils/stringutils.cpp"
     "./utils/wxfbipc.cpp"
    )

set( rad_SRCS
    ./rad/about.cpp
    ./rad/appdata.cpp
    ./rad/bitmaps.cpp
    ./rad/cmdproc.cpp
    ./rad/customkeys.cpp
    ./rad/genericpanel.cpp
    ./rad/mainframe.cpp
    ./rad/menueditor.cpp
    ./rad/palette.cpp
    ./rad/title.cpp
    ./rad/wxfbevent.cpp
    ./rad/wxfbmanager.cpp
    ./rad/codeeditor/codeeditor.cpp
    ./rad/cpppanel/cpppanel.cpp
    ./rad/inspector/objinspect.cpp
    ./rad/inspector/wxfbadvprops.cpp
    ./rad/luapanel/luapanel.cpp
    ./rad/phppanel/phppanel.cpp
    ./rad/pythonpanel/pythonpanel.cpp
    ./rad/xrcpanel/xrcpanel.cpp
    ./rad/xrcpreview/xrcpreview.cpp
    ./rad/dataobject/dataobject.cpp
    ./rad/designer/innerframe.cpp
    ./rad/designer/menubar.cpp
    ./rad/designer/resizablepanel.cpp
    ./rad/designer/visualeditor.cpp
    ./rad/designer/visualobj.cpp
    ./rad/geninheritclass/geninhertclass.cpp
    ./rad/geninheritclass/geninhertclass_gui.cpp
    ./rad/objecttree/objecttree.cpp
    )

add_executable( wxFormBuilder WIN32
    ${WXFB_SRCS}
    ${models_SRCS}
    ${utils_SRCS}
    ${rad_SRCS}
    ${WXFB_RESOURCES}
    )

target_link_libraries( wxFormBuilder
    wxfb-plugin-interface
    ${TICPP_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    )

add_dependencies( wxFormBuilder additional common forms layout containers )

if( MINGW )
    # WXFB_RESOURCES variable is set by the macro.
    mingw_resource_compiler( wxFormBuilder )
else()
    set( WXFB_RESOURCES wxFormBuilder.rc )
endif()

file(GLOB DEPLOY_FILES_AND_DIRS "${PROJECT_SOURCE_DIR}/output/*")
foreach(ITEM ${DEPLOY_FILES_AND_DIRS})
   IF( IS_DIRECTORY "${ITEM}" )
      LIST( APPEND DIRS_TO_DEPLOY "${ITEM}" )
   ELSE()
      LIST( APPEND FILES_TO_DEPLOY "${ITEM}" )
   ENDIF()
endforeach()
INSTALL( FILES ${FILES_TO_DEPLOY} DESTINATION ${WXFB_SHARE} )
INSTALL( DIRECTORY ${DIRS_TO_DEPLOY} DESTINATION ${WXFB_SHARE} )

install(TARGETS wxFormBuilder
    RUNTIME DESTINATION ${WXFB_BIN}
    LIBRARY DESTINATION ${WXFB_LIB}
)
