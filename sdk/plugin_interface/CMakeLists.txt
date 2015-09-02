

include_directories( BEFORE ${INC_BEFORE} )# Build steps
include_directories(
    ./
    ${TICPP_INCLUDE_DIRS}
    ${INC_AFTER}
    )

file(GLOB_RECURSE PLUGIN_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(wxfb-plugin-interface STATIC ${PLUGIN_SRCS})

target_link_libraries(wxfb-plugin-interface
    ${TICPP_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    )
