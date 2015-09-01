set( target containers )

include_directories( BEFORE ${INC_BEFORE} )# Build steps
include_directories(
    ./
    ../../sdk/plugin_interface
    ${TICPP_INCLUDE_DIRS}
    ${INC_AFTER}
    )

file(GLOB_RECURSE PLUGIN_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_library(${target} MODULE ${PLUGIN_SRCS})

target_link_libraries(${target}
    wxfb-plugin-interface
    ${TICPP_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    )

# Library
install(TARGETS ${target}
    LIBRARY DESTINATION ${WXFB_LIB}
)
